; REQUIRES: asserts
; RUN: opt < %s -passes=loop-interchange -cache-line-size=64 -verify-dom-info -verify-loop-info \
; RUN:     -S -debug 2>&1 | FileCheck %s

@aa = dso_local global [256 x [256 x float]] zeroinitializer, align 64
@bb = dso_local global [256 x [256 x float]] zeroinitializer, align 64

;;  for (int i=0;i<255;++i)
;;    for (int j=1;j<256;++j)
;;      aa[j][i] = aa[j-1][i+1] + bb[j][i];
;;
;; The direciton vector of `aa` is [< >]. We cannot interchange the loops
;; because the read/write order for `aa` cannot be changed.

; CHECK: Dependency matrix before interchange:
; CHECK-NEXT: < >
; CHECK-NEXT: Processing InnerLoopId = 1 and OuterLoopId = 0
; CHECK-NEXT: Failed interchange InnerLoopId = 1 and OuterLoopId = 0 due to dependence
; CHECK-NEXT: Not interchanging loops. Cannot prove legality.

define void @f() {
; Preheader:
entry:
  br label %for.cond1.preheader

; Loop:
for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %indvars.iv31 = phi i64 [ 0, %entry ], [ %indvars.iv.next32, %for.cond.cleanup3 ]
  %indvars.iv.next32 = add nuw nsw i64 %indvars.iv31, 1
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %exitcond34 = icmp ne i64 %indvars.iv.next32, 255
  br i1 %exitcond34, label %for.cond1.preheader, label %for.cond.cleanup

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %indvars.iv = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %0 = add nsw i64 %indvars.iv, -1
  %arrayidx6 = getelementptr inbounds [256 x [256 x float]], ptr @aa, i64 0, i64 %0, i64 %indvars.iv.next32
  %1 = load float, ptr %arrayidx6, align 4
  %arrayidx10 = getelementptr inbounds [256 x [256 x float]], ptr @bb, i64 0, i64 %indvars.iv, i64 %indvars.iv31
  %2 = load float, ptr %arrayidx10, align 4
  %add11 = fadd fast float %2, %1
  %arrayidx15 = getelementptr inbounds [256 x [256 x float]], ptr @aa, i64 0, i64 %indvars.iv, i64 %indvars.iv31
  store float %add11, ptr %arrayidx15, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 256
  br i1 %exitcond, label %for.body4, label %for.cond.cleanup3

; Exit blocks
for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void
}
