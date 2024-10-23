//===- Matchers.h - Various common matchers ---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides extra matchers that are very useful for mlir-query
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_IR_EXTRAMATCHERS_H
#define MLIR_IR_EXTRAMATCHERS_H

#include "MatchFinder.h"
#include "MatchersInternal.h"

namespace mlir {

namespace query {

namespace extramatcher {

namespace detail {

class DefinitionsMatcher {
public:
  DefinitionsMatcher(matcher::DynMatcher &&InnerMatcher, unsigned Hops)
      : InnerMatcher(std::move(InnerMatcher)), Hops(Hops) {}

private:
  bool matches(Operation *op, matcher::BoundOperationsGraphBuilder &Bound,
               unsigned TempHops) {

    llvm::DenseSet<mlir::Value> Ccache;
    llvm::SmallVector<std::pair<Operation *, size_t>, 4> TempStorage;
    TempStorage.push_back({op, TempHops});
    while (!TempStorage.empty()) {
      auto [CurrentOp, RemainingHops] = TempStorage.pop_back_val();

      matcher::BoundOperationNode *CurrentNode =
          Bound.addNode(CurrentOp, true, true);
      if (RemainingHops == 0) {
        continue;
      }

      for (auto Operand : CurrentOp->getOperands()) {
        if (auto DefiningOp = Operand.getDefiningOp()) {
          Bound.addEdge(CurrentOp, DefiningOp);
          if (!Ccache.contains(Operand)) {
            Ccache.insert(Operand);
            TempStorage.emplace_back(DefiningOp, RemainingHops - 1);
          }
        } else if (auto BlockArg = Operand.dyn_cast<BlockArgument>()) {
          auto *Block = BlockArg.getOwner();

          if (Block->isEntryBlock() &&
              isa<FunctionOpInterface>(Block->getParentOp())) {
            continue;
          }

          Operation *ParentOp = BlockArg.getOwner()->getParentOp();
          if (ParentOp) {
            Bound.addEdge(CurrentOp, ParentOp);
            if (!!Ccache.contains(BlockArg)) {
              Ccache.insert(BlockArg);
              TempStorage.emplace_back(ParentOp, RemainingHops - 1);
            }
          }
        }
      }
    }
    // We need at least 1 defining op
    return Ccache.size() >= 2;
  }

public:
  bool match(Operation *op, matcher::BoundOperationsGraphBuilder &Bound) {
    if (InnerMatcher.match(op) && matches(op, Bound, Hops)) {
      return true;
    }
    return false;
  }

private:
  matcher::DynMatcher InnerMatcher;
  unsigned Hops;
};
} // namespace detail

inline detail::DefinitionsMatcher
definedBy(mlir::query::matcher::DynMatcher InnerMatcher) {
  return detail::DefinitionsMatcher(std::move(InnerMatcher), 1);
}

inline detail::DefinitionsMatcher
getDefinitions(mlir::query::matcher::DynMatcher InnerMatcher, unsigned Hops) {
  assert(Hops > 0 && "hops must be >= 1");
  return detail::DefinitionsMatcher(std::move(InnerMatcher), Hops);
}

} // namespace extramatcher

} // namespace query

} // namespace mlir

#endif // MLIR_IR_EXTRAMATCHERS_H
