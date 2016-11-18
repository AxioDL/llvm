# RUN: llvm-mc -triple=powerpc-unknown-linux-gnu -filetype=obj %s | \
# RUN: llvm-readobj -r | FileCheck %s
	.text

	.globl foo
	.type foo,@function
foo:
	bl printf@plt
	bl printf
	beq printf
	beqa printf
	bl _GLOBAL_OFFSET_TABLE_@local-4
	lwz 4, smallval@sda21(13)
	lis 5, .LC1@ha
	addi 5, 5, .LC1@l
.LC1:
	.size foo, . - foo

	.sdata
	.globl smallval
smallval:
	.long 0xDEADBEEF
	.long foo
	.long .LC1 - .

# CHECK:      Relocations [
# CHECK-NEXT:   Section {{.*}} .rela.text {
# CHECK-NEXT:     0x0 R_PPC_PLTREL24 printf 0x0
# CHECK-NEXT:     0x4 R_PPC_REL24 printf 0x0
# CHECK-NEXT:     0x8 R_PPC_REL14 printf 0x0
# CHECK-NEXT:     0xC R_PPC_ADDR14 printf 0x0
# CHECK-NEXT:     0x10 R_PPC_LOCAL24PC _GLOBAL_OFFSET_TABLE_ 0xFFFFFFFC
# CHECK-NEXT:     0x15 R_PPC_EMB_SDA21 smallval 0x0
# CHECK-NEXT:     0x1A R_PPC_ADDR16_HA .text 0x20
# CHECK-NEXT:     0x1E R_PPC_ADDR16_LO .text 0x20
# CHECK-NEXT:   }
# CHECK-NEXT:   Section {{.*}} .rela.sdata {
# CHECK-NEXT:     0x4 R_PPC_ADDR32 foo 0x0
# CHECK-NEXT:     0x8 R_PPC_REL32 .text 0x20
# CHECK-NEXT:   }
# CHECK-NEXT: ]
