add_llvm_component_library(LLVMXorInfo
  XorTargetInfo.cpp

  LINK_COMPONENTS
  Support

  ADD_TO_COMPONENT
  Xor
  )
