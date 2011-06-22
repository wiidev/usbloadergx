/*   
	Custom IOS module for Wii.
        OH0 message loop
    Copyright (C) 2009 kwiirk.
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
 * oh0_loop.c - IOS module main code
 * even if we are "ehc" driver, we still pretend to be "oh0"
 * and implement "standard" ios oh0 usb api
 *
 *******************************************************************************
 *
 */



#include <stdio.h>
#include <string.h>
#include "syscalls.h"
#include "ehci_types.h"
#include "ehci.h"
#include "utils.h"
#include "libwbfs.h"
#include "ehci_interrupt.h"

#undef ehci_writel
#define ehci_writel(v,a) do{*((volatile u32*)(a))=(v);}while(0)

									

void ehci_usleep(int usec);
void ehci_msleep(int msec);

int ehci_release_externals_usb_ports(void);

void off_callback_hand(u32 flags);

int disable_ehc=0;

#undef NULL
#define NULL ((void *)0)
#define IOS_OPEN				0x01
#define IOS_CLOSE				0x02
#define IOS_IOCTL				0x06
#define IOS_IOCTLV				0x07

#define USB_IOCTL_CTRLMSG			0
#define USB_IOCTL_BLKMSG			1
#define USB_IOCTL_INTRMSG			2
#define USB_IOCTL_SUSPENDDEV			5
#define USB_IOCTL_RESUMEDEV			6
#define USB_IOCTL_GETDEVLIST			12
#define USB_IOCTL_DEVREMOVALHOOK		26
#define USB_IOCTL_DEVINSERTHOOK			27

#define UMS_BASE (('U'<<24)|('M'<<16)|('S'<<8))
#define USB_IOCTL_UMS_INIT	        	(UMS_BASE+0x1)
#define USB_IOCTL_UMS_GET_CAPACITY      	(UMS_BASE+0x2)
#define USB_IOCTL_UMS_READ_SECTORS      	(UMS_BASE+0x3)
#define USB_IOCTL_UMS_WRITE_SECTORS		(UMS_BASE+0x4)

#define USB_IOCTL_UMS_READ_STRESS		(UMS_BASE+0x5)

#define USB_IOCTL_UMS_SET_VERBOSE		(UMS_BASE+0x6)

#define USB_IOCTL_UMS_UMOUNT			(UMS_BASE+0x10)
#define USB_IOCTL_UMS_WATCHDOG			(UMS_BASE+0x80)

#define USB_IOCTL_UMS_TESTMODE			(UMS_BASE+0x81)

#define USB_IOCTL_UMS_OFF				(UMS_BASE+0x82)
#define USB_IOCTL_SET_PORT				(UMS_BASE+0x83)



#define WBFS_BASE (('W'<<24)|('F'<<16)|('S'<<8))
#define USB_IOCTL_WBFS_OPEN_DISC	        (WBFS_BASE+0x1)
#define USB_IOCTL_WBFS_READ_DISC	        (WBFS_BASE+0x2)
#define USB_IOCTL_WBFS_READ_DIRECT_DISC	    (WBFS_BASE+0x3)
#define USB_IOCTL_WBFS_STS_DISC				(WBFS_BASE+0x4)

//#define USB_IOCTL_WBFS_SPEED_LIMIT			(WBFS_BASE+0x80)

void USBStorage_Umount(void);

//#define DEVICE "/dev/usb/ehc"
#define DEVICE "/dev/usb123"

int verbose = 0;
#define ioctlv_u8(a) (*((u8*)(a).data))
#define ioctlv_u16(a) (*((u16*)(a).data))
#define ioctlv_u32(a) (*((u32*)(a).data))
#define ioctlv_len(a) (a).len
#define ioctlv_voidp(a) (a).data

wbfs_disc_t * wbfs_init_with_partition(u8*discid, int partition);


int USBStorage_DVD_Test(void);

#define WATCHDOG_TIMER 1000*1000*10


int test_mode=0;

char *parse_hex(char *base,int *val)
{
        int v = 0,done=0;
        char *ptr = base,c;
        while(!done)
        {
                c = *ptr++;
                if(c >= '0' &&  c <= '9')
                        v = v << 4 | (c-'0');
                else if(c >= 'a' &&  c <= 'f')
                        v = v << 4 | (10+c-'a');
                else if(c >= 'A' &&  c <= 'F')
                        v = v << 4 | (10+c-'A');
                else
                        done = 1;
        }
        if(ptr==base+1)//only incremented once
                return 0; //we did not found any hex numbers
        *val = v;
        return ptr-1;
}

