include "llvm/Target/Target.td"
include "XorRegisterInfo.td"
include "XorInstrInfo.td"

def XorInstrInfo : InstrInfo;
def Xor : Target {
    let InstructionSet = XorInstrInfo;
}