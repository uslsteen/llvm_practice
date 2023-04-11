//===----------------------------------------------------------------------===//
// This is the top level entry point for the Xor target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Xor Subtarget features
//===----------------------------------------------------------------------===//

//def FeatureDummy
//    : SubtargetFeature<"dummy", "Dummy", "true",
//                       "Dummy feature">;

defvar Xor32 = DefaultMode;

//===----------------------------------------------------------------------===//
// Target-dependent interfaces
//===----------------------------------------------------------------------===//

include "XorRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "XorSchedule.td"
include "XorCallingConv.td"

include "XorInstrInfo.td"
def XorInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Xor processors supported.
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic-32", NoSchedModel, []>;

def XorAsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = 1;
  let AllowDuplicateRegisterNames = 1;
}


// Will generate XorGenAsmWrite.inc included by XorInstPrinter.cpp, contents
//  as follows,
// void XorInstPrinter::printInstruction(const MCInst *MI, raw_ostream &O) {...}
// const char *XorInstPrinter::getRegisterName(unsigned RegNo) {...}
def Xor : Target {
  let InstructionSet = XorInstrInfo;
}