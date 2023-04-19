//===-- XorAttributes.h - Xor Attributes --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains enumerations for Xor attributes as defined in RISC-V
// ELF psABI specification.
//
// RISC-V ELF psABI specification
//
// https://github.com/Xor/Xor-elf-psabi-doc/blob/master/Xor-elf.md
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_SUPPORT_XorATTRIBUTES_H
#define LLVM_SUPPORT_XorATTRIBUTES_H

#include "llvm/Support/ELFAttributes.h"

namespace llvm {
namespace XorAttrs {

const TagNameMap &getXorAttributeTags();

enum AttrType : unsigned {
  // Attribute types in ELF/.Xor.attributes.
  STACK_ALIGN = 4,
  ARCH = 5,
  UNALIGNED_ACCESS = 6,
  PRIV_SPEC = 8,
  PRIV_SPEC_MINOR = 10,
  PRIV_SPEC_REVISION = 12,
};

enum StackAlign { ALIGN_4 = 4, ALIGN_16 = 16 };

enum { NOT_ALLOWED = 0, ALLOWED = 1 };

} // namespace XorAttrs
} // namespace llvm

#endif
