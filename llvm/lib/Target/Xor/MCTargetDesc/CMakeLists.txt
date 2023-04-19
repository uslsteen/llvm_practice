add_llvm_component_library(LLVMXorDesc
  XorInstPrinter.cpp
  XorMCAsmInfo.cpp
  XorMCTargetDesc.cpp
  XorInfo.cpp
  XorAsmBackend.cpp
  XorMCCodeEmitter.cpp
  XorMCExpr.cpp
  XorELFObjectWriter.cpp
  XorTargetStreamer.cpp
  XorELFStreamer.cpp
  XorMCObjectFIleInfo.cpp

  LINK_COMPONENTS
  MC
  XorInfo
  Support

  ADD_TO_COMPONENT
  Xor
)
