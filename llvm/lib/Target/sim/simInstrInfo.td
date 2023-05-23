//===----------------------------------------------------------------------===//
//
// This file contains the sim implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// RISC-V specific DAG Nodes.
//===----------------------------------------------------------------------===//

// Target-independent type requirements, but with target-specific formats.
def SDT_CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>,
                                       SDTCisVT<1, i32>]>;
def SDT_CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>,
                                     SDTCisVT<1, i32>]>;

// Target-dependent type requirements.
def SDT_simCall     : SDTypeProfile<0, -1, [SDTCisVT<0, XLenVT>]>;
def SDT_simSelectCC : SDTypeProfile<1, 5, [SDTCisSameAs<1, 2>,
                                             SDTCisVT<3, OtherVT>,
                                             SDTCisSameAs<0, 4>,
                                             SDTCisSameAs<4, 5>]>;
def SDT_simBrCC : SDTypeProfile<0, 4, [SDTCisSameAs<0, 1>,
                                         SDTCisVT<2, OtherVT>,
                                         SDTCisVT<3, OtherVT>]>;
def SDT_simReadCSR  : SDTypeProfile<1, 1, [SDTCisInt<0>, SDTCisInt<1>]>;
def SDT_simWriteCSR : SDTypeProfile<0, 2, [SDTCisInt<0>, SDTCisInt<1>]>;
def SDT_simSwapCSR  : SDTypeProfile<1, 2, [SDTCisInt<0>, SDTCisInt<1>,
                                             SDTCisInt<2>]>;
def SDT_simReadCycleWide : SDTypeProfile<2, 0, [SDTCisVT<0, i32>,
                                                  SDTCisVT<1, i32>]>;
def SDT_simIntUnaryOpW : SDTypeProfile<1, 1, [
  SDTCisSameAs<0, 1>, SDTCisVT<0, i64>
]>;
def SDT_simIntBinOpW : SDTypeProfile<1, 2, [
  SDTCisSameAs<0, 1>, SDTCisSameAs<0, 2>, SDTCisVT<0, i64>
]>;
def SDT_simIntShiftDOpW : SDTypeProfile<1, 3, [
  SDTCisSameAs<0, 1>, SDTCisSameAs<0, 2>, SDTCisVT<0, i64>, SDTCisVT<3, i64>
]>;

// Target-independent nodes, but with target-specific formats.
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_CallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_CallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

// Target-dependent nodes.
def sim_call      : SDNode<"simISD::CALL", SDT_simCall,
                             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                              SDNPVariadic]>;
def sim_ret_flag  : SDNode<"simISD::RET_FLAG", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def sim_uret_flag : SDNode<"simISD::URET_FLAG", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue]>;
def sim_sret_flag : SDNode<"simISD::SRET_FLAG", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue]>;
def sim_mret_flag : SDNode<"simISD::MRET_FLAG", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue]>;
def sim_selectcc  : SDNode<"simISD::SELECT_CC", SDT_simSelectCC>;
def sim_brcc      : SDNode<"simISD::BR_CC", SDT_simBrCC,
                             [SDNPHasChain]>;
def sim_tail      : SDNode<"simISD::TAIL", SDT_simCall,
                             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                              SDNPVariadic]>;
def sim_sllw      : SDNode<"simISD::SLLW", SDT_simIntBinOpW>;
def sim_sraw      : SDNode<"simISD::SRAW", SDT_simIntBinOpW>;
def sim_srlw      : SDNode<"simISD::SRLW", SDT_simIntBinOpW>;
def sim_read_csr  : SDNode<"simISD::READ_CSR", SDT_simReadCSR,
                             [SDNPHasChain]>;
def sim_write_csr : SDNode<"simISD::WRITE_CSR", SDT_simWriteCSR,
                             [SDNPHasChain]>;
def sim_swap_csr  : SDNode<"simISD::SWAP_CSR", SDT_simSwapCSR,
                             [SDNPHasChain]>;

