#ifndef __LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorFIXUPKINDS_H__
#define __LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorFIXUPKINDS_H__
#include "llvm/MC/MCFixup.h"

#undef Xor

namespace llvm {
namespace Xor {
enum Fixups {
  // 20-bit fixup corresponding to %hi(foo) for instructions like lui
  fixup_Xor_hi20 = FirstTargetFixupKind,
  // 12-bit fixup corresponding to %lo(foo) for instructions like addi
  fixup_Xor_lo12_i,
  // 12-bit fixup corresponding to %lo(foo) for the S-type store instructions
  fixup_Xor_lo12_s,
  // 20-bit fixup corresponding to %pcrel_hi(foo) for instructions like auipc
  fixup_Xor_pcrel_hi20,
  // 12-bit fixup corresponding to %pcrel_lo(foo) for instructions like addi
  fixup_Xor_pcrel_lo12_i,
  // 12-bit fixup corresponding to %pcrel_lo(foo) for the S-type store
  // instructions
  fixup_Xor_pcrel_lo12_s,
  // 20-bit fixup corresponding to %got_pcrel_hi(foo) for instructions like
  // auipc
  fixup_Xor_got_hi20,
  // 20-bit fixup corresponding to %tprel_hi(foo) for instructions like lui
  fixup_Xor_tprel_hi20,
  // 12-bit fixup corresponding to %tprel_lo(foo) for instructions like addi
  fixup_Xor_tprel_lo12_i,
  // 12-bit fixup corresponding to %tprel_lo(foo) for the S-type store
  // instructions
  fixup_Xor_tprel_lo12_s,
  // Fixup corresponding to %tprel_add(foo) for PseudoAddTPRel, used as a linker
  // hint
  fixup_Xor_tprel_add,
  // 20-bit fixup corresponding to %tls_ie_pcrel_hi(foo) for instructions like
  // auipc
  fixup_Xor_tls_got_hi20,
  // 20-bit fixup corresponding to %tls_gd_pcrel_hi(foo) for instructions like
  // auipc
  fixup_Xor_tls_gd_hi20,
  // 20-bit fixup for symbol references in the jal instruction
  fixup_Xor_jal,
  // 12-bit fixup for symbol references in the branch instructions
  fixup_Xor_branch,
  // 11-bit fixup for symbol references in the compressed jump instruction
  fixup_Xor_rvc_jump,
  // 8-bit fixup for symbol references in the compressed branch instruction
  fixup_Xor_rvc_branch,
  // Fixup representing a legacy no-pic function call attached to the auipc
  // instruction in a pair composed of adjacent auipc+jalr instructions.
  fixup_Xor_call,
  // Fixup representing a function call attached to the auipc instruction in a
  // pair composed of adjacent auipc+jalr instructions.
  fixup_Xor_call_plt,
  // Used to generate an R_Xor_RELAX relocation, which indicates the linker
  // may relax the instruction pair.
  fixup_Xor_relax,
  // Used to generate an R_Xor_ALIGN relocation, which indicates the linker
  // should fixup the alignment after linker relaxation.
  fixup_Xor_align,
  // 8-bit fixup corresponding to R_Xor_SET8 for local label assignment.
  fixup_Xor_set_8,
  // 8-bit fixup corresponding to R_Xor_ADD8 for 8-bit symbolic difference
  // paired relocations.
  fixup_Xor_add_8,
  // 8-bit fixup corresponding to R_Xor_SUB8 for 8-bit symbolic difference
  // paired relocations.
  fixup_Xor_sub_8,
  // 16-bit fixup corresponding to R_Xor_SET16 for local label assignment.
  fixup_Xor_set_16,
  // 16-bit fixup corresponding to R_Xor_ADD16 for 16-bit symbolic difference
  // paired reloctions.
  fixup_Xor_add_16,
  // 16-bit fixup corresponding to R_Xor_SUB16 for 16-bit symbolic difference
  // paired reloctions.
  fixup_Xor_sub_16,
  // 32-bit fixup corresponding to R_Xor_SET32 for local label assignment.
  fixup_Xor_set_32,
  // 32-bit fixup corresponding to R_Xor_ADD32 for 32-bit symbolic difference
  // paired relocations.
  fixup_Xor_add_32,
  // 32-bit fixup corresponding to R_Xor_SUB32 for 32-bit symbolic difference
  // paired relocations.
  fixup_Xor_sub_32,
  // 64-bit fixup corresponding to R_Xor_ADD64 for 64-bit symbolic difference
  // paired relocations.
  fixup_Xor_add_64,
  // 64-bit fixup corresponding to R_Xor_SUB64 for 64-bit symbolic difference
  // paired relocations.
  fixup_Xor_sub_64,
  // 6-bit fixup corresponding to R_Xor_SET6 for local label assignment in
  // DWARF CFA.
  fixup_Xor_set_6b,
  // 6-bit fixup corresponding to R_Xor_SUB6 for local label assignment in
  // DWARF CFA.
  fixup_Xor_sub_6b,

  // Used as a sentinel, must be the last
  fixup_Xor_invalid,
  NumTargetFixupKinds = fixup_Xor_invalid - FirstTargetFixupKind
};
} // namespace Xor
} // end namespace llvm

#endif // __LLVM_LIB_TARGET_Xor_MCTARGETDESC_XorFIXUPKINDS_H__