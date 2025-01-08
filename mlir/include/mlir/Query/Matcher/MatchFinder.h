//===- MatchFinder.h - ------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the MatchFinder class, which is used to find operations
// that match a given matcher.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_TOOLS_MLIRQUERY_MATCHER_MATCHERFINDER_H
#define MLIR_TOOLS_MLIRQUERY_MATCHER_MATCHERFINDER_H

#include "MatchersInternal.h"
#include "mlir/IR/Operation.h"

namespace mlir::query::matcher {

// MatchFinder is used to find all operations that match a given matcher.
class MatchFinder {
public:
  // Returns all operations that match the given matcher.
  static SetVector<Operation *>
  getMatches(Operation *root, QueryOptions &options, DynMatcher matcher) {
    SetVector<Operation *> backwardSlice;
    root->walk([&](Operation *subOp) {
      if (matcher.match(subOp)) {
        backwardSlice.insert(subOp);
      } else {
        matcher.match(subOp, backwardSlice, options);
        ////
      }
    });
    return backwardSlice;
  }
};

} // namespace mlir::query::matcher

#endif // MLIR_TOOLS_MLIRQUERY_MATCHER_MATCHERFINDER_H
