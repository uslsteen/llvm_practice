#ifndef LLVM_LIB_TARGET_Xor_Xor_H
#define LLVM_LIB_TARGET_Xor_Xor_H

#include "MCTargetDesc/XorMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class XorTargetMachine;
class FunctionPass;
class XorSubtarget;
class AsmPrinter;
class InstructionSelector;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineOperand;
class PassRegistry;

bool lowerXorMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                    AsmPrinter &AP);
bool LowerXorMachineOperandToMCOperand(const MachineOperand &MO,
                                         MCOperand &MCOp, const AsmPrinter &AP);

FunctionPass *createXorISelDag(XorTargetMachine &TM,
                                CodeGenOpt::Level OptLevel);

// namespace Xor {
// enum {
//   GP = Xor::R0,
//   RA = Xor::R1,
//   SP = Xor::R2,
//   FP = Xor::R3,
//   BP = Xor::R4,
// };
// } // namespace Xor

} // namespace llvm

#endif
