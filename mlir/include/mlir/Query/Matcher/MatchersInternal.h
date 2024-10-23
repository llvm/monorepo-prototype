//===- MatchersInternal.h - Structural query framework ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 wIth LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WItH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_TOOLS_MLIRQUERY_MATCHER_MATCHERSINTERNAL_H
#define MLIR_TOOLS_MLIRQUERY_MATCHER_MATCHERSINTERNAL_H

#include "mlir/IR/Matchers.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/MapVector.h"
#include <memory>
#include <stack>
#include <unordered_set>
#include <vector>

namespace mlir::query::matcher {

struct BoundOperationNode {
  Operation *op;
  std::vector<BoundOperationNode *> Parents;
  std::vector<BoundOperationNode *> Children;

  bool IsRootNode;
  bool DetailedPrinting;

  BoundOperationNode(Operation *op, bool IsRootNode = false,
                     bool DetailedPrinting = false)
      : op(op), IsRootNode(IsRootNode), DetailedPrinting(DetailedPrinting) {}
};

class BoundOperationsGraphBuilder {
public:
  BoundOperationNode *addNode(Operation *op, bool IsRootNode = false,
                              bool DetailedPrinting = false) {
    auto It = Nodes.find(op);
    if (It != Nodes.end()) {
      return It->second.get();
    }
    auto Node =
        std::make_unique<BoundOperationNode>(op, IsRootNode, DetailedPrinting);
    BoundOperationNode *NodePtr = Node.get();
    Nodes[op] = std::move(Node);
    return NodePtr;
  }

  void addEdge(Operation *parentOp, Operation *childOp) {
    BoundOperationNode *ParentNode = addNode(parentOp, false, false);
    BoundOperationNode *ChildNode = addNode(childOp, false, false);

    ParentNode->Children.push_back(ChildNode);
    ChildNode->Parents.push_back(ParentNode);
  }

  BoundOperationNode *getNode(Operation *op) const {
    auto It = Nodes.find(op);
    return It != Nodes.end() ? It->second.get() : nullptr;
  }

  const llvm::MapVector<Operation *, std::unique_ptr<BoundOperationNode>> &
  getNodes() const {
    return Nodes;
  }

private:
  llvm::MapVector<Operation *, std::unique_ptr<BoundOperationNode>> Nodes;
};

// Type traIt to detect if a matcher has a match(Operation*) method
template <typename T, typename = void>
struct has_simple_match : std::false_type {};

template <typename T>
struct has_simple_match<T, std::void_t<decltype(std::declval<T>().match(
                               std::declval<Operation *>()))>>
    : std::true_type {};

// Type traIt to detect if a matcher has a match(Operation*,
// BoundOperationsGraphBuilder&) method
template <typename T, typename = void>
struct has_bound_match : std::false_type {};

template <typename T>
struct has_bound_match<T, std::void_t<decltype(std::declval<T>().match(
                              std::declval<Operation *>(),
                              std::declval<BoundOperationsGraphBuilder &>()))>>
    : std::true_type {};

// Generic interface for matchers on an MLIR operation.
class MatcherInterface
    : public llvm::ThreadSafeRefCountedBase<MatcherInterface> {
public:
  virtual ~MatcherInterface() = default;
  virtual bool match(Operation *op) = 0;
  virtual bool match(Operation *op, BoundOperationsGraphBuilder &bound) = 0;
};

// MatcherFnImpl takes a matcher function object and implements
// MatcherInterface.
template <typename MatcherFn>
class MatcherFnImpl : public MatcherInterface {
public:
  MatcherFnImpl(MatcherFn &matcherFn) : matcherFn(matcherFn) {}

  bool match(Operation *op) override {
    if constexpr (has_simple_match<MatcherFn>::value)
      return matcherFn.match(op);
    return false;
  }

  bool match(Operation *op, BoundOperationsGraphBuilder &bound) override {
    if constexpr (has_bound_match<MatcherFn>::value)
      return matcherFn.match(op, bound);
    return false;
  }

private:
  MatcherFn matcherFn;
};

// Matcher wraps a MatcherInterface implementation and provides match()
// methods that redirect calls to the underlying implementation.
class DynMatcher {
public:
  // Takes ownership of the provided implementation pointer.
  DynMatcher(MatcherInterface *implementation, StringRef matcherName)
      : implementation(implementation), matcherName(matcherName.str()) {}

  template <typename MatcherFn>
  static std::unique_ptr<DynMatcher>
  constructDynMatcherFromMatcherFn(MatcherFn &matcherFn,
                                   StringRef matcherName) {
    auto impl = std::make_unique<MatcherFnImpl<MatcherFn>>(matcherFn);
    return std::make_unique<DynMatcher>(impl.release(), matcherName);
  }

  bool match(Operation *op) const { return implementation->match(op); }
  bool match(Operation *op, BoundOperationsGraphBuilder &bound) const {
    return implementation->match(op, bound);
  }

  void setFunctionName(StringRef name) { functionName = name.str(); }
  void setMatcherName(StringRef name) { matcherName = name.str(); }
  bool hasFunctionName() const { return !functionName.empty(); }
  StringRef getFunctionName() const { return functionName; }
  StringRef getMatcherName() const { return matcherName; }

private:
  llvm::IntrusiveRefCntPtr<MatcherInterface> implementation;
  std::string matcherName;
  std::string functionName;
};

} // namespace mlir::query::matcher

#endif // MLIR_TOOLS_MLIRQUERY_MATCHER_MATCHERSINTERNAL_H