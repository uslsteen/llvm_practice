//===-- simELFObjectWriter.cpp - sim ELF Writer -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/simFixupKinds.h"
#include "MCTargetDesc/simMCExpr.h"
#include "MCTargetDesc/simMCTargetDesc.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class simELFObjectWriter : public MCELFObjectTargetWriter {
public:
  simELFObjectWriter(uint8_t OSABI, bool Is64Bit);

  ~simELFObjectWriter() override;

  // Return true if the given relocation must be with a symbol rather than
  // section plus offset.
  bool needsRelocateWithSymbol(const MCSymbol &Sym,
                               unsigned Type) const override {
    // TODO: this is very conservative, update once RISC-V psABI requirements
    //       are clarified.
    return true;
  }

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};
}

simELFObjectWriter::simELFObjectWriter(uint8_t OSABI, bool Is64Bit)
    : MCELFObjectTargetWriter(Is64Bit, OSABI, ELF::EM_sim,
                              /*HasRelocationAddend*/ true) {}

simELFObjectWriter::~simELFObjectWriter() {}

unsigned simELFObjectWriter::getRelocType(MCContext &Ctx,
                                            const MCValue &Target,
                                            const MCFixup &Fixup,
                                            bool IsPCRel) const {
  const MCExpr *Expr = Fixup.getValue();
  // Determine the type of the relocation
  unsigned Kind = Fixup.getTargetKind();
  if (Kind >= FirstLiteralRelocationKind)
    return Kind - FirstLiteralRelocationKind;
  if (IsPCRel) {
    switch (Kind) {
    default:
      Ctx.reportError(Fixup.getLoc(), "Unsupported relocation type");
      return ELF::R_sim_NONE;
    case FK_Data_4:
    case FK_PCRel_4:
      return ELF::R_sim_32_PCREL;
    case sim::fixup_sim_pcrel_hi20:
      return ELF::R_sim_PCREL_HI20;
    case sim::fixup_sim_pcrel_lo12_i:
      return ELF::R_sim_PCREL_LO12_I;
    case sim::fixup_sim_pcrel_lo12_s:
      return ELF::R_sim_PCREL_LO12_S;
    case sim::fixup_sim_got_hi20:
      return ELF::R_sim_GOT_HI20;
    case sim::fixup_sim_tls_got_hi20:
      return ELF::R_sim_TLS_GOT_HI20;
    case sim::fixup_sim_tls_gd_hi20:
      return ELF::R_sim_TLS_GD_HI20;
    case sim::fixup_sim_jal:
      return ELF::R_sim_JAL;
    case sim::fixup_sim_branch:
      return ELF::R_sim_BRANCH;
    case sim::fixup_sim_rvc_jump:
      return ELF::R_sim_RVC_JUMP;
    case sim::fixup_sim_rvc_branch:
      return ELF::R_sim_RVC_BRANCH;
    case sim::fixup_sim_call:
      return ELF::R_sim_CALL;
    case sim::fixup_sim_call_plt:
      return ELF::R_sim_CALL_PLT;
    case sim::fixup_sim_add_8:
      return ELF::R_sim_ADD8;
    case sim::fixup_sim_sub_8:
      return ELF::R_sim_SUB8;
    case sim::fixup_sim_add_16:
      return ELF::R_sim_ADD16;
    case sim::fixup_sim_sub_16:
      return ELF::R_sim_SUB16;
    case sim::fixup_sim_add_32:
      return ELF::R_sim_ADD32;
    case sim::fixup_sim_sub_32:
      return ELF::R_sim_SUB32;
    case sim::fixup_sim_add_64:
      return ELF::R_sim_ADD64;
    case sim::fixup_sim_sub_64:
      return ELF::R_sim_SUB64;
    }
  }

  switch (Kind) {
  default:
    Ctx.reportError(Fixup.getLoc(), "Unsupported relocation type");
    return ELF::R_sim_NONE;
  case FK_Data_1:
    Ctx.reportError(Fixup.getLoc(), "1-byte data relocations not supported");
    return ELF::R_sim_NONE;
  case FK_Data_2:
    Ctx.reportError(Fixup.getLoc(), "2-byte data relocations not supported");
    return ELF::R_sim_NONE;
  case FK_Data_4:
    if (Expr->getKind() == MCExpr::Target &&
        cast<simMCExpr>(Expr)->getKind() == simMCExpr::VK_sim_32_PCREL)
      return ELF::R_sim_32_PCREL;
    return ELF::R_sim_32;
  case FK_Data_8:
    return ELF::R_sim_64;
  case sim::fixup_sim_hi20:
    return ELF::R_sim_HI20;
  case sim::fixup_sim_lo12_i:
    return ELF::R_sim_LO12_I;
  case sim::fixup_sim_lo12_s:
    return ELF::R_sim_LO12_S;
  case sim::fixup_sim_tprel_hi20:
    return ELF::R_sim_TPREL_HI20;
  case sim::fixup_sim_tprel_lo12_i:
    return ELF::R_sim_TPREL_LO12_I;
  case sim::fixup_sim_tprel_lo12_s:
    return ELF::R_sim_TPREL_LO12_S;
  case sim::fixup_sim_tprel_add:
    return ELF::R_sim_TPREL_ADD;
  case sim::fixup_sim_relax:
    return ELF::R_sim_RELAX;
  case sim::fixup_sim_align:
    return ELF::R_sim_ALIGN;
  case sim::fixup_sim_set_6b:
    return ELF::R_sim_SET6;
  case sim::fixup_sim_sub_6b:
    return ELF::R_sim_SUB6;
  case sim::fixup_sim_add_8:
    return ELF::R_sim_ADD8;
  case sim::fixup_sim_set_8:
    return ELF::R_sim_SET8;
  case sim::fixup_sim_sub_8:
    return ELF::R_sim_SUB8;
  case sim::fixup_sim_set_16:
    return ELF::R_sim_SET16;
  case sim::fixup_sim_add_16:
    return ELF::R_sim_ADD16;
  case sim::fixup_sim_sub_16:
    return ELF::R_sim_SUB16;
  case sim::fixup_sim_set_32:
    return ELF::R_sim_SET32;
  case sim::fixup_sim_add_32:
    return ELF::R_sim_ADD32;
  case sim::fixup_sim_sub_32:
    return ELF::R_sim_SUB32;
  case sim::fixup_sim_add_64:
    return ELF::R_sim_ADD64;
  case sim::fixup_sim_sub_64:
    return ELF::R_sim_SUB64;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createsimELFObjectWriter(uint8_t OSABI, bool Is64Bit) {
  return std::make_unique<simELFObjectWriter>(OSABI, Is64Bit);
}
