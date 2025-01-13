//===-- MemSpaces.cpp ---------------------------------------------*- C++ -*--//
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
  // for now, this should only be called to update the trait for mem regions
  // that have an unknown mem spaces since we assume everywhere else that the
  // memspace trait is set only for unknown mem spaces (setting this info
  // otherwise would go unused).
  assert(isa<UnknownSpaceRegion>(MR->getMemorySpace()));

  // Shouldn't use the memspace trait to associate UnknownSpaceRegion with an
  // already UnknownSpaceRegion
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

const MemSpaceRegion *getMemSpaceTrait(ProgramStateRef State,
                                       const MemRegion *MR) {
  if (!isa<UnknownSpaceRegion>(MR->getMemorySpace()))
    return nullptr;

  const MemSpaceRegion *const *Result = State->get<MemSpacesMap>(MR);
  if (!Result)
    return nullptr;
  return *Result;
}

} // namespace memspace
} // namespace ento
} // namespace clang
