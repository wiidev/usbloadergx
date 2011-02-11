/*   
	dev/mload: Custom IOS module for Wii, to load ios elfs, initialize USB 2.0 and others uses
	This module is derived from haxx.elf
	Copyright (C) 2009-2010 Hermes.
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
 * v1.2 - march  2008				- added some IOTCL, put it into its own module, by kwiirk
 *
 */


#include <stdio.h>
#include <string.h>
#include "syscalls.h"
#include "swi_mload.h"

#define MLOAD_VER	 5
#define MLOAD_SUBVER 2
#define STR_VERSION "dev/mload v5.2 (c) 2009-2010, Hermes\n"

#define IOS_OPEN				0x01
#define IOS_CLOSE				0x02
#define IOS_READ				0x03
#define IOS_WRITE				0x04
#define IOS_SEEK				0x05
#define IOS_IOCTL				0x06
#define IOS_IOCTLV				0x07

#define MLOAD_MLOAD_THREAD_ID	 0x4D4C4400
#define MLOAD_GET_IOS_BASE	     0x4D4C4401
#define MLOAD_GET_MLOAD_VERSION  0x4D4C4402

#define MLOAD_LOAD_MODULE		0x4D4C4480
#define MLOAD_RUN_MODULE		0x4D4C4481
#define MLOAD_RUN_THREAD        0x4D4C4482

#define MLOAD_STOP_THREAD		0x4D4C4484
#define MLOAD_CONTINUE_THREAD   0x4D4C4485

#define MLOAD_GET_LOAD_BASE     0x4D4C4490
#define MLOAD_MEMSET			0x4D4C4491

#define MLOAD_GET_EHCI_DATA		0x4D4C44A0
#define MLOAD_GET_LOG			0x4D4C44A1

#define MLOAD_SET_ES_IOCTLV		0x4D4C44B0
#define MLOAD_SET_SYSTEM_FUNC	0x4D4C44B1

#define MLOAD_GETW				0x4D4C44C0
#define MLOAD_GETH				0x4D4C44C1
#define MLOAD_GETB				0x4D4C44C2
#define MLOAD_SETW				0x4D4C44C3
#define MLOAD_SETH				0x4D4C44C4
#define MLOAD_SETB				0x4D4C44C5

#define DEVICE "/dev/mload"


u32 IOS_BASE=0;
u8 ES_patch_ioctvl[8] = {
	0x49, 0x00, 0x47, 0x08, /* addr in mload.elf */ 0x13, 0x8c, 0x00, 0x4+1 // (Thumb)
};


extern void direct_syscall(void);
u32 syscall_base=0;

extern int ES_ioctlv_ret(void *);

unsigned ES_ioctlv_vect=((unsigned) ES_ioctlv_ret);

int (*system_mode_func)(void)=0;

unsigned int heapspace[0x100/4] __attribute__ ((aligned (32)));

// from IOS ELF stripper of neimod

typedef struct 
{
        u32		ident0;
		u32		ident1;
		u32		ident2;
		u32		ident3;
        u32		machinetype;
        u32		version;
        u32		entry;
        u32     phoff;
        u32     shoff;
        u32		flags;
        u16     ehsize;
        u16     phentsize;
        u16     phnum;
        u16     shentsize;
        u16     shnum;
        u16     shtrndx;
} elfheader;

typedef struct 
{
       u32      type;
       u32      offset;
       u32      vaddr;
       u32      paddr;
       u32      filesz;
       u32      memsz;
       u32      flags;
       u32      align;
} elfphentry;

#define ioctlv_u8(a) (*((u8*)(a).data))
#define ioctlv_u16(a) (*((u16*)(a).data))
#define ioctlv_u32(a) (*((u32*)(a).data))
#define ioctlv_voidp(a) (a).data

extern u8 *mem_exe; // size 0x80000 (see crt0.s)


struct _data_elf
{
	void *start;
	int prio;
	void *stack;
	int size_stack;
}
data_elf;

#define getbe32(x) ((adr[x]<<24) | (adr[x+1]<<16) | (adr[x+2]<<8) | (adr[x+3]))

int load_elf(u32 elf);

u8 *text_log;
int text_pos=0;
int text_limit=4096;

extern void *ehci;
int tiny_ehci_init(void);


