; RUN: opt -S %s -passes=atomic-expand -mtriple=x86_64-linux-gnu | FileCheck %s

; This file tests the functions `llvm::convertAtomicLoadToIntegerType` and
; `llvm::convertAtomicStoreToIntegerType`. If X86 stops using this 
; functionality, please move this test to a target which still is.

define float @float_load_expand(ptr %ptr) {
; CHECK-LABEL: @float_load_expand
; CHECK: %1 = load atomic i32, ptr %ptr unordered, align 4
; CHECK: %2 = bitcast i32 %1 to float
; CHECK: ret float %2
  %res = load atomic float, ptr %ptr unordered, align 4
  ret float %res
}

define float @float_load_expand_seq_cst(ptr %ptr) {
; CHECK-LABEL: @float_load_expand_seq_cst
; CHECK: %1 = load atomic i32, ptr %ptr seq_cst, align 4
; CHECK: %2 = bitcast i32 %1 to float
; CHECK: ret float %2
  %res = load atomic float, ptr %ptr seq_cst, align 4
  ret float %res
}

define float @float_load_expand_vol(ptr %ptr) {
; CHECK-LABEL: @float_load_expand_vol
; CHECK: %1 = load atomic volatile i32, ptr %ptr unordered, align 4
; CHECK: %2 = bitcast i32 %1 to float
; CHECK: ret float %2
  %res = load atomic volatile float, ptr %ptr unordered, align 4
  ret float %res
}

define float @float_load_expand_addr1(ptr addrspace(1) %ptr) {
; CHECK-LABEL: @float_load_expand_addr1
; CHECK: %1 = load atomic i32, ptr addrspace(1) %ptr unordered, align 4
; CHECK: %2 = bitcast i32 %1 to float
; CHECK: ret float %2
  %res = load atomic float, ptr addrspace(1) %ptr unordered, align 4
  ret float %res
}

define void @float_store_expand(ptr %ptr, float %v) {
; CHECK-LABEL: @float_store_expand
; CHECK: %1 = bitcast float %v to i32
; CHECK: store atomic i32 %1, ptr %ptr unordered, align 4
  store atomic float %v, ptr %ptr unordered, align 4
  ret void
}

define void @float_store_expand_seq_cst(ptr %ptr, float %v) {
; CHECK-LABEL: @float_store_expand_seq_cst
; CHECK: %1 = bitcast float %v to i32
; CHECK: store atomic i32 %1, ptr %ptr seq_cst, align 4
  store atomic float %v, ptr %ptr seq_cst, align 4
  ret void
}

define void @float_store_expand_vol(ptr %ptr, float %v) {
; CHECK-LABEL: @float_store_expand_vol
; CHECK: %1 = bitcast float %v to i32
; CHECK: store atomic volatile i32 %1, ptr %ptr unordered, align 4
  store atomic volatile float %v, ptr %ptr unordered, align 4
  ret void
}

define void @float_store_expand_addr1(ptr addrspace(1) %ptr, float %v) {
; CHECK-LABEL: @float_store_expand_addr1
; CHECK: %1 = bitcast float %v to i32
; CHECK: store atomic i32 %1, ptr addrspace(1) %ptr unordered, align 4
  store atomic float %v, ptr addrspace(1) %ptr unordered, align 4
  ret void
}

define void @pointer_cmpxchg_expand(ptr %ptr, ptr %v) {
; CHECK-LABEL: @pointer_cmpxchg_expand
; CHECK: %1 = ptrtoint ptr %v to i64
; CHECK: %2 = cmpxchg ptr %ptr, i64 0, i64 %1 seq_cst monotonic
; CHECK: %3 = extractvalue { i64, i1 } %2, 0
; CHECK: %4 = extractvalue { i64, i1 } %2, 1
; CHECK: %5 = inttoptr i64 %3 to ptr
; CHECK: %6 = insertvalue { ptr, i1 } poison, ptr %5, 0
; CHECK: %7 = insertvalue { ptr, i1 } %6, i1 %4, 1
  cmpxchg ptr %ptr, ptr null, ptr %v seq_cst monotonic
  ret void
}

define void @pointer_cmpxchg_expand2(ptr %ptr, ptr %v) {
; CHECK-LABEL: @pointer_cmpxchg_expand2
; CHECK: %1 = ptrtoint ptr %v to i64
; CHECK: %2 = cmpxchg ptr %ptr, i64 0, i64 %1 release monotonic
; CHECK: %3 = extractvalue { i64, i1 } %2, 0
; CHECK: %4 = extractvalue { i64, i1 } %2, 1
; CHECK: %5 = inttoptr i64 %3 to ptr
; CHECK: %6 = insertvalue { ptr, i1 } poison, ptr %5, 0
; CHECK: %7 = insertvalue { ptr, i1 } %6, i1 %4, 1
  cmpxchg ptr %ptr, ptr null, ptr %v release monotonic
  ret void
}

