#include "simAsmBackend.h"
#include "simMCExpr.h"
#include "llvm/ADT/APInt.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/LEB128.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

Optional<MCFixupKind> simAsmBackend::getFixupKind(StringRef Name) const {
  if (STI.getTargetTriple().isOSBinFormatELF()) {
    unsigned Type;
    Type = llvm::StringSwitch<unsigned>(Name)
#define ELF_RELOC(X, Y) .Case(#X, Y)
#include "llvm/BinaryFormat/ELFRelocs/sim.def"
#undef ELF_RELOC
               .Case("BFD_RELOC_NONE", ELF::R_sim_NONE)
               .Case("BFD_RELOC_32", ELF::R_sim_32)
               .Case("BFD_RELOC_64", ELF::R_sim_64)
               .Default(-1u);
    if (Type != -1u)
      return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
  }
  return None;
}

const MCFixupKindInfo &simAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[] = {
      // This table *must* be in the order that the fixup_* kinds are defined in
      // simFixupKinds.h.
      //
      // name                      offset bits  flags
      {"fixup_sim_hi20", 12, 20, 0},
      {"fixup_sim_lo12_i", 20, 12, 0},
      {"fixup_sim_lo12_s", 0, 32, 0},
      {"fixup_sim_pcrel_hi20", 12, 20,
       MCFixupKindInfo::FKF_IsPCRel | MCFixupKindInfo::FKF_IsTarget},
      {"fixup_sim_pcrel_lo12_i", 20, 12,
       MCFixupKindInfo::FKF_IsPCRel | MCFixupKindInfo::FKF_IsTarget},
      {"fixup_sim_pcrel_lo12_s", 0, 32,
       MCFixupKindInfo::FKF_IsPCRel | MCFixupKindInfo::FKF_IsTarget},
      {"fixup_sim_got_hi20", 12, 20, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_sim_tprel_hi20", 12, 20, 0},
      {"fixup_sim_tprel_lo12_i", 20, 12, 0},
      {"fixup_sim_tprel_lo12_s", 0, 32, 0},
      {"fixup_sim_tprel_add", 0, 0, 0},
      {"fixup_sim_tls_got_hi20", 12, 20, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_sim_tls_gd_hi20", 12, 20, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_sim_jal", 12, 20, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_sim_branch", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_sim_rvc_jump", 2, 11, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_sim_rvc_branch", 0, 16, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_sim_call", 0, 64, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_sim_call_plt", 0, 64, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_sim_relax", 0, 0, 0},
      {"fixup_sim_align", 0, 0, 0},

      {"fixup_sim_set_8", 0, 8, 0},
      {"fixup_sim_add_8", 0, 8, 0},
      {"fixup_sim_sub_8", 0, 8, 0},

      {"fixup_sim_set_16", 0, 16, 0},
      {"fixup_sim_add_16", 0, 16, 0},
      {"fixup_sim_sub_16", 0, 16, 0},

      {"fixup_sim_set_32", 0, 32, 0},
      {"fixup_sim_add_32", 0, 32, 0},
      {"fixup_sim_sub_32", 0, 32, 0},

      {"fixup_sim_add_64", 0, 64, 0},
      {"fixup_sim_sub_64", 0, 64, 0},

      {"fixup_sim_set_6b", 2, 6, 0},
      {"fixup_sim_sub_6b", 2, 6, 0},
  };
  static_assert((array_lengthof(Infos)) == sim::NumTargetFixupKinds,
                "Not all fixup kinds added to Infos array");

  // Fixup kinds from .reloc directive are like R_sim_NONE. They
  // do not require any extra processing.
  if (Kind >= FirstLiteralRelocationKind)
    return MCAsmBackend::getFixupKindInfo(FK_NONE);

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");
  return Infos[Kind - FirstTargetFixupKind];
}

