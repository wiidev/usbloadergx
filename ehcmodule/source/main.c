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


/*******************************************************************************
 *
 * main.c - IOS module main code
 *
 *******************************************************************************
 *
 *
 * v1.0 - 26 July 2008				- initial release by neimod
 * v1.1 - 5 September 2008			- prepared for public release
 *
 */



#include <stdio.h>
#include <string.h>
#include "syscalls.h"
#include "swi_mload.h"

int tiny_ehci_init(void);

//int ehc_loop(void);

u8 heap_space2[0xe000] __attribute__ ((aligned (32)));

/* USB timer */


int timer1_queuehandle=-1;
int timer1_id=-1;

void ehci_usleep(u32 time)
{
static u32 message;
//int n;

	//os_message_queue_send(timer1_queuehandle, 0x555, 0);
	//os_restart_timer(timer1_id, time);
	timer1_id=os_create_timer(time, 1000*1000*10, timer1_queuehandle, 0x0);
    os_message_queue_receive(timer1_queuehandle,(void *) &message, 0);
	os_stop_timer(timer1_id);
	os_destroy_timer(timer1_id);

}

void ehci_msleep(int msec)
{
	ehci_usleep(((u32) msec)*1000);
}


#define get_timer()  (*(((volatile u32*)0x0D800010)))

void ehci_udelay(int usec)
{
        u32 tmr,temp;
		u32 time_usec;

        tmr = get_timer();
        time_usec=2*usec;
		
        while (1) {temp=get_timer()-tmr;if(((int) temp)<0) tmr = get_timer(); if(((int)temp) > time_usec) break;}
		
}
void ehci_mdelay(int msec)//@todo not really sleeping..
{
        u32 tmr,temp;
		u32 time_usec;

        tmr = get_timer();
        time_usec=2048*msec;

        while (1) {temp=get_timer()-tmr;if(((int) temp)<0) tmr = get_timer(); if(((int)temp) > time_usec) break;}
		

}




int ehc_loop(void);

int heaphandle=-1;
unsigned int heapspace[0x5000/*0x8800*/] __attribute__ ((aligned (32)));


void interrupt_vector(void);
void patch1_timer(void);
void patch2_timer_cont(void);
void int_send_device_message(u32);

void direct_os_sync_before_read(void* ptr, int size);
void direct_os_sync_after_write(void* ptr, int size);
void ic_invalidate(void);

static u32 vector[2]={ 0xE51FF004, 0}; // ldr pc,=addr

u32 syscall_base;


int my_di_os_message_queue_receive(int queuehandle, ipcmessage ** message,int flag);

u32 read_access_perm(void);
void write_access_perm(u32 flags);

static void di_patch(u32 addr1, u32 addr2)
{
u32 perm;

	perm=read_access_perm();
	write_access_perm(0xffffffff);
	
	if(*((u32 *) addr2)==0xE6000170) // detect an unused syscall in dev/di to store the entry
		{
		vector[1]= ((u32) my_di_os_message_queue_receive) | 1;
		memcpy((void *) addr2, vector, 8);
		direct_os_sync_after_write((void *) addr2, 8);

		*((u32 *) addr1)= 0xEA000000 | (((addr2-addr1)/4-2) & 0xFFFFFF); // change the jump
		direct_os_sync_after_write((void *) addr1, 4);
		}

	write_access_perm(perm);

}

