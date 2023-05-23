#ifndef __LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCEXPR_H__
#define __LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCEXPR_H__

#include "llvm/MC/MCExpr.h"

namespace llvm {

class StringRef;

class simMCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_sim_None,
    VK_sim_LO,
    VK_sim_HI,
    VK_sim_PCREL_LO,
    VK_sim_PCREL_HI,
    VK_sim_GOT_HI,
    VK_sim_TPREL_LO,
    VK_sim_TPREL_HI,
    VK_sim_TPREL_ADD,
    VK_sim_TLS_GOT_HI,
    VK_sim_TLS_GD_HI,
    VK_sim_CALL,
    VK_sim_CALL_PLT,
    VK_sim_32_PCREL,
    VK_sim_Invalid // Must be the last item
  };

private:
  const MCExpr *Expr;
  const VariantKind Kind;

  int64_t evaluateAsInt64(int64_t Value) const;

  explicit simMCExpr(const MCExpr *Expr, VariantKind Kind)
      : Expr(Expr), Kind(Kind) {}

public:
  static const simMCExpr *create(const MCExpr *Expr, VariantKind Kind,
                                 MCContext &Ctx);

  VariantKind getKind() const { return Kind; }

  const MCExpr *getSubExpr() const { return Expr; }

  /// Get the corresponding PC-relative HI fixup that a VK_sim_PCREL_LO
  /// points to, and optionally the fragment containing it.
  ///
  /// \returns nullptr if this isn't a VK_sim_PCREL_LO pointing to a
  /// known PC-relative HI fixup.
  const MCFixup *getPCRelHiFixup(const MCFragment **DFOut) const;

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }

  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override;

  bool evaluateAsConstant(int64_t &Res) const;

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  static bool classof(const simMCExpr *) { return true; }

  static VariantKind getVariantKindForName(StringRef name);
  static StringRef getVariantKindName(VariantKind Kind);
};

} // end namespace llvm.

#endif // __LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCEXPR_H__
