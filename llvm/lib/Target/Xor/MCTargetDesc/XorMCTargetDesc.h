#ifndef LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorMCTARGETDESC_H
#define LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {
class Target;
class Triple;

extern Target TheXorTarget;

} // End llvm namespace

// Defines symbolic names for Xor registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "XorGenRegisterInfo.inc"

// Defines symbolic names for the Xor instructions.
#define GET_INSTRINFO_ENUM
#include "XorGenInstrInfo.inc"

#endif