int copy_int_vect(u32 ios, u32 none)
{
	ic_invalidate();

	switch(ios)
	{
	case 36:
    // WARNING!!!: IOS 36 ins not recommended because it fails using the ehcmodule some times
		vector[1]= (u32) interrupt_vector;
		
		memcpy((void *) 0xFFFF1E78, vector,8); // fix interrupt jump
		direct_os_sync_after_write((void *)  0xFFFF1E78, 8);
		break;

	case 37:

		// patch for DI (IOS37 v3869) os_message_queue_receive() syscalls
		di_patch(0x20205DE8, 0x2020408c);

		vector[1]= (u32) 0xFFFF1F70;
		memcpy((void *) patch1_timer, vector,8); // patch1 -> timer
		direct_os_sync_after_write((void *) patch1_timer, 8);

		vector[1]= (u32) 0xFFFF1F8C;
		memcpy((void *) patch2_timer_cont, vector,8); // patch2-> next interrupt case
		direct_os_sync_after_write((void *) patch2_timer_cont, 8);

		vector[1]= (u32) 0xFFFF1E34;
		memcpy((void *) int_send_device_message, vector,8); // patch3 ->send device message
		direct_os_sync_after_write((void *) int_send_device_message, 8);

		vector[1]= (u32) interrupt_vector;
		
		memcpy((void *) 0xFFFF1F68, vector,8); // fix interrupt jump
		direct_os_sync_after_write((void *)  0xFFFF1F68, 8);
		break;

	case 38:
		
		// patch for DI (IOS38 v3867) os_message_queue_receive() syscalls
		di_patch(0x20205B14, 0x20203E6C);

		vector[1]= (u32) 0xFFFF1EB0;
		memcpy((void *) patch1_timer, vector, 8); // patch1 -> timer
		direct_os_sync_after_write((void *) patch1_timer, 8);

		vector[1]= (u32) 0xFFFF1ECC;
		memcpy((void *) patch2_timer_cont, vector, 8); // patch2-> next interrupt case
		direct_os_sync_after_write((void *) patch2_timer_cont, 8);

		vector[1]= (u32) 0xFFFF1D74;
		memcpy((void *) int_send_device_message, vector, 8); // patch3 ->send device message
		direct_os_sync_after_write((void *) int_send_device_message, 8);

		vector[1]= (u32) interrupt_vector;
		
		memcpy((void *) 0xFFFF1EA8, vector,8);
		direct_os_sync_after_write((void *)  0xFFFF1EA8, 8);
		break;

	case 57:
		// patch for DI (IOS57 v5661) os_message_queue_receive() syscalls
		di_patch(0x20205E84, 0x20203F60);

		vector[1]= (u32) 0xFFFF2130;
		memcpy((void *) patch1_timer, vector,8); // patch1 -> timer
		direct_os_sync_after_write((void *) patch1_timer, 8);

		vector[1]= (u32) 0xFFFF214C;
		memcpy((void *) patch2_timer_cont, vector,8); // patch2-> next interrupt case
		direct_os_sync_after_write((void *) patch2_timer_cont, 8);

		vector[1]= (u32) 0xFFFF1FF4;
		memcpy((void *) int_send_device_message, vector,8); // patch3 ->send device message
		direct_os_sync_after_write((void *) int_send_device_message, 8);

		vector[1]= (u32) interrupt_vector;
		
		memcpy((void *) 0xFFFF2128, vector,8); // fix interrupt jump
		direct_os_sync_after_write((void *)  0xFFFF2128, 8);
		break;

	case 60:

		// patch for DI (IOS60 v6174) os_message_queue_receive() syscalls
		di_patch(0x20205D94, 0x20203F60);

		vector[1]= (u32) 0xFFFF2130;
		memcpy((void *) patch1_timer, vector,8); // patch1 -> timer
		direct_os_sync_after_write((void *) patch1_timer, 8);

		vector[1]= (u32) 0xFFFF214C;
		memcpy((void *) patch2_timer_cont, vector,8); // patch2-> next interrupt case
		direct_os_sync_after_write((void *) patch2_timer_cont, 8);

		vector[1]= (u32) 0xFFFF1FF4;
		memcpy((void *) int_send_device_message, vector,8); // patch3 ->send device message
		direct_os_sync_after_write((void *) int_send_device_message, 8);

		vector[1]= (u32) interrupt_vector;
		
		memcpy((void *) 0xFFFF2128, vector,8); // fix interrupt jump
		direct_os_sync_after_write((void *)  0xFFFF2128, 8);
		break;

	}

		//*((volatile u32 *)0x0d8000c0) |=0x20;

return 0;
}

extern char use_usb_port1;
extern u32 current_port;

int main(void)
{

current_port= ((u32) use_usb_port1)!=0;
// changes IOS vector interrupt to crt0.s routine

//swi_mload_led_on();


syscall_base=swi_mload_get_syscall_base();
os_sync_after_write((void *) &syscall_base, 4);

swi_mload_call_func((void *) copy_int_vect, (void *) swi_mload_get_ios_base(), NULL);


heaphandle = os_heap_create(heapspace, sizeof(heapspace));

void* timer1_queuespace = os_heap_alloc(heaphandle, 0x80);

timer1_queuehandle = os_message_queue_create(timer1_queuespace, 32);



    if(tiny_ehci_init()<0) return -1;
	
  
    ehc_loop();

	return 0;
}