/*
int parse_and_open_device(char *devname,int fd)
{
        char *ptr = devname;
        int vid,pid;
        if (! (ptr = parse_hex(ptr,&vid)))
                return -6;
        if ( *ptr != '/' )
                return -6;
        ptr++;// skip /
        if (! (ptr = parse_hex(ptr,&pid)))
                return -6;
        if ( *ptr != '\0' )
                return -6;
        return ehci_open_device(vid,pid,fd);
}
*/

int DVD_speed_limit=0; // ingame it can fix x6 speed

int watchdog_enable=1;

// special ingame
int wbfs_disc_read2(wbfs_disc_t*d,u32 offset, u8 *data, u32 len);

// heap space for WBFS  and queue

extern int heaphandle;

void msleep(int msec);

u8 mem_sector[4096] __attribute__ ((aligned (32)));

void *WBFS_Alloc(int size)
{
  void * ret = 0;
 // ret= os_heap_alloc(heaphandle, size);
  ret= os_heap_alloc_aligned(heaphandle, size, 32);
  if(ret==0)
	{debug_printf("WBFS not enough memory! need %d\n",size);
    os_puts("WBFS not enough memory!\n");

    while(1) {swi_mload_led_on();ehci_msleep(200);swi_mload_led_off();ehci_msleep(200);}
	}
  return ret;
}

void WBFS_Free(void *ptr)
{
        return os_heap_free(heaphandle, ptr);
}

extern u8 *disc_buff;

u32 last_disc_lba=0;
u32 current_disc_lba=0xffffffff;


void wbfs_perform_disc(void);

// CISO mem area
int ciso_lba=-1;
int ciso_size=0;
u32 table_lba[2048];
u8 mem_index[2048] __attribute__ ((aligned (32)));


// offset -> disc_offset in words 
// data -> buffer
// len -> len to read in bytes

int WBFS_direct_disc_read(u32 offset, u8 *data, u32 len)
{
int r=true;
u32 lba;
u32 len2=len;
u8* data2=data;
u32 sec_size;
int l;
u8 *buff;

	os_sync_after_write(data2, len2);

	if(!disc_buff) return 0x8000;


	 
	last_disc_lba= USBStorage_Get_Capacity(&sec_size);

	if(last_disc_lba==0 || sec_size!=2048)
		{
		current_disc_lba=0xffffffff;
		return 0x8000;
		}

	if(ciso_lba>=0 && ciso_lba!=0x7fffffff && current_disc_lba==0xffffffff)
	{
	u32 lba_glob;

	current_disc_lba=0xffffffff;
	
	while(1)
		{
		lba_glob=ciso_lba+16;
	
		buff=(u8 *) (((u32)disc_buff+31) & ~31); // 32 bytes aligment
		r=USBStorage_Read_Sectors(ciso_lba, 16, buff); // read 16 cached sectors
		if(!r) return 0x8000;

		if((buff[0]=='C' && buff[1]=='I' && buff[2]=='S' && buff[3]=='O')) ciso_lba=0x7fffffff;
		else
			{
			if(ciso_lba!=0) {ciso_lba=0;continue;}
			ciso_lba=-1;
			}
		break;
		}

	ciso_size=(((u32)buff[4])+(((u32)buff[5])<<8)+(((u32)buff[6])<<16)+(((u32)buff[7])<<24))/2048;
	
	memset(mem_index,0,2048);

	if(ciso_lba==0x7fffffff)

		for(l=0;l<16384;l++)
			{
			if((l & 7)==0) table_lba[l>>3]=lba_glob;
			
			if(buff[8+l])
				{
				mem_index[l>>3]|=1<<(l & 7);
				lba_glob+=ciso_size;
				}
			}

	}


	buff=(u8 *) (((u32)disc_buff+31) & ~31); // 32 bytes aligment
	   
	while(len>0)
		{
		lba=offset>>9; // offset to LBA (sector have 512 words)

		if((lba & ~15)!=current_disc_lba)
			{
			u32 read_lba;

			current_disc_lba=(lba & ~15);

			read_lba=current_disc_lba;

			if(ciso_lba==0x7fffffff)
				{
				u32 temp=current_disc_lba/ciso_size;

                read_lba=table_lba[temp>>3];

				for(l=0;l<(temp & 7);l++) if((mem_index[temp>>3]>>l) & 1) read_lba+=ciso_size;

				read_lba+=current_disc_lba & (ciso_size-1);
			
				}
				
			l=(last_disc_lba-read_lba/*current_disc_lba*/);if(l>16) l=16;
			

			if(l<16) memset(buff,0,0x8000);
			if(l>0)
				{
				r=USBStorage_Read_Sectors(/*current_disc_lba*/read_lba, l, buff); // read 16 cached sectors
				if(!r) break;
				}
			}
		
		l=0x8000-((offset & 8191)<<2); // get max size in the cache relative to offset
		if(l>len) l=len;

		memcpy(data, &buff[((offset & 8191)<<2)], l);
		os_sync_after_write(data, l);

		data+=l;
		len-=l;
		offset+=l>>2;
		}

		if(!r) return 0x8000;
		os_sync_before_read(data2, len2);

return 0;
}



