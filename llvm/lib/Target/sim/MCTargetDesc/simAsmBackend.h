#ifndef __LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMASMBACKEND_H__
#define __LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMASMBACKEND_H__

#include "MCTargetDesc/simFixupKinds.h"
#include "MCTargetDesc/simInfo.h"
#include "MCTargetDesc/simMCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"

namespace llvm {
class MCAssembler;
class MCObjectTargetWriter;
class raw_ostream;
class MCTargetOptions;

class simAsmBackend : public MCAsmBackend {
  const MCSubtargetInfo &STI;
  uint8_t OSABI{};
  bool Is64Bit = false;
  bool ForceRelocs = false;
  const MCTargetOptions &TargetOptions;
  simABI::ABI TargetABI = simABI::ABI_ILP32;

public:
  simAsmBackend(const MCSubtargetInfo &STI, uint8_t OSABI, bool Is64Bit,
                  const MCTargetOptions &Options)
      : MCAsmBackend(support::little), STI(STI), OSABI(OSABI), Is64Bit(Is64Bit),
        TargetOptions(Options) {}
  ~simAsmBackend() override {}

  void setForceRelocs() { ForceRelocs = true; }

  // Return Size with extra Nop Bytes for alignment directive in code section.
  bool shouldInsertExtraNopBytesForCodeAlign(const MCAlignFragment &AF,
                                             unsigned &Size) override;

  // Insert target specific fixup type for alignment directive in code section.
  bool shouldInsertFixupForCodeAlign(MCAssembler &Asm,
                                     const MCAsmLayout &Layout,
                                     MCAlignFragment &AF) override;

  bool evaluateTargetFixup(const MCAssembler &Asm, const MCAsmLayout &Layout,
                           const MCFixup &Fixup, const MCFragment *DF,
                           const MCValue &Target, uint64_t &Value,
                           bool &WasForced) override;

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

  bool shouldForceRelocation(const MCAssembler &Asm, const MCFixup &Fixup,
                             const MCValue &Target) override;

  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override {
    llvm_unreachable("Handled by fixupNeedsRelaxationAdvanced");
  }

  bool fixupNeedsRelaxationAdvanced(const MCFixup &Fixup, bool Resolved,
                                    uint64_t Value,
                                    const MCRelaxableFragment *DF,
                                    const MCAsmLayout &Layout,
                                    const bool WasForced) const override;

  unsigned getNumFixupKinds() const override {
    return sim::NumTargetFixupKinds;
  }

  Optional<MCFixupKind> getFixupKind(StringRef Name) const override;

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;

  bool mayNeedRelaxation(const MCInst &Inst,
                         const MCSubtargetInfo &STI) const override;
  unsigned getRelaxedOpcode(unsigned Op) const;

  void relaxInstruction(MCInst &Inst,
                        const MCSubtargetInfo &STI) const override;

  bool relaxDwarfLineAddr(MCDwarfLineAddrFragment &DF, MCAsmLayout &Layout,
                          bool &WasRelaxed) const override;
  bool relaxDwarfCFA(MCDwarfCallFrameFragment &DF, MCAsmLayout &Layout,
                     bool &WasRelaxed) const override;

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;

  const MCTargetOptions &getTargetOptions() const { return TargetOptions; }
  simABI::ABI getTargetABI() const { return TargetABI; }
};
} // namespace llvm

#endif // __LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMASMBACKEND_H__
