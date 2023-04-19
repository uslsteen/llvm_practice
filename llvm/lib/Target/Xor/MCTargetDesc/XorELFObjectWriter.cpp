//===-- XorELFObjectWriter.cpp - Xor ELF Writer -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/XorFixupKinds.h"
#include "MCTargetDesc/XorMCExpr.h"
#include "MCTargetDesc/XorMCTargetDesc.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class XorELFObjectWriter : public MCELFObjectTargetWriter {
public:
  XorELFObjectWriter(uint8_t OSABI, bool Is64Bit);

  ~XorELFObjectWriter() override;

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
} // namespace

XorELFObjectWriter::XorELFObjectWriter(uint8_t OSABI, bool Is64Bit)
    : MCELFObjectTargetWriter(Is64Bit, OSABI, ELF::EM_Xor,
                              /*HasRelocationAddend*/ true) {}

XorELFObjectWriter::~XorELFObjectWriter() {}

unsigned XorELFObjectWriter::getRelocType(MCContext &Ctx, const MCValue &Target,
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
      return ELF::R_Xor_NONE;
    case FK_Data_4:
    case FK_PCRel_4:
      return ELF::R_Xor_32_PCREL;
    case Xor::fixup_Xor_pcrel_hi20:
      return ELF::R_Xor_PCREL_HI20;
    case Xor::fixup_Xor_pcrel_lo12_i:
      return ELF::R_Xor_PCREL_LO12_I;
    case Xor::fixup_Xor_pcrel_lo12_s:
      return ELF::R_Xor_PCREL_LO12_S;
    case Xor::fixup_Xor_got_hi20:
      return ELF::R_Xor_GOT_HI20;
    case Xor::fixup_Xor_tls_got_hi20:
      return ELF::R_Xor_TLS_GOT_HI20;
    case Xor::fixup_Xor_tls_gd_hi20:
      return ELF::R_Xor_TLS_GD_HI20;
    case Xor::fixup_Xor_jal:
      return ELF::R_Xor_JAL;
    case Xor::fixup_Xor_branch:
      return ELF::R_Xor_BRANCH;
    case Xor::fixup_Xor_rvc_jump:
      return ELF::R_Xor_RVC_JUMP;
    case Xor::fixup_Xor_rvc_branch:
      return ELF::R_Xor_RVC_BRANCH;
    case Xor::fixup_Xor_call:
      return ELF::R_Xor_CALL;
    case Xor::fixup_Xor_call_plt:
      return ELF::R_Xor_CALL_PLT;
    case Xor::fixup_Xor_add_8:
      return ELF::R_Xor_ADD8;
    case Xor::fixup_Xor_sub_8:
      return ELF::R_Xor_SUB8;
    case Xor::fixup_Xor_add_16:
      return ELF::R_Xor_ADD16;
    case Xor::fixup_Xor_sub_16:
      return ELF::R_Xor_SUB16;
    case Xor::fixup_Xor_add_32:
      return ELF::R_Xor_ADD32;
    case Xor::fixup_Xor_sub_32:
      return ELF::R_Xor_SUB32;
    case Xor::fixup_Xor_add_64:
      return ELF::R_Xor_ADD64;
    case Xor::fixup_Xor_sub_64:
      return ELF::R_Xor_SUB64;
    }
  }

  switch (Kind) {
  default:
    Ctx.reportError(Fixup.getLoc(), "Unsupported relocation type");
    return ELF::R_Xor_NONE;
  case FK_Data_1:
    Ctx.reportError(Fixup.getLoc(), "1-byte data relocations not supported");
    return ELF::R_Xor_NONE;
  case FK_Data_2:
    Ctx.reportError(Fixup.getLoc(), "2-byte data relocations not supported");
    return ELF::R_Xor_NONE;
  case FK_Data_4:
    if (Expr->getKind() == MCExpr::Target &&
        cast<XorMCExpr>(Expr)->getKind() == XorMCExpr::VK_Xor_32_PCREL)
      return ELF::R_Xor_32_PCREL;
    return ELF::R_Xor_32;
  case FK_Data_8:
    return ELF::R_Xor_64;
  case Xor::fixup_Xor_hi20:
    return ELF::R_Xor_HI20;
  case Xor::fixup_Xor_lo12_i:
    return ELF::R_Xor_LO12_I;
  case Xor::fixup_Xor_lo12_s:
    return ELF::R_Xor_LO12_S;
  case Xor::fixup_Xor_tprel_hi20:
    return ELF::R_Xor_TPREL_HI20;
  case Xor::fixup_Xor_tprel_lo12_i:
    return ELF::R_Xor_TPREL_LO12_I;
  case Xor::fixup_Xor_tprel_lo12_s:
    return ELF::R_Xor_TPREL_LO12_S;
  case Xor::fixup_Xor_tprel_add:
    return ELF::R_Xor_TPREL_ADD;
  case Xor::fixup_Xor_relax:
    return ELF::R_Xor_RELAX;
  case Xor::fixup_Xor_align:
    return ELF::R_Xor_ALIGN;
  case Xor::fixup_Xor_set_6b:
    return ELF::R_Xor_SET6;
  case Xor::fixup_Xor_sub_6b:
    return ELF::R_Xor_SUB6;
  case Xor::fixup_Xor_add_8:
    return ELF::R_Xor_ADD8;
  case Xor::fixup_Xor_set_8:
    return ELF::R_Xor_SET8;
  case Xor::fixup_Xor_sub_8:
    return ELF::R_Xor_SUB8;
  case Xor::fixup_Xor_set_16:
    return ELF::R_Xor_SET16;
  case Xor::fixup_Xor_add_16:
    return ELF::R_Xor_ADD16;
  case Xor::fixup_Xor_sub_16:
    return ELF::R_Xor_SUB16;
  case Xor::fixup_Xor_set_32:
    return ELF::R_Xor_SET32;
  case Xor::fixup_Xor_add_32:
    return ELF::R_Xor_ADD32;
  case Xor::fixup_Xor_sub_32:
    return ELF::R_Xor_SUB32;
  case Xor::fixup_Xor_add_64:
    return ELF::R_Xor_ADD64;
  case Xor::fixup_Xor_sub_64:
    return ELF::R_Xor_SUB64;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createXorELFObjectWriter(uint8_t OSABI, bool Is64Bit) {
  return std::make_unique<XorELFObjectWriter>(OSABI, Is64Bit);
}
