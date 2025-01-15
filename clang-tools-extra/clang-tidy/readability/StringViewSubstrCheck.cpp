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
  // Match string_view type
  const auto StringViewDecl = recordDecl(hasName("::std::basic_string_view"));
  const auto IsStringView = qualType(
      hasUnqualifiedDesugaredType(recordType(hasDeclaration(StringViewDecl))));

  // Match substr() call on string_views
  const auto SubstrCall = cxxMemberCallExpr(
      callee(memberExpr(hasDeclaration(cxxMethodDecl(hasName("substr"))))),
      on(expr(hasType(IsStringView)).bind("source")));

  // Match assignment to string_view's substr, but exclude assignments in
  // expressions
  Finder->addMatcher(
      stmt(cxxOperatorCallExpr(
               unless(isInTemplateInstantiation()),
               hasOverloadedOperatorName("="),
               hasArgument(0, expr(hasType(IsStringView)).bind("target")),
               hasArgument(1, SubstrCall.bind("substr_call")))
               .bind("assignment"),
           // Exclude assignments used in larger expressions
           unless(hasAncestor(varDecl())), unless(hasAncestor(callExpr()))),
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

  // Early return if StartArg is not 0
  if (!IsZero) {
    return;
  }

  bool IsFullCopy = false;

  // Check for length() or length() - expr pattern
  if (const auto *BinOp = dyn_cast<BinaryOperator>(LengthArg)) {
    if (BinOp->getOpcode() == BO_Sub) {
      const Expr *LHS = BinOp->getLHS();
      const Expr *RHS = BinOp->getRHS();

      // Check for length() call
      if (const auto *LengthCall = dyn_cast<CXXMemberCallExpr>(LHS)) {
        if (const auto *LengthMethod =
                dyn_cast<CXXMethodDecl>(LengthCall->getDirectCallee())) {
          if (LengthMethod->getName() == "length" ||
              LengthMethod->getName() == "size") {
            const Expr *LengthObject = LengthCall->getImplicitObjectArgument();
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
              IsFullCopy = true;
            } else {
              // remove_suffix case (works for both self and different vars)
              std::string RHSText =
                  Lexer::getSourceText(
                      CharSourceRange::getTokenRange(RHS->getSourceRange()),
                      *Result.SourceManager, Result.Context->getLangOpts())
                      .str();

              std::string TargetName = TargetDRE->getNameInfo().getAsString();
              std::string SourceName = SourceDRE->getNameInfo().getAsString();

              std::string Replacement =
                  IsSameVar ? (TargetName + ".remove_suffix(" + RHSText + ")")
                            : (TargetName + " = " + SourceName + ";\n" +
                               TargetName + ".remove_suffix(" + RHSText + ")");

              diag(Assignment->getBeginLoc(),
                   IsSameVar
                       ? "prefer 'remove_suffix' over 'substr' for removing "
                         "characters from the end"
                       : "prefer assignment and remove_suffix over substr")
                  << FixItHint::CreateReplacement(Assignment->getSourceRange(),
                                                  Replacement);
              return;
            }
          }
        }
      }
    }
  } else if (const auto *LengthCall = dyn_cast<CXXMemberCallExpr>(LengthArg)) {
    // Handle direct length() or size() call
    if (const auto *LengthMethod =
            dyn_cast<CXXMethodDecl>(LengthCall->getDirectCallee())) {
      if (LengthMethod->getName() == "length" ||
          LengthMethod->getName() == "size") {
        const Expr *LengthObject = LengthCall->getImplicitObjectArgument();
        const auto *LengthDRE =
            dyn_cast<DeclRefExpr>(LengthObject->IgnoreParenImpCasts());

        if (LengthDRE && LengthDRE->getDecl() == SourceDRE->getDecl()) {
          IsFullCopy = true;
        }
      }
    }
  }
  if (IsFullCopy) {
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
                   EndLoc.getLocWithOffset(1))); // +1 to include the semicolon.
      }
    } else {
      // Simplify copy between different variables
      std::string Replacement = TargetDRE->getNameInfo().getAsString() + " = " +
                                SourceDRE->getNameInfo().getAsString();
      diag(Assignment->getBeginLoc(), "prefer direct copy over substr")
          << FixItHint::CreateReplacement(Assignment->getSourceRange(),
                                          Replacement);
    }
    return;
  }
}

} // namespace clang::tidy::readability
