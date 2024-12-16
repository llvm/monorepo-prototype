//===--- StringViewSubstrCheck.h - clang-tidy---------------------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_READABILITY_STRINGVIEWSUBSTRCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_READABILITY_STRINGVIEWSUBSTRCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::readability {

/// Finds string_view substr() calls that can be replaced with remove_prefix()
/// or remove_suffix().
///
/// For the user-facing documentation see:
/// https://clang.llvm.org/extra/clang-tidy/checks/readability/string-view-substr.html
///
/// The check matches two patterns:
///   sv = sv.substr(N) -> sv.remove_prefix(N)
///   sv = sv.substr(0, sv.length() - N) -> sv.remove_suffix(N)
///
/// These replacements make the intent clearer and are more efficient as they
/// modify the string_view in place rather than creating a new one.
class StringViewSubstrCheck : public ClangTidyCheck {
public:
  StringViewSubstrCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace clang::tidy::readability

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_READABILITY_STRINGVIEWSUBSTRCHECK_H