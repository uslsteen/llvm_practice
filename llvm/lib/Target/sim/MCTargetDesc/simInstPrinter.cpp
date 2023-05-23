#include "simInstPrinter.h"
#include "MCTargetDesc/simInfo.h"
#include "MCTargetDesc/simMCExpr.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define GET_REGINFO_ENUM
#include "simGenRegisterInfo.inc"

#include "simGenInstrInfo.inc"
#include "simGenSubtargetInfo.inc"

#include "simGenAsmWriter.inc"

void simInstPrinter::printRegName(raw_ostream &O, unsigned RegNo) const {
  O << getRegisterName(RegNo);
}

void simInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                               StringRef Annot, const MCSubtargetInfo &STI,
                               raw_ostream &O) {
  printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void simInstPrinter::printOperand(const MCInst *MI, int OpNo, raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    O << MO.getImm();
    return;
  }

  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MO.getExpr()->print(O, &MAI);
}

void simInstPrinter::printBranchOperand(const MCInst *MI, uint64_t Address,
                                        unsigned OpNo, raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);
  if (!MO.isImm())
    return printOperand(MI, OpNo, O);

  if (PrintBranchImmAsAddress) {
    uint32_t Target = Address + MO.getImm();
    O << formatHex(static_cast<uint64_t>(Target));
  } else {
    O << MO.getImm();
  }
}

// Print architectural register names rather than the ABI names (such as x2
// instead of sp).
// TODO: Make RISCVInstPrinter::getRegisterName non-static so that this can a
// member.
static bool ArchRegNames;

const char *simInstPrinter::getRegisterName(unsigned RegNo) {
  return getRegisterName(RegNo,
                         ArchRegNames ? sim::NoRegAltName : sim::ABIRegAltName);
}
