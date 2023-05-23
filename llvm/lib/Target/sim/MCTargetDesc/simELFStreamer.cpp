//===-- simELFStreamer.cpp - sim ELF Target Streamer Methods ----------===//
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

#include "simELFStreamer.h"
#include "simAsmBackend.h"
#include "simInfo.h"
#include "simMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/LEB128.h"
#include "llvm/Support/simAttributes.h"

using namespace llvm;

// This part is for ELF object output.
simTargetELFStreamer::simTargetELFStreamer(MCStreamer &S,
                                               const MCSubtargetInfo &STI)
    : simTargetStreamer(S), CurrentVendor("sim") {
  MCAssembler &MCA = getStreamer().getAssembler();
  const FeatureBitset &Features = STI.getFeatureBits();
  auto &MAB = static_cast<simAsmBackend &>(MCA.getBackend());
  simABI::ABI ABI = MAB.getTargetABI();
  assert(ABI != simABI::ABI_Unknown && "Improperly initialised target ABI");

  unsigned EFlags = MCA.getELFHeaderEFlags();

  switch (ABI) {
  case simABI::ABI_ILP32:
    break;
  case simABI::ABI_Unknown:
    llvm_unreachable("Improperly initialised target ABI");
  }

  MCA.setELFHeaderEFlags(EFlags);
}

MCELFStreamer &simTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}

void simTargetELFStreamer::emitDirectiveOptionPush() {}
void simTargetELFStreamer::emitDirectiveOptionPop() {}
void simTargetELFStreamer::emitDirectiveOptionPIC() {}
void simTargetELFStreamer::emitDirectiveOptionNoPIC() {}
void simTargetELFStreamer::emitDirectiveOptionRVC() {}
void simTargetELFStreamer::emitDirectiveOptionNoRVC() {}
void simTargetELFStreamer::emitDirectiveOptionRelax() {}
void simTargetELFStreamer::emitDirectiveOptionNoRelax() {}

void simTargetELFStreamer::emitAttribute(unsigned Attribute, unsigned Value) {
  setAttributeItem(Attribute, Value, /*OverwriteExisting=*/true);
}

void simTargetELFStreamer::emitTextAttribute(unsigned Attribute,
                                               StringRef String) {
  setAttributeItem(Attribute, String, /*OverwriteExisting=*/true);
}

void simTargetELFStreamer::emitIntTextAttribute(unsigned Attribute,
                                                  unsigned IntValue,
                                                  StringRef StringValue) {
  setAttributeItems(Attribute, IntValue, StringValue,
                    /*OverwriteExisting=*/true);
}

void simTargetELFStreamer::finishAttributeSection() {
  if (Contents.empty())
    return;

  if (AttributeSection) {
    Streamer.SwitchSection(AttributeSection);
  } else {
    MCAssembler &MCA = getStreamer().getAssembler();
    AttributeSection = MCA.getContext().getELFSection(
        ".sim.attributes", ELF::SHT_sim_ATTRIBUTES, 0);
    Streamer.SwitchSection(AttributeSection);

    Streamer.emitInt8(ELFAttrs::Format_Version);
  }

  // Vendor size + Vendor name + '\0'
  const size_t VendorHeaderSize = 4 + CurrentVendor.size() + 1;

  // Tag + Tag Size
  const size_t TagHeaderSize = 1 + 4;

  const size_t ContentsSize = calculateContentSize();

  Streamer.emitInt32(VendorHeaderSize + TagHeaderSize + ContentsSize);
  Streamer.emitBytes(CurrentVendor);
  Streamer.emitInt8(0); // '\0'

  Streamer.emitInt8(ELFAttrs::File);
  Streamer.emitInt32(TagHeaderSize + ContentsSize);

  // Size should have been accounted for already, now
  // emit each field as its type (ULEB or String).
  for (AttributeItem item : Contents) {
    Streamer.emitULEB128IntValue(item.Tag);
    switch (item.Type) {
    default:
      llvm_unreachable("Invalid attribute type");
    case AttributeType::Numeric:
      Streamer.emitULEB128IntValue(item.IntValue);
      break;
    case AttributeType::Text:
      Streamer.emitBytes(item.StringValue);
      Streamer.emitInt8(0); // '\0'
      break;
    case AttributeType::NumericAndText:
      Streamer.emitULEB128IntValue(item.IntValue);
      Streamer.emitBytes(item.StringValue);
      Streamer.emitInt8(0); // '\0'
      break;
    }
  }

  Contents.clear();
}

