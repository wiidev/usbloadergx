/*   
	Custom IOS module for Wii.
    Copyright (C) 2008 neimod.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

	.section ".init"
	.global _start

	.align	4
	.arm

/*******************************************************************************
 *
 * crt0.s - IOS module startup code
 *
 *******************************************************************************
 *
 *
 * v1.0 - 26 July 2008				- initial release by neimod
 * v1.1 - 5 September 2008			- prepared for public release
 *
 */

	// DIP values
	//.EQU	ios_thread_arg, 3
	//.EQU	ios_thread_priority,	0x54
	// OH0 values
	.EQU	ios_thread_arg, 4
	.EQU	ios_thread_priority,	0x79
	.EQU	ios_thread_stacksize, 0x1000
	
// WARNING: You cannot change this code !!!!
_start:	
	b _start_2
	
	
        .global table_jump_ext
	.thumb_func
table_jump_ext:
        .code 16
	.align 2
	b ES_ioctlv_ret__
	nop
	b IRQ_9
	nop
	.code 32
	b swi_vector
	

        .global ES_ioctlv_ret__
	.thumb_func
ES_ioctlv_ret__:
	.code 16
	.align 2

	ldr	r1, = ES_ioctlv_vect
	ldr	r1, [r1]
	nop
	cmp	r1, #0
	beq	ES_ioctlv_ret
	bx	r1

/* return to dev/es ioctlv routine */

	.global ES_ioctlv_ret
	.thumb_func
	.code 16
ES_ioctlv_ret:
	push	{r4-r6,lr}
	sub	sp, sp, #0x20
	ldr r5, [r0,#8]
	add r1, r0, #0
	ldr r3, = 0x201000D5
	bx r3

// to call one far function in system mode using os_software_IRQ(9)	
         .code 16
	.global IRQ_9
	.thumb_func
IRQ_9:
	
	push    {r6}
	bl	call_system
	add	r5, r0, #0
	pop     {r6}

irq9_1:
	bl	exit_irq9
	add r0, r5, #0
	
	pop     {r4-r6}
	pop     {r1}
        bx      r1


	.code 16
exit_irq9:
	.thumb_func
	bx pc
	.align 4
	.code 32
	
	add     r0, r6, #0
	mrs     r1, cpsr
	bic     r1, r1, #0xc0
	orr     r1, r1, r0
	msr     cpsr_c, r1
	bx      lr

// END of critic area
	.align 4
	.code 32
_start_2:

	ldr sp, =ios_thread_stack
	
	/* Execute main program */
	mov		r0, #0						@ int argc
	mov		r1, #0						@ char *argv[]
	ldr		r3, =main
	bx		r3

	.align 4
	.code 32
        .global direct_syscall
direct_syscall:
	ldr     r12, =syscall_base
	ldr	r12, [r12]
	nop
	ldr     r12, [r12,r11,lsl#2]
	nop
	bx	r12

	.align 4
	.code 32
	.global direct_os_sync_before_read
direct_os_sync_before_read:


	mov     r11, #0x3f
	b	direct_syscall
	
	.align 4
	.code 32
	.global direct_os_sync_after_write
direct_os_sync_after_write:

	
	mov     r11, #0x40
	b	direct_syscall

	.align 4
	.code 32
	.global read_access_perm
read_access_perm:
	mrc     p15, 0, r0,c3,c0
	bx	lr

	.align 4
	.code 32
	.global write_access_perm
write_access_perm:
	mcr     p15, 0, r0,c3,c0
	bx	lr

	.align 4
	.code 32
	.global ic_invalidate
ic_invalidate:
	mov		r0, #0
	mcr		p15, 0, r0, c7, c5, 0
	bx		lr

// to exit in system mode using os_software_IRQ(10)
	.global swi_vector
	.code 16
swi_vector:
	bx	 pc
	.code 32
	ldr	sp, =swi_stack
	nop
	stmfd	sp!, {r1-r12,lr}
	nop
	mrs	r12, cpsr
	stmfd	sp!, {r12}
	nop

	ldr	r12,=swi_intr_addr
	str	lr, [r12]
	nop

	bl	_swi_handler_
	
	ldmfd	sp!, {r12}
	nop
	msr	cpsr_c, r12
	ldmfd	sp!, {r1-r12,lr}
	nop

	movs	pc, lr

_swi_handler_:
	ldr r12, =swi_handler
	bx r12
	

	.thumb_func
	.align
	.pool

	.pool

	/******************************************************************************
 *
 * IOS mem_exe for load modules (512KB)
 *
 *
 ******************************************************************************
 */
       


	.section ".mem_exe" ,"aw",%progbits 
	.global mem_exe
mem_exe:
	.space	0x80000



/*******************************************************************************
 *  IOS data section
 *
 *  Basically, this is required for the program header not to be messed up
 *  The program header will only be generated correctly if there is "something"
 *  in the ram segment, this makes sure of that by placing a silly string there.
 *******************************************************************************
 */
	.section ".ios_data" ,"aw",%progbits 
	.ascii  "IOS module"
	
	
/*******************************************************************************
 *  IOS bss section
 *
 *  This contains the module's thread stack
 *******************************************************************************
 */
	.section ".ios_bss", "a", %nobits
	.global ios_thread_stack_start  /* stack address decrease.. */
ios_thread_stack_start:
	.space	ios_thread_stacksize
	.global ios_thread_stack  /* stack address decrease.. */
ios_thread_stack:
	.space	0x900
swi_stack:
	
	.section ".ios_info_table","ax",%progbits

/*******************************************************************************
 *  IOS info table section
 *
 *  This contains the module's loader information
 *  The stripios tool will find this, and package it nicely for the IOS system
 *******************************************************************************
 */	
	.global ios_info_table
ios_info_table:
	
	.long	0x0
	.long	0x28		@ numentries * 0x28
	.long	0x6	
	.long	0xB
	.long	ios_thread_arg	@ passed to thread entry func, maybe module id
	.long	0x9
	.long	_start
	.long	0x7D
	.long	ios_thread_priority
	.long	0x7E
	.long	ios_thread_stacksize
	.long	0x7F
	.long	ios_thread_stack
	
	

	.end