// If linker relaxation is enabled, or the relax option had previously been
// enabled, always emit relocations even if the fixup can be resolved. This is
// necessary for correctness as offsets may change during relaxation.
bool simAsmBackend::shouldForceRelocation(const MCAssembler &Asm,
                                          const MCFixup &Fixup,
                                          const MCValue &Target) {
  if (Fixup.getKind() >= FirstLiteralRelocationKind)
    return true;
  switch (Fixup.getTargetKind()) {
  default:
    break;
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
  case FK_Data_8:
    if (Target.isAbsolute())
      return false;
    break;
  case sim::fixup_sim_got_hi20:
  case sim::fixup_sim_tls_got_hi20:
  case sim::fixup_sim_tls_gd_hi20:
    return true;
  }

  return ForceRelocs;
}

bool simAsmBackend::fixupNeedsRelaxationAdvanced(const MCFixup &Fixup,
                                                 bool Resolved, uint64_t Value,
                                                 const MCRelaxableFragment *DF,
                                                 const MCAsmLayout &Layout,
                                                 const bool WasForced) const {
  // Return true if the symbol is actually unresolved.
  // Resolved could be always false when shouldForceRelocation return true.
  // We use !WasForced to indicate that the symbol is unresolved and not forced
  // by shouldForceRelocation.
  if (!Resolved && !WasForced)
    return true;

  int64_t Offset = int64_t(Value);
  switch (Fixup.getTargetKind()) {
  default:
    return false;
  case sim::fixup_sim_rvc_branch:
    // For compressed branch instructions the immediate must be
    // in the range [-256, 254].
    return Offset > 254 || Offset < -256;
  case sim::fixup_sim_rvc_jump:
    // For compressed jump instructions the immediate must be
    // in the range [-2048, 2046].
    return Offset > 2046 || Offset < -2048;
  }
}

void simAsmBackend::relaxInstruction(MCInst &Inst,
                                     const MCSubtargetInfo &STI) const {
  // TODO: replace this with call to auto generated uncompressinstr() function.
  llvm_unreachable("Opcode not expected!");
}

bool simAsmBackend::relaxDwarfLineAddr(MCDwarfLineAddrFragment &DF,
                                       MCAsmLayout &Layout,
                                       bool &WasRelaxed) const {
  MCContext &C = Layout.getAssembler().getContext();

  int64_t LineDelta = DF.getLineDelta();
  const MCExpr &AddrDelta = DF.getAddrDelta();
  SmallVectorImpl<char> &Data = DF.getContents();
  SmallVectorImpl<MCFixup> &Fixups = DF.getFixups();
  size_t OldSize = Data.size();

  int64_t Value;
  bool IsAbsolute = AddrDelta.evaluateKnownAbsolute(Value, Layout);
  assert(IsAbsolute && "CFA with invalid expression");
  (void)IsAbsolute;

  Data.clear();
  Fixups.clear();
  raw_svector_ostream OS(Data);

  // INT64_MAX is a signal that this is actually a DW_LNE_end_sequence.
  if (LineDelta != INT64_MAX) {
    OS << uint8_t(dwarf::DW_LNS_advance_line);
    encodeSLEB128(LineDelta, OS);
  }

  unsigned Offset;
  std::pair<unsigned, unsigned> Fixup;

  // According to the DWARF specification, the `DW_LNS_fixed_advance_pc` opcode
  // takes a single unsigned half (unencoded) operand. The maximum encodable
  // value is therefore 65535.  Set a conservative upper bound for relaxation.
  if (Value > 60000) {
    unsigned PtrSize = C.getAsmInfo()->getCodePointerSize();

    OS << uint8_t(dwarf::DW_LNS_extended_op);
    encodeULEB128(PtrSize + 1, OS);

    OS << uint8_t(dwarf::DW_LNE_set_address);
    Offset = OS.tell();
    Fixup = PtrSize == 4
                ? std::make_pair(sim::fixup_sim_add_32, sim::fixup_sim_sub_32)
                : std::make_pair(sim::fixup_sim_add_64, sim::fixup_sim_sub_64);
    OS.write_zeros(PtrSize);
  } else {
    OS << uint8_t(dwarf::DW_LNS_fixed_advance_pc);
    Offset = OS.tell();
    Fixup = {sim::fixup_sim_add_16, sim::fixup_sim_sub_16};
    support::endian::write<uint16_t>(OS, 0, support::little);
  }

  const MCBinaryExpr &MBE = cast<MCBinaryExpr>(AddrDelta);
  Fixups.push_back(MCFixup::create(
      Offset, MBE.getLHS(), static_cast<MCFixupKind>(std::get<0>(Fixup))));
  Fixups.push_back(MCFixup::create(
      Offset, MBE.getRHS(), static_cast<MCFixupKind>(std::get<1>(Fixup))));

  if (LineDelta == INT64_MAX) {
    OS << uint8_t(dwarf::DW_LNS_extended_op);
    OS << uint8_t(1);
    OS << uint8_t(dwarf::DW_LNE_end_sequence);
  } else {
    OS << uint8_t(dwarf::DW_LNS_copy);
  }

  WasRelaxed = OldSize != Data.size();
  return true;
}

