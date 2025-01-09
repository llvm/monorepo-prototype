//===- BuildBuiltins.h - Utility builder for builtins ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements some functions for lowering compiler builtins,
// specifically for atomics. Currently, LLVM-IR has no representation of atomics
// that can be used independent of its arguments:
//
// * The instructions load atomic, store atomic, atomicrmw, and cmpxchg can only
//   be used with constant memory model, sync scope, data sizes (that must be
//   power-of-2), volatile and weak property, and should not be used with data
//   types that are untypically large which may slow down the compiler.
//
// * libcall (in GCC's case: libatomic; LLVM: Compiler-RT) functions work with
//   any data size, but are slower. Specialized functions for a selected number
//   of data sizes exist as well. They do not support sync scops, the volatile
//   or weakness property. These functions may be implemented using a lock and
//   availability depends on the target triple (e.g. GPU devices cannot
//   implement a global lock by design).
//
// Whe want to mimic Clang's behaviour:
//
// * Prefer atomic instructions over libcall functions whenever possible. When a
//   target backend does not support atomic instructions natively,
//   AtomicExpandPass, LowerAtomicPass, or some backend-specific pass lower will
//   convert such instructions to a libcall function call. The reverse is not
//   the case, i.e. once a libcall function is emitted, there is no pass that
//   optimizes it into an instruction.
//
// * When passed a non-constant enum argument which the instruction requires to
//   be constant, then emit a switch case for each enum case.
//
// Clang currently doesn't actually check whether the target actually supports
// atomic libcall functions so it will always fall back to a libcall function
// even if the target does not support it. That is, emitting an atomic builtin
// may fail and a frontend needs to handle this case.
//
// Clang also assumes that the maximum supported data size of atomic instruction
// is 16, despite this is target-dependent and should be queried using
// TargetLowing::getMaxAtomicSizeInBitsSupported(). However, TargetMachine
// (which is a factory for TargetLowing) is not available during Clang's CodeGen
// phase, it is only created for the LLVM pass pipeline.
//
// The functions in this file are intended to handle the complexity of builtins
// so frontends do not need to care about the details. In the future LLVM may
// introduce more generic atomic constructs that is lowered by an LLVM pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_BUILDBUILTINS_H
#define LLVM_TRANSFORMS_UTILS_BUILDBUILTINS_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/AtomicOrdering.h"
#include "llvm/Support/Error.h"
#include <cstdint>
#include <variant>

