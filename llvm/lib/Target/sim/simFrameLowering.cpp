#include "simFrameLowering.h"
#include "MCTargetDesc/simInfo.h"
#include "simMachineFunctionInfo.h"
#include "simSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include <algorithm>

#define DEBUG_TYPE "sim-frame-lowering"

using namespace llvm;

// seems done
void simFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                            BitVector &SavedRegs,
                                            RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  // Unconditionally spill RA and FP only if the function uses a frame
  // pointer.
  if (hasFP(MF)) {
    SavedRegs.set(sim::X1);
    SavedRegs.set(sim::X8);
  }
  // Mark BP as used if function has dedicated base pointer.
  if (hasBP(MF))
    SavedRegs.set(simABI::getBPReg());
}

// TODO: Build insns
void simFrameLowering::adjustReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MBBI,
                                 const DebugLoc &DL, Register DestReg,
                                 Register SrcReg, int64_t Val,
                                 MachineInstr::MIFlag Flag) const {
  // MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();
  const simInstrInfo *TII = STI.getInstrInfo();

  if (DestReg == SrcReg && Val == 0)
    return;

  if (isInt<12>(Val)) {
    BuildMI(MBB, MBBI, DL, TII->get(sim::ADDI), DestReg)
        .addReg(SrcReg)
        .addImm(Val)
        .setMIFlag(Flag);
  } else {
    // alloc vreg, load imm, add
    llvm_unreachable("");
  }
}

void simFrameLowering::adjustStackToMatchRecords(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    bool Allocate) const {
  llvm_unreachable("");
}

// Returns the register used to hold the frame pointer.
static Register getFPReg(const simSubtarget &STI) { return sim::X8; }

// Returns the register used to hold the stack pointer.
static Register getSPReg(const simSubtarget &STI) { return sim::X2; }

// Determines the size of the frame and maximum call frame size.
void simFrameLowering::determineFrameLayout(MachineFunction &MF) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Get the number of bytes to allocate from the FrameInfo.
  uint64_t FrameSize = MFI.getStackSize();

  // Get the alignment.
  Align StackAlign = getStackAlign();

  // Make sure the frame is aligned.
  FrameSize = alignTo(FrameSize, StackAlign);

  // Update frame info.
  MFI.setStackSize(FrameSize);
}

// TODO: seems ok
void simFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  auto *FI = MF.getInfo<simFunctionInfo>();
  const simRegisterInfo *RI = STI.getRegisterInfo();
  MachineBasicBlock::iterator MBBI = MBB.begin();

  Register FPReg = getFPReg(STI);
  Register SPReg = getSPReg(STI);
  // Register BPReg = simABI::getBPReg();

  // Debug location must be unknown since the first debug location is used
  // to determine the end of the prologue.
  DebugLoc DL;

  // Since spillCalleeSavedRegisters may have inserted a libcall, skip past
  // any instructions marked as FrameSetup
  while (MBBI != MBB.end() && MBBI->getFlag(MachineInstr::FrameSetup))
    ++MBBI;

  // Determine the correct frame layout
  determineFrameLayout(MF);

  uint64_t StackSize = alignTo(MFI.getStackSize(), getStackAlign());
  MFI.setStackSize(StackSize);

  if (!isInt<16>(StackSize)) {
    llvm_unreachable("Stack offs won't fit in sim::LDi");
  }

  // Early exit if there is no need to allocate on the stack
  if (StackSize == 0 && !MFI.adjustsStack())
    return;

  // Allocate space on the stack if necessary.
  adjustReg(MBB, MBBI, DL, SPReg, SPReg, -StackSize, MachineInstr::FrameSetup);

  const auto &CSI = MFI.getCalleeSavedInfo();

  // The frame pointer is callee-saved, and code has been generated for us to
  // save it to the stack. We need to skip over the storing of callee-saved
  // registers as the frame pointer must be modified after it has been saved
  // to the stack, not before.
  // FIXME: assumes exactly one instruction is used to save each callee-saved
  // register.
  std::advance(MBBI, CSI.size());

  if (!hasFP(MF)) {
    return;
  }

  // Generate new FP.
  adjustReg(MBB, MBBI, DL, FPReg, SPReg, StackSize - FI->getVarArgsSaveSize(),
            MachineInstr::FrameSetup);

  if (RI->hasStackRealignment(MF)) {
    llvm_unreachable(""); // TODO: realigned stack
  }
}

