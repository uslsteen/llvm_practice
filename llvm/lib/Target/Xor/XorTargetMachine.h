#ifndef LLVM_LIB_TARGET_Xor_XorTARGETMACHINE_H
#define LLVM_LIB_TARGET_Xor_XorTARGETMACHINE_H

#include "XorInstrInfo.h"
#include "XorSubtarget.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class XorTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  XorSubtarget Subtarget;
  // mutable StringMap<std::unique_ptr<XorSubtarget>> SubtargetMap;

public:
  XorTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                    CodeGenOpt::Level OL, bool JIT);
  ~XorTargetMachine() override;

  const XorSubtarget *getSubtargetImpl() const { return &Subtarget; }
  const XorSubtarget *getSubtargetImpl(const Function &) const override {
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

#endif
