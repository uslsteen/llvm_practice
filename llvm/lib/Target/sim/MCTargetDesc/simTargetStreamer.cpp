//===-- simTargetStreamer.cpp - sim Target Streamer Methods -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides sim specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "simTargetStreamer.h"
#include "simInfo.h"
#include "simMCTargetDesc.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/simAttributes.h"
#include "llvm/Support/simISAInfo.h"

using namespace llvm;

simTargetStreamer::simTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

void simTargetStreamer::finish() { finishAttributeSection(); }

void simTargetStreamer::emitDirectiveOptionPush() {}
void simTargetStreamer::emitDirectiveOptionPop() {}
void simTargetStreamer::emitDirectiveOptionPIC() {}
void simTargetStreamer::emitDirectiveOptionNoPIC() {}
void simTargetStreamer::emitDirectiveOptionRVC() {}
void simTargetStreamer::emitDirectiveOptionNoRVC() {}
void simTargetStreamer::emitDirectiveOptionRelax() {}
void simTargetStreamer::emitDirectiveOptionNoRelax() {}
void simTargetStreamer::emitAttribute(unsigned Attribute, unsigned Value) {}
void simTargetStreamer::finishAttributeSection() {}
void simTargetStreamer::emitTextAttribute(unsigned Attribute,
                                            StringRef String) {}
void simTargetStreamer::emitIntTextAttribute(unsigned Attribute,
                                               unsigned IntValue,
                                               StringRef StringValue) {}

void simTargetStreamer::emitTargetAttributes(const MCSubtargetInfo &STI) {
//   if (STI.hasFeature(sim::FeatureRV32E))
//     emitAttribute(simAttrs::STACK_ALIGN, simAttrs::ALIGN_4);
//   else
    emitAttribute(simAttrs::STACK_ALIGN, simAttrs::ALIGN_16);

  unsigned XLen = 32;
  std::vector<std::string> FeatureVector;
  simFeatures::toFeatureVector(FeatureVector, STI.getFeatureBits());

  auto ParseResult = llvm::simISAInfo::parseFeatures(XLen, FeatureVector);
  if (!ParseResult) {
    /* Assume any error about features should handled earlier.  */
    consumeError(ParseResult.takeError());
    llvm_unreachable("Parsing feature error when emitTargetAttributes?");
  } else {
    auto &ISAInfo = *ParseResult;
    emitTextAttribute(simAttrs::ARCH, ISAInfo->toString());
  }
}

// This part is for ascii assembly output
simTargetAsmStreamer::simTargetAsmStreamer(MCStreamer &S,
                                               formatted_raw_ostream &OS)
    : simTargetStreamer(S), OS(OS) {}

void simTargetAsmStreamer::emitDirectiveOptionPush() {
  OS << "\t.option\tpush\n";
}

void simTargetAsmStreamer::emitDirectiveOptionPop() {
  OS << "\t.option\tpop\n";
}

void simTargetAsmStreamer::emitDirectiveOptionPIC() {
  OS << "\t.option\tpic\n";
}

void simTargetAsmStreamer::emitDirectiveOptionNoPIC() {
  OS << "\t.option\tnopic\n";
}

void simTargetAsmStreamer::emitDirectiveOptionRVC() {
  OS << "\t.option\trvc\n";
}

void simTargetAsmStreamer::emitDirectiveOptionNoRVC() {
  OS << "\t.option\tnorvc\n";
}

void simTargetAsmStreamer::emitDirectiveOptionRelax() {
  OS << "\t.option\trelax\n";
}

void simTargetAsmStreamer::emitDirectiveOptionNoRelax() {
  OS << "\t.option\tnorelax\n";
}

void simTargetAsmStreamer::emitAttribute(unsigned Attribute, unsigned Value) {
  OS << "\t.attribute\t" << Attribute << ", " << Twine(Value) << "\n";
}

void simTargetAsmStreamer::emitTextAttribute(unsigned Attribute,
                                               StringRef String) {
  OS << "\t.attribute\t" << Attribute << ", \"" << String << "\"\n";
}

void simTargetAsmStreamer::emitIntTextAttribute(unsigned Attribute,
                                                  unsigned IntValue,
                                                  StringRef StringValue) {}

void simTargetAsmStreamer::finishAttributeSection() {}