extern int unplug_device;

int unplug_procedure(void);

extern int is_watchdog_read_sector;

extern u32 n_sec,sec_size;

u32 last_sector=0;


void direct_os_sync_before_read(void* ptr, int size);
void direct_os_sync_after_write(void* ptr, int size);
u32 read_access_perm(void);
void write_access_perm(u32 flags);

/******************************************************************************************************************************************************/
// dev/di ioctl os_message_queue_receive
/******************************************************************************************************************************************************/

int swi_di_queue(u32 cmd, ipcmessage *message)
{
u32 perm;

	perm=read_access_perm();
	write_access_perm(0xffffffff);
	message->ioctl.command=cmd;
	*((u32 *) message->ioctl.buffer_in)=cmd<<24;
	direct_os_sync_after_write(&message->ioctl.command,4);
	direct_os_sync_after_write(message->ioctl.buffer_in,4);
	write_access_perm(perm);

return 0;
}

int my_di_os_message_queue_receive(int queuehandle, ipcmessage ** message,int flag)
{
int ret,ret2;


ret= os_message_queue_receive(queuehandle, (void*) message, flag);


if(ret==0 && message && *message)
	{
	
	if((*message)->command==IOS_IOCTL)
		{
	    switch((*message)->ioctl.command)
			{
			case 0x7a:
				ret2=swi_mload_call_func((void *) swi_di_queue, (void *) 0x15, (void *) (*message));
				break;
			case 0x88:
				ret2=swi_mload_call_func((void *) swi_di_queue, (void *) 0x14, (void *) (*message));
				break;

			}
		
		}
  
	}

return ret;
}

/******************************************************************************************************************************************************/
// ehcmodule swi service
/******************************************************************************************************************************************************/
void release_wbfs_mem(void);

int swi_ehcmodule(u32 cmd, u32 param1, u32 param2, u32 param3)
{
s32 ret=-666;
	
	switch(cmd)
	{
	case 0: // get mem alloc handle (139264 bytes heap)
		ret=heaphandle;
		break;
	case 1: // obtain release_wbfs_mem() function to be sure you have free memory
		ret=(int) release_wbfs_mem;
		break;
	case 2:
		ret=0;disable_ehc=1; // disable ehcmodule device for direct access operations
		break;
	case 16: // get USBStorage_Read_Sectors() for direct operations (remember you disable_ehc must be 1)
		ret= (int) USBStorage_Read_Sectors;
		break;
	case 17:
		ret= (int) USBStorage_Write_Sectors;
		break;

	}

return ret;
}

