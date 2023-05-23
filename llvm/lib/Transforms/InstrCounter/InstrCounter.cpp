//
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
//
#include <iostream>
#include <map>
//
using namespace llvm;

//
namespace {
struct MnCounterPass : public FunctionPass {
  //
  std::map<std::string, int> mn_counter;
  static char ID;

  //
  MnCounterPass() : FunctionPass(ID) {}

  //
  virtual bool runOnFunction(Function &F) {
    //
    outs() << "Step into function " << F.getName() << '\n';
    //
    for (auto &B : F)
      for (auto &I : B)
        mn_counter[I.getOpcodeName()]++;

    for (auto &&it : mn_counter)
      outs() << it.first << " : " << it.second << "\n";
    //
    outs() << "\n";
    mn_counter.clear();
    //
    return true;
  }
};
} // namespace

char MnCounterPass::ID = 0;
static RegisterPass<MnCounterPass> X("icount",
                                     "Counts mnemonics per functions");

// static void RegisterInstrCounterPass(const PassManagerBuilder &,
//                                      legacy::PassManagerBase &PM) {
//   PM.add(new MnCounterPass());
// }
// 
// //
// static RegisterStandardPasses
//     RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
//                    RegisterInstrCounterPass);
