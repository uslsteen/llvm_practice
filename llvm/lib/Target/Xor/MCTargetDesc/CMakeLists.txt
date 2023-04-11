add_llvm_component_library(LLVMXorDesc
  XorInstPrinter.cpp
  XorMCAsmInfo.cpp
  XorMCTargetDesc.cpp
  XorInfo.cpp

  LINK_COMPONENTS
  MC
  XorInfo
  Support

  ADD_TO_COMPONENT
  Xor
)