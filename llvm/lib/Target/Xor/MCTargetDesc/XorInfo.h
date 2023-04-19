#ifndef __LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorINFO_H__
#define __LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorINFO_H__

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/MC/SubtargetFeature.h"

namespace llvm {

namespace XorII {
enum {
  InstFormatPseudo = 0,
  InstFormatR = 1,
  InstFormatR4 = 2,
  InstFormatI = 3,
  InstFormatS = 4,
  InstFormatB = 5,
  InstFormatU = 6,
  InstFormatJ = 7,

  InstFormatMask = 31,
  InstFormatShift = 0,
};

// RISC-V Specific Machine Operand Flags
enum {
  MO_None = 0,
  MO_CALL = 1,
  MO_PLT = 2,
  MO_LO = 3,
  MO_HI = 4,
  MO_PCREL_LO = 5,
  MO_PCREL_HI = 6,
  MO_GOT_HI = 7,
  MO_TPREL_LO = 8,
  MO_TPREL_HI = 9,
  MO_TPREL_ADD = 10,
  MO_TLS_GOT_HI = 11,
  MO_TLS_GD_HI = 12,

  // Used to differentiate between target-specific "direct" flags and "bitmask"
  // flags. A machine operand can only have one "direct" flag, but can have
  // multiple "bitmask" flags.
  MO_DIRECT_FLAG_MASK = 15
};

// Helper functions to read TSFlags.
/// \returns the format of the instruction.
static inline unsigned getFormat(uint64_t TSFlags) {
  return (TSFlags & InstFormatMask) >> InstFormatShift;
}

} // namespace XorII

namespace XorFeatures {

inline void validate(const Triple &TT, const FeatureBitset &FeatureBits) {}

inline void toFeatureVector(std::vector<std::string> &FeatureVector,
                            const FeatureBitset &FeatureBits) {}

} // namespace XorFeatures

namespace XorCC {
enum CondCode {
  COND_EQ,
  COND_NE,
  COND_LT,
  COND_GE,
  COND_LTU,
  COND_GEU,
  COND_INVALID
};

CondCode getOppositeBranchCondition(CondCode);

enum BRCondCode {
  BREQ = 0x0,
};
} // end namespace XorCC

namespace XorOp {
enum OperandType : unsigned {
  OPERAND_FIRST_Xor_IMM = MCOI::OPERAND_FIRST_TARGET,
  OPERAND_UIMM2 = OPERAND_FIRST_Xor_IMM,
  OPERAND_UIMM3,
  OPERAND_UIMM4,
  OPERAND_UIMM5,
  OPERAND_UIMM7,
  OPERAND_UIMM12,
  OPERAND_SIMM12,
  OPERAND_UIMM20,
  OPERAND_UIMMLOG2XLEN,
  OPERAND_RVKRNUM,
  OPERAND_LAST_Xor_IMM = OPERAND_RVKRNUM,
  // Operand is either a register or uimm5, this is used by V extension pseudo
  // instructions to represent a value that be passed as AVL to either vsetvli
  // or vsetivli.
  OPERAND_AVL,
};
} // namespace XorOp

namespace XorABI {

enum ABI { ABI_ILP32, ABI_Unknown };

// Returns the target ABI, or else a StringError if the requested ABIName is
// not supported for the given TT and FeatureBits combination.
ABI computeTargetABI(const Triple &TT, FeatureBitset FeatureBits,
                     StringRef ABIName);

ABI getTargetABI(StringRef ABIName);

// To avoid the BP value clobbered by a function call, we need to choose a
// callee saved register to save the value. RV32E only has X8 and X9 as callee
// saved registers and X8 will be used as fp. So we choose X9 as bp.
MCRegister getBPReg();

// Returns the register holding shadow call stack pointer.
MCRegister getSCSPReg();
} // namespace XorABI

} // end namespace llvm

#endif // __LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorINFO_H__
