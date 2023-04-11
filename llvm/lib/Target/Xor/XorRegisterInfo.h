#ifndef LLVM_LIB_TARGET_Xor_XorREGISTERINFO_H
#define LLVM_LIB_TARGET_Xor_XorREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "XorGenRegisterInfo.inc"

namespace llvm {

class TargetInstrInfo;
class XorSubtarget;

struct XorRegisterInfo : public XorGenRegisterInfo {
public:
  XorRegisterInfo();

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool requiresRegisterScavenging(const MachineFunction &MF) const override;

  void eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  // Debug information queries.
  Register getFrameRegister(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_Xor_XorREGISTERINFO_H
