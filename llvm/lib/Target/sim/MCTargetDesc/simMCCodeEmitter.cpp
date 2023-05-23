//===-- simMCCodeEmitter.cpp - Convert sim code to machine code -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the simMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/simInfo.h"
#include "MCTargetDesc/simFixupKinds.h"
#include "MCTargetDesc/simMCExpr.h"
#include "MCTargetDesc/simMCTargetDesc.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");
STATISTIC(MCNumFixups, "Number of MC fixups created");

namespace {
class simMCCodeEmitter : public MCCodeEmitter {
  simMCCodeEmitter(const simMCCodeEmitter &) = delete;
  void operator=(const simMCCodeEmitter &) = delete;
  MCContext &Ctx;
  MCInstrInfo const &MCII;

public:
  simMCCodeEmitter(MCContext &ctx, MCInstrInfo const &MCII)
      : Ctx(ctx), MCII(MCII) {}

  ~simMCCodeEmitter() override {}

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  void expandFunctionCall(const MCInst &MI, raw_ostream &OS,
                          SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;

  void expandAddTPRel(const MCInst &MI, raw_ostream &OS,
                      SmallVectorImpl<MCFixup> &Fixups,
                      const MCSubtargetInfo &STI) const;

  /// TableGen'erated function for getting the binary encoding for an
  /// instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// Return binary encoding of operand. If the machine operand requires
  /// relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getImmOpValueAsr1(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getImmOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const;

  unsigned getVMaskReg(const MCInst &MI, unsigned OpNo,
                       SmallVectorImpl<MCFixup> &Fixups,
                       const MCSubtargetInfo &STI) const;

private:
  FeatureBitset computeAvailableFeatures(const FeatureBitset &FB) const;
  void
  verifyInstructionPredicates(const MCInst &MI,
                              const FeatureBitset &AvailableFeatures) const;
};
} // end anonymous namespace

MCCodeEmitter *llvm::createsimMCCodeEmitter(const MCInstrInfo &MCII,
                                              const MCRegisterInfo &MRI,
                                              MCContext &Ctx) {
  return new simMCCodeEmitter(Ctx, MCII);
}

// Expand PseudoCALL(Reg), PseudoTAIL and PseudoJump to AUIPC and JALR with
// relocation types. We expand those pseudo-instructions while encoding them,
// meaning AUIPC and JALR won't go through sim MC to MC compressed
// instruction transformation. This is acceptable because AUIPC has no 16-bit
// form and C_JALR has no immediate operand field.  We let linker relaxation
// deal with it. When linker relaxation is enabled, AUIPC and JALR have a
// chance to relax to JAL.
// If the C extension is enabled, JAL has a chance relax to C_JAL.
void simMCCodeEmitter::expandFunctionCall(const MCInst &MI, raw_ostream &OS,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  MCInst TmpInst;
  MCOperand Func;
  MCRegister Ra;
//   if (MI.getOpcode() == sim::PseudoTAIL) {
//     Func = MI.getOperand(0);
//     Ra = sim::X6;
//   } else if (MI.getOpcode() == sim::PseudoCALLReg) {
//     Func = MI.getOperand(1);
//     Ra = MI.getOperand(0).getReg();
//   } else
  if (MI.getOpcode() == sim::PseudoCALL) {
    Func = MI.getOperand(0);
    Ra = sim::X1;
//   } else if (MI.getOpcode() == sim::PseudoJump) {
//     Func = MI.getOperand(1);
//     Ra = MI.getOperand(0).getReg();
//   }
  }
  uint32_t Binary;

  assert(Func.isExpr() && "Expected expression");

  const MCExpr *CallExpr = Func.getExpr();

  // Emit AUIPC Ra, Func with R_sim_CALL relocation type.
  TmpInst = MCInstBuilder(sim::AUIPC)
                .addReg(Ra)
                .addOperand(MCOperand::createExpr(CallExpr));
  Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);
  support::endian::write(OS, Binary, support::little);

//   if (MI.getOpcode() == sim::PseudoTAIL ||
//       MI.getOpcode() == sim::PseudoJump)
//     // Emit JALR X0, Ra, 0
//     TmpInst = MCInstBuilder(sim::JALR).addReg(sim::X0).addReg(Ra).addImm(0);
//   else
//     // Emit JALR Ra, Ra, 0
    TmpInst = MCInstBuilder(sim::JALR).addReg(Ra).addReg(Ra).addImm(0);
  Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);
  support::endian::write(OS, Binary, support::little);
}

