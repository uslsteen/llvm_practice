//===-- Xor.h - Top-level interface for Xor representation ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM Xor back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Xor_Xor_H
#define LLVM_LIB_TARGET_Xor_Xor_H

#include "MCTargetDesc/XorMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class XorTargetMachine;
  class FunctionPass;

} // end namespace llvm;

#endif