extern void swi_vector(void);
extern void load_swi_stack(void);


void direct_os_sync_before_read(void* ptr, int size);
void direct_os_sync_after_write(void* ptr, int size);
void ic_invalidate(void);

u32 read_access_perm(void);
void write_access_perm(u32 flags);

void find_and_patch_es(void)
{
u16 *addr=(u16 *) 0x13A74F00;

	while(((u32) addr)<0x13A76F00)
		{
		if(addr[0]==0x2007)
			{
			if(addr[1]==0x23A2 || addr[1]==0x4B0B) {addr[0]=0x2000;direct_os_sync_after_write((void *) addr, 2);break;}
			
			}
		addr++;
		}

}

// call IRQ_software(9) syscall to go here

int call_system(void)
{
static int one=1;


	if(system_mode_func) return system_mode_func();

    // hack to add SWI vector
	if(one)
	{
	u32 temp;
	one=0;

	IOS_BASE=0;

        temp=*((volatile u32 *) 0xFFFF0028);
		
		if(temp==0xFFFF1C70)		{IOS_BASE=36;syscall_base= 0xFFFF8980;}
		else if(temp==0xFFFF1D60)	{IOS_BASE=37;syscall_base= 0xFFFF91B0;}
		else if(temp==0xFFFF1CA0)	{IOS_BASE=38;syscall_base= 0xFFFF8AA0;}
		else if(temp==0xFFFF1F20)	
				{
				temp=*((volatile u32 *) 0xFFFF00FC);
				if(temp==0xFFFFD004)
					 {IOS_BASE=57;syscall_base= 0xFFFF9390;}
				else
				if(temp==0xFFFFCE24)
					{IOS_BASE=60;syscall_base= 0xFFFF9390;}
				}
	
	ic_invalidate();

	temp=read_access_perm();
	write_access_perm(0xffffffff);

	direct_os_sync_after_write((void *) &syscall_base, 4);

    // SWI patch

	*((volatile u32 *) 0xFFFF0028)=((u32) swi_vector);

	direct_os_sync_after_write((void *) 0xFFFF0028, 4);

	

	find_and_patch_es();

   
    // ES PATCH
	switch(IOS_BASE)
		{
		case 36:
		
			// patch 1
			
			*((u16 *) 0x13A75026)= 0xE000; 
			direct_os_sync_after_write((void *) 0x13A75026, 2);
			

			// patch 2 
			*((u16 *) 0x20102710)=0xe001; 
			direct_os_sync_after_write((void *) 0x20102710, 2);

			// patch 3 
			*((u16 *) 0x20104F5E)=0x46c0; 
			direct_os_sync_after_write((void *) 0x20104F5E, 2);

			// patch 4 
			*((u16 *) 0x201075EE)=0xe000; 
			direct_os_sync_after_write((void *) 0x201075EE, 2);
			
			break;
		
		case 37:
			/*
			// patch 1 
			*((u16 *) 0x20100D4A)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100D4A, 2);
			*((u16 *) 0x20100DC2)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100DC2, 2);
			

			// patch 2 
			*((u16 *) 0x201027A8)=0xd201; 
			direct_os_sync_after_write((void *) 0x201027A8, 2);

			// patch 3 
			*((u16 *) 0x201051A6)=0x46c0; 
			direct_os_sync_after_write((void *) 0x201051A6, 2);

			// patch 4 
			*((u16 *) 0x20107A9E)=0xe000; 
			direct_os_sync_after_write((void *) 0x20107A9E, 2);
			*/

			


			// patch 1 
			*((u16 *) 0x20100D4A)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100D4A, 2);
			*((u16 *) 0x20100DC2)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100DC2, 2);

			//*((u16 *) 0x20100D64)= 0x429A; 
			//direct_os_sync_after_write((void *) 0x20100D64, 2);

			// FFS access
			*((u16 *) 0x200012F2)= 0xE001; 
			direct_os_sync_after_write((void *) 0x200012F2, 2);
			

			// patch 2 
			*((u16 *) 0x201027AC)=0xd201; 
			direct_os_sync_after_write((void *) 0x201027AC, 2);

			// patch 3 
			*((u16 *) 0x2010522A)=0x46c0; 
			direct_os_sync_after_write((void *) 0x2010522A, 2);

			// patch 4 
			*((u16 *) 0x20107B22)=0xe000; 
			direct_os_sync_after_write((void *) 0x20107B22, 2);

			// patch 5 
			*((u16 *) 0x20105FC0)=0xe000; 
			direct_os_sync_after_write((void *) 0x20105FC0, 2);



			break;

		case 38:

			// patch 1 
			*((u16 *) 0x20100CC8)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100CC8, 2);

			*((u16 *) 0x20100D40)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100D40, 2);

			// patch 2 
			*((u16 *) 0x20102724)=0xd201; // 0x20102720
			direct_os_sync_after_write((void *) 0x20102724, 2);

			// patch 3 
			*((u16 *) 0x20104FF2)=0x46c0; //0x20104F6E
			direct_os_sync_after_write((void *) 0x20104FF2, 2);

			// patch 4 
			*((u16 *) 0x20107682)=0xe000; //0x201075FE
			direct_os_sync_after_write((void *) 0x20107682, 2);

			// FFS access
			*((u16 *) 0x2000347E)= 0xE001; 
			direct_os_sync_after_write((void *) 0x2000347E, 2);


			break;

		case 57:

			/*// patch 1 
			*((u16 *) 0x20100DA4)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100DA4, 2);
			*((u16 *) 0x20100E1C)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100E1C, 2);

			// patch 2 
			*((u16 *) 0x20102800)=0xd201; 
			direct_os_sync_after_write((void *) 0x20102800, 2);

			// patch 3 
			*((u16 *) 0x2010523A)=0x46c0; 
			direct_os_sync_after_write((void *) 0x2010523A, 2);

			// patch 4 
			*((u16 *) 0x20107B32)=0xe000; 
			direct_os_sync_after_write((void *) 0x20107B32, 2);
			*/

			// patch 1 
			*((u16 *) 0x20100E74)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100E74, 2);
			*((u16 *) 0x20100EEC)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100EEC, 2);

			// patch 2 
			*((u16 *) 0x20102C74)=0xd201; 
			direct_os_sync_after_write((void *) 0x20102C74, 2);

			// patch 3 
			*((u16 *) 0x2010576A)=0x46c0; 
			direct_os_sync_after_write((void *) 0x2010523A, 2);

			// patch 4 
			*((u16 *) 0x2010849A)=0xe000; 
			direct_os_sync_after_write((void *) 0x2010849A, 2);

			// patch 5 
			*((u16 *) 0x2010650C)=0xe000; 
			direct_os_sync_after_write((void *) 0x2010650C, 2); // ES_DECRYPT pass

			// FFS access
			*((u16 *) 0x20001306)= 0xE001; 
			direct_os_sync_after_write((void *) 0x20001306, 2);


			break;

		case 60:

			// patch 1 
			*((u16 *) 0x20100DA4)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100DA4, 2);
			*((u16 *) 0x20100E1C)= 0x2803; 
			direct_os_sync_after_write((void *) 0x20100E1C, 2);

			// patch 2 
			*((u16 *) 0x20102800)=0xd201; 
			direct_os_sync_after_write((void *) 0x20102800, 2);

			// patch 3 
			*((u16 *) 0x2010523A)=0x46c0; 
			direct_os_sync_after_write((void *) 0x2010523A, 2);

			// patch 4 
			*((u16 *) 0x20107B32)=0xe000; 
			direct_os_sync_after_write((void *) 0x20107B32, 2);

			// patch 5 
			*((u16 *) 0x20105FD0)=0xe000; 
			direct_os_sync_after_write((void *) 0x20105FD0, 2); // ES_DECRYPT pass

			// FFS access
			*((u16 *) 0x20001306)= 0xE001; 
			direct_os_sync_after_write((void *) 0x20001306, 2);

			break;

		}

	// ES ioctlv patch
	direct_os_sync_after_write((void *) ES_ioctlv_vect,4);

	memcpy((void *) 0x201000CC, (void *) ES_patch_ioctvl, 8);

	direct_os_sync_after_write((void *) 0x201000CC, 8);
		
    write_access_perm(temp);
		
	}

