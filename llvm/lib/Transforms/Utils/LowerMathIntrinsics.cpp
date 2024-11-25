//===- LowerMathIntrinsics.cpp ---------------------------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/LowerMathIntrinsics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "lower-math-intrinsics"

using namespace llvm;

bool llvm::lowerUnaryMathIntrinsicWithScalableVecArgAsLoop(Module &M,
                                                           CallInst *CI) {
  ScalableVectorType *ScalableTy =
      dyn_cast<ScalableVectorType>(CI->getArgOperand(0)->getType());
  if (!ScalableTy) {
    return false;
  }

  BasicBlock *PreLoopBB = CI->getParent();
  BasicBlock *PostLoopBB = nullptr;
  Function *ParentFunc = PreLoopBB->getParent();
  LLVMContext &Ctx = PreLoopBB->getContext();

  PostLoopBB = PreLoopBB->splitBasicBlock(CI);
  BasicBlock *LoopBB = BasicBlock::Create(Ctx, "", ParentFunc, PostLoopBB);
  PreLoopBB->getTerminator()->setSuccessor(0, LoopBB);

  // loop preheader
  IRBuilder<> PreLoopBuilder(PreLoopBB->getTerminator());
  Value *VScale = PreLoopBuilder.CreateVScale(
      ConstantInt::get(PreLoopBuilder.getInt64Ty(), 1));
  Value *N = ConstantInt::get(PreLoopBuilder.getInt64Ty(),
                              ScalableTy->getMinNumElements());
  Value *LoopEnd = PreLoopBuilder.CreateMul(VScale, N);

  // loop body
  IRBuilder<> LoopBuilder(LoopBB);
  Type *Int64Ty = LoopBuilder.getInt64Ty();

  PHINode *LoopIndex = LoopBuilder.CreatePHI(Int64Ty, 2);
  LoopIndex->addIncoming(ConstantInt::get(Int64Ty, 0U), PreLoopBB);
  PHINode *Vec = LoopBuilder.CreatePHI(ScalableTy, 2);
  Vec->addIncoming(CI->getArgOperand(0), PreLoopBB);

  Value *Elem = LoopBuilder.CreateExtractElement(Vec, LoopIndex);
  Function *Exp = Intrinsic::getOrInsertDeclaration(
      &M, CI->getIntrinsicID(), ScalableTy->getElementType());
  Value *Res = LoopBuilder.CreateCall(Exp, Elem);
  Value *NewVec = LoopBuilder.CreateInsertElement(Vec, Res, LoopIndex);
  Vec->addIncoming(NewVec, LoopBB);

  Value *One = ConstantInt::get(Int64Ty, 1U);
  Value *NextLoopIndex = LoopBuilder.CreateAdd(LoopIndex, One);
  LoopIndex->addIncoming(NextLoopIndex, LoopBB);

  Value *ExitCond =
      LoopBuilder.CreateICmp(CmpInst::ICMP_EQ, NextLoopIndex, LoopEnd);
  LoopBuilder.CreateCondBr(ExitCond, PostLoopBB, LoopBB);

  CI->replaceAllUsesWith(NewVec);
  CI->eraseFromParent();
  return true;
}
