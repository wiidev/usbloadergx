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
  
                
}
#define DUMP_PREFIX_OFFSET 1
#include "ehci.h"
#define ehci_readl(a) ((*((volatile u32*)(a))))
//#define ehci_writel(e,v,a) do{msleep(40);debug_printf("writel %08X %08X\n",a,v);*((volatile u32*)(a))=(v);}while(0)
#define ehci_writel(v,a) do{*((volatile u32*)(a))=(v);}while(0)

struct ehci_hcd _ehci;
struct ehci_hcd *ehci = &_ehci;

//#include "ehci.c"

u8 heap_space2[0x5000] __attribute__ ((aligned (32)));

#include "ehci-mem.c"

//static usbctrlrequest mem_usbctrlrequest __attribute__ ((aligned (32)));
static u8 mem_usbctrlrequest[sizeof(usbctrlrequest)+32]  __attribute__ ((aligned (32)));
static int ehci_init(void)
{
        int retval;
 	if ((retval = ehci_mem_init()) < 0)
		return retval;
	/*
	 * dedicate a qh for the async ring head, since we couldn't unlink
	 * a 'real' qh without stopping the async schedule [4.8].  use it
	 * as the 'reclamation list head' too.
	 * its dummy is used in hw_alt_next of many tds, to prevent the qh
	 * from automatically advancing to the next td after short reads.
	 */
	ehci->async->hw_next = QH_NEXT( ehci->async->qh_dma);
	ehci->async->hw_info1 = cpu_to_hc32( QH_HEAD);
	ehci->async->hw_token = cpu_to_hc32( QTD_STS_HALT);
	ehci->async->hw_qtd_next = EHCI_LIST_END();
	ehci->async->hw_alt_next = EHCI_LIST_END();//QTD_NEXT( ehci->async->dummy->qtd_dma);
    ehci->ctrl_buffer =  mem_usbctrlrequest ;//USB_Alloc(sizeof(usbctrlrequest));
	ehci->command = 0;

    ehci_dma_map_bidir(ehci->async,sizeof(struct ehci_qh));

	ehci_writel( 0x008000002, &ehci->regs->command); 
	msleep(20);
	ehci_writel( ehci->periodic_dma, &ehci->regs->frame_list); 
	ehci_writel( ehci->async->qh_dma, &ehci->regs->async_next); 
	ehci_writel( 0x00010001, &ehci->regs->command);
	msleep(20);
	ehci_writel( 1, &ehci->regs->configured_flag);
	ehci_writel( 0x00010021, &ehci->regs->command);
	msleep(20);


        return 0;
}
/*
int ehci_adquire_port(int port)
{
	u32 __iomem	*status_reg = &ehci->regs->port_status[port];
	u32 status = ehci_readl(status_reg); 

	//change owner, port disabled
	if(status & PORT_OWNER)
		{
		status ^= PORT_OWNER;
		status &= ~(PORT_PE | PORT_RWC_BITS);
		ehci_writel(status, status_reg);	
		msleep(5);
		status = ehci_readl(status_reg);
		}
        ehci_writel( 0x1803,status_reg);
        msleep(100);
        ehci_writel( 0x1903,status_reg);
		msleep(100);// wait 100ms for the reset sequence
        ehci_writel( 0x1801,status_reg);
        msleep(60);
		#if 0
		status &= ~PORT_PE;
		status |= 0x800 | PORT_RESET | PORT_POWER;       
        ehci_writel( status,status_reg);
        msleep(60);// wait 60ms for the reset sequence
        status=ehci_readl(status_reg);
        status &= ~(PORT_RWC_BITS | PORT_RESET); // force reset to complete 
		ehci_writel( status,status_reg);
		msleep(60);
		//enable port	
		#endif
		
		status = ehci_readl(status_reg);

		if((status & PORT_OWNER) || PORT_USB11(status)) return 1;
	
return 0;
}
*/
extern u8 *text_log;

int tiny_ehci_init(void)
{
        int retval;
//		int n;
        ehci = &_ehci;


	ehci->caps = (void*)0x0D040000;
	ehci->regs = (void*)(0x0D040000 +
                             HC_LENGTH(ehci_readl(&ehci->caps->hc_capbase)));
        ehci->num_port = 4; // aqui numero de puertos usb
	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = ehci_readl(&ehci->caps->hcs_params);

	text_log=ehci_maligned(4096, 4096, 4096);

	/* data structure init */
	retval = ehci_init();
	if (retval)
		return retval; 
	
	
    ehci_release_ports(); //quickly release all ports
  /*
	#ifdef USE_USB_PORT_1

    ehci_writel( PORT_OWNER, &ehci->regs->port_status[0]); // force port 0 to work as USB 1.1

    for(n=0;n<3;n++)
		{
		if(!ehci_adquire_port(1)) break;
		}

	#else
	
	ehci_writel( PORT_OWNER, &ehci->regs->port_status[1]); // force port 1 to work as USB 1.1

    for(n=0;n<3;n++)
		{
		if(!ehci_adquire_port(0)) break;
		}
	
	#endif
     */
	return 0;
}


int ehci_release_ports(void)
{
        int i;
        u32 __iomem	*status_reg = &ehci->regs->port_status[2];
        while(ehci_readl(&ehci->regs->port_status[2]) == 0x1000);// wait port 2 to init
        msleep(1);// wait another msec..
        for(i = 0;i<ehci->num_port; i++){    //release all ports
          status_reg = &ehci->regs->port_status[i];
          //u32 status = ehci_readl(status_reg);
          //if (i==2 || !(PORT_CONNECT&status) || PORT_USB11(status))
            ehci_writel( PORT_OWNER,status_reg); // release port.
        }
        return 0;
}

static u8* aligned_mem = 0;
static u8* aligned_base = 0;
/* @todo hum.. not that nice.. */
void*ehci_maligned(int size,int alignement,int crossing)
{
        if (!aligned_mem )
        {
                aligned_mem=aligned_base =  (u8 *)((((u32) heap_space2+4095) & ~4095));//(void*)0x13890000;
        }
        u32 addr=(u32)aligned_mem;
        alignement--;
        addr += alignement;
        addr &= ~alignement;
        if (((addr +size-1)& ~(crossing-1)) != (addr&~(crossing-1)))
                addr = (addr +size-1)&~(crossing-1);
        aligned_mem = (void*)(addr + size);
        if (aligned_mem>aligned_base + 0x4000) 
        {
                debug_printf("not enough aligned memory!\n");
		while(1) msleep(1);
        }
        memset((void*)addr,0,size);
        return (void*)addr;
}

dma_addr_t ehci_virt_to_dma(void *a)
{

        return (dma_addr_t)a;
}
dma_addr_t ehci_dma_map_to(void *buf,size_t len)
{
        os_sync_after_write(buf, len);
        return (dma_addr_t)buf;

}
dma_addr_t ehci_dma_map_from(void *buf,size_t len)
{
        os_sync_after_write(buf, len);
        return (dma_addr_t)buf;
}
dma_addr_t ehci_dma_map_bidir(void *buf,size_t len)
{
        //debug_printf("sync_after_write %p %x\n",buf,len);
 
        os_sync_after_write(buf, len);
        return (dma_addr_t)buf;
}
void ehci_dma_unmap_to(dma_addr_t buf,size_t len)
{
        os_sync_before_read((void*)buf, len);
}
void ehci_dma_unmap_from(dma_addr_t buf,size_t len)
{
        os_sync_before_read((void*)buf, len);
}
void ehci_dma_unmap_bidir(dma_addr_t buf,size_t len)
{
        os_sync_before_read((void*)buf, len);
}