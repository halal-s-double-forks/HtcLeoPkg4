/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define FUNCTION(x) .global x; x:

#define DSB .byte 0x4f, 0xf0, 0x7f, 0xf5
#define ISB .byte 0x6f, 0xf0, 0x7f, 0xf5
#define WSPL_VADDR 0x80000000

.text
.globl _start
_start:
	b	reset
	b	relocate

//if EDK2 is loaded by wince spl add romhdr
.org 0x40
	.word 0x43454345
	.word (romhdr-_start)+WSPL_VADDR  // virtual address of romhdr
	.word romhdr-_start               // file address of romhdr

.org 0x00000900
romhdr:
	.word 0x2000000             // dllfirst
	.word 0x2000000             // dlllast
	.word WSPL_VADDR            // physfirst
	.word WSPL_VADDR+0x000F0000 // physlast (using hardoded FV size, fix later?)
	.word 0                     // nummods (no TOCentry after ROMHDR)
	.word WSPL_VADDR+0x000F0000 // ulRAMStart
	.word WSPL_VADDR+0x000F0000 // ulRAMFree
	.word WSPL_VADDR+0x001F0000 // ulRAMEnd
	.word 0                     // ulCopyEntries
	.word 0                     // ulCopyOffset
	.word 0                     // ulProfileLen
	.word 0                     // ulProfileOffset
	.word 0                     // numfiles
	.word 0                     // ulKernelFlags
	.word 0x80808080            // ulFSRamPercent
	.word 0                     // ulDrivglobStart
	.word 0                     // ulDrivglobLen
	.hword 0x1C2                // usCPUType
	.hword 0x2                  // usMiscFlags
	.word 0                     // pExtensions
	.word 0                     // ulTrackingStart
	.word 0                     // ulTrackingLen
.org 0x00001000

reset:
	/* do some cpu setup */
	mrc	p15, 0, r0, c1, c0, 0
	/* XXX this is currently for arm926, revist with armv6 cores */
	/* new thumb behavior, low exception vectors, i/d cache disable, mmu disabled */
	bic	r0, r0, #(1<<15| 1<<13 | 1<<12)
	bic	r0, r0, #(1<<2 | 1<<0)
	mcr	p15, 0, r0, c1, c0, 0

	DSB
	ISB

	// htcleo_boot_s

	// cedesmith:
	// cotulla's code so kernel will not crash. aux control register
	// found more info here: http://www.spinics.net/lists/linux-arm-msm/msg00492.html
	// it looks it is Martijn Stolk's code
	mrc p15, 0, r0, c1, c0, 1
	bic r0, r0, #0x40					//; (1<<6)  IBE (0 = executes the CP15 Invalidate All and Invalidate by MVA instructions as a NOP instruction, reset value)
	bic r0, r0, #0x200000				//; (1<<21) undocumented bit
	mcr p15, 0, r0, c1, c0, 1

	//; ICIALL to invalidate entire I-Cache
    mcr    p15, 0, r0, c7, c5, 0    //; ICIALLU

    // disable dcache and i cache
	mrc p15, 0, r0, c1, c0, 0
	bic r0, r0, #(1<<0)					// disable mmu ( already disabled )
	bic r0, r0, #(1<<2)					// disable data cache
	bic r0, r0, #(1<<12)				// disable instruction cache
	mcr p15, 0, r0, c1, c0, 0
	ISB

	//; DCIALL to invalidate L2 cache bank (needs to be run 4 times, once per bank)
    //; This must be done early in code (prior to enabling the caches)
    mov    r0, #0x2
    mcr    p15, 0, r0, c9, c0, 6   //; DCIALL bank D ([15:14] == 2'b00)
    orr    r0, r0, #0x00004000
    mcr    p15, 0, r0, c9, c0, 6   //; DCIALL bank C ([15:14] == 2'b01)
    add    r0, r0, #0x00004000
    mcr    p15, 0, r0, c9, c0, 6   //; DCIALL bank B ([15:14] == 2'b10)
    add    r0, r0, #0x00004000
    mcr    p15, 0, r0, c9, c0, 6   //; DCIALL bank A ([15:14] == 2'b11)
	//; DCIALL to invalidate entire D-Cache
	mov    r0, #0
    mcr    p15, 0, r0, c9, c0, 6    //; DCIALL  r0
	DSB
	ISB

	//; Invalidate the UTLB
    mov r0, #0
    mcr    p15, 0, r0, c8, c7, 0    //; UTLBIALL
    ISB
.ltorg

.code 32
.text
_relocate:
	/* see if we need to relocate */
	adr	r0, _Payload 	// load the payload adress into r0

	ldr	r1, UefiBase 	// load the expected payload base into r1
	cmp	r0, r1		 	// compare the adresses
	beq	_Entry			// jump to entry if the adress is the same
	ldr	r2, UefiSize

_CopyLoop:
	/* memcpy */
	ldr	r3, [r0], #4
	str	r3, [r1], #4
	subs	r2, r2, #4	// substract the size that got copied from fd size
	bgt	_CopyLoop		// branch if greater than zero

	/* we're relocated, jump to the right address */

_Entry:
	ldr		r5, UefiBase
	bx		r5

UefiBase:
	.word UEFI_BASE

UefiSize:
	.word UEFI_SIZE

.ltorg

.text
.align 4
_Payload: