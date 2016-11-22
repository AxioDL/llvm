//===-- llvm/Target/TargetLoweringObjectFile.h - Object Info ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements classes used to handle lowerings specific to common
// object file formats.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_TARGETLOWERINGOBJECTFILE_H
#define LLVM_TARGET_TARGETLOWERINGOBJECTFILE_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/SectionKind.h"

namespace llvm {
  class MachineModuleInfo;
  class Mangler;
  class MCContext;
  class MCExpr;
  class MCSection;
  class MCSymbol;
  class MCSymbolRefExpr;
  class MCStreamer;
  class MCValue;
  class ConstantExpr;
  class GlobalValue;
  class TargetMachine;

class TargetLoweringObjectFile : public MCObjectFileInfo {
  MCContext *Ctx;

  /// Name-mangler for global names.
  Mangler *Mang = nullptr;

  TargetLoweringObjectFile(
    const TargetLoweringObjectFile&) = delete;
  void operator=(const TargetLoweringObjectFile&) = delete;

protected:
  bool SupportIndirectSymViaGOTPCRel;
  bool SupportGOTPCRelWithOffset;

  /// This section contains the static constructor pointer list.
  MCSection *StaticCtorSection;

  /// This section contains the static destructor pointer list.
  MCSection *StaticDtorSection;

public:
  MCContext &getContext() const { return *Ctx; }
  Mangler &getMangler() const { return *Mang; }

  TargetLoweringObjectFile()
      : MCObjectFileInfo(), Ctx(nullptr), Mang(nullptr),
        SupportIndirectSymViaGOTPCRel(false), SupportGOTPCRelWithOffset(true) {}

  virtual ~TargetLoweringObjectFile();

  /// This method must be called before any actual lowering is done.  This
  /// specifies the current context for codegen, and gives the lowering
  /// implementations a chance to set up their default sections.
  virtual void Initialize(MCContext &ctx, const TargetMachine &TM);

  virtual void emitPersonalityValue(MCStreamer &Streamer, const DataLayout &TM,
                                    const MCSymbol *Sym) const;

  /// Emit the module flags that the platform cares about.
  virtual void emitModuleFlags(MCStreamer &Streamer,
                               ArrayRef<Module::ModuleFlagEntry> Flags,
                               const TargetMachine &TM) const {}

  /// Given a constant with the SectionKind, return a section that it should be
  /// placed in.
  virtual MCSection *getSectionForConstant(const DataLayout &DL,
                                           SectionKind Kind,
                                           const Constant *C,
                                           unsigned &Align) const;

  /// Classify the specified global variable into a set of target independent
  /// categories embodied in SectionKind.
  static SectionKind getKindForGlobal(const GlobalObject *GO,
                                      const TargetMachine &TM);

  /// Determine if constant is allocated in a subtarget small data
  /// SectionKind.
  static bool isConstantInSmallSection(const DataLayout &DL,
                                       const Constant *C,
                                       const TargetMachine &TM);

  /// Determine if global variable is allocated in a subtarget small data
  /// SectionKind.
  static bool isGlobalInSmallSection(const GlobalObject *GO,
                                     const TargetMachine &TM);

  /// This method computes the appropriate section to emit the specified global
  /// variable or function definition. This should not be passed external (or
  /// available externally) globals.
  MCSection *SectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                              const TargetMachine &TM) const;

  /// This method computes the appropriate section to emit the specified global
  /// variable or function definition. This should not be passed external (or
  /// available externally) globals.
  MCSection *SectionForGlobal(const GlobalObject *GO,
                              const TargetMachine &TM) const {
    return SectionForGlobal(GO, getKindForGlobal(GO, TM), TM);
  }

  virtual void getNameWithPrefix(SmallVectorImpl<char> &OutName,
                                 const GlobalValue *GV,
                                 const TargetMachine &TM) const;

  virtual MCSection *getSectionForJumpTable(const Function &F,
                                            const TargetMachine &TM) const;

  virtual bool shouldPutJumpTableInFunctionSection(bool UsesLabelDifference,
                                                   const Function &F) const;

  /// Targets should implement this method to assign a section to globals with
  /// an explicit section specfied. The implementation of this method can
  /// assume that GO->hasSection() is true.
  virtual MCSection *
  getExplicitSectionGlobal(const GlobalObject *GO, SectionKind Kind,
                           const TargetMachine &TM) const = 0;

  /// Return an MCExpr to use for a reference to the specified global variable
  /// from exception handling information.
  virtual const MCExpr *getTTypeGlobalReference(const GlobalValue *GV,
                                                unsigned Encoding,
                                                const TargetMachine &TM,
                                                MachineModuleInfo *MMI,
                                                MCStreamer &Streamer) const;

  /// Return the MCSymbol for a private symbol with global value name as its
  /// base, with the specified suffix.
  MCSymbol *getSymbolWithGlobalValueBase(const GlobalValue *GV,
                                         StringRef Suffix,
                                         const TargetMachine &TM) const;

  // The symbol that gets passed to .cfi_personality.
  virtual MCSymbol *getCFIPersonalitySymbol(const GlobalValue *GV,
                                            const TargetMachine &TM,
                                            MachineModuleInfo *MMI) const;

  const MCExpr *getTTypeReference(const MCSymbolRefExpr *Sym, unsigned Encoding,
                                  MCStreamer &Streamer) const;

  virtual MCSection *getStaticCtorSection(unsigned Priority,
                                          const MCSymbol *KeySym) const {
    return StaticCtorSection;
  }

  virtual MCSection *getStaticDtorSection(unsigned Priority,
                                          const MCSymbol *KeySym) const {
    return StaticDtorSection;
  }

  /// \brief Create a symbol reference to describe the given TLS variable when
  /// emitting the address in debug info.
  virtual const MCExpr *getDebugThreadLocalSymbol(const MCSymbol *Sym) const;

  virtual const MCExpr *lowerRelativeReference(const GlobalValue *LHS,
                                               const GlobalValue *RHS,
                                               const TargetMachine &TM) const {
    return nullptr;
  }

  /// \brief Target supports replacing a data "PC"-relative access to a symbol
  /// through another symbol, by accessing the later via a GOT entry instead?
  bool supportIndirectSymViaGOTPCRel() const {
    return SupportIndirectSymViaGOTPCRel;
  }

  /// \brief Target GOT "PC"-relative relocation supports encoding an additional
  /// binary expression with an offset?
  bool supportGOTPCRelWithOffset() const {
    return SupportGOTPCRelWithOffset;
  }

  /// \brief Get the target specific PC relative GOT entry relocation
  virtual const MCExpr *getIndirectSymViaGOTPCRel(const MCSymbol *Sym,
                                                  const MCValue &MV,
                                                  int64_t Offset,
                                                  MachineModuleInfo *MMI,
                                                  MCStreamer &Streamer) const {
    return nullptr;
  }

  virtual void emitLinkerFlagsForGlobal(raw_ostream &OS,
                                        const GlobalValue *GV) const {}

protected:
  virtual MCSection *SelectSectionForGlobal(const GlobalObject *GO,
                                            SectionKind Kind,
                                            const TargetMachine &TM) const = 0;
  virtual bool isConstantInSmallSectionKind(const DataLayout &DL,
                                            const Constant *C) const {
    return false;
  }
  virtual bool isGlobalInSmallSectionKind(const GlobalObject *GO,
                                          const TargetMachine &TM) const {
    return false;
  }
};

} // end namespace llvm

#endif