def sim_read_cycle_wide : SDNode<"simISD::READ_CYCLE_WIDE",
                                   SDT_simReadCycleWide,
                                   [SDNPHasChain, SDNPSideEffect]>;

def sim_add_lo : SDNode<"simISD::ADD_LO", SDTIntBinOp>;
def sim_hi : SDNode<"simISD::HI", SDTIntUnaryOp>;
def sim_lla : SDNode<"simISD::LLA", SDTIntUnaryOp>;
def sim_add_tprel : SDNode<"simISD::ADD_TPREL",
                             SDTypeProfile<1, 3, [SDTCisSameAs<0, 1>,
                                                  SDTCisSameAs<0, 2>,
                                                  SDTCisSameAs<0, 3>,
                                                  SDTCisInt<0>]>>;

def sim_la : SDNode<"simISD::LA", SDTLoad,
                      [SDNPHasChain, SDNPMayLoad, SDNPMemOperand]>;
def sim_la_tls_ie : SDNode<"simISD::LA_TLS_IE", SDTLoad,
                             [SDNPHasChain, SDNPMayLoad, SDNPMemOperand]>;
def sim_la_tls_gd : SDNode<"simISD::LA_TLS_GD", SDTIntUnaryOp>;

//===----------------------------------------------------------------------===//
// sim Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//

class ImmXLenAsmOperand<string prefix, string suffix = ""> : AsmOperandClass {
  let Name = prefix # "ImmXLen" # suffix;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid", Name);
}

class ImmAsmOperand<string prefix, int width, string suffix> : AsmOperandClass {
  let Name = prefix # "Imm" # width # suffix;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid", Name);
}

// A parse method for (${gpr}) or 0(${gpr}), where the 0 is be silently ignored.
def ZeroOffsetMemOpOperand : AsmOperandClass {
  let Name = "ZeroOffsetMemOpOperand";
  let RenderMethod = "addRegOperands";
  let PredicateMethod = "isGPR";
  let ParserMethod = "parseZeroOffsetMemOp";
}

// Check if (add r, imm) can be optimized to (ADDI (ADDI r, imm0), imm1),
// in which imm = imm0 + imm1 and both imm0 and imm1 are simm12.
def AddiPair : PatLeaf<(imm), [{
  if (!N->hasOneUse())
    return false;
  // The immediate operand must be in range [-4096,-2049] or [2048,4094].
  int64_t Imm = N->getSExtValue();
  return (-4096 <= Imm && Imm <= -2049) || (2048 <= Imm && Imm <= 4094);
}]>;

// Return imm/2.
def AddiPairImmA : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getSExtValue() / 2, SDLoc(N),
                                   N->getValueType(0));
}]>;

// Return imm - imm/2.
def AddiPairImmB : SDNodeXForm<imm, [{
  int64_t Imm = N->getSExtValue();
  return CurDAG->getTargetConstant(Imm - Imm / 2, SDLoc(N),
                                   N->getValueType(0));
}]>;

def GPRMemZeroOffset : RegisterOperand<GPR> {
  let ParserMatchClass = ZeroOffsetMemOpOperand;
  let PrintMethod = "printZeroOffsetMemOp";
}

class SImmAsmOperand<int width, string suffix = "">
    : ImmAsmOperand<"S", width, suffix> {
}

class UImmAsmOperand<int width, string suffix = "">
    : ImmAsmOperand<"U", width, suffix> {
}

def UImmLog2XLenAsmOperand : AsmOperandClass {
  let Name = "UImmLog2XLen";
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidUImmLog2XLen";
}

def uimmlog2xlen : Operand<XLenVT>, ImmLeaf<XLenVT, [{
  return isUInt<5>(Imm);
}]> {
  let ParserMatchClass = UImmLog2XLenAsmOperand;
  // TODO: should ensure invalid shamt is rejected when decoding.
  let DecoderMethod = "decodeUImmOperand<6>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    if (STI.getTargetTriple().isArch64Bit())
      return isUInt<6>(Imm);
    return isUInt<5>(Imm);
  }];
  let OperandType = "OPERAND_UIMMLOG2XLEN";
  let OperandNamespace = "simOp";
}

