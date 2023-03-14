include "XorInstrFormats.td"

//===----------------------------------------------------------------------===//
//
// This file describes the Xor instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//

def simm12 : Operand<i32> {}

def uimmlog2 : Operand<i32> {}

let hasSideEffects = 0, mayLoad = 0,
    mayStore =
        0 in class ALU_rr<bits<7> funct7, bits<3> funct3, string opcodestr>
    : XorInstR<funct7, funct3, OPC_OP,
                (outs HartRegs
                 : $rd),
                (ins HartRegs
                 : $rs1, HartRegs
                 : $rs2), "$rd, $rs1, $rs2", []>;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in 
class ALU_ri<bits<3> funct3, string opcodestr>
    : XorInstI<funct3, OPC_OP_IMM,
                (outs HartRegs
                 : $rd),
                (ins HartRegs
                 : $rs1, simm12 : $imm12),
                opcodestr, "$rd, $rs1, $imm12", []>;

let hasSideEffects = 0, mayLoad = 0,
    mayStore =
        0 in class Shift_ri<bits<5> imm11_7, bits<3> funct3, string opcodestr>
    : XorInstIShift<imm11_7, funct3, OPC_OP_IMM,
                     (outs HartRegs
                      : $rd),
                     (ins HartRegs
                      : $rs1, uimmlog2 : $shamt),
                     opcodestr, "$rd, $rs1, $shamt">;

def ADDI : ALU_ri<0b000, "addi">;
def SLTI : ALU_ri<0b010, "sltu">;
def SLTIU : ALU_ri<0b011, "sltiu">;
def ANDI : ALU_ri<0b111, "andi">;
def ORI : ALU_ri<0b110, "ori">;
def XORI : ALU_ri<0b100, "xori">;

def ADD : ALU_rr<0b0000000, 0b000, "add">;
def SUB : ALU_rr<0b0100000, 0b000, "sub">;
def SLL : ALU_rr<0b0000000, 0b001, "sll">;
def SLT : ALU_rr<0b0000000, 0b010, "slt">;
def SLTU : ALU_rr<0b0000000, 0b011, "sltu">;
def XOR : ALU_rr<0b0000000, 0b100, "xor">;
def SRL : ALU_rr<0b0000000, 0b101, "srl">;
def SRA : ALU_rr<0b0100000, 0b101, "sra">;
def OR : ALU_rr<0b0000000, 0b110, "or">;
def AND : ALU_rr<0b0000000, 0b111, "and">;

def SLLI : Shift_ri<0b00000, 0b001, "slli">;
def SRLI : Shift_ri<0b00000, 0b101, "srli">;
def SRAI : Shift_ri<0b01000, 0b101, "srai">;