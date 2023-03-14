//===----------------------------------------------------------------------===//
//  Describe Xor instructions format
//
//===----------------------------------------------------------------------===//

class InstFormat<bits<5> val> {
  bits<5> Value = val;
}
def InstFormatR      : InstFormat<1>;
def InstFormatI      : InstFormat<3>;
def InstFormatS      : InstFormat<4>;
def InstFormatB      : InstFormat<5>;
def InstFormatU      : InstFormat<6>;
def InstFormatJ      : InstFormat<7>;

class XorInst<dag outs, dag ins, string asmstr, list<dag> pattern, InstFormat format>
    : Instruction {
  let Namespace = "Xor";
  let DecoderNamespace = "Xor";
  let Size = 4;

  field bits<32> Inst;
  field bits<7> Opcode = 0;

  let Inst{6 - 0} = Opcode;

  let OutOperandList = outs;
  let InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
}

class XorOpcode<string name, bits<7> val> {
  string Name = name;
  bits<7> Value = val;
}

def OPC_LOAD : XorOpcode<"LOAD", 0b0000011>;
def OPC_LOAD_FP : XorOpcode<"LOAD_FP", 0b0000111>;
def OPC_MISC_MEM : XorOpcode<"MISC_MEM", 0b0001111>;
def OPC_OP_IMM : XorOpcode<"OP_IMM", 0b0010011>;
def OPC_AUIPC : XorOpcode<"AUIPC", 0b0010111>;
def OPC_OP_IMM_32 : XorOpcode<"OP_IMM_32", 0b0011011>;
def OPC_STORE : XorOpcode<"STORE", 0b0100011>;
def OPC_STORE_FP : XorOpcode<"STORE_FP", 0b0100111>;
def OPC_AMO : XorOpcode<"AMO", 0b0101111>;
def OPC_OP : XorOpcode<"OP", 0b0110011>;
def OPC_LUI : XorOpcode<"LUI", 0b0110111>;
def OPC_OP_32 : XorOpcode<"OP_32", 0b0111011>;
def OPC_MADD : XorOpcode<"MADD", 0b1000011>;
def OPC_MSUB : XorOpcode<"MSUB", 0b1000111>;
def OPC_NMSUB : XorOpcode<"NMSUB", 0b1001011>;
def OPC_NMADD : XorOpcode<"NMADD", 0b1001111>;
def OPC_BRANCH : XorOpcode<"BRANCH", 0b1100011>;
def OPC_JALR : XorOpcode<"JALR", 0b1100111>;
def OPC_JAL : XorOpcode<"JAL", 0b1101111>;
def OPC_SYSTEM : XorOpcode<"SYSTEM", 0b1110011>;

// Base Instruction Formats described in RISC-V Unprivileged ISA

class XorInstR<bits<7> funct7, bits<3> funct3, XorOpcode op, dag outs,
                dag ins, string asmstr, list<dag> pattern>
    : XorInst<outs, ins, asmstr, pattern, InstFormatR> {
  bits<5> rs2;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31 - 25} = funct7;
  let Inst{24 - 20} = rs2;
  let Inst{19 - 15} = rs1;
  let Inst{14 - 12} = funct3;
  let Inst{11 - 7} = rd;
  let Opcode = op.Value;
}

class XorInstI<bits<3> funct3, XorOpcode op, dag outs, dag ins, string opstr,
                string asmstr, list<dag> pattern>
    : XorInst<outs, ins, asmstr, pattern, InstFormatI> {
  bits<12> imm12;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31 - 20} = imm12;
  let Inst{19 - 15} = rs1;
  let Inst{14 - 12} = funct3;
  let Inst{11 - 7} = rd;
  let Opcode = op.Value;
}

class XorInstS<bits<3> funct3, XorOpcode op, dag outs, dag ins, string opstr,
                string asmstr, list<dag> pattern>
    : XorInst<outs, ins, asmstr, pattern, InstFormatS> {
  bits<12> imm12;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31 - 25} = imm12{11 - 5};
  let Inst{24 - 20} = rs2;
  let Inst{19 - 15} = rs1;
  let Inst{14 - 12} = funct3;
  let Inst{11 - 7} = imm12{4 - 0};
  let Opcode = op.Value;
}

class XorInstB<bits<3> funct3, XorOpcode opcode, dag outs, dag ins,
                string opcodestr, string asmstr, list<dag> pattern>
    : XorInst<outs, ins, asmstr, pattern, InstFormatB> {
  bits<12> imm12;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31} = imm12{11};
  let Inst{30 - 25} = imm12{9 - 4};
  let Inst{24 - 20} = rs2;
  let Inst{19 - 15} = rs1;
  let Inst{14 - 12} = funct3;
  let Inst{11 - 8} = imm12{3 - 0};
  let Inst{7} = imm12{10};
  let Opcode = opcode.Value;
}

class XorInstU<XorOpcode opcode, dag outs, dag ins, string opcodestr,
                string asmstr, list<dag> pattern>
    : XorInst<outs, ins, asmstr, pattern, InstFormatU> {
  bits<20> imm20;
  bits<5> rd;

  let Inst{31 - 12} = imm20;
  let Inst{11 - 7} = rd;
  let Opcode = opcode.Value;
}

class XorInstJ<XorOpcode opcode, dag outs, dag ins, string opcodestr,
                string asmstr, list<dag> pattern>
    : XorInst<outs, ins, asmstr, pattern, InstFormatJ> {
  bits<5> rd;
  bits<20> imm20;

  let Inst{31} = imm20{19};
  let Inst{30 - 21} = imm20{9 - 0};
  let Inst{20} = imm20{10};
  let Inst{19 - 12} = imm20{18 - 11};
  let Inst{11 - 7} = rd;
  let Opcode = opcode.Value;
}

class XorInstIShift<bits<5> imm11_7, bits<3> funct3, XorOpcode opcode,
                     dag outs, dag ins, string opcodestr, string asmstr>
    : XorInst<outs, ins, asmstr, [], InstFormatI> {
  bits<6> shamt;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31 - 27} = imm11_7;
  let Inst{26} = 0;
  let Inst{25 - 20} = shamt;
  let Inst{19 - 15} = rs1;
  let Inst{14 - 12} = funct3;
  let Inst{11 - 7} = rd;
  let Opcode = opcode.Value;
}

class XorInstIShiftW<bits<7> imm11_5, bits<3> funct3, XorOpcode opcode,
                      dag outs, dag ins, string opcodestr, string asmstr>
    : XorInst<outs, ins, asmstr, [], InstFormatI> {
  bits<5> shamt;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31 - 25} = imm11_5;
  let Inst{24 - 20} = shamt;
  let Inst{19 - 15} = rs1;
  let Inst{14 - 12} = funct3;
  let Inst{11 - 7} = rd;
  let Opcode = opcode.Value;
}