def simm12 : Operand<XLenVT>, ImmLeaf<XLenVT, [{return isInt<12>(Imm);}]> {
  let ParserMatchClass = SImmAsmOperand<12>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<12>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<12>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_SIMM12";
  let OperandNamespace = "simOp";
}


// A 13-bit signed immediate where the least significant bit is zero.
def simm13_lsb0 : Operand<OtherVT> {
  let ParserMatchClass = SImmAsmOperand<13, "Lsb0">;
  let PrintMethod = "printBranchOperand";
  let EncoderMethod = "getImmOpValueAsr1";
  let DecoderMethod = "decodeSImmOperandAndLsl1<13>";
  let OperandType = "OPERAND_PCREL";
}

class UImm20Operand : Operand<XLenVT> {
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeUImmOperand<20>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isUInt<20>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_UIMM20";
  let OperandNamespace = "simOp";
}

def uimm20_lui : UImm20Operand {
  let ParserMatchClass = UImmAsmOperand<20, "LUI">;
}
def uimm20_auipc : UImm20Operand {
  let ParserMatchClass = UImmAsmOperand<20, "AUIPC">;
}

def Simm21Lsb0JALAsmOperand : SImmAsmOperand<21, "Lsb0JAL"> {
  let ParserMethod = "parseJALOffset";
}

def ixlenimm_li : Operand<XLenVT> {
  let ParserMatchClass = ImmXLenAsmOperand<"", "LI">;
}

// A 21-bit signed immediate where the least significant bit is zero.
def simm21_lsb0_jal : Operand<OtherVT> {
  let ParserMatchClass = Simm21Lsb0JALAsmOperand;
  let PrintMethod = "printBranchOperand";
  let EncoderMethod = "getImmOpValueAsr1";
  let DecoderMethod = "decodeSImmOperandAndLsl1<21>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<20, 1>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_PCREL";
}

def BareSymbol : AsmOperandClass {
  let Name = "BareSymbol";
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidBareSymbol";
  let ParserMethod = "parseBareSymbol";
}

// A bare symbol.
def bare_symbol : Operand<XLenVT> {
  let ParserMatchClass = BareSymbol;
}


def CallSymbol : AsmOperandClass {
  let Name = "CallSymbol";
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidCallSymbol";
  let ParserMethod = "parseCallSymbol";
}

// A bare symbol used in call/tail only.
def call_symbol : Operand<XLenVT> {
  let ParserMatchClass = CallSymbol;
}

//===----------------------------------------------------------------------===//
// Instruction Class Templates
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Inst formats
//===----------------------------------------------------------------------===//

include "simInstrFormats.td"


let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class BranchCC_rri<bits<3> funct3, string opcodestr>
    : simInstB<funct3, OPC_BRANCH, (outs),
              (ins GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm12),
              opcodestr, "$rs1, $rs2, $imm12">,
      Sched<[WriteJmp, ReadJmp, ReadJmp]> {
  let isBranch = 1;
  let isTerminator = 1;
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class ALU_ri<bits<3> funct3, string opcodestr>
    : simInstI<funct3, OPC_OP_IMM, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12),
              opcodestr, "$rd, $rs1, $imm12">,
      Sched<[WriteIALU, ReadIALU]>;


let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in {
class Load_ri<bits<3> funct3, string opcodestr>
    : simInstI<funct3, OPC_LOAD, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12),
              opcodestr, "$rd, ${imm12}(${rs1})">;

class HLoad_r<bits<7> funct7, bits<5> funct5, string opcodestr>
    : simInstR<funct7, 0b100, OPC_SYSTEM, (outs GPR:$rd),
              (ins GPRMemZeroOffset:$rs1), opcodestr, "$rd, $rs1"> {
  let rs2 = funct5;
}
}

