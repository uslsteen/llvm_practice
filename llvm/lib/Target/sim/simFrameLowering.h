#ifndef __LLVM_LIB_TARGET_SIM_SIMFRAMELOWERING_H__
#define __LLVM_LIB_TARGET_SIM_SIMFRAMELOWERING_H__

#include "sim.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class MachineFunction;
class simSubtarget;
class simInstrInfo;

class simFrameLowering : public TargetFrameLowering {
public:
  simFrameLowering(const simSubtarget &STI)
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), 0),
        STI(STI) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  void determineFrameLayout(MachineFunction &MF) const;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;

  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;

  void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                           RegScavenger *RS) const override;

  bool hasFP(const MachineFunction &MF) const override;
  bool hasBP(const MachineFunction &MF) const;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;

private:
  void adjustStackToMatchRecords(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 bool Allocate) const;

  void adjustReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                 const DebugLoc &DL, Register DestReg, Register SrcReg,
                 int64_t Val, MachineInstr::MIFlag Flag) const;

  const simSubtarget &STI;
};

} // end namespace llvm

#endif // __LLVM_LIB_TARGET_SIM_SIMFRAMELOWERING_H__
