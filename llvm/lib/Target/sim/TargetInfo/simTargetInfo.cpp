#include "sim.h"
#include "llvm/IR/Module.h"
#include "TargetInfo/simTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

Target &llvm::getThesimTarget() {
  static Target ThesimTarget;
  return ThesimTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializesimTargetInfo() {
  RegisterTarget<Triple::sim, false> X(getThesimTarget(), "sim", "sim (32-bit simulator arch)", "sim");
}