// Operands for stores are in the order srcreg, base, offset rather than
// reflecting the order these fields are specified in the instruction
// encoding.
let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in {
class Store_rri<bits<3> funct3, string opcodestr>
    : simInstS<funct3, OPC_STORE, (outs),
              (ins GPR:$rs2, GPR:$rs1, simm12:$imm12),
              opcodestr, "$rs2, ${imm12}(${rs1})">;
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Shift_ri<bits<5> imm11_7, bits<3> funct3, string opcodestr>
    : simInstIShift<imm11_7, funct3, OPC_OP_IMM, (outs GPR:$rd),
                   (ins GPR:$rs1, uimmlog2xlen:$shamt), opcodestr,
                   "$rd, $rs1, $shamt">,
      Sched<[WriteShiftImm, ReadShiftImm]>;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class ALU_rr<bits<7> funct7, bits<3> funct3, string opcodestr>
    : simInstR<funct7, funct3, OPC_OP, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
              opcodestr, "$rd, $rs1, $rs2">;

//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
let isReMaterializable = 1, isAsCheapAsAMove = 1 in
def LUI : simInstU<OPC_LUI, (outs GPR:$rd), (ins uimm20_lui:$imm20),
                  "lui", "$rd, $imm20">, Sched<[WriteIALU]>;

def AUIPC : simInstU<OPC_AUIPC, (outs GPR:$rd), (ins uimm20_auipc:$imm20),
                    "auipc", "$rd, $imm20">, Sched<[WriteIALU]>;

let isCall = 1 in
def JAL : simInstJ<OPC_JAL, (outs GPR:$rd), (ins simm21_lsb0_jal:$imm20),
                  "jal", "$rd, $imm20">, Sched<[WriteJal]>;

let isCall = 1 in
def JALR : simInstI<0b000, OPC_JALR, (outs GPR:$rd),
                   (ins GPR:$rs1, simm12:$imm12),
                   "jalr", "$rd, ${imm12}(${rs1})">,
           Sched<[WriteJalr, ReadJalr]>;
} // hasSideEffects = 0, mayLoad = 0, mayStore = 0

def BEQ  : BranchCC_rri<0b000, "beq">;
def BNE  : BranchCC_rri<0b001, "bne">;
def BLT  : BranchCC_rri<0b100, "blt">;
def BGE  : BranchCC_rri<0b101, "bge">;
def BLTU : BranchCC_rri<0b110, "bltu">;
def BGEU : BranchCC_rri<0b111, "bgeu">;

def LB  : Load_ri<0b000, "lb">, Sched<[WriteLDB, ReadMemBase]>;
def LH  : Load_ri<0b001, "lh">, Sched<[WriteLDH, ReadMemBase]>;
def LW  : Load_ri<0b010, "lw">, Sched<[WriteLDW, ReadMemBase]>;
def LBU : Load_ri<0b100, "lbu">, Sched<[WriteLDB, ReadMemBase]>;
def LHU : Load_ri<0b101, "lhu">, Sched<[WriteLDH, ReadMemBase]>;

def SB : Store_rri<0b000, "sb">, Sched<[WriteSTB, ReadStoreData, ReadMemBase]>;
def SH : Store_rri<0b001, "sh">, Sched<[WriteSTH, ReadStoreData, ReadMemBase]>;
def SW : Store_rri<0b010, "sw">, Sched<[WriteSTW, ReadStoreData, ReadMemBase]>;


def SLLI : Shift_ri<0b00000, 0b001, "slli">;
def SRLI : Shift_ri<0b00000, 0b101, "srli">;
def SRAI : Shift_ri<0b01000, 0b101, "srai">;

