//===-- llvm/Target/MipsTargetObjectFile.h - Mips Object Info ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPSTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_MIPS_MIPSTARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {
class MipsTargetMachine;
  class MipsTargetObjectFile : public TargetLoweringObjectFileELF {
    MCSection *SmallDataSection;
    MCSection *SmallBSSSection;
    const MipsTargetMachine *TM;

    bool isGlobalInSmallSectionKind(const GlobalObject *GO,
                                    const TargetMachine &TM) const override;
  public:

    void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

    MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                      const TargetMachine &TM) const override;

    /// Return true if this constant should be placed into small data section.
    bool isConstantInSmallSection(const DataLayout &DL, const Constant *CN,
                                  const TargetMachine &TM) const;

    MCSection *getSectionForConstant(const DataLayout &DL, SectionKind Kind,
                                     const Constant *C,
                                     unsigned &Align) const override;
  };
} // end namespace llvm

#endif
