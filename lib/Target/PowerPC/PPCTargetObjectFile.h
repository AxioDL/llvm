//===-- PPCTargetObjectFile.h - PPC Object Info -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_POWERPC_PPCTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_POWERPC_PPCTARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

  /// PPC64LinuxTargetObjectFile - This implementation is used for
  /// 64-bit PowerPC Linux.
  class PPC64LinuxTargetObjectFile : public TargetLoweringObjectFileELF {

    void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

    MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                      const TargetMachine &TM) const override;

    /// \brief Describe a TLS variable address within debug info.
    const MCExpr *getDebugThreadLocalSymbol(const MCSymbol *Sym) const override;
  };

  /// PPCEmbeddedTargetObjectFile - This implementation is used for
  /// 32-bit PPC-EABI.
  class PPCEmbeddedTargetObjectFile : public TargetLoweringObjectFileELF {
    MCSection *SmallDataSection = nullptr;
    MCSection *SmallData2Section = nullptr;
    MCSection *SmallBssSection = nullptr;

    bool isConstantInSmallSectionKind(const DataLayout &DL,
                                      const Constant *C) const override;

    bool isGlobalInSmallSectionKind(const GlobalObject *GO,
                                    const TargetMachine &TM) const override;

  public:

    void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

    MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                      const TargetMachine &TM) const override;

    MCSection *getSectionForConstant(const DataLayout &DL,
                                     SectionKind Kind,
                                     const Constant *C,
                                     unsigned &Align) const override;
  };

}  // end namespace llvm

#endif
