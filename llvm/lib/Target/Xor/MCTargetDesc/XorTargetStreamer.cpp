//===-- XorTargetStreamer.cpp - Xor Target Streamer Methods -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Xor specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "XorInfo.h"
#include "XorMCTargetDesc.h"
#include "XorTargetStreamer.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/XorAttributes.h"
#include "llvm/Support/XorISAInfo.h"

using namespace llvm;

XorTargetStreamer::XorTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

void XorTargetStreamer::finish() { finishAttributeSection(); }

void XorTargetStreamer::emitDirectiveOptionPush() {}
void XorTargetStreamer::emitDirectiveOptionPop() {}
void XorTargetStreamer::emitDirectiveOptionPIC() {}
void XorTargetStreamer::emitDirectiveOptionNoPIC() {}
void XorTargetStreamer::emitDirectiveOptionRVC() {}
void XorTargetStreamer::emitDirectiveOptionNoRVC() {}
void XorTargetStreamer::emitDirectiveOptionRelax() {}
void XorTargetStreamer::emitDirectiveOptionNoRelax() {}
void XorTargetStreamer::emitAttribute(unsigned Attribute, unsigned Value) {}
void XorTargetStreamer::finishAttributeSection() {}
void XorTargetStreamer::emitTextAttribute(unsigned Attribute,
                                          StringRef String) {}
void XorTargetStreamer::emitIntTextAttribute(unsigned Attribute,
                                             unsigned IntValue,
                                             StringRef StringValue) {}

void XorTargetStreamer::emitTargetAttributes(const MCSubtargetInfo &STI) {
  //   if (STI.hasFeature(Xor::FeatureRV32E))
  //     emitAttribute(XorAttrs::STACK_ALIGN, XorAttrs::ALIGN_4);
  //   else
  emitAttribute(XorAttrs::STACK_ALIGN, XorAttrs::ALIGN_16);

  unsigned XLen = 32;
  std::vector<std::string> FeatureVector;
  XorFeatures::toFeatureVector(FeatureVector, STI.getFeatureBits());

  auto ParseResult = llvm::XorISAInfo::parseFeatures(XLen, FeatureVector);
  if (!ParseResult) {
    /* Assume any error about features should handled earlier.  */
    consumeError(ParseResult.takeError());
    llvm_unreachable("Parsing feature error when emitTargetAttributes?");
  } else {
    auto &ISAInfo = *ParseResult;
    emitTextAttribute(XorAttrs::ARCH, ISAInfo->toString());
  }
}

// This part is for ascii assembly output
XorTargetAsmStreamer::XorTargetAsmStreamer(MCStreamer &S,
                                           formatted_raw_ostream &OS)
    : XorTargetStreamer(S), OS(OS) {}

void XorTargetAsmStreamer::emitDirectiveOptionPush() {
  OS << "\t.option\tpush\n";
}

void XorTargetAsmStreamer::emitDirectiveOptionPop() {
  OS << "\t.option\tpop\n";
}

void XorTargetAsmStreamer::emitDirectiveOptionPIC() {
  OS << "\t.option\tpic\n";
}

void XorTargetAsmStreamer::emitDirectiveOptionNoPIC() {
  OS << "\t.option\tnopic\n";
}

void XorTargetAsmStreamer::emitDirectiveOptionRVC() {
  OS << "\t.option\trvc\n";
}

void XorTargetAsmStreamer::emitDirectiveOptionNoRVC() {
  OS << "\t.option\tnorvc\n";
}

void XorTargetAsmStreamer::emitDirectiveOptionRelax() {
  OS << "\t.option\trelax\n";
}

void XorTargetAsmStreamer::emitDirectiveOptionNoRelax() {
  OS << "\t.option\tnorelax\n";
}

void XorTargetAsmStreamer::emitAttribute(unsigned Attribute, unsigned Value) {
  OS << "\t.attribute\t" << Attribute << ", " << Twine(Value) << "\n";
}

void XorTargetAsmStreamer::emitTextAttribute(unsigned Attribute,
                                             StringRef String) {
  OS << "\t.attribute\t" << Attribute << ", \"" << String << "\"\n";
}

void XorTargetAsmStreamer::emitIntTextAttribute(unsigned Attribute,
                                                unsigned IntValue,
                                                StringRef StringValue) {}

void XorTargetAsmStreamer::finishAttributeSection() {}
