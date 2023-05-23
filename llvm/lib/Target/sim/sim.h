#ifndef __LLVM_LIB_TARGET_SIM_SIM_H__
#define __LLVM_LIB_TARGET_SIM_SIM_H__

#include "MCTargetDesc/simMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class simTargetMachine;
class FunctionPass;
class simSubtarget;
class AsmPrinter;
class InstructionSelector;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineOperand;
class PassRegistry;

bool lowersimMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                    AsmPrinter &AP);
bool LowersimMachineOperandToMCOperand(const MachineOperand &MO,
                                         MCOperand &MCOp, const AsmPrinter &AP);

FunctionPass *createsimISelDag(simTargetMachine &TM,
                                CodeGenOpt::Level OptLevel);


// namespace sim {
// enum {
// };
// } // namespace sim

} // namespace llvm

#endif // __LLVM_LIB_TARGET_SIM_SIM_H__
