//===--- UseSpanFirstLastCheck.cpp - clang-tidy -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UseSpanFirstLastCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang::tidy::readability {

void UseSpanFirstLastCheck::registerMatchers(MatchFinder *Finder) {
  const auto HasSpanType =
      hasType(hasUnqualifiedDesugaredType(recordType(hasDeclaration(
          classTemplateSpecializationDecl(hasName("::std::span"))))));

  // Match span.subspan(0, n) -> first(n)
  Finder->addMatcher(
      cxxMemberCallExpr(
          callee(memberExpr(hasDeclaration(cxxMethodDecl(hasName("subspan"))))),
          on(expr(HasSpanType).bind("span_object")),
          hasArgument(0, integerLiteral(equals(0))),
          hasArgument(1, expr().bind("count")), argumentCountIs(2))
          .bind("first_subspan"),
      this);

  // Match span.subspan(size() - n) or span.subspan(std::ranges::size(span) - n)
  // -> last(n)
  const auto SizeCall = anyOf(
      cxxMemberCallExpr(
          callee(memberExpr(hasDeclaration(cxxMethodDecl(hasName("size")))))),
      callExpr(callee(
          functionDecl(hasAnyName("::std::size", "::std::ranges::size")))));

  Finder->addMatcher(
      cxxMemberCallExpr(
          callee(memberExpr(hasDeclaration(cxxMethodDecl(hasName("subspan"))))),
          on(expr(HasSpanType).bind("span_object")),
          hasArgument(0, binaryOperator(hasOperatorName("-"), hasLHS(SizeCall),
                                        hasRHS(expr().bind("count")))),
          argumentCountIs(1))
          .bind("last_subspan"),
      this);
}

void UseSpanFirstLastCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *SpanObj = Result.Nodes.getNodeAs<Expr>("span_object");
  if (!SpanObj)
    return;

  StringRef SpanText = Lexer::getSourceText(
      CharSourceRange::getTokenRange(SpanObj->getSourceRange()),
      *Result.SourceManager, Result.Context->getLangOpts());

  if (const auto *FirstCall =
          Result.Nodes.getNodeAs<CXXMemberCallExpr>("first_subspan")) {
    const auto *Count = Result.Nodes.getNodeAs<Expr>("count");
    assert(Count && "Count expression must exist due to AST matcher");

    StringRef CountText = Lexer::getSourceText(
        CharSourceRange::getTokenRange(Count->getSourceRange()),
        *Result.SourceManager, Result.Context->getLangOpts());

    std::string Replacement =
        (Twine(SpanText) + ".first(" + CountText + ")").str();

    diag(FirstCall->getBeginLoc(), "prefer 'span::first()' over 'subspan()'")
        << FixItHint::CreateReplacement(FirstCall->getSourceRange(),
                                        Replacement);
  }

  if (const auto *LastCall =
          Result.Nodes.getNodeAs<CXXMemberCallExpr>("last_subspan")) {
    const auto *Count = Result.Nodes.getNodeAs<Expr>("count");
    assert(Count && "Count expression must exist due to AST matcher");

    StringRef CountText = Lexer::getSourceText(
        CharSourceRange::getTokenRange(Count->getSourceRange()),
        *Result.SourceManager, Result.Context->getLangOpts());

    std::string Replacement = SpanText.str() + ".last(" + CountText.str() + ")";

    diag(LastCall->getBeginLoc(), "prefer 'span::last()' over 'subspan()'")
        << FixItHint::CreateReplacement(LastCall->getSourceRange(),
                                        Replacement);
  }
}
} // namespace clang::tidy::readability
