//===-- MemSpaces.cpp ---------------------------------------------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines getters and setters for the memory space trait which
// associates memory regions with memory spaces in the program state. It also
// defines the MemSpacesMap which maps memory regions to memory spaces.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Core/PathSensitive/MemSpaces.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"
#include <cassert>

REGISTER_MAP_WITH_PROGRAMSTATE(MemSpacesMap, const clang::ento::MemRegion *,
                               const clang::ento::MemSpaceRegion *)

namespace clang {
namespace ento {
namespace memspace {

ProgramStateRef setMemSpaceTrait(ProgramStateRef State, const MemRegion *MR,
                                 const MemSpaceRegion *MS) {
  // For now, this should only be called to update the trait for memory regions
  // that have an unknown memory spaces since we assume everywhere else that the
  // memory space trait is set only for unknown memory spaces (setting this info
  // otherwise would go unused).
  assert(isa<UnknownSpaceRegion>(MR->getMemorySpace()));

  // Shouldn't use the memory space trait to associate UnknownSpaceRegion with
  // an already UnknownSpaceRegion
  assert(!isa<UnknownSpaceRegion>(MS));

  ProgramStateRef NewState = State->set<MemSpacesMap>(MR, MS);
  return NewState;
}

bool hasMemSpaceTrait(ProgramStateRef State, const MemRegion *MR) {
  if (!isa<UnknownSpaceRegion>(MR->getMemorySpace()))
    return false;

  const MemSpaceRegion *const *Result = State->get<MemSpacesMap>(MR);
  return Result;
}

const MemSpaceRegion *getMemSpace(ProgramStateRef State,
                                       const MemRegion *MR) {
  const MemSpaceRegion *const *Result = State->get<MemSpacesMap>(MR);
  if (Result)
    return *Result;

  return MR->getMemorySpace();
}

} // namespace memspace
} // namespace ento
} // namespace clang
