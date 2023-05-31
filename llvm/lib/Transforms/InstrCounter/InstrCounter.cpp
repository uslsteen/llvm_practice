#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>
#include <map>
#include <string>

using namespace llvm;

#define DEBUG_TYPE "hello"

STATISTIC(InstrCounter, "Counts number of opcodes");

namespace {
//
struct InstrCounterPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  std::map<std::string, int> mn_counter;

  InstrCounterPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    errs() << "Step into function " << F.getName() << '\n';
    //
    for (auto &B : F)
      for (auto &I : B)
        mn_counter[I.getOpcodeName()]++;

    for (auto &&it : mn_counter)
      errs() << it.first << " : " << it.second << "\n";
    //
    errs() << "\n";
    mn_counter.clear();
    return false;
  }
};
} // namespace

char InstrCounterPass::ID = 0;
static RegisterPass<InstrCounterPass> X("icounter", "Instruction counter Pass");

// //
// #include "llvm/ADT/Statistic.h"
// #include "llvm/IR/Function.h"
// #include "llvm/IR/LegacyPassManager.h"
// #include "llvm/Pass.h"
// #include "llvm/Support/raw_ostream.h"
// #include "llvm/Transforms/IPO/PassManagerBuilder.h"
// //
// #include <iostream>
// #include <map>
// //
// using namespace llvm;
//
// //
// namespace {
// struct MnCounterPass : public FunctionPass {
//   //
//   std::map<std::string, int> mn_counter;
//   static char ID;
//   //
//   MnCounterPass() : FunctionPass(ID) {}
//
//   //
//   bool runOnFunction(Function &F) override {
//     //
//     errs() << "Step into function " << F.getName() << '\n';
//     //
//     for (auto &B : F)
//       for (auto &I : B)
//         mn_counter[I.getOpcodeName()]++;
//
//     for (auto &&it : mn_counter)
//       errs() << it.first << " : " << it.second << "\n";
//     //
//     errs() << "\n";
//     mn_counter.clear();
//     //
//     return true;
//   }
// };
// } // namespace
//
// char MnCounterPass::ID = 0;
// static RegisterPass<MnCounterPass> X("icount",
//                                      "Counts mnemonics per functions");
