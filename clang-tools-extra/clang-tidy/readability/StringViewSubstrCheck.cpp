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

  // Get the DeclRefExpr for the target and source
  const auto *TargetDRE = dyn_cast<DeclRefExpr>(Target->IgnoreParenImpCasts());
  const auto *SourceDRE = dyn_cast<DeclRefExpr>(Source->IgnoreParenImpCasts());

  if (!TargetDRE || !SourceDRE) {
    return;
  }

  const Expr *StartArg = SubstrCall->getArg(0);
  const Expr *LengthArg =
      SubstrCall->getNumArgs() > 1 ? SubstrCall->getArg(1) : nullptr;

  std::string StartText =
      Lexer::getSourceText(
          CharSourceRange::getTokenRange(StartArg->getSourceRange()),
          *Result.SourceManager, Result.Context->getLangOpts())
          .str();

  const bool IsSameVar = (TargetDRE->getDecl() == SourceDRE->getDecl());

  // Case 1: Check for remove_prefix pattern
  if (!LengthArg || isa<CXXDefaultArgExpr>(LengthArg)) {
    if (IsSameVar) {
      std::string Replacement = TargetDRE->getNameInfo().getAsString() +
                                ".remove_prefix(" + StartText + ")";
      diag(Assignment->getBeginLoc(), "prefer 'remove_prefix' over 'substr' "
                                      "for removing characters from the start")
          << FixItHint::CreateReplacement(Assignment->getSourceRange(),
                                          Replacement);
    }
    return;
  }

  // Check if StartArg resolves to 0
  bool IsZero = false;

  // Handle cases where StartArg is an integer literal
  if (const auto *IL =
          dyn_cast<IntegerLiteral>(StartArg->IgnoreParenImpCasts())) {
    IsZero = IL->getValue() == 0;
  }

  // Optional: Handle cases where StartArg evaluates to zero
  if (!IsZero) {
    // Add logic for other constant evaluation (e.g., constexpr evaluation)
    const auto &EvalResult = StartArg->EvaluateKnownConstInt(*Result.Context);
    IsZero = !EvalResult.isNegative() && EvalResult == 0;
  }

  // If StartArg resolves to 0, handle the case
  if (IsZero) {
    bool isFullCopy = false;

    // Check for length() or length() - expr pattern
    if (const auto *BinOp = dyn_cast<BinaryOperator>(LengthArg)) {
      if (BinOp->getOpcode() == BO_Sub) {
        const Expr *LHS = BinOp->getLHS();
        const Expr *RHS = BinOp->getRHS();

        // Check for length() call
        if (const auto *LengthCall = dyn_cast<CXXMemberCallExpr>(LHS)) {
          if (const auto *LengthMethod =
                  dyn_cast<CXXMethodDecl>(LengthCall->getDirectCallee())) {
            if (LengthMethod->getName() == "length") {
              const Expr *LengthObject =
                  LengthCall->getImplicitObjectArgument();
              const auto *LengthDRE =
                  dyn_cast<DeclRefExpr>(LengthObject->IgnoreParenImpCasts());

              if (!LengthDRE || LengthDRE->getDecl() != SourceDRE->getDecl()) {
                return;
              }

              // Check if RHS is 0 or evaluates to 0
              bool IsZero = false;
              if (const auto *IL =
                      dyn_cast<IntegerLiteral>(RHS->IgnoreParenImpCasts())) {
                IsZero = IL->getValue() == 0;
              }

              if (IsZero) {
                isFullCopy = true;
              } else if (IsSameVar) {
                // remove_suffix case (only for self-assignment)
                std::string RHSText =
                    Lexer::getSourceText(
                        CharSourceRange::getTokenRange(RHS->getSourceRange()),
                        *Result.SourceManager, Result.Context->getLangOpts())
                        .str();

                std::string Replacement =
                    TargetDRE->getNameInfo().getAsString() + ".remove_suffix(" +
                    RHSText + ")";
                diag(Assignment->getBeginLoc(),
                     "prefer 'remove_suffix' over 'substr' for removing "
                     "characters from the end")
                    << FixItHint::CreateReplacement(
                           Assignment->getSourceRange(), Replacement);
                return;
              }
            }
          }
        }
      }
    } else if (const auto *LengthCall =
                   dyn_cast<CXXMemberCallExpr>(LengthArg)) {
      // Handle direct length() call
      if (const auto *LengthMethod =
              dyn_cast<CXXMethodDecl>(LengthCall->getDirectCallee())) {
        if (LengthMethod->getName() == "length") {
          const Expr *LengthObject = LengthCall->getImplicitObjectArgument();
          const auto *LengthDRE =
              dyn_cast<DeclRefExpr>(LengthObject->IgnoreParenImpCasts());

          if (LengthDRE && LengthDRE->getDecl() == SourceDRE->getDecl()) {
            isFullCopy = true;
          }
        }
      }
    }
    if (isFullCopy) {
      if (IsSameVar) {
        // Remove redundant self-copy, including the semicolon
        SourceLocation EndLoc = Assignment->getEndLoc();
        while (EndLoc.isValid()) {
          const char *endPtr = Result.SourceManager->getCharacterData(EndLoc);
          if (*endPtr == ';')
            break;
          EndLoc = Lexer::getLocForEndOfToken(EndLoc, 0, *Result.SourceManager,
                                              Result.Context->getLangOpts());
        }
        if (EndLoc.isValid()) {
          diag(Assignment->getBeginLoc(), "redundant self-copy")
              << FixItHint::CreateRemoval(CharSourceRange::getCharRange(
                     Assignment->getBeginLoc(),
                     EndLoc.getLocWithOffset(
                         1))); // +1 to include the semicolon.
        }
      } else {
        // Simplify copy between different variables
        std::string Replacement = TargetDRE->getNameInfo().getAsString() +
                                  " = " +
                                  SourceDRE->getNameInfo().getAsString();
        diag(Assignment->getBeginLoc(), "prefer direct copy over substr")
            << FixItHint::CreateReplacement(Assignment->getSourceRange(),
                                            Replacement);
      }

      return;
    }
  }
}

} // namespace clang::tidy::readability
