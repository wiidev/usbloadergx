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

	.EQU	ios_thread_arg, 4
	.EQU	ios_thread_priority,	0x78 //0x48
	.EQU	ios_thread_stacksize, 0x3000
	

_start:	

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
	
	.global ic_invalidate
ic_invalidate:
	mov		r0, #0
	mcr		p15, 0, r0, c7, c5, 0
	bx		lr

// bypass to interrupt vector

	.align 4
	.code 32

	.global interrupt_vector
interrupt_vector:
	tst     r8, #0x10
	beq     int_cont1

	bic     r8, r8, #0x10 // disable next EHCI treatment

	mov     r2, #0x10
	str     r2, [r7]
	nop
	
        mov     r2, sp
	nop
	ldr	sp, =_interrupt_stack
	nop
	
	stmfd	sp!, {r1-r12,lr}
	nop

	bl	_ehci_vector_

	ldmfd	sp!, {r1-r12,lr}
	nop
	mov     sp, r2

	tst     r0, #0x1
        beq     int_cont1

	nop
	mov	r0,#4
	bl	int_send_device_message
	
	
int_cont1:
	tst     r8, #0x1
	beq     patch2_timer_cont
// int timer
	.global patch1_timer
patch1_timer:
	ldr pc, =0xFFFF1E80
	nop
	.global patch2_timer_cont
patch2_timer_cont:
	ldr pc, =0xFFFF1E9C
	nop

	.global int_send_device_message
int_send_device_message:
	ldr pc, =0xFFFF1D44
	nop

_ehci_vector_:
	
	ldr	r2,=ehci_vector
	bx	r2

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

/*******************************************************************************
 *
 * DRIVER CONFIGURATION AREA
 *
 *******************************************************************************
 */

	.string "EHC_CFG"
	.long 0x12340001
	.global use_usb_port1
use_usb_port1:
	.byte 0x0

	.global use_reset_bulk
use_reset_bulk:
	.byte 0x0

/* force_flags 1 ->force GetMaxLun, 2-> force SetConfiguration */
	.global force_flags
force_flags:
	.byte 0x0
	
	.global use_alternative_timeout
use_alternative_timeout:
	.byte 0x0
	
	.align
	.pool
	

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
	
	.global ios_thread_stack_start
ios_thread_stack_start:
	.space	ios_thread_stacksize
	.global ios_thread_stack /* stack decrements from high address.. */
ios_thread_stack:
	.space 0x200
_interrupt_stack:
	
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


	.pool
	.end
