	.llen	132
	.include	as.inc
	.text
	.alignoff
	.align	4

/	the init gdt is in the text segment since the data segment
/	is relocated after we switch into 386 mode (when a
/	valid gdt is required)

gdtinit:				/ used before turning on paging
	.value	119			/ 120 bytes => 15 entries in gdt
					/ gdt physical addr is 0x0040_2000+gdt
	.long	[[[-SBASE]+PBASE]<<BPCSHIFT]+gdt
	.value	0
gdtmap:					/ used after paging is enabled
	.value	119
	.long	gdt
	.value	0

idtmap:					/ used after paging is enabled
	.value	2047
	.long	idt
	.value	0
bdtmap:					/ used after paging is enabled
	.value	2047				/ 1023
	.long	bdt
	.value	0

////
/
/ Macro SEGMENT specifies a segment descriptor.
/ base is 32 bits;  limit and attr are 16 bits.
/
////
SEGMENT	.macro	base,limit,attr
	.value	limit
	.value	base
	.byte	[base] >> 16
	.value	attr
	.byte	[base] >> 24
	.endm

////
/
/ "segment xxxx" below gives the value in a segment register corresponding
/ to the given descriptor.  The low 3 bits in a segment register are not
/ used in indexing into the descriptor table.
/
////

gdt:
	/ segment 0000
	SEGMENT	0,0,0			/ null entry

	/ segment 0008 - SEG_386_UI
	SEGMENT	0,0xFFFF,0xCFFB		/ user code (386 mode)

	/ segment 0010 - SEG_386_UD
	SEGMENT	0,0xFFFF,0xCFF3		/ user data (386 mode)

	/ segment 0018 - SEG_386_KI
	SEGMENT	0,0xFFFF,0xCF9B		/ kernel code

	/ segment 0020 - SEG_386_KD
	SEGMENT	0,0xFFFF,0xCF93		/ kernel data

	/ segment 0028 - SEG_286_UI
	SEGMENT	0,0xF,0x80FB		/ user code (286 common I/D spaces)

	/ segment 0030 - SEG_286_UD
	SEGMENT	0,0xF,0x80F3		/ user data (286 mode) 

	/ segment 0038 - SEG_TSS
	SEGMENT	0xFFC00000,0xEB,0x0089

	/ segment 0040 - SEG_ROM
	SEGMENT	0xFFFC0000,0xF,0x8093

	/ segment 0048 - SEG_VIDEOa
	SEGMENT	0xFFFB0000,0xF,0x8093	/ 000B0000 -> FFFB0000 (video A)

	/ segment 0050 - SEG_VIDEOb
	SEGMENT	0xFFFA0000,0xF,0x8093	/ 000B8000 -> FFFA0000 (video B)

	/ segment 0058 - SEG_386_II	/ init code (text)
	SEGMENT	0x400000+[PBASE<<BPCSHIFT],0xFFFF,0xCF9B

	/ segment 0060 - SEG_386_ID	/ init code (data)
	SEGMENT	0x400000+[PBASE<<BPCSHIFT],0xFFFF,0xCF93

	/ segment 0068 - SEG_286_UII
	SEGMENT	0x400000,0xF,0x80FB	/ user code (286 separate I/D spaces)

	/ segment 0070 - SEG_LDT
	SEGMENT	0xFFC00000,0xF,0x0082	/ ldt segment (2 descriptors)

/	The two entries in the ldt are call gates whose format is somewhat
/	different from the other segment descriptors
/
/	BCS compatibility requires an LDT

ldt:
	/ segment 0000
	.long	syc32				/ call gate for system call
	.long	0xFFC0EC01

	/ segment 0008
	.long	sig32				/ call gate for signal return
	.long	0xFFC0EC01

ldtend:

