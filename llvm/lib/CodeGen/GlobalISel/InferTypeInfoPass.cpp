//===- llvm/CodeGen/GlobalISel/InferTypeInfoPass.cpp - StripTypeInfoPass ---*-
// C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the InferTypeInfoPass class.
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/GlobalISel/InferTypeInfoPass.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/GlobalISel/GenericMachineInstrs.h"
#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"
#include "llvm/CodeGen/GlobalISel/LoadStoreOpt.h"
#include "llvm/CodeGen/GlobalISel/Utils.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsAMDGPU.h"
#include "llvm/InitializePasses.h"

#define DEBUG_TYPE "mir-infer-type-info"

using namespace llvm;

char InferTypeInfo::ID = 0;

INITIALIZE_PASS_BEGIN(InferTypeInfo, DEBUG_TYPE, "TODO", false, false)
INITIALIZE_PASS_END(InferTypeInfo, DEBUG_TYPE, "TODO", false, false)

void InferTypeInfo::init(MachineFunction &MF) {
  this->MF = &MF;
  MRI = &MF.getRegInfo();
  Builder.setMF(MF);
}

void InferTypeInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  MachineFunctionPass::getAnalysisUsage(AU);
}

static LLT updateType(LLT Ty, bool FP) {
  LLT InferredScalarTy =
    FP ? LLT::floatingPoint(Ty.getScalarSizeInBits(), LLT::FPInfo::IEEE_FLOAT)
         : LLT::integer(Ty.getScalarSizeInBits());
  LLT InferredTy =
      Ty.isVector() ? Ty.changeElementType(InferredScalarTy) : InferredScalarTy;

  return InferredTy;
}

void InferTypeInfo::updateDef(Register Reg) {
  LLT Ty = MRI->getType(Reg);
  LLT InferredTy = updateType(Ty, true);

  MRI->setType(Reg, InferredTy);
}

void InferTypeInfo::updateUse(MachineOperand &Op, bool FP) {
  assert(Op.isReg());
  LLT Ty = MRI->getType(Op.getReg());
  LLT InferredTy = updateType(Ty, FP);

  MachineOperand *Def = MRI->getOneDef(Op.getReg());
  MachineInstr *MI = Op.getParent();
  MachineBasicBlock *MBB = MI->getParent();

  Builder.setInsertPt(*MBB, MI);
  auto Bitcast = Builder.buildBitcast(InferredTy, Def->getReg());
  Op.setReg(Bitcast.getReg(0));
}

constexpr unsigned MaxFPRSearchDepth = 5;

