/*   
	EHCI glue. A bit hacky for the moment. needs cleaning..

    Copyright (C) 2008 kwiirk.

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
#include <string.h>
#include <setjmp.h>
#include "syscalls.h"

#include "ehci_types.h"
#include "utils.h"
#include "ehci_interrupt.h"
#include "swi_mload.h"

#define static
#define inline extern


#define readl(a) (*((volatile u32*)(a)))
#define writel(v,a) do{*((volatile u32*)(a))=(v);}while(0)
#define ehci_dbg(a...) debug_printf(a)
#define printk(a...) debug_printf(a)
#define get_timer()  (*(((volatile u32*)0x0D800010)))


void BUG(void)
{
        debug_printf("bug\n");
//        stack_trace();
      //  while(1);
}
#define BUG_ON(a) if(a)BUG()

void ehci_usleep(int usec);
void ehci_msleep(int msec);
/*
void udelay(int usec)
{
        u32 tmr,temp;
		u32 time_usec;

        tmr = get_timer();
        time_usec=2*usec;
		
        while (1) {temp=get_timer()-tmr;if(temp > time_usec) break;}
		
}
void msleep(int msec)//@todo not really sleeping..
{
        u32 tmr,temp;
		u32 time_usec;

        tmr = get_timer();
        time_usec=2048*msec;

        while (1) {temp=get_timer()-tmr;if(temp > time_usec) break;}
		

}
*/
extern u32 __exe_start_virt__;
extern u32 __ram_start_virt__;

extern u32 ios_thread_stack;

#define cpu_to_le32(a) swab32(a)
#define le32_to_cpu(a) swab32(a)
#define cpu_to_le16(a) swab16(a)
#define le16_to_cpu(a) swab16(a)
#define cpu_to_be32(a) (a)
#define be32_to_cpu(a) (a)
void print_hex_dump_bytes(char *header,int prefix,u8 *buf,int len)
{
        int i;
        if (len>0x100)len=0x100;
        debug_printf("%s  %08X\n",header,(u32)buf);
        for (i=0;i<len;i++){
                debug_printf("%02x ",buf[i]);
                if((i&0xf) == 0xf) 
                        debug_printf("\n");
        }
        debug_printf("\n");
                
}
#define DUMP_PREFIX_OFFSET 1
#include "ehci.h"
#define ehci_readl(a) ((*((volatile u32*)(a))))
//#define ehci_writel(e,v,a) do{msleep(40);debug_printf("writel %08X %08X\n",a,v);*((volatile u32*)(a))=(v);}while(0)
#define ehci_writel(v,a) do{*((volatile u32*)(a))=(v);}while(0)

struct ehci_hcd _ehci;
struct ehci_hcd *ehci = &_ehci;

#include "ehci.c"


int usb_os_init(void);

#define MLOAD_GET_EHCI_DATA		0x4D4C44A0

#if 1

int system_cmd(int cmd)
{
return 0;
}


#endif


static struct ehci_qtd *qtd_dummy_first=NULL ;
static struct ehci_qtd * qtd_header=NULL;
static struct ehci_qh * qh_header=NULL;
//void *global_buffer=NULL;

struct ehci_qh * qh_pointer[64];

extern struct ehci_qh	*in_qh;
extern struct ehci_qh	*out_qh;
extern struct ehci_qh	*dummy_qh;


inline dma_addr_t get_qtd_dummy(void)
{
	return qtd_dummy_first->qtd_dma;
}

