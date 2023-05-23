#include "simInfo.h"

#define GET_REGINFO_ENUM
#include "simGenRegisterInfo.inc"

namespace llvm {
namespace simABI {
MCRegister getBPReg() { return sim::X9; }
MCRegister getSCSPReg() { return sim::X18; }
} // namespace simABI
} // namespace llvm
