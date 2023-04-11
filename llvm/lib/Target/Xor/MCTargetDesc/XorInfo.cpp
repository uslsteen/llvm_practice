

#include "XorInfo.h"

#define GET_REGINFO_ENUM
#include "XorGenRegisterInfo.inc"

namespace llvm {
namespace XorABI {
MCRegister getBPReg() { return Xor::X9; }
MCRegister getSCSPReg() { return Xor::X18; }
} // namespace XorABI
} // namespace llvm