def ADDI  : ALU_ri<0b000, "addi">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def ADDIW : simInstI<0b000, OPC_OP_IMM_32, (outs GPR:$rd),
                    (ins GPR:$rs1, simm12:$imm12),
                    "addiw", "$rd, $rs1, $imm12">,
            Sched<[WriteIALU32, ReadIALU32]>;

def ADD  : ALU_rr<0b0000000, 0b000, "add">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SUB  : ALU_rr<0b0100000, 0b000, "sub">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SLL  : ALU_rr<0b0000000, 0b001, "sll">, Sched<[WriteShiftReg, ReadShiftReg, ReadShiftReg]>;
def SLT  : ALU_rr<0b0000000, 0b010, "slt">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SLTU : ALU_rr<0b0000000, 0b011, "sltu">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def XOR  : ALU_rr<0b0000000, 0b100, "xor">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SRL  : ALU_rr<0b0000000, 0b101, "srl">, Sched<[WriteShiftReg, ReadShiftReg, ReadShiftReg]>;
def SRA  : ALU_rr<0b0100000, 0b101, "sra">, Sched<[WriteShiftReg, ReadShiftReg, ReadShiftReg]>;
def OR   : ALU_rr<0b0000000, 0b110, "or">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def AND  : ALU_rr<0b0000000, 0b111, "and">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;

def SLTI  : ALU_ri<0b010, "slti">;
def SLTIU : ALU_ri<0b011, "sltiu">;

let isReMaterializable = 1, isAsCheapAsAMove = 1 in {
def XORI  : ALU_ri<0b100, "xori">;
def ORI   : ALU_ri<0b110, "ori">;
}

def ANDI  : ALU_ri<0b111, "andi">;

//===----------------------------------------------------------------------===//
// Pseudo-instructions and codegen patterns
//
// Naming convention: For 'generic' pattern classes, we use the naming
// convention PatTy1Ty2. For pattern classes which offer a more complex
// expansion, prefix the class name, e.g. BccPat.
//===----------------------------------------------------------------------===//

/// Branches and jumps

// Match `sim_brcc` and lower to the appropriate RISC-V branch instruction.
class BccPat<CondCode Cond, simInstB Inst>
    : Pat<(sim_brcc GPR:$rs1, GPR:$rs2, Cond, bb:$imm12),
          (Inst GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm12)>;

def : BccPat<SETEQ, BEQ>;
def : BccPat<SETNE, BNE>;
def : BccPat<SETLT, BLT>;
def : BccPat<SETGE, BGE>;
def : BccPat<SETULT, BLTU>;
def : BccPat<SETUGE, BGEU>;

let isBarrier = 1, isBranch = 1, isTerminator = 1 in
def PseudoBR : Pseudo<(outs), (ins simm21_lsb0_jal:$imm20), [(br bb:$imm20)]>,
               PseudoInstExpansion<(JAL X0, simm21_lsb0_jal:$imm20)>;

// PseudoCALL is a pseudo instruction which will eventually expand to auipc
// and jalr while encoding. This is desirable, as an auipc+jalr pair with
// R_sim_CALL and R_sim_RELAX relocations can be be relaxed by the linker
// if the offset fits in a signed 21-bit immediate.
// Define AsmString to print "call" when compile with -S flag.
// Define isCodeGenOnly = 0 to support parsing assembly "call" instruction.
let isCall = 1, Defs = [X1], isCodeGenOnly = 0, Size = 8 in
def PseudoCALL : Pseudo<(outs), (ins call_symbol:$func), []> {
  let AsmString = "call\t$func";
}

def : Pat<(sim_call tglobaladdr:$func), (PseudoCALL tglobaladdr:$func)>;
def : Pat<(sim_call texternalsym:$func), (PseudoCALL texternalsym:$func)>;


let isCall = 1, Defs = [X1] in
def PseudoCALLIndirect : Pseudo<(outs), (ins GPRJALR:$rs1),
                                [(sim_call GPRJALR:$rs1)]>,
                         PseudoInstExpansion<(JALR X1, GPR:$rs1, 0)>;

