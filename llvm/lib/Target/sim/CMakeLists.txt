add_llvm_component_group(sim)

set(LLVM_TARGET_DEFINITIONS sim.td)

# Generate simGenRegisterInfo.inc and simGenInstrInfo.inc which included by
#  your hand code C++ files.
# simGenRegisterInfo.inc came from simRegisterInfo.td, simGenInstrInfo.inc
#  came from simInstrInfo.td.
tablegen(LLVM simGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM simGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM simGenCallingConv.inc -gen-callingconv)
tablegen(LLVM simGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM simGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM simGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM simGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM simGenDAGISel.inc -gen-dag-isel)

#tablegen(LLVM simGenDisassemblerTables.inc -gen-disassembler)

# simCommonTableGen must be defined
add_public_tablegen_target(simCommonTableGen)

# simCodeGen should match with LLVMBuild.txt simCodeGen
add_llvm_target(simCodeGen
  simAsmPrinter.cpp
  simFrameLowering.cpp
  simInstrInfo.cpp
  simISelDagToDag.cpp
  simISelLowering.cpp
  simMachineFunctionInfo.cpp
  simMCInstLower.cpp
  simRegisterInfo.cpp
  simSubtarget.cpp
  simTargetMachine.cpp
  simMatInt.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  Core
  MC
  simDesc
  simInfo
  SelectionDAG
  Support
  Target
  GlobalISel

  ADD_TO_COMPONENT
  sim
)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
