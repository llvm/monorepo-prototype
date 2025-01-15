//===-- MemSpaces.h -----------------------------------------------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares getters and setters for the memory space trait which
// associates memory regions with memory spaces in the program state. We are
// still transitioning to having all memory space information stored in the
// trait, so currently memory regions still have memory spaces as a class field.
// Because of this, these getters still consider memory spaces set as class
// fields in memory region types.
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

[[nodiscard]] const MemSpaceRegion *getMemSpace(ProgramStateRef State,
                                                const MemRegion *MR);

[[nodiscard]] bool hasMemSpaceTrait(ProgramStateRef State, const MemRegion *MR);

template <typename FirstT, typename... RestT>
[[nodiscard]] bool isMemSpaceOrTrait(ProgramStateRef State,
                                     const MemRegion *MR) {
  return isa<FirstT, RestT...>(MR->getMemorySpace()) ||
         isa_and_nonnull<FirstT, RestT...>(getMemSpace(State, MR));
}

} // namespace memspace
} // namespace ento
} // namespace clang

#endif // LLVM_CLANG_STATICANALYZER_CHECKERS_MEMSPACES_H