int ehc_loop(void)
{
	ipcmessage* message;
	int timer2_id=-1;
	extern char initial_port;
	int init_mode=initial_port;
	static bool first_read=true;
	char port;

	extern int ums_init_done[2];
	extern u32 current_port;
	

	int must_read_sectors=0;
	

	
	void* queuespace = os_heap_alloc(heaphandle, 0x80);


	int queuehandle = os_message_queue_create(queuespace, 32);

	
	init_thread_ehci();

	os_thread_set_priority(os_get_thread_id(), /*os_thread_get_priority()-1*/0x78);

	os_device_register(DEVICE, queuehandle);
	timer2_id=os_create_timer(WATCHDOG_TIMER, WATCHDOG_TIMER, queuehandle, 0x0);
	
     int ums_mode = 0;
//     int already_discovered = 0;
     wbfs_disc_t *d = 0;
      
	 int usb_lock=0;

	 int watch_time_on=1;

	// register SWI function (0xcd)

	swi_mload_add_handler(0xcd, swi_ehcmodule);

	

	while(1)
	{
		int result = 1;
		int ack = 1;
		volatile int ret;
	
		// Wait for message to arrive
		ret=os_message_queue_receive(queuehandle, (void*)&message, 0);
		if(ret) continue;

	
		// timer message WATCHDOG
		//if((int) message==0x555) continue;

		if(watch_time_on)
			os_stop_timer(timer2_id); // stops watchdog timer
		watch_time_on=0;
		
		is_watchdog_read_sector=0;

		if((int) message==0x0)
		{
		if(test_mode && !disable_ehc)
			watchdog_enable=0; // test mode blocks watchdog
		
		if(must_read_sectors && watchdog_enable && !disable_ehc)
			{
			int n,r;

			if(unplug_device)
				{
				for(n=0;n<3;n++)
					if(!unplug_procedure()) break;
				}

			if(unplug_device==0)
				{

				if(sec_size!=0 && sec_size<=4096) 
					{
					extern int is_dvd[2];

					is_watchdog_read_sector=1;

					r=USBStorage_Read_Sectors(last_sector, 1, mem_sector);

					is_watchdog_read_sector=0;
					if(r!=0 && !is_dvd[current_port])
						last_sector+=0x1000000/sec_size; // steps of 16MB
					if(last_sector>=n_sec) last_sector=0;
					}
				
				}
			
			if(!disable_ehc)
				{
				watch_time_on=1;
				os_restart_timer(timer2_id, WATCHDOG_TIMER);
				}
			}
		continue;
		}
     

                //print_hex_dump_bytes("msg",0, message,sizeof(*message));
		switch( message->command )
		{
			case IOS_OPEN:
			{
			
                                //debug_printf("%s try open %sfor fd %d\n",DEVICE,message->open.device,message->open.resultfd);
				// Checking device name
				if (0 == strcmp(message->open.device, DEVICE))
                                  {
									result = message->open.resultfd;
                                     
										
										
                                  }
				else
				if (0 == strcmp(message->open.device, DEVICE"/OFF"))
                                  {
									result = message->open.resultfd;   										
										disable_ehc=1;
										
									    must_read_sectors=0;
										watchdog_enable=0;

										ehci_int_passive_callback(off_callback_hand);
										ehci_writel (STS_PCD, &ehci->regs->intr_enable);
										ehci_release_externals_usb_ports();

										//swi_mload_led_on();

                                  }
				/*else if (!ums_mode && 0 == memcmp(message->open.device, DEVICE"/", sizeof(DEVICE)) && !disable_ehc)
                                        result = parse_and_open_device(message->open.device+sizeof(DEVICE),message->open.resultfd);
				*/
				else
					result = -6;
			}	
			break;

			case IOS_CLOSE:
			{
			
                                //debug_printf("close  fd %d\n",message->fd);
                                //USBStorage_Umount();
                                //ehci_release_externals_usb_ports();
                                
                                if(ums_mode == message->fd)
                                        ums_mode = 0;
                                else
                                        ehci_close_devices();
								
				result = 0;
			}	
			break;

			case IOS_IOCTL:
			{
			
            break;
            }
			case IOS_IOCTLV:
			{
                                ioctlv *vec = message->ioctlv.vector;
                                void *dev =NULL;
                                int i,in = message->ioctlv.num_in,io= message->ioctlv.num_io;
                                if( 0==(message->ioctl.command>>24) && !ums_mode)
                                        dev = ehci_fd_to_dev(message->fd);
                                os_sync_before_read( vec, (in+io)*sizeof(ioctlv));
                                for(i=0;i<in+io;i++){
                                        os_sync_before_read( vec[i].data, vec[i].len);
                                        //print_hex_dump_bytes("vec",0, vec[i].data,vec[i].len);
                                }
                               
							    if(disable_ehc)
									{
									result=-1;
									}
								else
                                switch( message->ioctl.command )
                                {
                                case  USB_IOCTL_CTRLMSG:
                                        //debug_printf("ctrl message%x\n",dev);
                                        if(!dev)result= -6;
                                        else
                                        result = ehci_control_message(dev,ioctlv_u8(vec[0]),ioctlv_u8(vec[1]),
                                                                      swab16(ioctlv_u16(vec[2])),swab16(ioctlv_u16(vec[3])),
                                                                      swab16(ioctlv_u16(vec[4])),ioctlv_voidp(vec[6]));
                                        break;
                                case  USB_IOCTL_BLKMSG:
                                        //debug_printf("bulk message\n");
                                        if(!dev)result= -6;
                                        else
                                                result = ehci_bulk_message(dev,ioctlv_u8(vec[0]),ioctlv_u16(vec[1]),
                                                                   ioctlv_voidp(vec[2]));
                                        break;
                                case  USB_IOCTL_INTRMSG:
                                        debug_printf("intr message\n");
                                case  USB_IOCTL_SUSPENDDEV:
                                case  USB_IOCTL_RESUMEDEV:
                                        debug_printf("or resume/suspend message\n");
                                        result = 0;//-1;// not supported
                                        break;
                                case  USB_IOCTL_GETDEVLIST:
                                        debug_printf("get dev list\n");
                                        //if(dev)result= -6;
                                        //else
                                        result = ehci_get_device_list(ioctlv_u8(vec[0]),ioctlv_u8(vec[1]),
                                                                      ioctlv_voidp(vec[2]),ioctlv_voidp(vec[3]));
                                        break;
                                case  USB_IOCTL_DEVREMOVALHOOK:
                                case  USB_IOCTL_DEVINSERTHOOK:
                                        debug_printf("removal/insert hook\n");
                                        ack = 0; // dont reply to those, as we dont detect anything
                                        break;
                                case USB_IOCTL_UMS_INIT: 
										must_read_sectors=0;

                                        result = USBStorage_Init(init_mode);
                                        if(result>=0 && result <3) 
                                        {
                                        	if(result==0 || result==2)
	                                        	current_port = 0;
	                                        else
		                                        current_port = 1;
		                                }																			
										
										//result=-os_thread_get_priority();
										if(result>=0) {must_read_sectors=1;watchdog_enable=1;}
                                        ums_mode = message->fd;
										
                                        break;
								case USB_IOCTL_UMS_UMOUNT:
										must_read_sectors=0;
										watchdog_enable=0;
                                        USBStorage_Umount();
										result =0;
										break;
								case USB_IOCTL_UMS_TESTMODE:
										test_mode=ioctlv_u32(vec[0]);
										result =0;
										break;
										
								case USB_IOCTL_SET_PORT:										
										result =0;
										port=ioctlv_u32(vec[0]);
										init_mode=port;
										//if(ums_init_done==0) init_mode=port;
										//else
										{
											if(port==0 || port==1) current_port=port;
											result = current_port;
										}
										
										break;

								case USB_IOCTL_UMS_OFF:
										{
									
                                        disable_ehc=1;
										
									    must_read_sectors=0;
										watchdog_enable=0;

										ehci_int_passive_callback(off_callback_hand);
										ehci_writel (STS_PCD, &ehci->regs->intr_enable);
										ehci_release_externals_usb_ports();

										result =0;
										}
								
                                case USB_IOCTL_UMS_GET_CAPACITY:
									    n_sec =  USBStorage_Get_Capacity(&sec_size);
										if(ioctlv_voidp(vec[0]))
											{
											*((u32 *) ioctlv_voidp(vec[0]))= sec_size;											
											}
										
										result =n_sec ;
                                        break;
                                case USB_IOCTL_UMS_READ_SECTORS:
										#ifdef VIGILANTE
										enable_button=1;
										#endif
										if(watchdog_enable && timer2_id!=-1)
										{
											os_stop_timer(timer2_id); // stops the timeout timer
											os_destroy_timer(timer2_id);
											timer2_id=-1;
										}	
										result =   USBStorage_Read_Sectors(ioctlv_u32(vec[0]),ioctlv_u32(vec[1]), ioctlv_voidp(vec[2]));
										if(first_read)
										{
											void s_printf(char *format,...);
											//#define s_printf(a...)
											first_read=false;
										
											if(result>0)
												s_printf("first read sector (%i) OK\n",ioctlv_u32(vec[0]));
											else
												s_printf("first read sector (%i) ERROR\n",ioctlv_u32(vec[0]));									
										}
										if(watchdog_enable && timer2_id==-1)timer2_id=os_create_timer(WATCHDOG_TIMER, WATCHDOG_TIMER, queuehandle, 0x0);
							
                                        break;
                                case USB_IOCTL_UMS_WRITE_SECTORS:
										#ifdef VIGILANTE
										enable_button=1;
										#endif

                                        result =  USBStorage_Write_Sectors(ioctlv_u32(vec[0]),ioctlv_u32(vec[1]), ioctlv_voidp(vec[2]));
                                        break;
                                case USB_IOCTL_UMS_READ_STRESS:
                                      //  result =   USBStorage_Read_Stress(ioctlv_u32(vec[0]),ioctlv_u32(vec[1]), ioctlv_voidp(vec[2]));
                                        break;
                                case USB_IOCTL_UMS_SET_VERBOSE:
                                        verbose = !verbose;
                                        result =  0;
                                        break;
								/*case USB_IOCTL_WBFS_SPEED_LIMIT:
									    DVD_speed_limit=ioctlv_u32(vec[0]);
										break;*/
								case USB_IOCTL_UMS_WATCHDOG:
									    watchdog_enable=ioctlv_u32(vec[0]);
										break;
                                case USB_IOCTL_WBFS_OPEN_DISC:
                                        ums_mode = message->fd;
								        u8 *discid;
											
										int partition=0;
										#ifdef VIGILANTE
										enable_button=1;
										#endif

										discid=ioctlv_voidp(vec[0]);
										if(discid[0]=='_' && discid[1]=='D' && discid[2]=='V' && discid[3]=='D')
											{
											result = 0;watchdog_enable=1;
											ciso_lba=0;
                                            if(vec[1].len==4)
												{ 
												memcpy(&partition, ioctlv_voidp(vec[1]), 4);
											    ciso_lba=partition;
												}

											//ciso_lba=265;
											wbfs_perform_disc();
											}
										else   
											{

											if(vec[1].len==4) memcpy(&partition, ioctlv_voidp(vec[1]), 4);
											d = wbfs_init_with_partition(discid, partition);
											if(!d)
                                               result = -1;
											else
												{result = 0;watchdog_enable=1;}
											}
                                        
										must_read_sectors=1;
										
                                        break;
								case USB_IOCTL_WBFS_STS_DISC:
									    result=USBStorage_DVD_Test();
										if(result==0) current_disc_lba=0xffffffff; // test fail
								     
										break;
								case USB_IOCTL_WBFS_READ_DIRECT_DISC: // used to read USB DVD
									    usb_lock=1;
										watchdog_enable=1;
										if(watchdog_enable && timer2_id!=-1)
										{
											os_stop_timer(timer2_id); // stops the timeout timer
											os_destroy_timer(timer2_id);
											timer2_id=-1;
										}
                                        result = WBFS_direct_disc_read(ioctlv_u32(vec[0]),ioctlv_voidp(vec[2]),ioctlv_u32(vec[1]));
                                        if(watchdog_enable)timer2_id=os_create_timer(WATCHDOG_TIMER, WATCHDOG_TIMER, queuehandle, 0x0);
										usb_lock=0;
										break;
									 
                                case USB_IOCTL_WBFS_READ_DISC:
                                        /*if (verbose)
                                                debug_printf("r%x %x\n",ioctlv_u32(vec[0]),ioctlv_u32(vec[1]));
                                        else
                                                debug_printf("r%x %x\r",ioctlv_u32(vec[0]),ioctlv_u32(vec[1]));
												*/
                                        if(!d /*|| usb_lock*/)
                                                result = -1;
                                        else
									{
									
											usb_lock=1;
											//os_stop_timer(timer2_id);
											if(watchdog_enable && timer2_id!=-1)
											{
												os_stop_timer(timer2_id); // stops the timeout timer
												os_destroy_timer(timer2_id);
												timer2_id=-1;
											}
                                            result = wbfs_disc_read(d,ioctlv_u32(vec[0]),ioctlv_voidp(vec[2]),ioctlv_u32(vec[1]));
                                            if(watchdog_enable)timer2_id=os_create_timer(WATCHDOG_TIMER, WATCHDOG_TIMER, queuehandle, 0x0);
											usb_lock=0;
                                       if(result){
                                          //debug_printf("wbfs failed! %d\n",result);
                                          //result = 0x7800; // wii games shows unrecoverable error..
                                          result = 0;//0x8000; 
                                        }
										//result=0;
									}
										
                                  break;
                                }
                                for(i=in;i<in+io;i++){
                                        //print_hex_dump_bytes("iovec",0, vec[i].data,vec[i].len>0x20?0x20:vec[i].len);
                                        os_sync_after_write( vec[i].data, vec[i].len);
                                }

                                break;
                        }
			default:
				result = -1;
				//ack = 0;
			break;
		}
		
        if(watchdog_enable)
			{
			watch_time_on=1;
			os_restart_timer(timer2_id, WATCHDOG_TIMER);
			}
		// Acknowledge message
		
		if (ack)
			os_message_queue_ack( (void*)message, result );

		
	}
   
	return 0;
}
