//===----------------------------------------------------------------------===//
//
// Implements the info about sim target spec.
//
//===----------------------------------------------------------------------===//

#include "simTargetMachine.h"
#include "sim.h"
//#include "simTargetTransformInfo.h"
#include "TargetInfo/simTargetInfo.h"
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

#define DEBUG_TYPE "sim"

using namespace llvm;

static Reloc::Model getRelocModel(Optional<Reloc::Model> RM) {
  return RM.getValueOr(Reloc::Static);
}

/// simTargetMachine ctor - Create an ILP32 Architecture model
simTargetMachine::simTargetMachine(const Target &T, const Triple &TT,
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

simTargetMachine::~simTargetMachine() = default;

namespace {

/// sim Code Generator Pass Configuration Options.
class simPassConfig : public TargetPassConfig {
public:
  simPassConfig(simTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  simTargetMachine &getsimTargetMachine() const {
    return getTM<simTargetMachine>();
  }

  bool addInstSelector() override;
  // void addPreEmitPass() override;
  // void addPreRegAlloc() override;
};

} // end anonymous namespace

TargetPassConfig *simTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new simPassConfig(*this, PM);
}

bool simPassConfig::addInstSelector() {
  addPass(createsimISelDag(getsimTargetMachine(), getOptLevel()));
  return false;
}

// void simPassConfig::addPreEmitPass() { llvm_unreachable(""); }

// void simPassConfig::addPreRegAlloc() { llvm_unreachable(""); }

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializesimTarget() {
  RegisterTargetMachine<simTargetMachine> X(getThesimTarget());
}

#if 0
TargetTransformInfo
simTargetMachine::getTargetTransformInfo(const Function &F) {
  return TargetTransformInfo(simTTIImpl(this, F));
}
#endif