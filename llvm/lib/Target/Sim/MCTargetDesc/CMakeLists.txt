add_llvm_component_library(LLVMXorDesc
  XorMCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  XorInfo
  Support

  ADD_TO_COMPONENT
  Xor
  )