size_t simTargetELFStreamer::calculateContentSize() const {
  size_t Result = 0;
  for (AttributeItem item : Contents) {
    switch (item.Type) {
    case AttributeType::Hidden:
      break;
    case AttributeType::Numeric:
      Result += getULEB128Size(item.Tag);
      Result += getULEB128Size(item.IntValue);
      break;
    case AttributeType::Text:
      Result += getULEB128Size(item.Tag);
      Result += item.StringValue.size() + 1; // string + '\0'
      break;
    case AttributeType::NumericAndText:
      Result += getULEB128Size(item.Tag);
      Result += getULEB128Size(item.IntValue);
      Result += item.StringValue.size() + 1; // string + '\0';
      break;
    }
  }
  return Result;
}

namespace {
class simELFStreamer : public MCELFStreamer {
  static std::pair<unsigned, unsigned> getRelocPairForSize(unsigned Size) {
    switch (Size) {
    default:
      llvm_unreachable("unsupported fixup size");
    case 1:
      return std::make_pair(sim::fixup_sim_add_8, sim::fixup_sim_sub_8);
    case 2:
      return std::make_pair(sim::fixup_sim_add_16,
                            sim::fixup_sim_sub_16);
    case 4:
      return std::make_pair(sim::fixup_sim_add_32,
                            sim::fixup_sim_sub_32);
    case 8:
      return std::make_pair(sim::fixup_sim_add_64,
                            sim::fixup_sim_sub_64);
    }
  }

  static bool requiresFixups(MCContext &C, const MCExpr *Value,
                             const MCExpr *&LHS, const MCExpr *&RHS) {
    const auto *MBE = dyn_cast<MCBinaryExpr>(Value);
    if (MBE == nullptr)
      return false;

    MCValue E;
    if (!Value->evaluateAsRelocatable(E, nullptr, nullptr))
      return false;
    if (E.getSymA() == nullptr || E.getSymB() == nullptr)
      return false;

    const auto &A = E.getSymA()->getSymbol();
    const auto &B = E.getSymB()->getSymbol();

    LHS =
        MCBinaryExpr::create(MCBinaryExpr::Add, MCSymbolRefExpr::create(&A, C),
                             MCConstantExpr::create(E.getConstant(), C), C);
    RHS = E.getSymB();

    return (A.isInSection() ? A.getSection().hasInstructions()
                            : !A.getName().empty()) ||
           (B.isInSection() ? B.getSection().hasInstructions()
                            : !B.getName().empty());
  }

public:
  simELFStreamer(MCContext &C, std::unique_ptr<MCAsmBackend> MAB,
                   std::unique_ptr<MCObjectWriter> MOW,
                   std::unique_ptr<MCCodeEmitter> MCE)
      : MCELFStreamer(C, std::move(MAB), std::move(MOW), std::move(MCE)) {}

  void emitValueImpl(const MCExpr *Value, unsigned Size, SMLoc Loc) override {
    const MCExpr *A, *B;
    if (!requiresFixups(getContext(), Value, A, B))
      return MCELFStreamer::emitValueImpl(Value, Size, Loc);

    MCStreamer::emitValueImpl(Value, Size, Loc);

    MCDataFragment *DF = getOrCreateDataFragment();
    flushPendingLabels(DF, DF->getContents().size());
    MCDwarfLineEntry::make(this, getCurrentSectionOnly());

    unsigned Add, Sub;
    std::tie(Add, Sub) = getRelocPairForSize(Size);

    DF->getFixups().push_back(MCFixup::create(
        DF->getContents().size(), A, static_cast<MCFixupKind>(Add), Loc));
    DF->getFixups().push_back(MCFixup::create(
        DF->getContents().size(), B, static_cast<MCFixupKind>(Sub), Loc));

    DF->getContents().resize(DF->getContents().size() + Size, 0);
  }
};
} // namespace

namespace llvm {
MCELFStreamer *createsimELFStreamer(MCContext &C,
                                      std::unique_ptr<MCAsmBackend> MAB,
                                      std::unique_ptr<MCObjectWriter> MOW,
                                      std::unique_ptr<MCCodeEmitter> MCE,
                                      bool RelaxAll) {
  simELFStreamer *S =
      new simELFStreamer(C, std::move(MAB), std::move(MOW), std::move(MCE));
  S->getAssembler().setRelaxAll(RelaxAll);
  return S;
}
} // namespace llvm