// TODO: seems ok
void simFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const simRegisterInfo *RI = STI.getRegisterInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  auto *UFI = MF.getInfo<simFunctionInfo>();
  Register FPReg = getFPReg(STI);
  Register SPReg = getSPReg(STI);

  // Get the insert location for the epilogue. If there were no terminators in
  // the block, get the last instruction.
  MachineBasicBlock::iterator MBBI = MBB.end();
  DebugLoc DL;
  if (!MBB.empty()) {
    MBBI = MBB.getFirstTerminator();
    if (MBBI == MBB.end())
      MBBI = MBB.getLastNonDebugInstr();
    DL = MBBI->getDebugLoc();

    // If this is not a terminator, the actual insert location should be after
    // the last instruction.
    if (!MBBI->isTerminator())
      MBBI = std::next(MBBI);

    // TODO: is it necessary?
    while (MBBI != MBB.begin() &&
           std::prev(MBBI)->getFlag(MachineInstr::FrameDestroy))
      --MBBI;
  }

  const auto &CSI = MFI.getCalleeSavedInfo();

  // Skip to before the restores of callee-saved registers
  // FIXME: assumes exactly one instruction is used to restore each
  // callee-saved register.
  auto LastFrameDestroy = MBBI;
  if (!CSI.empty())
    LastFrameDestroy = std::prev(MBBI, CSI.size());

  uint64_t StackSize = MFI.getStackSize();
  uint64_t FPOffset = StackSize - UFI->getVarArgsSaveSize();

  // Restore the stack pointer using the value of the frame pointer. Only
  // necessary if the stack pointer was modified, meaning the stack size is
  // unknown.
  if (RI->hasStackRealignment(MF) || MFI.hasVarSizedObjects()) {
    llvm_unreachable("");
    assert(hasFP(MF) && "frame pointer should not have been eliminated");
    adjustReg(MBB, LastFrameDestroy, DL, SPReg, FPReg, -FPOffset,
              MachineInstr::FrameDestroy);
  }

  // Deallocate stack
  adjustReg(MBB, MBBI, DL, SPReg, SPReg, StackSize, MachineInstr::FrameDestroy);
}

// TODO: seems ok
bool simFrameLowering::spillCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    ArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  if (CSI.empty())
    return true;

  MachineFunction *MF = MBB.getParent();
  const TargetInstrInfo &TII = *MF->getSubtarget().getInstrInfo();

  for (auto &CS : CSI) {
    // Insert the spill to the stack frame.
    Register Reg = CS.getReg();
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.storeRegToStackSlot(MBB, MI, Reg, !MBB.isLiveIn(Reg), CS.getFrameIdx(),
                            RC, TRI);
  }

  return true;
}

// TODO: seems ok
bool simFrameLowering::restoreCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    MutableArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  if (CSI.empty())
    return true;

  MachineFunction *MF = MBB.getParent();
  const TargetInstrInfo &TII = *MF->getSubtarget().getInstrInfo();

  // Insert in reverse order.
  // loadRegFromStackSlot can insert multiple instructions.
  for (auto &CS : reverse(CSI)) {
    Register Reg = CS.getReg();
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.loadRegFromStackSlot(MBB, MI, Reg, CS.getFrameIdx(), RC, TRI);
    assert(MI != MBB.begin() && "loadRegFromStackSlot didn't insert any code!");
  }

  return true;
}

