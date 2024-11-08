//===- NVVMUpgradeAnnotations.cpp - Upgrade NVVM Annotations --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass replaces deprecated metadata in nvvm.annotation with a more modern
// IR representation.
//
//===----------------------------------------------------------------------===//

#include "NVPTX.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include <cstdint>

#define DEBUG_TYPE "nvvm-upgrade-annotations"

using namespace llvm;

namespace llvm {
void initializeNVVMUpgradeAnnotationsLegacyPassPass(PassRegistry &);
} // namespace llvm

namespace {

class NVVMUpgradeAnnotationsLegacyPass : public ModulePass {
public:
  static char ID;
  NVVMUpgradeAnnotationsLegacyPass() : ModulePass(ID) {
    initializeNVVMUpgradeAnnotationsLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }
  bool runOnModule(Module &M) override;
};
} // namespace

char NVVMUpgradeAnnotationsLegacyPass::ID = 0;

bool static autoUpgradeAnnotation(Function *F, StringRef K, const Metadata *V) {
  if (K == "kernel") {
    assert(mdconst::extract<ConstantInt>(V)->getZExtValue() == 1);
    F->addFnAttr("nvvm.kernel");
    return true;
  }
  if (K == "align") {
    const uint64_t AlignBits = mdconst::extract<ConstantInt>(V)->getZExtValue();
    const unsigned Idx = (AlignBits >> 16);
    const Align StackAlign = Align(AlignBits & 0xFFFF);
    // TODO: Skip adding the stackalign attribute for returns, for now.
    if (!Idx)
      return false;
    F->addAttributeAtIndex(
        Idx, Attribute::getWithStackAlignment(F->getContext(), StackAlign));
    return true;
  }

  return false;
}

// Iterate over nvvm.annotations rewriting them as appropiate.
void static upgradeNVAnnotations(Module &M) {
  NamedMDNode *NamedMD = M.getNamedMetadata("nvvm.annotations");
  if (!NamedMD)
    return;

  SmallVector<MDNode *, 8> NewNodes;
  SmallSet<const MDNode *, 8> SeenNodes;
  for (MDNode *MD : NamedMD->operands()) {
    if (SeenNodes.contains(MD))
      continue;
    SeenNodes.insert(MD);

    Function *F = mdconst::dyn_extract_or_null<Function>(MD->getOperand(0));
    if (!F)
      continue;

    assert(MD && "Invalid MDNode for annotation");
    assert((MD->getNumOperands() % 2) == 1 && "Invalid number of operands");

    SmallVector<Metadata *, 8> NewOperands;
    // start index = 1, to skip the global variable key
    // increment = 2, to skip the value for each property-value pairs
    for (unsigned j = 1, je = MD->getNumOperands(); j < je; j += 2) {
      MDString *K = cast<MDString>(MD->getOperand(j));
      const MDOperand &V = MD->getOperand(j + 1);
      bool Upgraded = autoUpgradeAnnotation(F, K->getString(), V);
      if (!Upgraded)
        NewOperands.append({K, V});
    }

    if (!NewOperands.empty()) {
      NewOperands.insert(NewOperands.begin(), MD->getOperand(0));
      NewNodes.push_back(MDNode::get(M.getContext(), NewOperands));
    }
  }

  NamedMD->clearOperands();
  for (MDNode *N : NewNodes)
    NamedMD->addOperand(N);
}

PreservedAnalyses NVVMUpgradeAnnotationsPass::run(Module &M,
                                                  ModuleAnalysisManager &AM) {
  upgradeNVAnnotations(M);
  return PreservedAnalyses::all();
}

bool NVVMUpgradeAnnotationsLegacyPass::runOnModule(Module &M) {
  upgradeNVAnnotations(M);
  return false;
}

INITIALIZE_PASS(NVVMUpgradeAnnotationsLegacyPass, DEBUG_TYPE,
                "NVVMUpgradeAnnotations", false, false)

ModulePass *llvm::createNVVMUpgradeAnnotationsPass() {
  return new NVVMUpgradeAnnotationsLegacyPass();
}