// Expand PseudoAddTPRel to a simple ADD with the correct relocation.
void simMCCodeEmitter::expandAddTPRel(const MCInst &MI, raw_ostream &OS,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  MCOperand DestReg = MI.getOperand(0);
  MCOperand SrcReg = MI.getOperand(1);
  MCOperand TPReg = MI.getOperand(2);
  assert(TPReg.isReg() && TPReg.getReg() == sim::X4 &&
         "Expected thread pointer as second input to TP-relative add");

  MCOperand SrcSymbol = MI.getOperand(3);
  assert(SrcSymbol.isExpr() &&
         "Expected expression as third input to TP-relative add");

  const simMCExpr *Expr = dyn_cast<simMCExpr>(SrcSymbol.getExpr());
  assert(Expr && Expr->getKind() == simMCExpr::VK_sim_TPREL_ADD &&
         "Expected tprel_add relocation on TP-relative symbol");

  // Emit the correct tprel_add relocation for the symbol.
  Fixups.push_back(MCFixup::create(
      0, Expr, MCFixupKind(sim::fixup_sim_tprel_add), MI.getLoc()));

  // Emit fixup_sim_relax for tprel_add where the relax feature is enabled.
//   if (STI.getFeatureBits()[sim::FeatureRelax]) {
//     const MCConstantExpr *Dummy = MCConstantExpr::create(0, Ctx);
//     Fixups.push_back(MCFixup::create(
//         0, Dummy, MCFixupKind(sim::fixup_sim_relax), MI.getLoc()));
//   }

  // Emit a normal ADD instruction with the given operands.
  MCInst TmpInst = MCInstBuilder(sim::ADD)
                       .addOperand(DestReg)
                       .addOperand(SrcReg)
                       .addOperand(TPReg);
  uint32_t Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);
  support::endian::write(OS, Binary, support::little);
}

void simMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  verifyInstructionPredicates(MI,
                              computeAvailableFeatures(STI.getFeatureBits()));

  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  // Get byte count of instruction.
  unsigned Size = Desc.getSize();

  // simInstrInfo::getInstSizeInBytes expects that the total size of the
  // expanded instructions for each pseudo is correct in the Size field of the
  // tablegen definition for the pseudo.
  if (MI.getOpcode() == sim::PseudoCALL) {
    expandFunctionCall(MI, OS, Fixups, STI);
    MCNumEmitted += 2;
    return;
  }

//   if (MI.getOpcode() == sim::PseudoAddTPRel) {
//     expandAddTPRel(MI, OS, Fixups, STI);
//     MCNumEmitted += 1;
//     return;
//   }

  switch (Size) {
  default:
    llvm_unreachable("Unhandled encodeInstruction length!");
  case 2: {
    uint16_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
    support::endian::write<uint16_t>(OS, Bits, support::little);
    break;
  }
  case 4: {
    uint32_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
    support::endian::write(OS, Bits, support::little);
    break;
  }
  }

  ++MCNumEmitted; // Keep track of the # of mi's emitted.
}

unsigned
simMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {

  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  llvm_unreachable("Unhandled expression!");
  return 0;
}

unsigned
simMCCodeEmitter::getImmOpValueAsr1(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  if (MO.isImm()) {
    unsigned Res = MO.getImm();
    assert((Res & 1) == 0 && "LSB is non-zero");
    return Res >> 1;
  }

  return getImmOpValue(MI, OpNo, Fixups, STI);
}