return 0x555;
}


// SWI handler

u8 * swi_intr_addr;

int (*swi_table[256]) (u32 arg0, u32 arg1,u32 arg2, u32 arg3);

int (* swi_func)(void * in, void * out);

int swi_handler(u32 arg0, u32 arg1,u32 arg2, u32 arg3)
{

// detect SWI instruction
 
	// 32 bits aligned
	if(swi_intr_addr[-4]==0xdf) swi_intr_addr-=3;  // 16 bits function
			else swi_intr_addr--; // 32 bits function



	if(*(swi_intr_addr)==0xcc)
		{

		switch(arg0)
			{
			// add SWI handler
			case 0:
				swi_table[arg1]= (void *) arg2;
				break;
			// get EHCI DATA
			case 1:
				return (int) ehci;
			// memcpy ( RAM cached to cached)
			case 2:
				{
				u32 temp;
				temp=read_access_perm();
				write_access_perm(0xffffffff);
				memcpy((void *) arg1, (void *) arg2, arg3);
				direct_os_sync_after_write((void *) arg1, arg3);
				write_access_perm(temp);
				}
				break;
			// get register
			case 3:
				return *((volatile u32 *) arg1);
			// put register
			case 4:
				*((volatile u32 *) arg1)=arg2;
				break;
			// set register
			case 5:
				*((volatile u32 *) arg1)|=arg2;
				break;

			// clr register
			case 6:
				*((volatile u32 *) arg1)&=~arg2;
				break;

            // function to call in os_software_IRQ(9)
			case 7:
				system_mode_func=(void *) arg1;
				break;

            // log buffer function
			case 8:
				switch(arg1)
				{
				case 1:
					memset( (void *) text_log, 0, text_limit);
					text_pos=0;
					break;
				case 2:
					 text_pos= 0;
				     text_log= (void *) arg2;
					 text_limit= (int) arg3;
					 memset( (void *) text_log, 0, text_limit);
					 break;
				}
				return (int) text_log;
			case 9:
				// memcpy ( RAM uncached to cached)
				{
				u32 temp;
				temp=read_access_perm();
				write_access_perm(0xffffffff);
				direct_os_sync_before_read((void *) arg2, arg3);
				memcpy((void *) arg1, (void *) arg2, arg3);
				direct_os_sync_after_write((void *) arg1, arg3);
				write_access_perm(temp);
				}
				break;

			// call func
			case 16:
				swi_func=  (void *) arg1;
				return  swi_func((void *) arg2, (void *) arg3);

			// get syscalls base
			case 17:
				return ((int) syscall_base);

			// get IOS base
            case 18:
				return ((int) IOS_BASE);

			// get mload version
            case 19:
				return ((MLOAD_VER<<4)+MLOAD_SUBVER);
				
			// led on
			case 128:
				*((volatile u32 *)0x0d8000c0) |=0x20;
				break;
			// led off
			case 129:
				*((volatile u32 *)0x0d8000c0) &=~0x20;
				break;
			// led blink
			case 130:
				*((volatile u32 *)0x0d8000c0) ^=0x20;
				break;


			// test
			case 200:
				if(arg3==0x666)
					*((volatile u32 *)0x0d8000c0) |=0x20;
				break;
 
			}
		}
	else
		{
		if(swi_table[*(swi_intr_addr)]) return swi_table[*(swi_intr_addr)](arg0, arg1, arg2, arg3);
		else return arg0;
		}

return 0;
}