define void @pointer_cmpxchg_expand3(ptr %ptr, ptr %v) {
; CHECK-LABEL: @pointer_cmpxchg_expand3
; CHECK: %1 = ptrtoint ptr %v to i64
; CHECK: %2 = cmpxchg ptr %ptr, i64 0, i64 %1 seq_cst seq_cst
; CHECK: %3 = extractvalue { i64, i1 } %2, 0
; CHECK: %4 = extractvalue { i64, i1 } %2, 1
; CHECK: %5 = inttoptr i64 %3 to ptr
; CHECK: %6 = insertvalue { ptr, i1 } poison, ptr %5, 0
; CHECK: %7 = insertvalue { ptr, i1 } %6, i1 %4, 1
  cmpxchg ptr %ptr, ptr null, ptr %v seq_cst seq_cst
  ret void
}

define void @pointer_cmpxchg_expand4(ptr %ptr, ptr %v) {
; CHECK-LABEL: @pointer_cmpxchg_expand4
; CHECK: %1 = ptrtoint ptr %v to i64
; CHECK: %2 = cmpxchg weak ptr %ptr, i64 0, i64 %1 seq_cst seq_cst
; CHECK: %3 = extractvalue { i64, i1 } %2, 0
; CHECK: %4 = extractvalue { i64, i1 } %2, 1
; CHECK: %5 = inttoptr i64 %3 to ptr
; CHECK: %6 = insertvalue { ptr, i1 } poison, ptr %5, 0
; CHECK: %7 = insertvalue { ptr, i1 } %6, i1 %4, 1
  cmpxchg weak ptr %ptr, ptr null, ptr %v seq_cst seq_cst
  ret void
}

define void @pointer_cmpxchg_expand5(ptr %ptr, ptr %v) {
; CHECK-LABEL: @pointer_cmpxchg_expand5
; CHECK: %1 = ptrtoint ptr %v to i64
; CHECK: %2 = cmpxchg volatile ptr %ptr, i64 0, i64 %1 seq_cst seq_cst
; CHECK: %3 = extractvalue { i64, i1 } %2, 0
; CHECK: %4 = extractvalue { i64, i1 } %2, 1
; CHECK: %5 = inttoptr i64 %3 to ptr
; CHECK: %6 = insertvalue { ptr, i1 } poison, ptr %5, 0
; CHECK: %7 = insertvalue { ptr, i1 } %6, i1 %4, 1
  cmpxchg volatile ptr %ptr, ptr null, ptr %v seq_cst seq_cst
  ret void
}

define void @pointer_cmpxchg_expand6(ptr addrspace(1) %ptr, 
                                     ptr addrspace(2) %v) {
; CHECK-LABEL: @pointer_cmpxchg_expand6
; CHECK: %1 = ptrtoint ptr addrspace(2) %v to i64
; CHECK: %2 = cmpxchg ptr addrspace(1) %ptr, i64 0, i64 %1 seq_cst seq_cst
; CHECK: %3 = extractvalue { i64, i1 } %2, 0
; CHECK: %4 = extractvalue { i64, i1 } %2, 1
; CHECK: %5 = inttoptr i64 %3 to ptr addrspace(2)
; CHECK: %6 = insertvalue { ptr addrspace(2), i1 } poison, ptr addrspace(2) %5, 0
; CHECK: %7 = insertvalue { ptr addrspace(2), i1 } %6, i1 %4, 1
  cmpxchg ptr addrspace(1) %ptr, ptr addrspace(2) null, ptr addrspace(2) %v seq_cst seq_cst
  ret void
}

define <2 x ptr> @atomic_vec2_ptr_align(ptr %x) nounwind {
; CHECK-LABEL: @atomic_vec2_ptr_align(
; CHECK-NEXT:    [[TMP1:%.*]] = call i128 @__atomic_load_16(ptr [[X:%.*]], i32 2)
; CHECK-NEXT:    [[TMP2:%.*]] = trunc i128 [[TMP1]] to i64
; CHECK-NEXT:    [[TMP3:%.*]] = lshr i128 [[TMP1]], 64
; CHECK-NEXT:    [[TMP4:%.*]] = trunc i128 [[TMP3]] to i64
; CHECK-NEXT:    [[TMP5:%.*]] = insertelement <2 x i64> poison, i64 [[TMP2]], i32 0
; CHECK-NEXT:    [[TMP6:%.*]] = insertelement <2 x i64> [[TMP5]], i64 [[TMP4]], i32 1
; CHECK-NEXT:    [[TMP7:%.*]] = inttoptr <2 x i64> [[TMP6]] to <2 x ptr>
; CHECK-NEXT:    ret <2 x ptr> [[TMP7]]
;
  %ret = load atomic <2 x ptr>, ptr %x acquire, align 16
  ret <2 x ptr> %ret
}
