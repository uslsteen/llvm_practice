#include "simRegisterInfo.h"
#include "sim.h"
#include "simInstrInfo.h"
//#include "simMachineFunctionInfo.h"
#include "simSubtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

static_assert(sim::X1 == sim::X0 + 1, "Register list not consecutive");
static_assert(sim::X31 == sim::X0 + 31, "Register list not consecutive");

#define DEBUG_TYPE "sim-reg-info"

#define GET_REGINFO_TARGET_DESC
#include "simGenRegisterInfo.inc"

simRegisterInfo::simRegisterInfo() : simGenRegisterInfo(sim::X1) {}

#if 0
bool simRegisterInfo::needsFrameMoves(const MachineFunction &MF) {
  return MF.needsFrameMoves();
}
#endif

const MCPhysReg *
simRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  if (MF->getFunction().getCallingConv() == CallingConv::GHC)
    return CSR_NoRegs_SaveList;
  if (MF->getFunction().hasFnAttribute("interrupt"))
    return CSR_Interrupt_SaveList;

  return CSR_ILP32_LP64_SaveList;
}

// TODO: check cconv
BitVector simRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  // Use markSuperRegs to ensure any register aliases are also reserved
  markSuperRegs(Reserved, sim::X0); // zero
  markSuperRegs(Reserved, sim::X2); // sp
  markSuperRegs(Reserved, sim::X3); // gp
  markSuperRegs(Reserved, sim::X4); // tp
  return Reserved;
}

bool simRegisterInfo::requiresRegisterScavenging(
    const MachineFunction &MF) const {
  return false; // TODO: what for?
}

#if 0
bool simRegisterInfo::useFPForScavengingIndex(
    const MachineFunction &MF) const {
  llvm_unreachable("");
}
#endif

// TODO: rewrite!
void simRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");

  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  DebugLoc DL = MI.getDebugLoc();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  Register FrameReg;
  int Offset = getFrameLowering(MF)
                   ->getFrameIndexReference(MF, FrameIndex, FrameReg)
                   .getFixed();
  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  if (!isInt<16>(Offset)) {
    llvm_unreachable("");
  }

  MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, false, false, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
}

Register simRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = getFrameLowering(MF);
  return TFI->hasFP(MF) ? sim::X8 : sim::X2;
}

const uint32_t *
simRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                      CallingConv::ID CC) const {
  auto &Subtarget = MF.getSubtarget<simSubtarget>();

  if (CC == CallingConv::GHC)
    return CSR_NoRegs_RegMask;
  switch (Subtarget.getTargetABI()) {
  default:
    llvm_unreachable("Unrecognized ABI");
  case simABI::ABI_ILP32:
    return CSR_ILP32_LP64_RegMask;
  }
}
