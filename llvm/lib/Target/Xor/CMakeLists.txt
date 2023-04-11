add_llvm_component_group(Xor)

set(LLVM_TARGET_DEFINITIONS Xor.td)

# Generate XorGenRegisterInfo.inc and XorGenInstrInfo.inc which included by
#  your hand code C++ files.
# XorGenRegisterInfo.inc came from XorRegisterInfo.td, XorGenInstrInfo.inc
#  came from XorInstrInfo.td.
tablegen(LLVM XorGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM XorGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM XorGenCallingConv.inc -gen-callingconv)
tablegen(LLVM XorGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM XorGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM XorGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM XorGenDAGISel.inc -gen-dag-isel)

#tablegen(LLVM XorGenDisassemblerTables.inc -gen-disassembler)

# XorCommonTableGen must be defined
add_public_tablegen_target(XorCommonTableGen)

# XorCodeGen should match with LLVMBuild.txt XorCodeGen
add_llvm_target(XorCodeGen
  XorAsmPrinter.cpp
  XorFrameLowering.cpp
  XorInstrInfo.cpp
  XorISelDagToDag.cpp
  XorISelLowering.cpp
  XorMachineFunctionInfo.cpp
  XorMCInstLower.cpp
  XorRegisterInfo.cpp
  XorSubtarget.cpp
  XorTargetMachine.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  Core
  MC
  XorDesc
  XorInfo
  SelectionDAG
  Support
  Target
  GlobalISel

  ADD_TO_COMPONENT
  Xor
)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