bool InferTypeInfo::shouldBeFP(MachineOperand &Op, unsigned Depth = 0) const {
  if (Depth > MaxFPRSearchDepth)
    return false;

  if (!Op.isReg())
    return false;

  MachineInstr &MI = *Op.getParent();

  auto Pred = [&](MachineOperand &O) { return shouldBeFP(O, Depth + 1); };

  // TODO: cache FP registers

  switch (MI.getOpcode()) {
  // def and use fp instructions
  case TargetOpcode::G_FABS:
  case TargetOpcode::G_FADD:
  case TargetOpcode::G_FCANONICALIZE:
  case TargetOpcode::G_FCEIL:
  case TargetOpcode::G_FCONSTANT:
  case TargetOpcode::G_FCOPYSIGN:
  case TargetOpcode::G_FCOS:
  case TargetOpcode::G_FDIV:
  case TargetOpcode::G_FEXP2:
  case TargetOpcode::G_FEXP:
  case TargetOpcode::G_FFLOOR:
  case TargetOpcode::G_FLOG10:
  case TargetOpcode::G_FLOG2:
  case TargetOpcode::G_FLOG:
  case TargetOpcode::G_FMA:
  case TargetOpcode::G_FMAD:
  case TargetOpcode::G_FMAXIMUM:
  case TargetOpcode::G_FMAXNUM:
  case TargetOpcode::G_FMAXNUM_IEEE:
  case TargetOpcode::G_FMINIMUM:
  case TargetOpcode::G_FMINNUM:
  case TargetOpcode::G_FMINNUM_IEEE:
  case TargetOpcode::G_FMUL:
  case TargetOpcode::G_FNEARBYINT:
  case TargetOpcode::G_FNEG:
  case TargetOpcode::G_FPEXT:
  case TargetOpcode::G_FPOW:
  case TargetOpcode::G_FPTRUNC:
  case TargetOpcode::G_FREM:
  case TargetOpcode::G_FRINT:
  case TargetOpcode::G_FSIN:
  case TargetOpcode::G_FTAN:
  case TargetOpcode::G_FACOS:
  case TargetOpcode::G_FASIN:
  case TargetOpcode::G_FATAN:
  case TargetOpcode::G_FATAN2:
  case TargetOpcode::G_FCOSH:
  case TargetOpcode::G_FSINH:
  case TargetOpcode::G_FTANH:
  case TargetOpcode::G_FSQRT:
  case TargetOpcode::G_FSUB:
  case TargetOpcode::G_INTRINSIC_ROUND:
  case TargetOpcode::G_INTRINSIC_ROUNDEVEN:
  case TargetOpcode::G_INTRINSIC_TRUNC:
  case TargetOpcode::G_VECREDUCE_FADD:
  case TargetOpcode::G_VECREDUCE_FMUL:
  case TargetOpcode::G_VECREDUCE_FMAX:
  case TargetOpcode::G_VECREDUCE_FMIN:
  case TargetOpcode::G_VECREDUCE_FMAXIMUM:
  case TargetOpcode::G_VECREDUCE_FMINIMUM:
  case TargetOpcode::G_VECREDUCE_SEQ_FADD:
  case TargetOpcode::G_VECREDUCE_SEQ_FMUL:
    return true;
  // use only fp instructions
  case TargetOpcode::G_SITOFP:
  case TargetOpcode::G_UITOFP:
    return Op.isDef();
  // def only fp instructions
  case TargetOpcode::G_FPTOSI:
  case TargetOpcode::G_FPTOUI:
  case TargetOpcode::G_FPTOSI_SAT:
  case TargetOpcode::G_FPTOUI_SAT:
  case TargetOpcode::G_FCMP:
  case TargetOpcode::G_LROUND:
  case TargetOpcode::G_LLROUND:
    return Op.isUse();
  case TargetOpcode::G_FREEZE:
  case TargetOpcode::G_IMPLICIT_DEF:
  case TargetOpcode::G_PHI:
  case TargetOpcode::G_SELECT:
  case TargetOpcode::G_BUILD_VECTOR:
  case TargetOpcode::G_CONCAT_VECTORS:
  case TargetOpcode::G_INSERT_SUBVECTOR:
  case TargetOpcode::G_EXTRACT_SUBVECTOR:
  case TargetOpcode::G_SHUFFLE_VECTOR:
  case TargetOpcode::G_SPLAT_VECTOR:
  case TargetOpcode::G_STEP_VECTOR:
  case TargetOpcode::G_VECTOR_COMPRESS: {
    return all_of(MI.all_defs(),
                  [&](MachineOperand &O) {
                    return all_of(MRI->use_operands(O.getReg()), Pred);
                  }) &&
           all_of(MI.all_uses(), [&](MachineOperand &O) {
             return all_of(MRI->def_operands(O.getReg()), Pred);
           });
  }
  case TargetOpcode::G_INSERT_VECTOR_ELT:
  case TargetOpcode::G_EXTRACT_VECTOR_ELT: {
    MachineOperand &Dst = MI.getOperand(0);
    MachineOperand &LHS = MI.getOperand(1);
    MachineOperand &RHS = MI.getOperand(2);

    return all_of(MRI->use_operands(Dst.getReg()), Pred) &&
           (!LHS.isReg() || all_of(MRI->def_operands(LHS.getReg()), Pred)) &&
           (!RHS.isReg() || all_of(MRI->def_operands(RHS.getReg()), Pred));
  }
  case TargetOpcode::G_STORE:
  case TargetOpcode::G_INDEXED_STORE: {
    MachineOperand &Val = MI.getOperand(0);
    return Op.getReg() == Val.getReg() && all_of(MRI->def_operands(Op.getReg()), Pred);
  } 
  case TargetOpcode::G_INDEXED_LOAD:
  case TargetOpcode::G_LOAD: {
    MachineOperand &Dst = MI.getOperand(0);
    return Op.getReg() == Dst.getReg() && all_of(MRI->use_operands(Dst.getReg()), Pred);
  }
  case TargetOpcode::G_ATOMICRMW_FADD:
  case TargetOpcode::G_ATOMICRMW_FSUB:
  case TargetOpcode::G_ATOMICRMW_FMAX:
  case TargetOpcode::G_ATOMICRMW_FMIN: {
    MachineOperand &WriteBack = MI.getOperand(0);
    MachineOperand &FPOp = MI.getOperand(2);
    return Op.getReg() == WriteBack.getReg() || Op.getReg() == FPOp.getReg();
  }
  case TargetOpcode::G_INTRINSIC_CONVERGENT:
  case TargetOpcode::G_INTRINSIC_CONVERGENT_W_SIDE_EFFECTS:
  case TargetOpcode::G_INTRINSIC_W_SIDE_EFFECTS:
  case TargetOpcode::G_INTRINSIC: {
      GIntrinsic *Intrinsic = dyn_cast<GIntrinsic>(&MI);
      if (!Intrinsic)
        return false;

      switch (Intrinsic->getIntrinsicID()) {
        case Intrinsic::amdgcn_rcp:
        case Intrinsic::amdgcn_log:
        case Intrinsic::amdgcn_exp2:
        case Intrinsic::amdgcn_rsq:
        case Intrinsic::amdgcn_sqrt:
        case Intrinsic::amdgcn_fdot2_f16_f16:
        case Intrinsic::amdgcn_mfma_f32_4x4x4f16:
          return true;
        default:
          return false;
      }
      return false;
  }
  default:
    break;
  }

  return false;
}

bool InferTypeInfo::inferTypeInfo(MachineFunction &MF) {
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB.instrs()) {

      for (auto &Def : MI.all_defs()) {
        if (shouldBeFP(Def)) {
          updateDef(Def.getReg());
          Changed |= true;
        }
      }

      for (auto &Use : MI.all_uses()) {
        bool IsFPDef =
            MRI->getVRegDef(Use.getReg()) &&
            all_of(MRI->def_operands(Use.getReg()),
                   [&](MachineOperand &Op) { return shouldBeFP(Op); });
        bool IsFPUse = shouldBeFP(Use);

        if (IsFPUse && !IsFPDef) {
          updateUse(Use, true);
          Changed |= true;
        } else if (!IsFPUse && IsFPDef) {
          updateUse(Use, false);
          Changed |= true;
        }
      }

      for (auto &MemOp: MI.memoperands()) {
        bool IsFP = any_of(MI.all_defs(), [&](MachineOperand &O){ return shouldBeFP(O); }) ||
          any_of(MI.all_uses(), [&](MachineOperand &O){ return shouldBeFP(O); });

          if (!IsFP)
            continue;

          LLT Ty = MemOp->getType();
          LLT NewTy = updateType(Ty, true);
          MemOp->setType(NewTy);
      }
    }
  }

  return Changed;
}

bool InferTypeInfo::runOnMachineFunction(MachineFunction &MF) {
  init(MF);
  bool Changed = false;
  Changed |= inferTypeInfo(MF);
  return Changed;
}