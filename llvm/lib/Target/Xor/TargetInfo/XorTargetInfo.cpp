#include "Xor.h"
#include "llvm/IR/Module.h"
#include "TargetInfo/XorTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheXorTarget() {
  static Target TheXorTarget;
  return TheXorTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXorTargetInfo() {
  RegisterTarget<Triple::Xor, false> X(getTheXorTarget(), "Xor", "Xor (32-bit Xorulator arch)", "Xor");
}
