//===--- StringViewSubstrCheck.cpp - clang-tidy------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "StringViewSubstrCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang::tidy::readability {

void StringViewSubstrCheck::registerMatchers(MatchFinder *Finder) {
  const auto HasStringViewType = hasType(hasUnqualifiedDesugaredType(recordType(
      hasDeclaration(recordDecl(hasName("::std::basic_string_view"))))));

  // Match assignment to string_view's substr
  Finder->addMatcher(
      cxxOperatorCallExpr(
          hasOverloadedOperatorName("="),
          hasArgument(0, expr(HasStringViewType).bind("target")),
          hasArgument(
              1, cxxMemberCallExpr(callee(memberExpr(hasDeclaration(
                                       cxxMethodDecl(hasName("substr"))))),
                                   on(expr(HasStringViewType).bind("source")))
                     .bind("substr_call")))
          .bind("assignment"),
      this);
}

void StringViewSubstrCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Assignment =
      Result.Nodes.getNodeAs<CXXOperatorCallExpr>("assignment");
  const auto *Target = Result.Nodes.getNodeAs<Expr>("target");
  const auto *Source = Result.Nodes.getNodeAs<Expr>("source");
  const auto *SubstrCall =
      Result.Nodes.getNodeAs<CXXMemberCallExpr>("substr_call");

  if (!Assignment || !Target || !Source || !SubstrCall) {
    return;
  }

  // Get the DeclRefExpr for the target and source to compare variables
  const auto *TargetDRE = dyn_cast<DeclRefExpr>(Target->IgnoreParenImpCasts());
  const auto *SourceDRE = dyn_cast<DeclRefExpr>(Source->IgnoreParenImpCasts());

  // Only handle self-assignment cases
  if (!TargetDRE || !SourceDRE ||
      TargetDRE->getDecl() != SourceDRE->getDecl()) {
    return;
  }

  const Expr *StartArg = SubstrCall->getArg(0);
  const Expr *LengthArg =
      SubstrCall->getNumArgs() > 1 ? SubstrCall->getArg(1) : nullptr;

  // Get source text of first argument
  std::string StartText =
      Lexer::getSourceText(
          CharSourceRange::getTokenRange(StartArg->getSourceRange()),
          *Result.SourceManager, Result.Context->getLangOpts())
          .str();

  // Case 1: Check for remove_prefix pattern - only when the second arg is
  // missing (uses npos)
  if (!LengthArg || isa<CXXDefaultArgExpr>(LengthArg)) {
    std::string Replacement = TargetDRE->getNameInfo().getAsString() +
                              ".remove_prefix(" + StartText + ")";
    diag(Assignment->getBeginLoc(), "prefer 'remove_prefix' over 'substr' for "
                                    "removing characters from the start")
        << FixItHint::CreateReplacement(Assignment->getSourceRange(),
                                        Replacement);
    return;
  }

  // Case 2: Check for remove_suffix pattern
  if (StartText == "0") {
    if (const auto *BinOp = dyn_cast<BinaryOperator>(LengthArg)) {
      if (BinOp->getOpcode() == BO_Sub) {
        const Expr *LHS = BinOp->getLHS();
        const Expr *RHS = BinOp->getRHS();

        // Check if LHS is a length() call on the same string_view
        if (const auto *LengthCall = dyn_cast<CXXMemberCallExpr>(LHS)) {
          if (const auto *LengthMethod =
                  dyn_cast<CXXMethodDecl>(LengthCall->getDirectCallee())) {
            if (LengthMethod->getName() == "length") {
              // Verify the length() call is on the same string_view
              const Expr *LengthObject =
                  LengthCall->getImplicitObjectArgument();
              const auto *LengthDRE =
                  dyn_cast<DeclRefExpr>(LengthObject->IgnoreParenImpCasts());

              if (!LengthDRE || LengthDRE->getDecl() != TargetDRE->getDecl()) {
                return;
              }

              // Must be a simple non-zero integer literal
              const auto *IL =
                  dyn_cast<IntegerLiteral>(RHS->IgnoreParenImpCasts());
              if (!IL || IL->getValue() == 0) {
                return;
              }

              std::string RHSText =
                  Lexer::getSourceText(
                      CharSourceRange::getTokenRange(RHS->getSourceRange()),
                      *Result.SourceManager, Result.Context->getLangOpts())
                      .str();

              std::string Replacement = TargetDRE->getNameInfo().getAsString() +
                                        ".remove_suffix(" + RHSText + ")";
              diag(Assignment->getBeginLoc(),
                   "prefer 'remove_suffix' over 'substr' for removing "
                   "characters from the end")
                  << FixItHint::CreateReplacement(Assignment->getSourceRange(),
                                                  Replacement);
              return;
            }
          }
        }
      }
    }
  }
}

} // namespace clang::tidy::readability