bool simAsmBackend::relaxDwarfCFA(MCDwarfCallFrameFragment &DF,
                                  MCAsmLayout &Layout, bool &WasRelaxed) const {

  const MCExpr &AddrDelta = DF.getAddrDelta();
  SmallVectorImpl<char> &Data = DF.getContents();
  SmallVectorImpl<MCFixup> &Fixups = DF.getFixups();
  size_t OldSize = Data.size();

  int64_t Value;
  bool IsAbsolute = AddrDelta.evaluateKnownAbsolute(Value, Layout);
  assert(IsAbsolute && "CFA with invalid expression");
  (void)IsAbsolute;

  Data.clear();
  Fixups.clear();
  raw_svector_ostream OS(Data);

  assert(
      Layout.getAssembler().getContext().getAsmInfo()->getMinInstAlignment() ==
          1 &&
      "expected 1-byte alignment");
  if (Value == 0) {
    WasRelaxed = OldSize != Data.size();
    return true;
  }

  auto AddFixups = [&Fixups, &AddrDelta](unsigned Offset,
                                         std::pair<unsigned, unsigned> Fixup) {
    const MCBinaryExpr &MBE = cast<MCBinaryExpr>(AddrDelta);
    Fixups.push_back(MCFixup::create(
        Offset, MBE.getLHS(), static_cast<MCFixupKind>(std::get<0>(Fixup))));
    Fixups.push_back(MCFixup::create(
        Offset, MBE.getRHS(), static_cast<MCFixupKind>(std::get<1>(Fixup))));
  };

  if (isUIntN(6, Value)) {
    OS << uint8_t(dwarf::DW_CFA_advance_loc);
    AddFixups(0, {sim::fixup_sim_set_6b, sim::fixup_sim_sub_6b});
  } else if (isUInt<8>(Value)) {
    OS << uint8_t(dwarf::DW_CFA_advance_loc1);
    support::endian::write<uint8_t>(OS, 0, support::little);
    AddFixups(1, {sim::fixup_sim_set_8, sim::fixup_sim_sub_8});
  } else if (isUInt<16>(Value)) {
    OS << uint8_t(dwarf::DW_CFA_advance_loc2);
    support::endian::write<uint16_t>(OS, 0, support::little);
    AddFixups(1, {sim::fixup_sim_set_16, sim::fixup_sim_sub_16});
  } else if (isUInt<32>(Value)) {
    OS << uint8_t(dwarf::DW_CFA_advance_loc4);
    support::endian::write<uint32_t>(OS, 0, support::little);
    AddFixups(1, {sim::fixup_sim_set_32, sim::fixup_sim_sub_32});
  } else {
    llvm_unreachable("unsupported CFA encoding");
  }

  WasRelaxed = OldSize != Data.size();
  return true;
}

