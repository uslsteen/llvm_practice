add_llvm_library(LLVMInstrCounter MODULE
    InstrCounter.cpp

    PLUGIN_TOOL
    opt

    LINK_COMPONENTS
    Core
    Support
)