namespace llvm {
class Value;
class TargetLibraryInfo;
class DataLayout;
class IRBuilderBase;
class Type;
class TargetLowering;

namespace SyncScope {
typedef uint8_t ID;
}

/// Emit the __atomic_load builtin. This may either be lowered to the load LLVM
/// instruction, or to one of the following libcall functions: __atomic_load_1,
/// __atomic_load_2, __atomic_load_4, __atomic_load_8, __atomic_load_16,
/// __atomic_load.
Error emitAtomicLoadBuiltin(
    Value *AtomicPtr, Value *RetPtr, bool IsVolatile,
    std::variant<Value *, AtomicOrdering, AtomicOrderingCABI> Memorder,
    std::variant<Value *, SyncScope::ID, StringRef> Scope, Type *DataTy,
    std::optional<uint64_t> DataSize, std::optional<uint64_t> AvailableSize,
    MaybeAlign Align, IRBuilderBase &Builder, const DataLayout &DL,
    const TargetLibraryInfo *TLI, const TargetLowering *TL,
    ArrayRef<std::pair<uint32_t, StringRef>> SyncScopes,
    StringRef FallbackScope, llvm::Twine Name = Twine(),
    bool AllowInstruction = true, bool AllowSwitch = true,
    bool AllowSizedLibcall = true, bool AllowLibcall = true);

/// Emit the __atomic_store builtin. It may either be lowered to the store LLVM
/// instruction, or to one of the following libcall functions: __atomic_store_1,
/// __atomic_store_2, __atomic_store_4, __atomic_store_8, __atomic_store_16,
/// __atomic_static.
Error emitAtomicStoreBuiltin(
    Value *AtomicPtr, Value *ValPtr, bool IsVolatile,
    std::variant<Value *, AtomicOrdering, AtomicOrderingCABI> Memorder,
    std::variant<Value *, SyncScope::ID, StringRef> Scope, Type *DataTy,
    std::optional<uint64_t> DataSize, std::optional<uint64_t> AvailableSize,
    MaybeAlign Align, IRBuilderBase &Builder, const DataLayout &DL,
    const TargetLibraryInfo *TLI, const TargetLowering *TL,
    ArrayRef<std::pair<uint32_t, StringRef>> SyncScopes,
    StringRef FallbackScope, llvm::Twine Name = Twine(),
    bool AllowInstruction = true, bool AllowSwitch = true,
    bool AllowSizedLibcall = true, bool AllowLibcall = true);

/// Emit the __atomic_compare_exchange builtin. This may either be
/// lowered to the cmpxchg LLVM instruction, or to one of the following libcall
/// functions: __atomic_compare_exchange_1, __atomic_compare_exchange_2,
/// __atomic_compare_exchange_4, __atomic_compare_exchange_8,
/// __atomic_compare_exchange_16, __atomic_compare_exchange.
///
/// Also see:
/// https://llvm.org/docs/Atomics.html
/// https://llvm.org/docs/LangRef.html#cmpxchg-instruction
/// https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
/// https://gcc.gnu.org/wiki/Atomic/GCCMM/LIbrary#GCC_intrinsics
///
/// @param AtomicPtr   The memory location accessed atomically.
/// @Param ExpectedPtr Pointer to the data expected at \p Ptr. The exchange will
///                    only happen if the value at \p Ptr is equal to this
///                    (unless IsWeak is set). Data at \p ExpectedPtr may or may
///                    not be be overwritten, so do not use after this call.
/// @Param DesiredPtr  Pointer to the data that the data at \p Ptr is replaced
///                    with.
/// @param IsWeak      If true, the exchange may not happen even if the data at
///                    \p Ptr equals to \p ExpectedPtr.
/// @param IsVolatile  Whether to mark the access as volatile.
/// @param SuccessMemorder If the exchange succeeds, memory is affected
///                    according to the memory model.
/// @param FailureMemorder If the exchange fails, memory is affected according
///                    to the memory model. It is considered an atomic "read"
///                    for the purpose of identifying release sequences. Must
///                    not be release, acquire-release, and at most as strong as
///                    \p SuccessMemorder.
/// @param Scope       (optional) The synchronization scope (domain of threads
///                    where this access has to be atomic, e.g. CUDA
///                    warp/block/grid-level atomics) of this access. Defaults
///                    to system scope.
/// @param DataTy      (optional) Type of the value to be accessed. cmpxchg
///                    supports integer and pointers only. If any other type or
///                    omitted, type-prunes to an integer the holds at least \p
///                    DataSize bytes.
/// @param ActualPtr   (optional) Receives the value at \p Ptr before the atomic
///                    exchange is attempted. This means:
///                    In case of success:
///                      The value at \p Ptr before the update. That is, the
///                      value passed behind \p ExpectedPtr.
///                    In case of failure (including spurious failures if
///                    IsWeak):
///                      The current value at \p Ptr, i.e. the operation
///                      effectively was an atomic load of that value using
///                      FailureMemorder semantics.
/// @param DataSize    Number of bytes to be exchanged.
/// @param AvailableSize The total size that can be used for the atomic
///                    operation. It may include trailing padding in addition to
///                    the data type's size to allow the use power-of-two
///                    instructions/calls.
/// @param Align       (optional) Known alignment of /p Ptr. If omitted,
///                    alignment is inferred from /p Ptr itself and falls back
///                    to no alignment.
/// @param Builder     User to emit instructions.
/// @param DL          The target's data layout.
/// @param TLI         The target's libcall library availability.
/// @param TL          (optional) Used to determine which instructions the
///                    target support. If omitted, assumes all accesses up to a
///                    size of 16 bytes are supported.
/// @param SyncScopes  Available scopes for the target. Only needed if /p Scope
///                    is not a constant.
/// @param FallbackScope Fallback scope if /p Scope is not an available scope.
/// @param AllowInstruction Whether a 'cmpxchg' can be emitted. False is used by
///                    AtomicExpandPass that replaces cmpxchg instructions not
///                    supported by the target.
/// @param AllowSwitch If one of IsWeak,SuccessMemorder,FailureMemorder,Scope is
///                    not a constant, allow emitting a switch for each possible
///                    value since cmpxchg only allows constant arguments for
///                    these.
/// @param AllowSizedLibcall Allow emitting calls to __atomic_compare_exchange_n
///                    libcall functions.
///
/// @return A boolean value that indicates whether the exchange has happened
///         (true) or not (false).
Expected<Value *> emitAtomicCompareExchangeBuiltin(
    Value *AtomicPtr, Value *ExpectedPtr, Value *DesiredPtr,
    std::variant<Value *, bool> IsWeak, bool IsVolatile,
    std::variant<Value *, AtomicOrdering, AtomicOrderingCABI> SuccessMemorder,
    std::variant<std::monostate, Value *, AtomicOrdering, AtomicOrderingCABI>
        FailureMemorder,
    std::variant<Value *, SyncScope::ID, StringRef> Scope, Value *PrevPtr,
    Type *DataTy, std::optional<uint64_t> DataSize,
    std::optional<uint64_t> AvailableSize, MaybeAlign Align,
    IRBuilderBase &Builder, const DataLayout &DL, const TargetLibraryInfo *TLI,
    const TargetLowering *TL,
    ArrayRef<std::pair<uint32_t, StringRef>> SyncScopes,
    StringRef FallbackScope, llvm::Twine Name = Twine(),
    bool AllowInstruction = true, bool AllowSwitch = true,
    bool AllowSizedLibcall = true, bool AllowLibcall = true);

} // namespace llvm

#endif /* LLVM_TRANSFORMS_UTILS_BUILDBUILTINS_H */
