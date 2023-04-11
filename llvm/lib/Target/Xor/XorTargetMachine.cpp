//===----------------------------------------------------------------------===//
//
// Implements the info about Xor target spec.
//
//===----------------------------------------------------------------------===//

#include "XorTargetMachine.h"
#include "Xor.h"
//#include "XorTargetTransformInfo.h"
#include "TargetInfo/XorTargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetOptions.h"

#define DEBUG_TYPE "Xor"

using namespace llvm;

static Reloc::Model getRelocModel(Optional<Reloc::Model> RM) {
  return RM.getValueOr(Reloc::Static);
}

/// XorTargetMachine ctor - Create an ILP32 Architecture model
XorTargetMachine::XorTargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     Optional<Reloc::Model> RM,
                                     Optional<CodeModel::Model> CM,
                                     CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T,
                        "e-m:e-p:32:32-i1:8:32-i8:8:32-i16:16:32-i32:32:32-"
                        "f32:32:32-i64:32-f64:32-a:0:32-n32",
                        TT, CPU, FS, Options, getRelocModel(RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

XorTargetMachine::~XorTargetMachine() = default;

namespace {

/// Xor Code Generator Pass Configuration Options.
class XorPassConfig : public TargetPassConfig {
public:
  XorPassConfig(XorTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  XorTargetMachine &getXorTargetMachine() const {
    return getTM<XorTargetMachine>();
  }

  bool addInstSelector() override;
  // void addPreEmitPass() override;
  // void addPreRegAlloc() override;
};

} // end anonymous namespace

TargetPassConfig *XorTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new XorPassConfig(*this, PM);
}

bool XorPassConfig::addInstSelector() {
  addPass(createXorISelDag(getXorTargetMachine(), getOptLevel()));
  return false;
}

// void XorPassConfig::addPreEmitPass() { llvm_unreachable(""); }

// void XorPassConfig::addPreRegAlloc() { llvm_unreachable(""); }

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXorTarget() {
  RegisterTargetMachine<XorTargetMachine> X(getTheXorTarget());
}

#if 0
TargetTransformInfo
XorTargetMachine::getTargetTransformInfo(const Function &F) {
  return TargetTransformInfo(XorTTIImpl(this, F));
}
#endif