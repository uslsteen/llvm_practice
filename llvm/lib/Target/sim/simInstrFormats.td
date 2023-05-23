//===----------------------------------------------------------------------===//
//  Describe sim instructions format
//
//  CPU INSTRUCTION FORMATS
//
//  FF FF FF FF
//        ^^ ^^ - r3_imm - third reg or imm
//      ^       - r2     - second reg
//     ^        - r1     - first reg
//  ^^          - opcode - operation code
//
//===----------------------------------------------------------------------===//

// Generic sim Format
class InstFormat<bits<5> val> {
  bits<5> Value = val;
}

def InstFormatPseudo      : InstFormat<0>;
def InstFormatR           : InstFormat<1>;
def InstFormatI           : InstFormat<2>;
def InstFormatS           : InstFormat<3>;
def InstFormatB           : InstFormat<4>;
def InstFormatU           : InstFormat<5>;
def InstFormatJ           : InstFormat<6>;

// The following opcode names match those given in Table 19.1 in the
// RISC-V User-level ISA specification ("RISC-V base opcode map").
class simOpcode<string name, bits<7> val> {
  string Name = name;
  bits<7> Value = val;
}

def OPC_LOAD      : simOpcode<"LOAD",      0b0000011>;
def OPC_LOAD_FP   : simOpcode<"LOAD_FP",   0b0000111>;
def OPC_MISC_MEM  : simOpcode<"MISC_MEM",  0b0001111>;
def OPC_OP_IMM    : simOpcode<"OP_IMM",    0b0010011>;
def OPC_AUIPC     : simOpcode<"AUIPC",     0b0010111>;
def OPC_OP_IMM_32 : simOpcode<"OP_IMM_32", 0b0011011>;
def OPC_STORE     : simOpcode<"STORE",     0b0100011>;
def OPC_STORE_FP  : simOpcode<"STORE_FP",  0b0100111>;
def OPC_AMO       : simOpcode<"AMO",       0b0101111>;
def OPC_OP        : simOpcode<"OP",        0b0110011>;
def OPC_LUI       : simOpcode<"LUI",       0b0110111>;
def OPC_OP_32     : simOpcode<"OP_32",     0b0111011>;
def OPC_MADD      : simOpcode<"MADD",      0b1000011>;
def OPC_MSUB      : simOpcode<"MSUB",      0b1000111>;
def OPC_NMSUB     : simOpcode<"NMSUB",     0b1001011>;
def OPC_NMADD     : simOpcode<"NMADD",     0b1001111>;
def OPC_OP_FP     : simOpcode<"OP_FP",     0b1010011>;
def OPC_OP_V      : simOpcode<"OP_V",      0b1010111>;
def OPC_BRANCH    : simOpcode<"BRANCH",    0b1100011>;
def OPC_JALR      : simOpcode<"JALR",      0b1100111>;
def OPC_JAL       : simOpcode<"JAL",       0b1101111>;
def OPC_SYSTEM    : simOpcode<"SYSTEM",    0b1110011>;

class simInst<dag outs, dag ins, string opcodestr, string argstr,
             list<dag> pattern, InstFormat format>
    : Instruction {
  let Namespace = "sim";
  let DecoderNamespace = "sim";
  let Size = 4;

  field bits<32> Inst;

  bits<7> Opcode = 0;

  let Inst{6-0} = Opcode;

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = opcodestr # "\t" # argstr;
  let Pattern = pattern;

  let TSFlags{4-0} = format.Value;
}

// Pseudo instructions
class Pseudo<dag outs, dag ins, list<dag> pattern, string opcodestr = "", string argstr = "">
    : simInst<outs, ins, opcodestr, argstr, pattern, InstFormatPseudo> {
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}

// Pseudo load instructions.
class PseudoLoad<string opcodestr, RegisterClass rdty = GPR>
    : Pseudo<(outs rdty:$rd), (ins bare_symbol:$addr), [], opcodestr, "$rd, $addr"> {
  let hasSideEffects = 0;
  let mayLoad = 1;
  let mayStore = 0;
  let isCodeGenOnly = 0;
  let isAsmParserOnly = 1;
}

// Pseudo store instructions.
class PseudoStore<string opcodestr, RegisterClass rsty = GPR>
    : Pseudo<(outs GPR:$tmp), (ins rsty:$rs, bare_symbol:$addr), [], opcodestr, "$rs, $addr, $tmp"> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 1;
  let isCodeGenOnly = 0;
  let isAsmParserOnly = 1;
}

class simInstR<bits<7> funct7, bits<3> funct3, simOpcode opcode, dag outs,
              dag ins, string opcodestr, string argstr>
    : simInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<5> rs2;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-25} = funct7;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class simInstI<bits<3> funct3, simOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : simInst<outs, ins, opcodestr, argstr, [], InstFormatI> {
  bits<12> imm12;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-20} = imm12;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class simInstS<bits<3> funct3, simOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : simInst<outs, ins, opcodestr, argstr, [], InstFormatS> {
  bits<12> imm12;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31-25} = imm12{11-5};
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = imm12{4-0};
  let Opcode = opcode.Value;
}

class simInstB<bits<3> funct3, simOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : simInst<outs, ins, opcodestr, argstr, [], InstFormatB> {
  bits<12> imm12;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31} = imm12{11};
  let Inst{30-25} = imm12{9-4};
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-8} = imm12{3-0};
  let Inst{7} = imm12{10};
  let Opcode = opcode.Value;
}

class simInstU<simOpcode opcode, dag outs, dag ins, string opcodestr,
              string argstr>
    : simInst<outs, ins, opcodestr, argstr, [], InstFormatU> {
  bits<20> imm20;
  bits<5> rd;

  let Inst{31-12} = imm20;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class simInstJ<simOpcode opcode, dag outs, dag ins, string opcodestr,
              string argstr>
    : simInst<outs, ins, opcodestr, argstr, [], InstFormatJ> {
  bits<20> imm20;
  bits<5> rd;

  let Inst{31} = imm20{19};
  let Inst{30-21} = imm20{9-0};
  let Inst{20} = imm20{10};
  let Inst{19-12} = imm20{18-11};
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class simInstIShift<bits<5> imm11_7, bits<3> funct3, simOpcode opcode,
                   dag outs, dag ins, string opcodestr, string argstr>
    : simInst<outs, ins, opcodestr, argstr, [], InstFormatI> {
  bits<6> shamt;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-27} = imm11_7;
  let Inst{26} = 0;
  let Inst{25-20} = shamt;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class simInstIShiftW<bits<7> imm11_5, bits<3> funct3, simOpcode opcode,
                    dag outs, dag ins, string opcodestr, string argstr>
    : simInst<outs, ins, opcodestr, argstr, [], InstFormatI> {
  bits<5> shamt;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-25} = imm11_5;
  let Inst{24-20} = shamt;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}