let isBarrier = 1, isReturn = 1, isTerminator = 1 in
def PseudoRET : Pseudo<(outs), (ins), [(sim_ret_flag)]>,
                PseudoInstExpansion<(JALR X0, X1, 0)>;

let isBarrier = 1, isBranch = 1, isIndirectBranch = 1, isTerminator = 1 in
def PseudoBRIND : Pseudo<(outs), (ins GPRJALR:$rs1, simm12:$imm12), []>,
                  PseudoInstExpansion<(JALR X0, GPR:$rs1, simm12:$imm12)>;

def : InstAlias<"nop",           (ADDI      X0,      X0,       0)>;
def : InstAlias<"ret",                   (JALR      X0,      X1, 0), 4>;

// Note that the size is 32 because up to 8 32-bit instructions are needed to
// generate an arbitrary 64-bit immediate. However, the size does not really
// matter since PseudoLI is currently only used in the AsmParser where it gets
// expanded to real instructions immediately.
let hasSideEffects = 0, mayLoad = 0, mayStore = 0, Size = 32,
    isCodeGenOnly = 0, isAsmParserOnly = 1 in
def PseudoLI : Pseudo<(outs GPR:$rd), (ins ixlenimm_li:$imm), [],
                      "li", "$rd, $imm">;

def PseudoLB  : PseudoLoad<"lb">;
def PseudoLBU : PseudoLoad<"lbu">;
def PseudoLH  : PseudoLoad<"lh">;
def PseudoLHU : PseudoLoad<"lhu">;
def PseudoLW  : PseudoLoad<"lw">;

def PseudoSB  : PseudoStore<"sb">;
def PseudoSH  : PseudoStore<"sh">;
def PseudoSW  : PseudoStore<"sw">;

def : Pat<(brind GPRJALR:$rs1), (PseudoBRIND GPRJALR:$rs1, 0)>;
def : Pat<(brind (add GPRJALR:$rs1, simm12:$imm12)),
          (PseudoBRIND GPRJALR:$rs1, simm12:$imm12)>;

let EmitPriority = 0 in {
def : InstAlias<"lb $rd, (${rs1})",
                (LB  GPR:$rd, GPR:$rs1, 0)>;
def : InstAlias<"lh $rd, (${rs1})",
                (LH  GPR:$rd, GPR:$rs1, 0)>;
def : InstAlias<"lw $rd, (${rs1})",
                (LW  GPR:$rd, GPR:$rs1, 0)>;
def : InstAlias<"lbu $rd, (${rs1})",
                (LBU  GPR:$rd, GPR:$rs1, 0)>;
def : InstAlias<"lhu $rd, (${rs1})",
                (LHU  GPR:$rd, GPR:$rs1, 0)>;

def : InstAlias<"sb $rs2, (${rs1})",
                (SB  GPR:$rs2, GPR:$rs1, 0)>;
def : InstAlias<"sh $rs2, (${rs1})",
                (SH  GPR:$rs2, GPR:$rs1, 0)>;
def : InstAlias<"sw $rs2, (${rs1})",
                (SW  GPR:$rs2, GPR:$rs1, 0)>;
}
def IsOrAdd: PatFrag<(ops node:$A, node:$B), (or node:$A, node:$B), [{
  return isOrEquivalentToAdd(N);
}]>;

class PatGpr<SDPatternOperator OpNode, simInst Inst>
    : Pat<(OpNode GPR:$rs1), (Inst GPR:$rs1)>;
class PatGprGpr<SDPatternOperator OpNode, simInst Inst>
    : Pat<(OpNode GPR:$rs1, GPR:$rs2), (Inst GPR:$rs1, GPR:$rs2)>;

class PatGprImm<SDPatternOperator OpNode, simInst Inst, ImmLeaf ImmType>
    : Pat<(XLenVT (OpNode (XLenVT GPR:$rs1), ImmType:$imm)),
          (Inst GPR:$rs1, ImmType:$imm)>;