int swi_handler_text(u32 arg0, u32 arg1,u32 arg2, u32 arg3)
{
int n,max;
u8 *p;

	if(arg0!=4) return (int) arg0;

	p=(u8 *) arg1;

	for(n=0;n<40*3;n++) if(p[n]==0) break;

	max=(text_limit/40)*40;

	if((text_pos+n)>(max))
		{
		text_pos=max-n;
		memcpy((void *) &text_log[0],(void *) &text_log[n], max-n);
		
		}

	memcpy((void *) &text_log[text_pos], (void *) p, n);
    text_pos+=n;

return 0;
}

int shadow_mload=0;

int main(void)
{
	ipcmessage* message;
    unsigned int offset = 0;

	
    
	mem_exe[0]=0; // don't remove this !!!!!
	

	tiny_ehci_init();

	unsigned int heaphandle = os_heap_create(heapspace, sizeof(heapspace));
	void* queuespace = os_heap_alloc(heaphandle, 0x20);

	unsigned int queuehandle = os_message_queue_create(queuespace, 8);

	memset( (void *) swi_table,0, 256);
	swi_table[0xab]= swi_handler_text;
	memset( (void *) text_log, 0, text_limit);
	

	os_software_IRQ(9); // patch the SWI vector (see call_system())

	os_device_register(DEVICE, queuehandle);

	os_puts(STR_VERSION);
	
	while(1)
	{	
		int result = 1;
		int ack = 1;

		// Wait for message to arrive
		os_message_queue_receive(queuehandle, (void*)&message, 0);

		switch( message->command )
		{
			case IOS_OPEN:
			{
                                //debug_printf("%s try open %sfor fd %d\n",DEVICE,message->open.device,message->open.resultfd);
				// Checking device name
				if (0 == strcmp(message->open.device, DEVICE))
					{
					if(shadow_mload) result=-6;
					else result = message->open.resultfd;        
					}
				else
				if (0 == strcmp(message->open.device, DEVICE"/OFF"))
					{
					shadow_mload=1;
					result=-6;
					}
				
				else
					result = -6;
			}	
			break;

			case IOS_CLOSE:
			{
                            
				// do nothing
				result = 0;
			}	
			break;

			case IOS_READ:
			{
				// Read from Starlet memory
				
				#if 0
				// NOTE: no aligned is better
				memcpy(message->read.data, (void*)offset, message->read.length);
				// Clean cache
				os_sync_after_write( message->read.data, message->read.length );
				#else
				swi_mload_memcpy_from_uncached((void *) message->read.data, (void*)offset, message->read.length);
				offset += message->read.length;
				#endif
			}	
			break;

			case IOS_WRITE:
			{
				// Write to Starlet memory
				// Invalidate cache
			#if 0
				os_sync_before_read( message->write.data, message->write.length );
				memcpy((void*)offset, message->write.data, message->write.length);
			#else
			swi_mload_memcpy((void*)offset, message->write.data, message->write.length);
			offset += message->write.length;
			#endif
			}	
			break;

			case IOS_SEEK:
			{
				// Change current offset
				switch(message->seek.origin)
				{	
					case SEEK_SET:
					{
						offset = message->seek.offset;
						break;
					}

					case SEEK_CUR:
					{
						offset += message->seek.offset;
						break;
					}

					case SEEK_END:
					{
						offset = - message->seek.offset;
						break;
					}
				}
			result=offset;
			}	
			break;


			case IOS_IOCTL:
			{
			
                break;
            }

			case IOS_IOCTLV:
			{
                                ioctlv *vec = message->ioctlv.vector;

                                int i,in = message->ioctlv.num_in,io= message->ioctlv.num_io;
                               
                                os_sync_before_read( vec, (in+io)*sizeof(ioctlv));

                                for(i=0;i<in+io;i++){
                                        os_sync_before_read( vec[i].data, vec[i].len);
                                      
                                }

                                switch( message->ioctl.command )
                                {
								
								case MLOAD_MLOAD_THREAD_ID:
										
										result=os_get_thread_id();
										
										break;
								case MLOAD_GET_IOS_BASE:
										result= (u32) IOS_BASE;
										break;
								case MLOAD_GET_MLOAD_VERSION:
										result= (u32) ((MLOAD_VER<<4)+MLOAD_SUBVER);
										break;
								
								case MLOAD_GET_EHCI_DATA:

										result= (u32) ehci;
										break;

								case MLOAD_GET_LOAD_BASE:
									    
										result=0;
									    ioctlv_u32(vec[0])= 0x13700000;
										ioctlv_u32(vec[1])= 0x80000;
										break;
								
                                case MLOAD_LOAD_MODULE:

                                        result =  load_elf((u32) ioctlv_voidp(vec[0]));
                                        break;
								
								case MLOAD_RUN_MODULE:

										result=os_thread_create( data_elf.start, NULL, data_elf.stack, data_elf.size_stack, data_elf.prio, 0);
										if(result>=0) os_thread_continue(result);
										
										break;	

								case MLOAD_RUN_THREAD:

										result=os_thread_create((void *) ioctlv_u32(vec[0]), NULL, (void *) ioctlv_u32(vec[1]), ioctlv_u32(vec[2]), ioctlv_u32(vec[3]), 0);
										if(result>=0) os_thread_continue(result);
										
										break;

								case MLOAD_STOP_THREAD:
										
										result=os_thread_stop(ioctlv_u32(vec[0]));

										
										break;
								case MLOAD_CONTINUE_THREAD:
										
										result=os_thread_continue(ioctlv_u32(vec[0]));
										
										break;


								case MLOAD_MEMSET:
										result=0;
										os_sync_before_read((void *) ioctlv_u32(vec[0]), ioctlv_u32(vec[2]));
										memset((void *) ioctlv_u32(vec[0]), ioctlv_u32(vec[1]), ioctlv_u32(vec[2]));
										
										break;

								case MLOAD_SET_ES_IOCTLV: // changes the current vector for dev/es ioctl (put 0 to disable it)
										result=0;
										
										ES_ioctlv_vect=ioctlv_u32(vec[0]);
										os_sync_after_write( &ES_ioctlv_vect, 4);
										break;
										
								case MLOAD_SET_SYSTEM_FUNC:
									    result=0;
										
										system_mode_func=(void *) ioctlv_u32(vec[0]);
										
										break;

								case MLOAD_GET_LOG:
										result=text_limit;
										offset =(unsigned int) text_log;
										break;
								

								case MLOAD_GETW:
										result=0;
										ioctlv_u32(vec[1])=*((volatile u32*) ioctlv_u32(vec[0]));
										break;
								case MLOAD_GETH:
										result=0;
										ioctlv_u16(vec[1])=*((volatile u16*) ioctlv_u32(vec[0]));
										break;
								case MLOAD_GETB:
										result=0;
										ioctlv_u8(vec[1])=*((volatile u8*) ioctlv_u32(vec[0]));
										break;

								case MLOAD_SETW:
										result=0;
										*((volatile u32*) ioctlv_u32(vec[0]))=ioctlv_u32(vec[1]);
										break;
								case MLOAD_SETH:
										result=0;
										*((volatile u16*) ioctlv_u32(vec[0]))=ioctlv_u16(vec[1]);
										break;
								case MLOAD_SETB:
										result=0;
										*((volatile u8*) ioctlv_u32(vec[0]))=ioctlv_u8(vec[1]);
										break;

							
                                }
                                for(i=in;i<in+io;i++){
                                        os_sync_after_write( vec[i].data, vec[i].len);
                                }

                                break;
                        }
			default:
				result = -1;
				//ack = 0;
			break;
		}
                //debug_printf("return %d\n",result);
		// Acknowledge message
		if (ack)
			os_message_queue_ack( (void*)message, result );
	}
   
	return 0;
}


