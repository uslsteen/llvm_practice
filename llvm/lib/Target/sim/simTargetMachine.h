#ifndef __LLVM_LIB_TARGET_SIM_SIMTARGETMACHINE_H__
#define __LLVM_LIB_TARGET_SIM_SIMTARGETMACHINE_H__

#include "simInstrInfo.h"
#include "simSubtarget.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class simTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  simSubtarget Subtarget;
  // mutable StringMap<std::unique_ptr<simSubtarget>> SubtargetMap;

public:
  simTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                    CodeGenOpt::Level OL, bool JIT);
  ~simTargetMachine() override;

  const simSubtarget *getSubtargetImpl() const { return &Subtarget; }
  const simSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

#if 0
  bool
  addPassesToEmitFile(PassManagerBase &, raw_pwrite_stream &,
                      raw_pwrite_stream *, CodeGenFileType,
                      bool /*DisableVerify*/ = true,
                      MachineModuleInfoWrapperPass *MMIWP = nullptr) override {
    return false;
  }
#endif
  // TargetTransformInfo getTargetTransformInfo(const Function &F) override;
};
} // end namespace llvm

#endif // __LLVM_LIB_TARGET_SIM_SIMTARGETMACHINE_H__
