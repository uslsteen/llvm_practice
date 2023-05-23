#include "simMCTargetDesc.h"
#include "TargetInfo/simTargetInfo.h"
#include "simInfo.h"
#include "simInstPrinter.h"
#include "simELFStreamer.h"
#include "simMCObjectFIleInfo.h"
#include "simMCAsmInfo.h"
#include "simTargetStreamer.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_REGINFO_MC_DESC
#include "simGenRegisterInfo.inc"

#define GET_INSTRINFO_MC_DESC
#include "simGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "simGenSubtargetInfo.inc"

static MCInstrInfo *createsimMCInstrInfo() {
  auto *X = new MCInstrInfo();
  InitsimMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createsimMCRegisterInfo(const Triple &TT) {
  auto *X = new MCRegisterInfo();
  InitsimMCRegisterInfo(X, sim::X1);
  return X;
}

static MCSubtargetInfo *createsimMCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  return createsimMCSubtargetInfoImpl(TT, CPU, /*TuneCPU=*/CPU, FS);
}

static MCAsmInfo *createsimMCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new simMCAsmInfo(TT);
  MCRegister SP = MRI.getDwarfRegNum(sim::X2, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, SP, 0);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstPrinter *createsimMCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  return new simInstPrinter(MAI, MII, MRI);
}

static MCTargetStreamer *createsimTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint,
                                                 bool isVerboseAsm) {
  return new simTargetStreamer(S);
}

static MCObjectFileInfo *
createsimMCObjectFileInfo(MCContext &Ctx, bool PIC,
                            bool LargeCodeModel = false) {
  MCObjectFileInfo *MOFI = new simMCObjectFileInfo();
  MOFI->initMCObjectFileInfo(Ctx, PIC, LargeCodeModel);
  return MOFI;
}

static MCTargetStreamer *
createsimObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  const Triple &TT = STI.getTargetTriple();
  if (TT.isOSBinFormatELF())
    return new simTargetELFStreamer(S, STI);
  return nullptr;
}

class simMCInstrAnalysis : public MCInstrAnalysis {
public:
  explicit simMCInstrAnalysis(const MCInstrInfo *Info)
      : MCInstrAnalysis(Info) {}

  bool evaluateBranch(const MCInst &Inst, uint64_t Addr, uint64_t Size,
                      uint64_t &Target) const override {
    if (isConditionalBranch(Inst)) {
      int64_t Imm;
      if (Size == 2)
        Imm = Inst.getOperand(1).getImm();
      else
        Imm = Inst.getOperand(2).getImm();
      Target = Addr + Imm;
      return true;
    }

    if (Inst.getOpcode() == sim::JAL) {
      Target = Addr + Inst.getOperand(1).getImm();
      return true;
    }

    return false;
  }
};

static MCInstrAnalysis *createsimInstrAnalysis(const MCInstrInfo *Info) {
  return new simMCInstrAnalysis(Info);
}

static MCTargetStreamer *createsimNullTargetStreamer(MCStreamer &S) {
  return new simTargetStreamer(S);
}

namespace {
MCStreamer *createsimELFStreamer(const Triple &T, MCContext &Context,
                                   std::unique_ptr<MCAsmBackend> &&MAB,
                                   std::unique_ptr<MCObjectWriter> &&MOW,
                                   std::unique_ptr<MCCodeEmitter> &&MCE,
                                   bool RelaxAll) {
  return createsimELFStreamer(Context, std::move(MAB), std::move(MOW),
                                std::move(MCE), RelaxAll);
}
} // end anonymous namespace

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializesimTargetMC() {
  // Register the MC asm info.
  Target &ThesimTarget = getThesimTarget();
  RegisterMCAsmInfoFn X(ThesimTarget, createsimMCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCObjectFileInfo(ThesimTarget, createsimMCObjectFileInfo);
  TargetRegistry::RegisterMCInstrInfo(ThesimTarget, createsimMCInstrInfo);
  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(ThesimTarget, createsimMCRegisterInfo);

  TargetRegistry::RegisterMCAsmBackend(ThesimTarget, createsimAsmBackend);
  TargetRegistry::RegisterMCCodeEmitter(ThesimTarget, createsimMCCodeEmitter);
  TargetRegistry::RegisterMCInstPrinter(ThesimTarget, createsimMCInstPrinter);
  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(ThesimTarget,
                                          createsimMCSubtargetInfo);
  TargetRegistry::RegisterELFStreamer(ThesimTarget, createsimELFStreamer);
  TargetRegistry::RegisterObjectTargetStreamer(
        ThesimTarget, createsimObjectTargetStreamer);
  TargetRegistry::RegisterMCInstrAnalysis(ThesimTarget, createsimInstrAnalysis);
  // Register the MCInstPrinter
  TargetRegistry::RegisterMCInstPrinter(ThesimTarget, createsimMCInstPrinter);

  TargetRegistry::RegisterAsmTargetStreamer(ThesimTarget,
                                            createsimTargetAsmStreamer);

  TargetRegistry::RegisterNullTargetStreamer(ThesimTarget,
                                               createsimNullTargetStreamer);                   
}
