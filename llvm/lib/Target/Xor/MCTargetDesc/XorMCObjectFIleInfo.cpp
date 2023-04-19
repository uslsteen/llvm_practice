//===-- XorMCObjectFileInfo.cpp - Xor object file properties ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the XorMCObjectFileInfo properties.
//
//===----------------------------------------------------------------------===//

#include "XorMCObjectFIleInfo.h"
#include "XorMCTargetDesc.h"
#include "llvm/MC/MCContext.h"

using namespace llvm;

unsigned XorMCObjectFileInfo::getTextSectionAlignment() const {
  const MCSubtargetInfo *STI = getContext().getSubtargetInfo();
  return 4;
}
