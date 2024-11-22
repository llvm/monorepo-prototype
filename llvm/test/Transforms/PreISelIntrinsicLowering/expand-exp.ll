; RUN: opt -passes=pre-isel-intrinsic-lowering -S < %s | FileCheck %s

define <vscale x 4 x float> @softmax_kernel() {
; CHECK-LABEL: define <vscale x 4 x float> @softmax_kernel(
; CHECK-NEXT:    [[VSCALE:%.*]] = call i64 @llvm.vscale.i64()
; CHECK-NEXT:    [[LOOPEND:%.*]] = mul i64 [[VSCALE]], 4
; CHECK-NEXT:    br label %[[LOOPBODY:.*]]
; CHECK:       [[LOOPBODY]]:
; CHECK-NEXT:    [[IDX:%.*]] = phi i64 [ 0, %0 ], [ [[NEW_IDX:%.*]], %[[LOOPBODY]] ]
; CHECK-NEXT:    [[VEC:%.*]] = phi <vscale x 4 x float> [ zeroinitializer, %0 ], [ [[NEW_VEC:.*]], %[[LOOPBODY]] ]
; CHECK-NEXT:    [[ELEM:%.*]] = extractelement <vscale x 4 x float> [[VEC]], i64 [[IDX]]
; CHECK-NEXT:    [[RES:%.*]] = call float @llvm.exp.f32(float [[ELEM]])
; CHECK-NEXT:    [[NEW_VEC:%.*]] = insertelement <vscale x 4 x float> [[VEC]], float [[RES]], i64 [[IDX]]
; CHECK-NEXT:    [[NEW_IDX]] = add i64 [[IDX]], 1
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq i64 [[NEW_IDX]], [[LOOPEND]]
; CHECK-NEXT:    br i1 [[CMP]], label %[[LOOPEXIT:.*]], label %[[LOOPBODY]]
; CHECK:       [[LOOPEXIT]]:
; CHECK-NEXT:    ret <vscale x 4 x float> [[NEW_VEC]]
  %1 = call <vscale x 4 x float> @llvm.exp.nxv4f32(<vscale x 4 x float> zeroinitializer)
  ret <vscale x 4 x float> %1
}

declare <vscale x 4 x float> @llvm.exp.nxv4f32(<vscale x 4 x float>)
