//===-- XorTargetInfo.cpp - Xor Target Implementation -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Xor.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target llvm::TheXorTarget;

extern "C" void LLVMInitializeXorTargetInfo() {
  RegisterTarget<Triple::Xor,
                 /*HasJIT=*/false>
      X(TheXorTarget, "Xor", "Xor (32-bit Xorulator arch)", "Xor");
}