void init_qh_and_qtd(void)
{
int n;
struct ehci_qtd * qtd;

struct ehci_qh * qh;

	if(!qh_header) 
		{
		//u32 mem = (u32) USB_Alloc(4096*3);
		//mem=(mem+4095) & ~4095;
		qh_header= (struct ehci_qh *) ehci->async;//mem;
		qtd_header= (struct ehci_qtd *) ehci->qtds[0];
		}


qtd=qtd_header;//= (struct ehci_qtd *) (((u32)qh_header)+4096);

for(n=0;n<EHCI_MAX_QTD;n++)
	{
    
    ehci->qtds[n]=qtd;
	
	memset((void *) ehci->qtds[n], 0, sizeof(struct ehci_qtd));
	ehci_dma_map_bidir((void *) ehci->qtds[n],sizeof(struct ehci_qtd));
	
	qtd=(struct ehci_qtd *) (((((u32) qtd)+sizeof(struct ehci_qtd)+31) & ~31));
	}

for(n=0;n<EHCI_MAX_QTD;n++)
	{
    
	memset((void *) qtd, 0, sizeof(struct ehci_qtd));
	ehci_dma_map_bidir((void *) qtd,sizeof(struct ehci_qtd));
	
	qtd=(struct ehci_qtd *) (((((u32) qtd)+sizeof(struct ehci_qtd)+31) & ~31));
	}

qtd_dummy_first=qtd;

qh=qh_header;

for(n=0;n<6;n++)
	{
    
    qh_pointer[n]=qh;

	memset((void *) qh_pointer[n], 0, sizeof(struct ehci_qh));
	qh->qh_dma = ehci_virt_to_dma(qh);
	qh_pointer[n]->hw_info1 = cpu_to_hc32((QH_HEAD*(n!=0)));
	qh_pointer[n]->hw_info2 = cpu_to_hc32(0);
	qh_pointer[n]->hw_token = cpu_to_hc32( QTD_STS_HALT);
	qh=(struct ehci_qh *) (((((u32) qh)+sizeof(struct ehci_qh)+31) & ~31));
	qh_pointer[n]->hw_next = QH_NEXT( ehci_virt_to_dma(qh));
	qh_pointer[n]->hw_qtd_next =EHCI_LIST_END();
	qh_pointer[n]->hw_alt_next =  EHCI_LIST_END();
	
	ehci_dma_map_bidir((void *) qh_pointer[n],sizeof(struct ehci_qh));
	}
n--;
qh_pointer[n]->hw_next = QH_NEXT( ehci_virt_to_dma(qh_header));
ehci_dma_map_bidir((void *) qh_pointer[n],sizeof(struct ehci_qh));
}

void create_qtd_dummy(void)
{
int n;
struct ehci_qtd * qtd, *qtd_next;


qtd=qtd_dummy_first;

for(n=0;;n++)
	{
	qtd_next=(struct ehci_qtd *) (((((u32) qtd)+sizeof(struct ehci_qtd)+31) & ~31));
	ehci_qtd_init(qtd);
	
	//qtd_fill( qtd, 0, 0, QTD_STS_HALT, 0);
	if(n<3) 
		{
		qtd->hw_next= QTD_NEXT(qtd_next->qtd_dma);
		qtd->hw_alt_next= EHCI_LIST_END(); //QTD_NEXT(qtd_next->qtd_dma);
		ehci_dma_map_bidir((void *) qtd,sizeof(struct ehci_qtd));
		}
	else
		{
		ehci_dma_map_bidir(qtd,sizeof(struct ehci_qtd));
		break;
		}
	qtd=qtd_next;
	}

}



/*
int hola(void *i, void *o)
{
	int n;

for(n=0;n<10;n++)
	{
	*((volatile u32 *)0x0d8000c0) ^=0x20;
	ehci_mdelay(50);
	}
}
*/



