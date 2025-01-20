//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the base layer of the matcher framework.
//
// Matchers are methods that return a Matcher which provides a method
// match(Operation *op)
// match(Operation *op, SetVector<Operation *> &matchedOps, QueryOptions
// &options)
//
// The matcher functions are defined in include/mlir/IR/Matchers.h.
// This file contains the wrapper classes needed to construct matchers for
// mlir-query.
// SPDX-License-Identifier: Apache-2.0 WItH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef MLIR_TOOLS_MLIRQUERY_MATCHER_MATCHERSINTERNAL_H
#define MLIR_TOOLS_MLIRQUERY_MATCHER_MATCHERSINTERNAL_H

#include "mlir/IR/Matchers.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

namespace mlir {
namespace query {
struct QueryOptions;
}
} // namespace mlir

namespace mlir::query::matcher {
template <typename T, typename = void>
struct has_simple_match : std::false_type {};

template <typename T>
struct has_simple_match<T, std::void_t<decltype(std::declval<T>().match(
                               std::declval<Operation *>()))>>
    : std::true_type {};

template <typename T, typename = void>
struct has_bound_match : std::false_type {};

template <typename T>
struct has_bound_match<T, std::void_t<decltype(std::declval<T>().match(
                              std::declval<Operation *>(),
                              std::declval<SetVector<Operation *> &>(),
                              std::declval<QueryOptions &>()))>>
    : std::true_type {};

// Generic interface for matchers on an MLIR operation.
class MatcherInterface
    : public llvm::ThreadSafeRefCountedBase<MatcherInterface> {
public:
  virtual ~MatcherInterface() = default;
  virtual bool match(Operation *op) = 0;
  virtual bool match(Operation *op, SetVector<Operation *> &matchedOps,
                     QueryOptions &options) = 0;
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

  bool match(Operation *op, SetVector<Operation *> &matchedOps,
             QueryOptions &options) override {
    if constexpr (has_bound_match<MatcherFn>::value)
      return matcherFn.match(op, matchedOps, options);
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
  bool match(Operation *op, SetVector<Operation *> &matchedOps,
             QueryOptions &options) const {
    return implementation->match(op, matchedOps, options);
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
