#ifndef LLVM_LIB_TARGET_Xor_XorTARGETSTREAMER_H
#define LLVM_LIB_TARGET_Xor_XorTARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"

namespace llvm {

class XorTargetStreamer : public MCTargetStreamer {
public:
  XorTargetStreamer(MCStreamer &S);
  ~XorTargetStreamer() override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_Xor_XorTARGETSTREAMER_H