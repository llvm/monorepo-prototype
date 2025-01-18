//===- HLSLRootSignature.h - HLSL Root Signature helper objects -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file This file contains helper objects for working with HLSL Root
/// Signatures.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_FRONTEND_HLSL_HLSLROOTSIGNATURE_H
#define LLVM_FRONTEND_HLSL_HLSLROOTSIGNATURE_H

#include <stdint.h>

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Endian.h"

namespace llvm {
namespace hlsl {
namespace root_signature {

// This is a copy from DebugInfo/CodeView/CodeView.h
#define RS_DEFINE_ENUM_CLASS_FLAGS_OPERATORS(Class)                            \
  inline Class operator|(Class a, Class b) {                                   \
    return static_cast<Class>(llvm::to_underlying(a) |                         \
                              llvm::to_underlying(b));                         \
  }                                                                            \
  inline Class operator&(Class a, Class b) {                                   \
    return static_cast<Class>(llvm::to_underlying(a) &                         \
                              llvm::to_underlying(b));                         \
  }                                                                            \
  inline Class operator~(Class a) {                                            \
    return static_cast<Class>(~llvm::to_underlying(a));                        \
  }                                                                            \
  inline Class &operator|=(Class &a, Class b) {                                \
    a = a | b;                                                                 \
    return a;                                                                  \
  }                                                                            \
  inline Class &operator&=(Class &a, Class b) {                                \
    a = a & b;                                                                 \
    return a;                                                                  \
  }

// Definition of the various enumerations and flags
enum class DescriptorRangeFlags : unsigned {
  None = 0,
  DescriptorsVolatile = 0x1,
  DataVolatile = 0x2,
  DataStaticWhileSetAtExecute = 0x4,
  DataStatic = 0x8,
  DescriptorsStaticKeepingBufferBoundsChecks = 0x10000,
  ValidFlags = 0x1000f,
  ValidSamplerFlags = DescriptorsVolatile,
};
RS_DEFINE_ENUM_CLASS_FLAGS_OPERATORS(DescriptorRangeFlags)

enum class ShaderVisibility {
  All = 0,
  Vertex = 1,
  Hull = 2,
  Domain = 3,
  Geometry = 4,
  Pixel = 5,
  Amplification = 6,
  Mesh = 7,
};

// Definitions of the in-memory data layout structures

// Models the different registers: bReg | tReg | uReg | sReg
enum class RegisterType { BReg, TReg, UReg, SReg };
struct Register {
  RegisterType ViewType;
  uint32_t Number;
};

static const uint32_t DescriptorTableOffsetAppend = 0xffffffff;
// Models DTClause : CBV | SRV | UAV | Sampler by collecting like parameters
enum class ClauseType { CBV, SRV, UAV, Sampler };
struct DescriptorTableClause {
  ClauseType Type;
  Register Register;
  uint32_t NumDescriptors = 1;
  uint32_t Space = 0;
  uint32_t Offset = DescriptorTableOffsetAppend;
  DescriptorRangeFlags Flags;

  DescriptorTableClause(ClauseType Type) : Type(Type) {
    switch (Type) {
    case ClauseType::CBV:
      Flags = DescriptorRangeFlags::DataStaticWhileSetAtExecute;
      break;
    case ClauseType::SRV:
      Flags = DescriptorRangeFlags::DataStaticWhileSetAtExecute;
      break;
    case ClauseType::UAV:
      Flags = DescriptorRangeFlags::DataVolatile;
      break;
    case ClauseType::Sampler:
      Flags = DescriptorRangeFlags::None;
      break;
    }
  }
};

// Models the end of a descriptor table and stores its visibility
struct DescriptorTable {
  ShaderVisibility Visibility = ShaderVisibility::All;
  uint32_t NumClauses = 0; // The number of clauses in the table
};

// Models RootElement : DescriptorTable | DescriptorTableClause
struct RootElement {
  enum class ElementType {
    DescriptorTable,
    DescriptorTableClause,
  };

  ElementType Tag;
  union {
    DescriptorTable Table;
    DescriptorTableClause Clause;
  };

  // Constructors
  RootElement(DescriptorTable Table)
      : Tag(ElementType::DescriptorTable), Table(Table) {}
  RootElement(DescriptorTableClause Clause)
      : Tag(ElementType::DescriptorTableClause), Clause(Clause) {}
};

} // namespace root_signature
} // namespace hlsl
} // namespace llvm

#endif // LLVM_FRONTEND_HLSL_HLSLROOTSIGNATURE_H
