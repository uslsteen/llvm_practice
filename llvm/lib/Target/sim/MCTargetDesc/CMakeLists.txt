add_llvm_component_library(LLVMsimDesc
  simInstPrinter.cpp
  simMCAsmInfo.cpp
  simMCTargetDesc.cpp
  simInfo.cpp
  simAsmBackend.cpp
  simMCCodeEmitter.cpp
  simMCExpr.cpp
  simELFObjectWriter.cpp
  simTargetStreamer.cpp
  simELFStreamer.cpp
  simMCObjectFIleInfo.cpp

  LINK_COMPONENTS
  MC
  simInfo
  Support

  ADD_TO_COMPONENT
  sim
  )
