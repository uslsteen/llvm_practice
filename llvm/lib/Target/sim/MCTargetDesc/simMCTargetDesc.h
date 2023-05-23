#ifndef __LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCTARGETDESC_H__
#define __LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCTARGETDESC_H__

#include "llvm/Support/DataTypes.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Config/config.h"

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


extern Target ThesimTarget;

MCCodeEmitter *createsimMCCodeEmitter(const MCInstrInfo &MCII,
                                        const MCRegisterInfo &MRI,
                                        MCContext &Ctx);

std::unique_ptr<MCObjectTargetWriter> createsimELFObjectWriter(uint8_t OSABI,
                                                                 bool Is64Bit);
                                                         
MCAsmBackend *createsimAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const MCTargetOptions &Options);
} // End llvm namespace

// Defines symbolic names for sim registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "simGenRegisterInfo.inc"

// Defines symbolic names for the sim instructions.
#define GET_INSTRINFO_ENUM
#include "simGenInstrInfo.inc"

#endif // __LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCTARGETDESC_H__