idt:
	.long	trap0
	.value	0xEE00,0xFFC0
	.long	trap1
	.value	0xEE00,0xFFC0
	.long	trap2
	.value	0xEE00,0xFFC0
	.long	trap3
	.value	0xEE00,0xFFC0
	.long	trap4
	.value	0xEE00,0xFFC0
	.long	trap5
	.value	0xEE00,0xFFC0
	.long	trap6
	.value	0xEE00,0xFFC0
	.long	trap7
	.value	0xEE00,0xFFC0
	.long	trap8
	.value	0xEE00,0xFFC0
	.long	trap9
	.value	0xEE00,0xFFC0
	.long	trap10
	.value	0xEE00,0xFFC0
	.long	trap11
	.value	0xEE00,0xFFC0
	.long	trap12
	.value	0xEE00,0xFFC0
	.long	trap13				/trap13
	.value	0xEE00,0xFFC0
	.long	trap14				/trap14
	.value	0xEE00,0xFFC0
	.long	0,0
	.long	trap16
	.value	0xEE00,0xFFC0
	.org	.+0x78
	.long	clk
	.value	0xEE00,0xFFC0
	.long	dev1
	.value	0xEE00,0xFFC0
	.long	dev9
	.value	0xEE00,0xFFC0
	.long	dev3
	.value	0xEE00,0xFFC0
	.long	dev4
	.value	0xEE00,0xFFC0
	.long	dev5
	.value	0xEE00,0xFFC0
	.long	dev6
	.value	0xEE00,0xFFC0
	.long	dev7
	.value	0xEE00,0xFFC0
	.org	.+0x240
	.long	dev8
	.value	0xEE00,0xFFC0
	.long	dev9
	.value	0xEE00,0xFFC0
	.long	dev10
	.value	0xEE00,0xFFC0
	.long	dev11
	.value	0xEE00,0xFFC0
	.long	dev12
	.value	0xEE00,0xFFC0
	.long	dev13
	.value	0xEE00,0xFFC0
	.long	dev14
	.value	0xEE00,0xFFC0
	.long	dev15
	.value	0xEE00,0xFFC0
	.org	.+0x40
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	syc
	.value	0xEE00,0xFFC0
	.long	0
idtend:

bdt:
	.long	trap0
	.value	0xEE00,0xFFC0
	.long	trap1
	.value	0xEE00,0xFFC0
	.long	trap2
	.value	0xEE00,0xFFC0
	.long	trap3
	.value	0xEE00,0xFFC0
	.long	trap4
	.value	0xEE00,0xFFC0
	.long	trap5
	.value	0xEE00,0xFFC0
	.long	trap6
	.value	0xEE00,0xFFC0
	.long	trap7
	.value	0xEE00,0xFFC0
	.long	trap8
	.value	0xEE00,0xFFC0
	.long	trap9
	.value	0xEE00,0xFFC0
	.long	trap10
	.value	0xEE00,0xFFC0
	.long	trap11
	.value	0xEE00,0xFFC0
	.long	trap12
	.value	0xEE00,0xFFC0
	.long	loc10
	.value	0xEE00,0xFFC0
	.long	loc10
	.value	0xEE00,0xFFC0
	.long	0,0
	.long	trap16
	.value	0xEE00,0xFFC0
	.org	.+0x78 
	.long	clk
	.value	0xEE00,0xFFC0
	.long	dev1
	.value	0xEE00,0xFFC0
	.long	dev9
	.value	0xEE00,0xFFC0
	.long	dev3
	.value	0xEE00,0xFFC0
	.long	dev4
	.value	0xEE00,0xFFC0
	.long	dev5
	.value	0xEE00,0xFFC0
	.long	dev6
	.value	0xEE00,0xFFC0
	.long	dev7
	.value	0xEE00,0xFFC0
	.org	.+0x240
	.long	dev8
	.value	0xEE00,0xFFC0
	.long	dev9
	.value	0xEE00,0xFFC0
	.long	dev10
	.value	0xEE00,0xFFC0
	.long	dev11
	.value	0xEE00,0xFFC0
	.long	dev12
	.value	0xEE00,0xFFC0
	.long	dev13
	.value	0xEE00,0xFFC0
	.long	dev14
	.value	0xEE00,0xFFC0
	.long	dev15
	.value	0xEE00,0xFFC0
	.org	.+0x40
bdtend:
