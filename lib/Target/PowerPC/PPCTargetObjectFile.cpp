//===-- PPCTargetObjectFile.cpp - PPC Object Info -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PPCTargetObjectFile.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSectionELF.h"
#include "PPCSubtarget.h"
#include "PPCTargetMachine.h"

using namespace llvm;

static cl::opt<unsigned>
SSThreshold("ppc-ssection-threshold", cl::Hidden,
            cl::desc("PPC: Small data and bss section threshold size (default=8)"),
            cl::init(8));

void
PPC64LinuxTargetObjectFile::
Initialize(MCContext &Ctx, const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
  InitializeELF(TM.Options.UseInitArray);
}

MCSection *PPC64LinuxTargetObjectFile::SelectSectionForGlobal(
    const GlobalObject *GO, SectionKind Kind, const TargetMachine &TM) const {
  // Here override ReadOnlySection to DataRelROSection for PPC64 SVR4 ABI
  // when we have a constant that contains global relocations.  This is
  // necessary because of this ABI's handling of pointers to functions in
  // a shared library.  The address of a function is actually the address
  // of a function descriptor, which resides in the .opd section.  Generated
  // code uses the descriptor directly rather than going via the GOT as some
  // other ABIs do, which means that initialized function pointers must
  // reference the descriptor.  The linker must convert copy relocs of
  // pointers to functions in shared libraries into dynamic relocations,
  // because of an ordering problem with initialization of copy relocs and
  // PLT entries.  The dynamic relocation will be initialized by the dynamic
  // linker, so we must use DataRelROSection instead of ReadOnlySection.
  // For more information, see the description of ELIMINATE_COPY_RELOCS in
  // GNU ld.
  if (Kind.isReadOnly()) {
    const auto *GVar = dyn_cast<GlobalVariable>(GO);

    if (GVar && GVar->isConstant() && GVar->getInitializer()->needsRelocation())
      Kind = SectionKind::getReadOnlyWithRel();
  }

  return TargetLoweringObjectFileELF::SelectSectionForGlobal(GO, Kind, TM);
}

const MCExpr *PPC64LinuxTargetObjectFile::
getDebugThreadLocalSymbol(const MCSymbol *Sym) const {
  const MCExpr *Expr =
    MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_DTPREL, getContext());
  return MCBinaryExpr::createAdd(Expr,
                                 MCConstantExpr::create(0x8000, getContext()),
                                 getContext());
}


// A address must be loaded from a small section if its size is less than the
// small section size threshold. Data in this section must be addressed against
// non-volatile r13 for .sdata/.sbss and r2 for .sdata2.
static bool isInSmallSection(uint64_t Size) {
  // gcc has traditionally not treated zero-sized objects as small data, so this
  // is effectively part of the ABI.
  return Size > 0 && Size <= SSThreshold;
}

/// Return true if this global address should be placed into small data/bss
/// section. This method does all the work, directly influencing section
/// kind.
bool PPCEmbeddedTargetObjectFile::
isGlobalInSmallSectionKind(const GlobalObject *GO,
                           const TargetMachine &TM) const {
  // Only global variables, not functions.
  const GlobalVariable *GVA = dyn_cast<GlobalVariable>(GO);
  if (!GVA)
    return false;

  Type *Ty = GVA->getValueType();
  return isInSmallSection(
      GVA->getParent()->getDataLayout().getTypeAllocSize(Ty));
}

void
PPCEmbeddedTargetObjectFile::
Initialize(MCContext &Ctx, const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
  InitializeELF(TM.Options.UseInitArray);

  SmallDataSection = getContext().getELFSection(
      ".sdata", ELF::SHT_PROGBITS, ELF::SHF_WRITE | ELF::SHF_ALLOC);
  SmallData2Section = getContext().getELFSection(
      ".sdata2", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
  SmallBssSection = getContext().getELFSection(
      ".sbss", ELF::SHT_NOBITS, ELF::SHF_WRITE | ELF::SHF_ALLOC);
}

MCSection *PPCEmbeddedTargetObjectFile::SelectSectionForGlobal(
    const GlobalObject *GO, SectionKind Kind, const TargetMachine &TM) const {
  // Handle Small Section classification here.
  if (Kind.isSmallBSS())
    return SmallBssSection;
  if (Kind.isSmallReadOnly())
    return SmallData2Section;
  if (Kind.isSmallKind())
    return SmallDataSection;

  return TargetLoweringObjectFileELF::SelectSectionForGlobal(GO, Kind, TM);
}
