#ifndef __LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorMCTARGETDESC_H__
#define __LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorMCTARGETDESC_H__

#include "llvm/Config/config.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {
class Triple;
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class Target;

extern Target TheXorTarget;

MCCodeEmitter *createXorMCCodeEmitter(const MCInstrInfo &MCII,
                                      const MCRegisterInfo &MRI,
                                      MCContext &Ctx);

std::unique_ptr<MCObjectTargetWriter> createXorELFObjectWriter(uint8_t OSABI,
                                                               bool Is64Bit);

MCAsmBackend *createXorAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                  const MCRegisterInfo &MRI,
                                  const MCTargetOptions &Options);
} // namespace llvm

// Defines symbolic names for Xor registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "XorGenRegisterInfo.inc"

// Defines symbolic names for the Xor instructions.
#define GET_INSTRINFO_ENUM
#include "XorGenInstrInfo.inc"

#endif // __LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorMCTARGETDESC_H__
