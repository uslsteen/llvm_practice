//===----------------------------------------------------------------------===//
// This describes the calling conventions for sim architecture.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// sim Return Value Calling Convention
//===----------------------------------------------------------------------===//


def CSR_ILP32_LP64
    : CalleeSavedRegs<(add X1, X3, X4, X8, X9, (sequence "X%u", 18, 27))>;

// Needed for implementation of RISCVRegisterInfo::getNoPreservedMask()
def CSR_NoRegs : CalleeSavedRegs<(add)>;

// Interrupt handler needs to save/restore all registers that are used,
// both Caller and Callee saved registers.
def CSR_Interrupt : CalleeSavedRegs<(add X1,
    (sequence "X%u", 3, 9),
    (sequence "X%u", 10, 11),
    (sequence "X%u", 12, 17),
    (sequence "X%u", 18, 27),
    (sequence "X%u", 28, 31))>;
