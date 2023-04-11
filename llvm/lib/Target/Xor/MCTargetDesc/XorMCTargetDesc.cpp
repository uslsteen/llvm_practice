#include "XorMCTargetDesc.h"
#include "TargetInfo/XorTargetInfo.h"
#include "XorInfo.h"
#include "XorInstPrinter.h"
#include "XorMCAsmInfo.h"
#include "XorTargetStreamer.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

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

XorTargetStreamer::XorTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}
XorTargetStreamer::~XorTargetStreamer() = default;

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint,
                                                 bool isVerboseAsm) {
  return new XorTargetStreamer(S);
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXorTargetMC() {
  // Register the MC asm info.
  Target &TheXorTarget = getTheXorTarget();
  RegisterMCAsmInfoFn X(TheXorTarget, createXorMCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheXorTarget, createXorMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheXorTarget, createXorMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheXorTarget,
                                          createXorMCSubtargetInfo);

  // Register the MCInstPrinter
  TargetRegistry::RegisterMCInstPrinter(TheXorTarget, createXorMCInstPrinter);

  TargetRegistry::RegisterAsmTargetStreamer(TheXorTarget,
                                            createTargetAsmStreamer);
}