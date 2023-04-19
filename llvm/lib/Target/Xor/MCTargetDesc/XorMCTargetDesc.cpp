#include "TargetInfo/XorTargetInfo.h"
#include "XorELFStreamer.h"
#include "XorInfo.h"
#include "XorInstPrinter.h"
#include "XorMCAsmInfo.h"
#include "XorMCObjectFIleInfo.h"
#include "XorMCTargetDesc.h"
#include "XorTargetStreamer.h"
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
#include "XorGenRegisterInfo.inc"

#define GET_INSTRINFO_MC_DESC
#include "XorGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "XorGenSubtargetInfo.inc"

static MCInstrInfo *createXorMCInstrInfo() {
  auto *X = new MCInstrInfo();
  InitXorMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createXorMCRegisterInfo(const Triple &TT) {
  auto *X = new MCRegisterInfo();
  InitXorMCRegisterInfo(X, Xor::X1);
  return X;
}

static MCSubtargetInfo *createXorMCSubtargetInfo(const Triple &TT,
                                                 StringRef CPU, StringRef FS) {
  return createXorMCSubtargetInfoImpl(TT, CPU, /*TuneCPU=*/CPU, FS);
}

static MCAsmInfo *createXorMCAsmInfo(const MCRegisterInfo &MRI,
                                     const Triple &TT,
                                     const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new XorMCAsmInfo(TT);
  MCRegister SP = MRI.getDwarfRegNum(Xor::X2, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, SP, 0);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstPrinter *createXorMCInstPrinter(const Triple &T,
                                             unsigned SyntaxVariant,
                                             const MCAsmInfo &MAI,
                                             const MCInstrInfo &MII,
                                             const MCRegisterInfo &MRI) {
  return new XorInstPrinter(MAI, MII, MRI);
}

static MCTargetStreamer *createXorTargetAsmStreamer(MCStreamer &S,
                                                    formatted_raw_ostream &OS,
                                                    MCInstPrinter *InstPrint,
                                                    bool isVerboseAsm) {
  return new XorTargetStreamer(S);
}

static MCObjectFileInfo *
createXorMCObjectFileInfo(MCContext &Ctx, bool PIC,
                          bool LargeCodeModel = false) {
  MCObjectFileInfo *MOFI = new XorMCObjectFileInfo();
  MOFI->initMCObjectFileInfo(Ctx, PIC, LargeCodeModel);
  return MOFI;
}

static MCTargetStreamer *
createXorObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  const Triple &TT = STI.getTargetTriple();
  if (TT.isOSBinFormatELF())
    return new XorTargetELFStreamer(S, STI);
  return nullptr;
}

class XorMCInstrAnalysis : public MCInstrAnalysis {
public:
  explicit XorMCInstrAnalysis(const MCInstrInfo *Info)
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

    if (Inst.getOpcode() == Xor::JAL) {
      Target = Addr + Inst.getOperand(1).getImm();
      return true;
    }

    return false;
  }
};

static MCInstrAnalysis *createXorInstrAnalysis(const MCInstrInfo *Info) {
  return new XorMCInstrAnalysis(Info);
}

static MCTargetStreamer *createXorNullTargetStreamer(MCStreamer &S) {
  return new XorTargetStreamer(S);
}

namespace {
MCStreamer *createXorELFStreamer(const Triple &T, MCContext &Context,
                                 std::unique_ptr<MCAsmBackend> &&MAB,
                                 std::unique_ptr<MCObjectWriter> &&MOW,
                                 std::unique_ptr<MCCodeEmitter> &&MCE,
                                 bool RelaxAll) {
  return createXorELFStreamer(Context, std::move(MAB), std::move(MOW),
                              std::move(MCE), RelaxAll);
}
} // end anonymous namespace

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXorTargetMC() {
  // Register the MC asm info.
  Target &TheXorTarget = getTheXorTarget();
  RegisterMCAsmInfoFn X(TheXorTarget, createXorMCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCObjectFileInfo(TheXorTarget,
                                           createXorMCObjectFileInfo);
  TargetRegistry::RegisterMCInstrInfo(TheXorTarget, createXorMCInstrInfo);
  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheXorTarget, createXorMCRegisterInfo);

  TargetRegistry::RegisterMCAsmBackend(TheXorTarget, createXorAsmBackend);
  TargetRegistry::RegisterMCCodeEmitter(TheXorTarget, createXorMCCodeEmitter);
  TargetRegistry::RegisterMCInstPrinter(TheXorTarget, createXorMCInstPrinter);
  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheXorTarget,
                                          createXorMCSubtargetInfo);
  TargetRegistry::RegisterELFStreamer(TheXorTarget, createXorELFStreamer);
  TargetRegistry::RegisterObjectTargetStreamer(TheXorTarget,
                                               createXorObjectTargetStreamer);
  TargetRegistry::RegisterMCInstrAnalysis(TheXorTarget, createXorInstrAnalysis);
  // Register the MCInstPrinter
  TargetRegistry::RegisterMCInstPrinter(TheXorTarget, createXorMCInstPrinter);

  TargetRegistry::RegisterAsmTargetStreamer(TheXorTarget,
                                            createXorTargetAsmStreamer);

  TargetRegistry::RegisterNullTargetStreamer(TheXorTarget,
                                             createXorNullTargetStreamer);
}
