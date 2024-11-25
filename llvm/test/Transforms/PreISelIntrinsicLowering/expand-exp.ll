; RUN: opt -passes=pre-isel-intrinsic-lowering -S < %s | FileCheck %s
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64"

define <vscale x 4 x float> @scalable_vec_exp(<vscale x 4 x float> %input) {
; CHECK-LABEL: define <vscale x 4 x float> @scalable_vec_exp(
; CHECK-NEXT:    [[VSCALE:%.*]] = call i64 @llvm.vscale.i64()
; CHECK-NEXT:    [[LOOPEND:%.*]] = mul i64 [[VSCALE]], 4
; CHECK-NEXT:    br label %[[LOOPBODY:.*]]
; CHECK:       [[LOOPBODY]]:
; CHECK-NEXT:    [[IDX:%.*]] = phi i64 [ 0, %0 ], [ [[NEW_IDX:%.*]], %[[LOOPBODY]] ]
; CHECK-NEXT:    [[VEC:%.*]] = phi <vscale x 4 x float> [ %input, %0 ], [ [[NEW_VEC:.*]], %[[LOOPBODY]] ]
; CHECK-NEXT:    [[ELEM:%.*]] = extractelement <vscale x 4 x float> [[VEC]], i64 [[IDX]]
; CHECK-NEXT:    [[RES:%.*]] = call float @llvm.exp.f32(float [[ELEM]])
; CHECK-NEXT:    [[NEW_VEC:%.*]] = insertelement <vscale x 4 x float> [[VEC]], float [[RES]], i64 [[IDX]]
; CHECK-NEXT:    [[NEW_IDX]] = add i64 [[IDX]], 1
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq i64 [[NEW_IDX]], [[LOOPEND]]
; CHECK-NEXT:    br i1 [[CMP]], label %[[LOOPEXIT:.*]], label %[[LOOPBODY]]
; CHECK:       [[LOOPEXIT]]:
; CHECK-NEXT:    ret <vscale x 4 x float> [[NEW_VEC]]
  %output = call <vscale x 4 x float> @llvm.exp.nxv4f32(<vscale x 4 x float> %input)
  ret <vscale x 4 x float> %output
}

; CHECK: declare i64 @llvm.vscale.i64() #1
; CHECK: declare float @llvm.exp.f32(float) #0
declare <vscale x 4 x float> @llvm.exp.nxv4f32(<vscale x 4 x float>) #0

; CHECK: attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
; CHECK-NEXT: attributes #1 = { nocallback nofree nosync nounwind willreturn memory(none) }
attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
