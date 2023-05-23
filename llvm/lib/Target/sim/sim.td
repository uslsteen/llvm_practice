//===----------------------------------------------------------------------===//
// This is the top level entry point for the sim target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// sim Subtarget features
//===----------------------------------------------------------------------===//

//def FeatureDummy
//    : SubtargetFeature<"dummy", "Dummy", "true",
//                       "Dummy feature">;

defvar sim32 = DefaultMode;

//===----------------------------------------------------------------------===//
// Target-dependent interfaces
//===----------------------------------------------------------------------===//

include "simRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "simSchedule.td"
include "simCallingConv.td"

include "simInstrInfo.td"
def simInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// sim processors supported.
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic-32", NoSchedModel, []>;

def simAsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = 1;
  let AllowDuplicateRegisterNames = 1;
}


// Will generate simGenAsmWrite.inc included by simInstPrinter.cpp, contents
//  as follows,
// void simInstPrinter::printInstruction(const MCInst *MI, raw_ostream &O) {...}
// const char *simInstPrinter::getRegisterName(unsigned RegNo) {...}
def sim : Target {
  let InstructionSet = simInstrInfo;
}