// Given a compressed control flow instruction this function returns
// the expanded instruction.
unsigned simAsmBackend::getRelaxedOpcode(unsigned Op) const { return Op; }

bool simAsmBackend::mayNeedRelaxation(const MCInst &Inst,
                                      const MCSubtargetInfo &STI) const {
  return getRelaxedOpcode(Inst.getOpcode()) != Inst.getOpcode();
}

bool simAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                 const MCSubtargetInfo *STI) const {
  bool HasStdExtC = false;
  unsigned MinNopLen = HasStdExtC ? 2 : 4;

  if ((Count % MinNopLen) != 0)
    return false;

  // The canonical nop on RISC-V is addi x0, x0, 0.
  for (; Count >= 4; Count -= 4)
    OS.write("\x13\0\0\0", 4);

  // The canonical nop on RVC is c.nop.
  if (Count && HasStdExtC)
    OS.write("\x01\0", 2);

  return true;
}

static uint64_t adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                 MCContext &Ctx) {
  switch (Fixup.getTargetKind()) {
  default:
    llvm_unreachable("Unknown fixup kind!");
  case sim::fixup_sim_got_hi20:
  case sim::fixup_sim_tls_got_hi20:
  case sim::fixup_sim_tls_gd_hi20:
    llvm_unreachable("Relocation should be unconditionally forced\n");
  case sim::fixup_sim_set_8:
  case sim::fixup_sim_add_8:
  case sim::fixup_sim_sub_8:
  case sim::fixup_sim_set_16:
  case sim::fixup_sim_add_16:
  case sim::fixup_sim_sub_16:
  case sim::fixup_sim_set_32:
  case sim::fixup_sim_add_32:
  case sim::fixup_sim_sub_32:
  case sim::fixup_sim_add_64:
  case sim::fixup_sim_sub_64:
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
  case FK_Data_8:
  case FK_Data_6b:
    return Value;
  case sim::fixup_sim_set_6b:
    return Value & 0x03;
  case sim::fixup_sim_lo12_i:
  case sim::fixup_sim_pcrel_lo12_i:
  case sim::fixup_sim_tprel_lo12_i:
    return Value & 0xfff;
  case sim::fixup_sim_lo12_s:
  case sim::fixup_sim_pcrel_lo12_s:
  case sim::fixup_sim_tprel_lo12_s:
    return (((Value >> 5) & 0x7f) << 25) | ((Value & 0x1f) << 7);
  case sim::fixup_sim_hi20:
  case sim::fixup_sim_pcrel_hi20:
  case sim::fixup_sim_tprel_hi20:
    // Add 1 if bit 11 is 1, to compensate for low 12 bits being negative.
    return ((Value + 0x800) >> 12) & 0xfffff;
  case sim::fixup_sim_jal: {
    if (!isInt<21>(Value))
      Ctx.reportError(Fixup.getLoc(), "fixup value out of range");
    if (Value & 0x1)
      Ctx.reportError(Fixup.getLoc(), "fixup value must be 2-byte aligned");
    // Need to produce imm[19|10:1|11|19:12] from the 21-bit Value.
    unsigned Sbit = (Value >> 20) & 0x1;
    unsigned Hi8 = (Value >> 12) & 0xff;
    unsigned Mid1 = (Value >> 11) & 0x1;
    unsigned Lo10 = (Value >> 1) & 0x3ff;
    // Inst{31} = Sbit;
    // Inst{30-21} = Lo10;
    // Inst{20} = Mid1;
    // Inst{19-12} = Hi8;
    Value = (Sbit << 19) | (Lo10 << 9) | (Mid1 << 8) | Hi8;
    return Value;
  }
  case sim::fixup_sim_branch: {
    if (!isInt<13>(Value))
      Ctx.reportError(Fixup.getLoc(), "fixup value out of range");
    if (Value & 0x1)
      Ctx.reportError(Fixup.getLoc(), "fixup value must be 2-byte aligned");
    // Need to extract imm[12], imm[10:5], imm[4:1], imm[11] from the 13-bit
    // Value.
    unsigned Sbit = (Value >> 12) & 0x1;
    unsigned Hi1 = (Value >> 11) & 0x1;
    unsigned Mid6 = (Value >> 5) & 0x3f;
    unsigned Lo4 = (Value >> 1) & 0xf;
    // Inst{31} = Sbit;
    // Inst{30-25} = Mid6;
    // Inst{11-8} = Lo4;
    // Inst{7} = Hi1;
    Value = (Sbit << 31) | (Mid6 << 25) | (Lo4 << 8) | (Hi1 << 7);
    return Value;
  }
  case sim::fixup_sim_call:
  case sim::fixup_sim_call_plt: {
    // Jalr will add UpperImm with the sign-extended 12-bit LowerImm,
    // we need to add 0x800ULL before extract upper bits to reflect the
    // effect of the sign extension.
    uint64_t UpperImm = (Value + 0x800ULL) & 0xfffff000ULL;
    uint64_t LowerImm = Value & 0xfffULL;
    return UpperImm | ((LowerImm << 20) << 32);
  }
  case sim::fixup_sim_rvc_jump: {
    // Need to produce offset[11|4|9:8|10|6|7|3:1|5] from the 11-bit Value.
    unsigned Bit11 = (Value >> 11) & 0x1;
    unsigned Bit4 = (Value >> 4) & 0x1;
    unsigned Bit9_8 = (Value >> 8) & 0x3;
    unsigned Bit10 = (Value >> 10) & 0x1;
    unsigned Bit6 = (Value >> 6) & 0x1;
    unsigned Bit7 = (Value >> 7) & 0x1;
    unsigned Bit3_1 = (Value >> 1) & 0x7;
    unsigned Bit5 = (Value >> 5) & 0x1;
    Value = (Bit11 << 10) | (Bit4 << 9) | (Bit9_8 << 7) | (Bit10 << 6) |
            (Bit6 << 5) | (Bit7 << 4) | (Bit3_1 << 1) | Bit5;
    return Value;
  }
  case sim::fixup_sim_rvc_branch: {
    // Need to produce offset[8|4:3], [reg 3 bit], offset[7:6|2:1|5]
    unsigned Bit8 = (Value >> 8) & 0x1;
    unsigned Bit7_6 = (Value >> 6) & 0x3;
    unsigned Bit5 = (Value >> 5) & 0x1;
    unsigned Bit4_3 = (Value >> 3) & 0x3;
    unsigned Bit2_1 = (Value >> 1) & 0x3;
    Value = (Bit8 << 12) | (Bit4_3 << 10) | (Bit7_6 << 5) | (Bit2_1 << 3) |
            (Bit5 << 2);
    return Value;
  }
  }
}