class PatGprSimm12<SDPatternOperator OpNode, simInstI Inst>
    : PatGprImm<OpNode, Inst, simm12>;
class PatGprUimmLog2XLen<SDPatternOperator OpNode, simInstIShift Inst>
    : PatGprImm<OpNode, Inst, uimmlog2xlen>;

def AddrFI : ComplexPattern<iPTR, 1, "SelectAddrFI", [frameindex], []>;
def BaseAddr : ComplexPattern<iPTR, 1, "SelectBaseAddr">;

/// Simple arithmetic operations

def : PatGprGpr<add, ADD>;
def : PatGprSimm12<add, ADDI>;
def : PatGprGpr<sub, SUB>;
def : PatGprGpr<or, OR>;
def : PatGprSimm12<or, ORI>;
def : PatGprGpr<and, AND>;
def : PatGprSimm12<and, ANDI>;
def : PatGprGpr<xor, XOR>;
def : PatGprSimm12<xor, XORI>;
def : PatGprUimmLog2XLen<shl, SLLI>;
def : PatGprUimmLog2XLen<srl, SRLI>;
def : PatGprUimmLog2XLen<sra, SRAI>;

/// FrameIndex calculations

def : Pat<(add (XLenVT AddrFI:$Rs), simm12:$imm12),
          (ADDI (XLenVT AddrFI:$Rs), simm12:$imm12)>;
def : Pat<(IsOrAdd (XLenVT AddrFI:$Rs), simm12:$imm12),
          (ADDI (XLenVT AddrFI:$Rs), simm12:$imm12)>;

/// Simple optimization
def : Pat<(add GPR:$rs1, (AddiPair:$rs2)),
          (ADDI (ADDI GPR:$rs1, (AddiPairImmB AddiPair:$rs2)),
                (AddiPairImmA GPR:$rs2))>;

/// Loads

multiclass LdPat<PatFrag LoadOp, simInst Inst, ValueType vt = XLenVT> {
  def : Pat<(vt (LoadOp BaseAddr:$rs1)), (Inst BaseAddr:$rs1, 0)>;
  def : Pat<(vt (LoadOp (add BaseAddr:$rs1, simm12:$imm12))),
            (Inst BaseAddr:$rs1, simm12:$imm12)>;
  def : Pat<(vt (LoadOp (IsOrAdd AddrFI:$rs1, simm12:$imm12))),
            (Inst AddrFI:$rs1, simm12:$imm12)>;
}

defm : LdPat<sextloadi8, LB>;
defm : LdPat<extloadi8, LB>;
defm : LdPat<sextloadi16, LH>;
defm : LdPat<extloadi16, LH>;
defm : LdPat<load, LW, i32>;
defm : LdPat<zextloadi8, LBU>;
defm : LdPat<zextloadi16, LHU>;

/// Stores

multiclass StPat<PatFrag StoreOp, simInst Inst, RegisterClass StTy,
                 ValueType vt> {
  def : Pat<(StoreOp (vt StTy:$rs2), BaseAddr:$rs1),
            (Inst StTy:$rs2, BaseAddr:$rs1, 0)>;
  def : Pat<(StoreOp (vt StTy:$rs2), (add BaseAddr:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, BaseAddr:$rs1, simm12:$imm12)>;
  def : Pat<(StoreOp (vt StTy:$rs2), (IsOrAdd AddrFI:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, AddrFI:$rs1, simm12:$imm12)>;
}

defm : StPat<truncstorei8, SB, GPR, XLenVT>;
defm : StPat<truncstorei16, SH, GPR, XLenVT>;
defm : StPat<store, SW, GPR, i32>;

// Pessimistically assume the stack pointer will be clobbered
let Defs = [X2], Uses = [X2] in {
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP   : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              [(callseq_end timm:$amt1, timm:$amt2)]>;
} // Defs = [X2], Uses = [X2]
