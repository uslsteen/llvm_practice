//===- simMatInt.cpp - Immediate materialisation -------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "simMatInt.h"
#include "MCTargetDesc/simMCTargetDesc.h"
#include "llvm/ADT/APInt.h"
#include "llvm/Support/MathExtras.h"
using namespace llvm;

static int getInstSeqCost(simMatInt::InstSeq &Res, bool HasRVC) {
  if (!HasRVC)
    return Res.size();

    llvm_unreachable("wtf compressed?(hochy kushats)");
}

// Recursively generate a sequence for materializing an integer.
static void generateInstSeqImpl(int64_t Val,
                                const FeatureBitset &ActiveFeatures,
                                simMatInt::InstSeq &Res) {
  if (isInt<32>(Val)) {
    // Depending on the active bits in the immediate Value v, the following
    // instruction sequences are emitted:
    //
    // v == 0                        : ADDI
    // v[0,12) != 0 && v[12,32) == 0 : ADDI
    // v[0,12) == 0 && v[12,32) != 0 : LUI
    // v[0,32) != 0                  : LUI+ADDI(W)
    int64_t Hi20 = ((Val + 0x800) >> 12) & 0xFFFFF;
    int64_t Lo12 = SignExtend64<12>(Val);

    if (Hi20)
      Res.push_back(simMatInt::Inst(sim::LUI, Hi20));

    if (Lo12 || Hi20 == 0) {
      unsigned AddiOpc = sim::ADDI;
      Res.push_back(simMatInt::Inst(AddiOpc, Lo12));
    }
    return;
  }

  llvm_unreachable("64 bit wtf");
}

static unsigned extractRotateInfo(int64_t Val) {
  // for case: 0b111..1..xxxxxx1..1..
  unsigned LeadingOnes = countLeadingOnes((uint64_t)Val);
  unsigned TrailingOnes = countTrailingOnes((uint64_t)Val);
  if (TrailingOnes > 0 && TrailingOnes < 64 &&
      (LeadingOnes + TrailingOnes) > (64 - 12))
    return 64 - TrailingOnes;

  // for case: 0bxxx1..1..1...xxx
  unsigned UpperTrailingOnes = countTrailingOnes(Hi_32(Val));
  unsigned LowerLeadingOnes = countLeadingOnes(Lo_32(Val));
  if (UpperTrailingOnes < 32 &&
      (UpperTrailingOnes + LowerLeadingOnes) > (64 - 12))
    return 32 - UpperTrailingOnes;

  return 0;
}

namespace llvm {
namespace simMatInt {
InstSeq generateInstSeq(int64_t Val, const FeatureBitset &ActiveFeatures) {
  simMatInt::InstSeq Res;
  generateInstSeqImpl(Val, ActiveFeatures, Res);
  return Res;
}

int getIntMatCost(const APInt &Val, unsigned Size,
                  const FeatureBitset &ActiveFeatures, bool CompressionCost) {
  int PlatRegSize = 32;

  // Split the constant into platform register sized chunks, and calculate cost
  // of each chunk.
  int Cost = 0;
  for (unsigned ShiftVal = 0; ShiftVal < Size; ShiftVal += PlatRegSize) {
    APInt Chunk = Val.ashr(ShiftVal).sextOrTrunc(PlatRegSize);
    InstSeq MatSeq = generateInstSeq(Chunk.getSExtValue(), ActiveFeatures);
    Cost += getInstSeqCost(MatSeq, false);
  }
  return std::max(1, Cost);
}
} // namespace simMatInt
} // namespace llvm
