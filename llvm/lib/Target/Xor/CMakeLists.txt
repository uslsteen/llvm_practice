add_llvm_component_group(Xor)

set(LLVM_TARGET_DEFINITIONS Xor.td)

# Generate XorGenRegisterInfo.inc and XorGenInstrInfo.inc which included by
#  your hand code C++ files.
# XorGenRegisterInfo.inc came from XorRegisterInfo.td, XorGenInstrInfo.inc
#  came from XorInstrInfo.td.
tablegen(LLVM XorGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM XorGenInstrInfo.inc -gen-instr-info)

# XorCommonTableGen must be defined
add_public_tablegen_target(XorCommonTableGen)

# XorCodeGen should match with LLVMBuild.txt XorCodeGen
add_llvm_target(XorCodeGen
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