bool simAsmBackend::evaluateTargetFixup(const MCAssembler &Asm,
                                        const MCAsmLayout &Layout,
                                        const MCFixup &Fixup,
                                        const MCFragment *DF,
                                        const MCValue &Target, uint64_t &Value,
                                        bool &WasForced) {
  const MCFixup *AUIPCFixup;
  const MCFragment *AUIPCDF;
  MCValue AUIPCTarget;
  switch (Fixup.getTargetKind()) {
  default:
    llvm_unreachable("Unexpected fixup kind!");
  case sim::fixup_sim_pcrel_hi20:
    AUIPCFixup = &Fixup;
    AUIPCDF = DF;
    AUIPCTarget = Target;
    break;
  case sim::fixup_sim_pcrel_lo12_i:
  case sim::fixup_sim_pcrel_lo12_s: {
    AUIPCFixup = cast<simMCExpr>(Fixup.getValue())->getPCRelHiFixup(&AUIPCDF);
    if (!AUIPCFixup) {
      Asm.getContext().reportError(Fixup.getLoc(),
                                   "could not find corresponding %pcrel_hi");
      return true;
    }

    // MCAssembler::evaluateFixup will emit an error for this case when it sees
    // the %pcrel_hi, so don't duplicate it when also seeing the %pcrel_lo.
    const MCExpr *AUIPCExpr = AUIPCFixup->getValue();
    if (!AUIPCExpr->evaluateAsRelocatable(AUIPCTarget, &Layout, AUIPCFixup))
      return true;
    break;
  }
  }

  if (!AUIPCTarget.getSymA() || AUIPCTarget.getSymB())
    return false;

  const MCSymbolRefExpr *A = AUIPCTarget.getSymA();
  const MCSymbol &SA = A->getSymbol();
  if (A->getKind() != MCSymbolRefExpr::VK_None || SA.isUndefined())
    return false;

  auto *Writer = Asm.getWriterPtr();
  if (!Writer)
    return false;

  bool IsResolved = Writer->isSymbolRefDifferenceFullyResolvedImpl(
      Asm, SA, *AUIPCDF, false, true);
  if (!IsResolved)
    return false;

  Value = Layout.getSymbolOffset(SA) + AUIPCTarget.getConstant();
  Value -= Layout.getFragmentOffset(AUIPCDF) + AUIPCFixup->getOffset();

  if (shouldForceRelocation(Asm, *AUIPCFixup, AUIPCTarget)) {
    WasForced = true;
    return false;
  }

  return true;
}

void simAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                               const MCValue &Target,
                               MutableArrayRef<char> Data, uint64_t Value,
                               bool IsResolved,
                               const MCSubtargetInfo *STI) const {
  MCFixupKind Kind = Fixup.getKind();
  if (Kind >= FirstLiteralRelocationKind)
    return;
  MCContext &Ctx = Asm.getContext();
  MCFixupKindInfo Info = getFixupKindInfo(Kind);
  if (!Value)
    return; // Doesn't change encoding.
  // Apply any target-specific value adjustments.
  Value = adjustFixupValue(Fixup, Value, Ctx);

  // Shift the value into position.
  Value <<= Info.TargetOffset;

  unsigned Offset = Fixup.getOffset();
  unsigned NumBytes = alignTo(Info.TargetSize + Info.TargetOffset, 8) / 8;

  assert(Offset + NumBytes <= Data.size() && "Invalid fixup offset!");

  // For each byte of the fragment that the fixup touches, mask in the
  // bits from the fixup value.
  for (unsigned i = 0; i != NumBytes; ++i) {
    Data[Offset + i] |= uint8_t((Value >> (i * 8)) & 0xff);
  }
}

// Linker relaxation may change code size. We have to insert Nops
// for .align directive when linker relaxation enabled. So then Linker
// could satisfy alignment by removing Nops.
// The function return the total Nops Size we need to insert.
bool simAsmBackend::shouldInsertExtraNopBytesForCodeAlign(
    const MCAlignFragment &AF, unsigned &Size) {
  // Calculate Nops Size only when linker relaxation enabled.
  return false;
}

// We need to insert R_sim_ALIGN relocation type to indicate the
// position of Nops and the total bytes of the Nops have been inserted
// when linker relaxation enabled.
// The function insert fixup_sim_align fixup which eventually will
// transfer to R_sim_ALIGN relocation type.
bool simAsmBackend::shouldInsertFixupForCodeAlign(MCAssembler &Asm,
                                                  const MCAsmLayout &Layout,
                                                  MCAlignFragment &AF) {
  // Insert the fixup only when linker relaxation enabled.
  return false;
}

std::unique_ptr<MCObjectTargetWriter>
simAsmBackend::createObjectTargetWriter() const {
  return createsimELFObjectWriter(OSABI, Is64Bit);
}

MCAsmBackend *llvm::createsimAsmBackend(const Target &T,
                                        const MCSubtargetInfo &STI,
                                        const MCRegisterInfo &MRI,
                                        const MCTargetOptions &Options) {
  const Triple &TT = STI.getTargetTriple();
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TT.getOS());
  return new simAsmBackend(STI, OSABI, TT.isArch64Bit(), Options);
}
