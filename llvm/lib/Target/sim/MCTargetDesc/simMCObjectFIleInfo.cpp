//===-- simMCObjectFileInfo.cpp - sim object file properties ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the simMCObjectFileInfo properties.
//
//===----------------------------------------------------------------------===//

#include "simMCObjectFIleInfo.h"
#include "simMCTargetDesc.h"
#include "llvm/MC/MCContext.h"

using namespace llvm;

unsigned simMCObjectFileInfo::getTextSectionAlignment() const {
  const MCSubtargetInfo *STI = getContext().getSubtargetInfo();
  return 4;
}