unsigned simMCCodeEmitter::getImmOpValue(const MCInst &MI, unsigned OpNo,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  bool EnableRelax = false; // STI.getFeatureBits()[sim::FeatureRelax];
  const MCOperand &MO = MI.getOperand(OpNo);

  MCInstrDesc const &Desc = MCII.get(MI.getOpcode());
  unsigned MIFrm = simII::getFormat(Desc.TSFlags);

  // If the destination is an immediate, there is nothing to do.
  if (MO.isImm())
    return MO.getImm();

  assert(MO.isExpr() &&
         "getImmOpValue expects only expressions or immediates");
  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();
  sim::Fixups FixupKind = sim::fixup_sim_invalid;
  bool RelaxCandidate = false;
  if (Kind == MCExpr::Target) {
    const simMCExpr *RVExpr = cast<simMCExpr>(Expr);

    switch (RVExpr->getKind()) {
    case simMCExpr::VK_sim_None:
    case simMCExpr::VK_sim_Invalid:
    case simMCExpr::VK_sim_32_PCREL:
      llvm_unreachable("Unhandled fixup kind!");
    case simMCExpr::VK_sim_TPREL_ADD:
      // tprel_add is only used to indicate that a relocation should be emitted
      // for an add instruction used in TP-relative addressing. It should not be
      // expanded as if representing an actual instruction operand and so to
      // encounter it here is an error.
      llvm_unreachable(
          "VK_sim_TPREL_ADD should not represent an instruction operand");
    case simMCExpr::VK_sim_LO:
      if (MIFrm == simII::InstFormatI)
        FixupKind = sim::fixup_sim_lo12_i;
      else if (MIFrm == simII::InstFormatS)
        FixupKind = sim::fixup_sim_lo12_s;
      else
        llvm_unreachable("VK_sim_LO used with unexpected instruction format");
      RelaxCandidate = true;
      break;
    case simMCExpr::VK_sim_HI:
      FixupKind = sim::fixup_sim_hi20;
      RelaxCandidate = true;
      break;
    case simMCExpr::VK_sim_PCREL_LO:
      if (MIFrm == simII::InstFormatI)
        FixupKind = sim::fixup_sim_pcrel_lo12_i;
      else if (MIFrm == simII::InstFormatS)
        FixupKind = sim::fixup_sim_pcrel_lo12_s;
      else
        llvm_unreachable(
            "VK_sim_PCREL_LO used with unexpected instruction format");
      RelaxCandidate = true;
      break;
    case simMCExpr::VK_sim_PCREL_HI:
      FixupKind = sim::fixup_sim_pcrel_hi20;
      RelaxCandidate = true;
      break;
    case simMCExpr::VK_sim_GOT_HI:
      FixupKind = sim::fixup_sim_got_hi20;
      break;
    case simMCExpr::VK_sim_TPREL_LO:
      if (MIFrm == simII::InstFormatI)
        FixupKind = sim::fixup_sim_tprel_lo12_i;
      else if (MIFrm == simII::InstFormatS)
        FixupKind = sim::fixup_sim_tprel_lo12_s;
      else
        llvm_unreachable(
            "VK_sim_TPREL_LO used with unexpected instruction format");
      RelaxCandidate = true;
      break;
    case simMCExpr::VK_sim_TPREL_HI:
      FixupKind = sim::fixup_sim_tprel_hi20;
      RelaxCandidate = true;
      break;
    case simMCExpr::VK_sim_TLS_GOT_HI:
      FixupKind = sim::fixup_sim_tls_got_hi20;
      break;
    case simMCExpr::VK_sim_TLS_GD_HI:
      FixupKind = sim::fixup_sim_tls_gd_hi20;
      break;
    case simMCExpr::VK_sim_CALL:
      FixupKind = sim::fixup_sim_call;
      RelaxCandidate = true;
      break;
    case simMCExpr::VK_sim_CALL_PLT:
      FixupKind = sim::fixup_sim_call_plt;
      RelaxCandidate = true;
      break;
    }
  } else if (Kind == MCExpr::SymbolRef &&
             cast<MCSymbolRefExpr>(Expr)->getKind() == MCSymbolRefExpr::VK_None) {
    if (MIFrm == simII::InstFormatJ) {
      FixupKind = sim::fixup_sim_jal;
    } else if (MIFrm == simII::InstFormatB) {
      FixupKind = sim::fixup_sim_branch;
    } else {
      llvm_unreachable("Unhandled fixup");
    } 
    // } else if (MIFrm == simII::InstFormatCJ) {
    //   FixupKind = sim::fixup_sim_rvc_jump;
    // } else if (MIFrm == simII::InstFormatCB) {
    //   FixupKind = sim::fixup_sim_rvc_branch;
    // }
  }

  assert(FixupKind != sim::fixup_sim_invalid && "Unhandled expression!");

  Fixups.push_back(
      MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));
  ++MCNumFixups;

  // Ensure an R_sim_RELAX relocation will be emitted if linker relaxation is
  // enabled and the current fixup will result in a relocation that may be
  // relaxed.
  if (EnableRelax && RelaxCandidate) {
    const MCConstantExpr *Dummy = MCConstantExpr::create(0, Ctx);
    Fixups.push_back(
    MCFixup::create(0, Dummy, MCFixupKind(sim::fixup_sim_relax),
                    MI.getLoc()));
    ++MCNumFixups;
  }

  return 0;
}

unsigned simMCCodeEmitter::getVMaskReg(const MCInst &MI, unsigned OpNo,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
    llvm_unreachable("Andrey && Dmitryy && Kirill && Vasilii were here");
}

#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "simGenMCCodeEmitter.inc"