// TODO: seems ok
void simFrameLowering::processFunctionBeforeFrameFinalized(
    MachineFunction &MF, RegScavenger *RS) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  auto *UFI = MF.getInfo<simFunctionInfo>();

  if (!isInt<16>(MFI.estimateStackSize(MF))) {
    llvm_unreachable(""); // TODO: scavenging?
  }

  if (MFI.getCalleeSavedInfo().empty()) {
    UFI->setCalleeSavedStackSize(0);
    return;
  }

  unsigned Size = 0;
  for (const auto &Info : MFI.getCalleeSavedInfo()) {
    int FrameIdx = Info.getFrameIdx();
    if (MFI.getStackID(FrameIdx) != TargetStackID::Default)
      continue;

    Size += MFI.getObjectSize(FrameIdx);
  }
  UFI->setCalleeSavedStackSize(Size);
}

// Eliminate ADJCALLSTACKDOWN, ADJCALLSTACKUP pseudo instructions.
// TODO: seems ok
MachineBasicBlock::iterator simFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  Register SPReg = getSPReg(STI);
  DebugLoc DL = MI->getDebugLoc();

  if (!hasReservedCallFrame(MF)) {
    // If space has not been reserved for a call frame, ADJCALLSTACKDOWN and
    // ADJCALLSTACKUP must be converted to instructions manipulating the stack
    // pointer. This is necessary when there is a variable length stack
    // allocation (e.g. alloca), which means it's not possible to allocate
    // space for outgoing arguments from within the function prologue.
    int64_t Amount = MI->getOperand(0).getImm();

    if (Amount != 0) {
      // Ensure the stack remains aligned after adjustment.
      Amount = alignSPAdjust(Amount);

      if (MI->getOpcode() == sim::ADJCALLSTACKDOWN)
        Amount = -Amount;

      adjustReg(MBB, MI, DL, SPReg, SPReg, Amount, MachineInstr::NoFlags);
    }
  }

  return MBB.erase(MI);
}

bool simFrameLowering::hasFP(const MachineFunction &MF) const {
  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();

  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(
             MF) || // -fomit-frame-pointer
         RegInfo->hasStackRealignment(MF) ||
         MFI.hasVarSizedObjects() || MFI.isFrameAddressTaken();
}

bool simFrameLowering::hasBP(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo *TRI = STI.getRegisterInfo();

  return MFI.hasVarSizedObjects() && TRI->hasStackRealignment(MF);
}

// TODO: rewrite!
StackOffset simFrameLowering::getFrameIndexReference(const MachineFunction &MF,
                                                     int FI,
                                                     Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo *RI = MF.getSubtarget().getRegisterInfo();
  const auto *UFI = MF.getInfo<simFunctionInfo>();

  // Callee-saved registers should be referenced relative to the stack
  // pointer (positive offset), otherwise use the frame pointer (negative
  // offset).
  const auto &CSI = MFI.getCalleeSavedInfo();
  int MinCSFI = 0;
  int MaxCSFI = -1;
  int Offset;

  Offset = MFI.getObjectOffset(FI) - getOffsetOfLocalArea() +
           MFI.getOffsetAdjustment();

  if (CSI.size()) {
    MinCSFI = CSI[0].getFrameIdx();
    MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
  }

  if (FI >= MinCSFI && FI <= MaxCSFI) {
    FrameReg = getSPReg(STI);
    Offset += MFI.getStackSize();
  } else if (RI->hasStackRealignment(MF) && !MFI.isFixedObjectIndex(FI)) {
    // TODO: realigned stack
    llvm_unreachable("");
  } else {
    // TODO: what's going on here
    FrameReg = RI->getFrameRegister(MF);
    if (hasFP(MF)) {
      Offset += UFI->getVarArgsSaveSize();
    } else {
      Offset += MFI.getStackSize();
    }
  }

  return StackOffset::getFixed(Offset);
}

// Not preserve stack space within prologue for outgoing variables when the
// function contains variable size objects or there are vector objects accessed
// by the frame pointer.
// Let eliminateCallFramePseudoInstr preserve stack space for it.
bool simFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  return !MF.getFrameInfo().hasVarSizedObjects();
}