void reinit_ehci_headers(void)
{
		init_qh_and_qtd();
        
        create_qtd_dummy();

		ehci->async=   qh_pointer[0];
		ehci->asyncqh= qh_pointer[1];
		in_qh=qh_pointer[2];
		out_qh=qh_pointer[3];
		dummy_qh=qh_pointer[4];

		ehci_dma_unmap_bidir((dma_addr_t) ehci->async,sizeof(struct ehci_qh));

		ehci->async->ehci = ehci;
		ehci->async->qtd_head = NULL;
		ehci->async->qh_dma = ehci_virt_to_dma(ehci->async);
		ehci->async->hw_next = QH_NEXT(dummy_qh->qh_dma/* ehci->async->qh_dma*/);
		ehci->async->hw_info1 = cpu_to_hc32( QH_HEAD);
		ehci->async->hw_info2 = cpu_to_hc32( 0);
		ehci->async->hw_token = cpu_to_hc32( QTD_STS_HALT);
		

	    ehci->async->hw_qtd_next =EHCI_LIST_END();
		ehci->async->hw_alt_next =EHCI_LIST_END(); //QTD_NEXT(get_qtd_dummy());
	
		ehci_dma_map_bidir(ehci->async,sizeof(struct ehci_qh));

		ehci_dma_unmap_bidir((dma_addr_t)ehci->asyncqh,sizeof(struct ehci_qh));
		ehci->asyncqh->ehci = ehci;
		ehci->asyncqh->qtd_head = NULL;
		ehci->asyncqh->qh_dma = ehci_virt_to_dma(ehci->asyncqh);

		ehci_dma_unmap_bidir((dma_addr_t)in_qh,sizeof(struct ehci_qh));
		in_qh->ehci = ehci;
		in_qh->qtd_head = NULL;
		in_qh->qh_dma = ehci_virt_to_dma(in_qh);
		ehci_dma_map_bidir(in_qh,sizeof(struct ehci_qh));

		ehci_dma_unmap_bidir((dma_addr_t)out_qh,sizeof(struct ehci_qh));
		out_qh->ehci = ehci;
		out_qh->qtd_head = NULL;
		out_qh->qh_dma = ehci_virt_to_dma(out_qh);
		ehci_dma_map_bidir(out_qh,sizeof(struct ehci_qh));
}



int tiny_ehci_init(void)
{
int i;
        ehci = &_ehci;

		
        if(usb_os_init()<0)
                return -1;
	
	if(1) 
	{ // From Hermes: ohci mem is readed from dev/mload: (ehci init is from here)
/*	int fd;
		fd = os_open("/dev/mload",1);
		if(fd<0) return -1;
		ehci= (struct ehci_hcd *) os_ioctlv(fd, MLOAD_GET_EHCI_DATA ,0,0,0);
		
		os_close(fd);
		*/
		ehci=swi_mload_EHCI_data();
		
		// stops EHCI
		ehci_writel( 0x00010020 , &ehci->regs->command);
		do
		{
			if(!(ehci_readl( &ehci->regs->command) & 1))break;
		} while(1);

		
		ehci_dma_map_bidir(ehci,sizeof(struct ehci_hcd));

		for (i = 0; i < DEFAULT_I_TDPS; i++)
		{
		ehci->periodic [i] = EHCI_LIST_END();
		ehci_dma_map_bidir((void *) ehci->periodic [i],4);
		}
        
		
        
		reinit_ehci_headers();

	
	//////////////////////////////////////////////////////////////////////////////////////////////
	/* WARNING: This ignore the port 1 (external) and 2,3 (internals) for USB 2.0 operations    */
	/* from cIOS mload 1.6 port 1 is forced to USB 1.1. Only port 0 can work as USB 2.0         */
	
	ehci->num_port=2;
ehci_release_ports();
	
	//ehci_writel( 0x00080021, &ehci->regs->command);
	//ehci_writel(0, &ehci->regs->frame_list);

    ehci_writel(ehci->async->qh_dma, &ehci->regs->async_next);
	ehci_writel (/*INTR_MASK*/STS_PCD, &ehci->regs->intr_enable);
#define t125us (1)
	ehci_writel( (t125us<<16) | 0x0021 , &ehci->regs->command);
	ehci_readl( &ehci->regs->command);

	//swi_mload_led_on();
	//swi_mload_call_func(hola,NULL,NULL);
	

    /////////////////////////////////////////////////////////////////////////////////////////////
	}

	return 0;
}
