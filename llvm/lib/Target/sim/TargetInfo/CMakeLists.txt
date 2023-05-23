add_llvm_component_library(LLVMsimInfo
  simTargetInfo.cpp

  LINK_COMPONENTS
  Support

  ADD_TO_COMPONENT
  sim
  )
