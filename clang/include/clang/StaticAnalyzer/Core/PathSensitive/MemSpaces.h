//===-- MemSpaces.h -----------------------------------------------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// TODO
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_STATICANALYZER_CHECKERS_MEMSPACES_H
#define LLVM_CLANG_STATICANALYZER_CHECKERS_MEMSPACES_H

#include "clang/StaticAnalyzer/Core/PathSensitive/MemRegion.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramState_Fwd.h"

namespace clang {
namespace ento {

class MemRegion;
class MemSpaceRegion;

namespace memspace {

[[nodiscard]] ProgramStateRef setMemSpaceTrait(ProgramStateRef State,
                                               const MemRegion *MR,
                                               const MemSpaceRegion *MS);

[[nodiscard]] const MemSpaceRegion *getMemSpaceTrait(ProgramStateRef State,
                                                     const MemRegion *MR);

[[nodiscard]] bool hasMemSpaceTrait(ProgramStateRef State, const MemRegion *MR);

template <typename FirstT, typename... RestT>
[[nodiscard]] bool isMemSpaceOrTrait(ProgramStateRef State,
                                     const MemRegion *MR) {
  return isa<FirstT, RestT...>(MR->getMemorySpace()) ||
         isa_and_nonnull<FirstT, RestT...>(getMemSpaceTrait(State, MR));
}

} // namespace memspace
} // namespace ento
} // namespace clang

#endif // LLVM_CLANG_STATICANALYZER_CHECKERS_MEMSPACES_H