int load_elf(u32 elf)
{
int n,m;
int p;
u8 *adr;

elfheader *head=(void *) elf;
elfphentry *entries;

if(head->ident0!=0x7F454C46) return -1;
if(head->ident1!=0x01020161) return -1;
if(head->ident2!=0x01000000) return -1;

p=head->phoff;

data_elf.start=(void *)  head->entry;

for(n=0; n<head->phnum; n++)
	{
	entries=(void *) (elf+p);
	p+=sizeof(elfphentry);

	if(entries->type == 4)
		{
		adr=(void *) (elf + entries->offset);

        if(getbe32(0)!=0) return -2; // bad info (sure)

		for(m=4; m < entries->memsz; m+=8)
			{
			switch(getbe32(m))
				{
				case 0x9:
					data_elf.start= (void *) getbe32(m+4);
					break;
				case 0x7D:
					data_elf.prio= getbe32(m+4);
					break;
				case 0x7E:
					data_elf.size_stack= getbe32(m+4);
					break;
				case 0x7F:
					data_elf.stack= (void *) (getbe32(m+4));
					break;
				
				}

			}

		}
    else
	if(entries->type == 1  && entries->memsz != 0 && entries->vaddr!=0)
		{
	
		os_sync_before_read((void *) entries->vaddr, entries->memsz );

		memset((void *) entries->vaddr, 0, entries->memsz);
		memcpy((void *) entries->vaddr, (void *) (elf + entries->offset), entries->filesz);

		os_sync_after_write((void *) entries->vaddr, entries->memsz );
			
		}
	}

return 0;
}