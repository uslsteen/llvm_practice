#ifndef LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorMCASMINFO_H
#define LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {

class Triple;

class XorMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit XorMCAsmInfo(const Triple &TT);
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorMCASMINFO_H
