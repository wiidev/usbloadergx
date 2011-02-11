/* simplest usb-ehci driver which features:

   control and bulk transfers only
   only one transfer pending
   driver is synchronous (waiting for the end of the transfer)
   endianess independant
   no uncached memory allocation needed

   this driver is originally based on the GPL linux ehci-hcd driver

 * Original Copyright (c) 2001 by David Brownell
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

extern char use_usb_port1;

/* magic numbers that can affect system performance */
#define	EHCI_TUNE_CERR		0	/* 0-3 qtd retries; 0 == don't stop */ /* by  Hermes: i have replaced 3 by 0 and now it don´t hang when i extract the device*/
#define	EHCI_TUNE_RL_HS		4 //4	/* nak throttle; see 4.9 */ 
#define	EHCI_TUNE_RL_TT		0
#define	EHCI_TUNE_MULT_HS	1	/* 1-3 transactions/uframe; 4.10.3 */
#define	EHCI_TUNE_MULT_TT	1
#define	EHCI_TUNE_FLS		2	/* (small) 256 frame schedule */

void s_printf(char *format,...);
//#define s_printf(a...) 
bool enable_urb_debug=false;

int ehci_release_port(int);
void ehci_close_devices(void);


extern int verbose;
#ifdef DEBUG
static int num_command_before_no_verbose = 100;
#endif
static void 
dbg_qtd (const char *label, struct ehci_qtd *qtd)
{
	ehci_dbg( "%s td %p n%08x %08x t%08x p0=%08x\n", label, qtd,
		hc32_to_cpup( &qtd->hw_next),
		hc32_to_cpup( &qtd->hw_alt_next),
		hc32_to_cpup( &qtd->hw_token),
		hc32_to_cpup( &qtd->hw_buf [0]));
	if (qtd->hw_buf [1])
		ehci_dbg( "  p1=%08x p2=%08x p3=%08x p4=%08x\n",
			hc32_to_cpup( &qtd->hw_buf[1]),
			hc32_to_cpup( &qtd->hw_buf[2]),
			hc32_to_cpup( &qtd->hw_buf[3]),
			hc32_to_cpup( &qtd->hw_buf[4]));
}

static void 
dbg_qh (const char *label, struct ehci_qh *qh)
{
	ehci_dbg ( "%s qh %p n%08x info %x %x qtd %x\n", label,
                  qh, 
                  hc32_to_cpu(qh->hw_next), 
                  hc32_to_cpu(qh->hw_info1), 
                  hc32_to_cpu(qh->hw_info2),
                  hc32_to_cpu(qh->hw_current));
	dbg_qtd ("overlay",  (struct ehci_qtd *) &qh->hw_qtd_next);
}

static void
dbg_command (void)
{
#ifdef DEBUG
        u32 command=ehci_readl( &ehci->regs->command);
        u32 async=ehci_readl( &ehci->regs->async_next);
        
        ehci_dbg ("async_next: %08x\n",async);
        ehci_dbg (
		"command %06x %s=%d ithresh=%d%s%s%s%s %s %s\n",
		 command,
		(command & CMD_PARK) ? "park" : "(park)",
		CMD_PARK_CNT (command),
		(command >> 16) & 0x3f,
		(command & CMD_LRESET) ? " LReset" : "",
		(command & CMD_IAAD) ? " IAAD" : "",
		(command & CMD_ASE) ? " Async" : "",
		(command & CMD_PSE) ? " Periodic" : "",
		(command & CMD_RESET) ? " Reset" : "",
		(command & CMD_RUN) ? "RUN" : "HALT"
		);
#endif
}
static void
dbg_status (void)
{
#ifdef DEBUG
        u32 status=ehci_readl( &ehci->regs->status);
        ehci_dbg (
		"status %04x%s%s%s%s%s%s%s%s%s%s\n",
		status,
		(status & STS_ASS) ? " Async" : "",
		(status & STS_PSS) ? " Periodic" : "",
		(status & STS_RECL) ? " Recl" : "",
		(status & STS_HALT) ? " Halt" : "",
		(status & STS_IAA) ? " IAA" : "",
		(status & STS_FATAL) ? " FATAL" : "",
		(status & STS_FLR) ? " FLR" : "",
		(status & STS_PCD) ? " PCD" : "",
		(status & STS_ERR) ? " ERR" : "",
		(status & STS_INT) ? " INT" : ""
		);
#endif
}

void debug_qtds(void)
{
        struct ehci_qh *qh = ehci->async; 
        struct ehci_qtd *qtd;
        dbg_qh ("qh",qh);
        dbg_command ();
        dbg_status ();
        for(qtd = qh->qtd_head; qtd; qtd = qtd->next)
        {
                ehci_dma_unmap_bidir(qtd->qtd_dma,sizeof(struct ehci_qtd));
                dbg_qtd("qtd",qtd);
                ehci_dma_map_bidir(qtd,sizeof(struct ehci_qtd));
        }

}
void dump_qh(struct ehci_qh	*qh)
{
        struct ehci_qtd	*qtd;
        dbg_command ();
        dbg_status ();
        ehci_dma_unmap_bidir(qh->qh_dma,sizeof(struct ehci_qh));
        dbg_qh("qh",qh);
        print_hex_dump_bytes("qh:",DUMP_PREFIX_OFFSET,(void*)qh,12*4);
        for(qtd = qh->qtd_head; qtd; qtd = qtd->next){
                u32 *buf;
                ehci_dma_unmap_bidir(qtd->qtd_dma,sizeof(struct ehci_qtd));
                dbg_qtd("qtd",qtd);
                print_hex_dump_bytes("qtd:",DUMP_PREFIX_OFFSET,(void*)qtd,8*4);
                buf = (u32*)hc32_to_cpu(qtd->hw_buf[0]);
                if(buf)
                        print_hex_dump_bytes("qtd buf:",DUMP_PREFIX_OFFSET,(void*)(buf),8*4);

        }
}

/*-------------------------------------------------------------------------*/

/*
 * handshake - spin reading hc until handshake completes or fails
 * @ptr: address of hc register to be read
 * @mask: bits to look at in result of read
 * @done: value of those bits when handshake succeeds
 * @usec: timeout in microseconds
 *
 * Returns negative errno, or zero on success
 *
 * Success happens when the "mask" bits have the specified value (hardware
 * handshake done).  There are two failure modes:  "usec" have passed (major
 * hardware flakeout), or the register reads as all-ones (hardware removed).
 *
 * That last failure should_only happen in cases like physical cardbus eject
 * before driver shutdown. But it also seems to be caused by bugs in cardbus
 * bridge shutdown:  shutting down the bridge before the devices using it.
 */

int unplug_device=0;

#define INTR_MASK (STS_IAA | STS_FATAL | STS_PCD | STS_ERR | STS_INT)

void ehci_clear_flags_interrupt(void)
{

	
ehci_writel(INTR_MASK, &ehci->regs->status);
		
}


#define get_timer()  (*(((volatile u32*)0x0D800010)))

void ehci_mdelay(int msec);


void ehci_udelay(u32);

static int handshake(void __iomem * null, void __iomem *ptr,
		      u32 mask, u32 done, int usec)
{
	u32	result;
   
	u32 tmr,diff=0;
	
	

    tmr = get_timer();
	usec<<=1;
	
    
	do {
		ehci_usleep(10);
	
		result = ehci_readl( ptr);
		
		result &= mask;
		
		
		if (result == done) return 0;
                

	diff=get_timer();
	diff-=tmr;

	if(((int)diff)<0)
		{
		// error en diferencial: teoricamente imposible, pero...
		tmr=get_timer();
		}
	} while (diff < usec/*usec > 0*/);


return -ETIMEDOUT;
}

static struct ehci_qh cached_qh  __attribute__ ((aligned (32)));
static struct ehci_qh temp_qh  __attribute__ ((aligned (32)));

#include "ehci-mem.c"

/* one-time init, only for memory state */
static int ehci_init(void)
{
//        int retval;
/* 	if ((retval = ehci_mem_init()) < 0)
		return retval;*/
static void *my_buff=0;

if(!my_buff) my_buff=ehci->ctrl_buffer;
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
    ehci->ctrl_buffer =  my_buff; //USB_Alloc(sizeof(usbctrlrequest));
	ehci->command = 0;

	ehci_writel( 0x000000000, &ehci->regs->command); 
	ehci_writel( 0, &ehci->regs->configured_flag);
	ehci_writel( ehci->periodic_dma, &ehci->regs->frame_list); 
	ehci_writel( ehci->async->qh_dma, &ehci->regs->async_next); 
	ehci_writel( 0x00010001, &ehci->regs->command);
	ehci_writel( 1, &ehci->regs->configured_flag);
	ehci_writel( 0x00010021, &ehci->regs->command);


        return 0;
}

/* fill a qtd, returning how much of the buffer we were able to queue up */
static int
qtd_fill(struct ehci_qtd *qtd, dma_addr_t buf,
		  size_t len, int token, int maxpacket)
{
	int	i, count;
	u64	addr = buf;
        //ehci_dbg("fill qtd with dma %X len %X\n",buf,len);
	/* one buffer entry per 4K ... first might be short or unaligned */
	qtd->hw_buf[0] = cpu_to_hc32( (u32)addr);
	qtd->hw_buf_hi[0] = 0;
	count = 0x1000 - (buf & 0x0fff);	/* rest of that page */
	if (likely (len < count))		/* ... iff needed */
		count = len;
	else {
		buf +=  0x1000;
		buf &= ~0x0fff;

		/* per-qtd limit: from 16K to 20K (best alignment) */
		for (i = 1; count < len && i < 5; i++) {
			addr = buf;
			qtd->hw_buf[i] = cpu_to_hc32( (u32)addr);
			qtd->hw_buf_hi[i] = cpu_to_hc32(
					(u32)(addr >> 32));
			buf += 0x1000;
			if ((count + 0x1000) < len)
				count += 0x1000;
			else
				count = len;
		}

		/* short packets may only terminate transfers */
		if (count != len)
			count -= (count % maxpacket);
		
	}
	qtd->hw_token = cpu_to_hc32( (count << 16) | token);
	qtd->length = count;

// añadido por mi
	qtd->hw_next=EHCI_LIST_END();
	qtd->hw_alt_next=EHCI_LIST_END();

	return count;
}

// high bandwidth multiplier, as encoded in highspeed endpoint descriptors
#define hb_mult(wMaxPacketSize) (1 + (((wMaxPacketSize) >> 11) & 0x03))
// ... and packet size, for any kind of endpoint descriptor
#define max_packet(wMaxPacketSize) ((wMaxPacketSize) & 0x07ff)

/*
 * reverse of qh_urb_transaction:  free a list of TDs.
 * also count the actual transfer length.
 * 
 */

static int qh_end_transfer ( struct ehci_qtd *qtd_head)
{
 struct ehci_qtd *qtd;
        u32 token;
        int error = 0;
        for(qtd = qtd_head; qtd; qtd = qtd->next){
                token = hc32_to_cpu( qtd->hw_token);
                
			
				if (likely (QTD_PID (token) != 2))
                        qtd->urb->actual_length += qtd->length - QTD_LENGTH (token);

                if (/*!(qtd->length ==0 && ((token & 0xff)==QTD_STS_HALT)) &&*/
                    qtd->length !=0 && (token & QTD_STS_HALT)) {
                        ehci_dbg("\nqtd error!:");
                        if(enable_urb_debug)s_printf("\nqtd error!:");
                        if (token & QTD_STS_BABBLE) {
                                ehci_dbg(" BABBLE");
                                if(enable_urb_debug)s_printf(" BABBLE");
                        }  
                        if (token & QTD_STS_MMF) {
                                /* fs/ls interrupt xfer missed the complete-split */
                                ehci_dbg(" missed micro frame");
                                if(enable_urb_debug)s_printf("  missed micro frame");
                        }
                        if (token & QTD_STS_DBE) {
                                ehci_dbg(" databuffer error");
                                if(enable_urb_debug)s_printf("  databuffer error");
                        }
                        if (token & QTD_STS_XACT) {
                                ehci_dbg(" wrong ack");
                                if(enable_urb_debug)s_printf(" wrong ack");
                        }
                        if (QTD_CERR (token)==0)
                                ehci_dbg(" too many errors");
                                if(enable_urb_debug)s_printf(" too many errors");
                        ehci_dbg("\n");
                        if(enable_urb_debug)s_printf("\n");
                        error = -1;
						break;
                }
		
		
        }
		
        if(error){

                qtd->urb->actual_length = error;
        }
		
        ehci->qtd_used = 0;
return error;
}

/*
 * create a list of filled qtds for this URB; won't link into qh.
 */

struct ehci_qtd *qh_urb_transaction (
	struct ehci_urb		*urb
) {
	struct ehci_qtd		*qtd, *qtd_prev;
        struct ehci_qtd		*head;
	dma_addr_t		buf;
	int			len, maxpacket;
	int			is_input;
	u32			token;

	/*
	 * URBs map to sequences of QTDs:  one logical transaction
	 */
	 
	//create_qtd_dummy();
	
	head = qtd = qtd_prev= ehci_qtd_alloc ();


	if(!head) return NULL;

	qtd->urb = urb;

        urb->actual_length = 0;
	token = QTD_STS_ACTIVE;
	token |= (EHCI_TUNE_CERR << 10);
	/* for split transactions, SplitXState initialized to zero */


	len = urb->transfer_buffer_length;
	is_input = urb->input;
	if (urb->ep==0) {/* is control */
		/* SETUP pid */
		qtd_fill( qtd, urb->setup_dma,
				sizeof (usbctrlrequest),
				token | (2 /* "setup" */ << 8), 8);

		/* ... and always at least one more pid */
		token ^= QTD_TOGGLE;
		qtd_prev = qtd;
		qtd = ehci_qtd_alloc ();
		if(!qtd) goto cleanup;
		qtd->urb = urb;
		qtd_prev->hw_next = QTD_NEXT( qtd->qtd_dma);
		qtd_prev->next = qtd;

		/* for zero length DATA stages, STATUS is always IN */
		if (len == 0)
			token |= (1 /* "in" */ << 8);
	}

	/*
	 * data transfer stage:  buffer setup
	 */
	buf = urb->transfer_dma;

	if (is_input)
		token |= (1 /* "in" */ << 8);
	/* else it's already initted to "out" pid (0 << 8) */

	maxpacket = max_packet(urb->maxpacket);
	

	/*
	 * buffer gets wrapped in one or more qtds;
	 * last one may be "short" (including zero len)
	 * and may serve as a control status ack
	 */
	for (;;) {
		int this_qtd_len;

		this_qtd_len = qtd_fill( qtd, buf, len, token, maxpacket);
		len -= this_qtd_len;
		buf += this_qtd_len;

		/*
		 * short reads advance to a "magic" dummy instead of the next
		 * qtd ... that forces the queue to stop, for manual cleanup.
		 * (this will usually be overridden later.)
		 */
		if (is_input)
			qtd->hw_alt_next =EHCI_LIST_END();// QTD_NEXT(get_qtd_dummy());
		
		/* qh makes control packets use qtd toggle; maybe switch it */
		if ((maxpacket & (this_qtd_len + (maxpacket - 1))) == 0)
			token ^= QTD_TOGGLE;
		
		if (likely (len <= 0))
			break;

		qtd_prev = qtd;
		qtd = ehci_qtd_alloc ();
		if(!qtd) goto cleanup;
		qtd->urb = urb;
		qtd_prev->hw_next = QTD_NEXT( qtd->qtd_dma);
		qtd_prev->next = qtd;
	}

        /* unless the bulk/interrupt caller wants a chance to clean
	 * up after short reads, hc should advance qh past this urb
	 */

		qtd->hw_alt_next =EHCI_LIST_END(); 

	/*
	 * control requests may need a terminating data "status" ack;
	 * bulk ones may need a terminating short packet (zero length).
	 */
	
	if (likely (urb->transfer_buffer_length != 0)) {
		int	one_more = 0;

		if (urb->ep==0) {
			one_more = 1;
			token ^= 0x0100;	/* "in" <--> "out"  */
			token |= QTD_TOGGLE;	/* force DATA1 */
		}
		else if(!(urb->transfer_buffer_length % maxpacket) && !is_input) {
		//one_more = 1;	
		}
		if (one_more) {
			qtd_prev = qtd;
			qtd = ehci_qtd_alloc ();
			if(!qtd) goto cleanup;
			qtd->urb = urb;

			qtd_prev->hw_next = QTD_NEXT( qtd->qtd_dma);
			
            qtd_prev->next = qtd;

			/* never any data in such packets */
			qtd_fill( qtd, 0, 0, token, 0);
		}
	}

	/* by default, enable interrupt on urb completion */

	  qtd->hw_token |= cpu_to_hc32( QTD_IOC);
	 // qtd->hw_alt_next =QTD_NEXT(get_qtd_dummy());

	return head;

cleanup:
	return NULL;
}

u32 usb_timeout=1000*1000;

int mode_int=0;


u32 current_port=0;

struct ehci_qh	*in_qh=NULL;   // bulk in
struct ehci_qh	*out_qh=NULL;  // bulk out
struct ehci_qh	*dummy_qh=NULL;	// next qh (dummy)

extern struct ehci_qh * qh_pointer[64];


void read_cache_data(char *in, int len);



void inline ehci_stop(void)
{
u32 temp;

temp=ehci_readl( &ehci->regs->command);
	while(temp & CMD_ASE)
		{
		temp&= ~CMD_ASE;
		ehci_writel(temp, &ehci->regs->command);
		ehci_usleep(10);
		temp=ehci_readl( &ehci->regs->command);
		
		} 
	//while(temp & CMD_ASE);
}
	


void inline ehci_run(void)
{
u32 temp;
temp=ehci_readl( &ehci->regs->command);

	while(!(temp & CMD_ASE))
		{
		temp|= CMD_ASE;
		ehci_writel(temp, &ehci->regs->command);
		ehci_usleep(10);
		temp=ehci_readl( &ehci->regs->command);
		} 
	//while(!(temp & CMD_ASE));
}


int ehci_wait(u32 mode, struct ehci_qh *qh)
{
u32 temp;
u32 time_count=0;

	if(mode==0)
		{
		
		while(1)
				{

				temp=ehci_readl(&ehci->regs->async_next);

				if((temp>= (u32) qh_pointer[4]->qh_dma && temp<= (u32) qh_pointer[5]->qh_dma))
					{ehci_stop();break;}
				ehci_usleep(10);time_count++;if(time_count>=5000) break;
				}
		}
	else
	if(mode==1)
		{
		while(ehci_readl(&ehci->regs->async_next)==qh->qh_dma)
			{ehci_usleep(10);time_count++;if(time_count>=5000) break;}
		}

return 0;
}


// WARNING!: this routine works in Interrupt Mode
// off_callback_hand when you disables ehcmodule access

void off_callback_hand(u32 flags)
{
int n;
u32 temp;

 if(flags & STS_PCD)
		{

	    for(n=0;n<2;n++)
			{
		    temp=ehci_readl(&ehci->regs->port_status[n]);
			

			if((temp & 0x2003)==3) // on
				{
				 ehci_writel(PORT_OWNER /*| PORT_CSC*/, &ehci->regs->port_status[n]);
				}
			}
		}
}

// WARNING!: this routine works in Interrupt Mode
// passive_callback is used when EHCI driver is waiting to transfer datas

void passive_callback_hand(u32 flags)
{
int n;
u32 temp;

 if(flags & STS_PCD)
		{

	    for(n=0;n<4;n++)
			{
		    temp=ehci_readl(&ehci->regs->port_status[n]);

			if(n==current_port)
				{
				if((temp & 1)!=1)  // off
					{
					unplug_device=2;
//					*((volatile u32 *)0x0d8000c0) &=~0x20; // LED OFF (you can do it in Interrupt mode)
	
					}
				//if(temp & 2) ehci_writel(0x1001 | PORT_CSC, &ehci->regs->port_status[n]);
				}
			else
			if((temp & 0x2003)==3) // on
				{
				 if(n!=current_port) {ehci_writel(PORT_OWNER /*| PORT_CSC*/, &ehci->regs->port_status[n]);}
				}
			}
		}
}

// WARNING!: this routine works in Interrupt Mode

// interrupt_callback is used when EHCI driver is working
void direct_os_sync_before_read(void* ptr, int size);
void direct_os_sync_after_write(void* ptr, int size);

//static struct ehci_qh int_temp_qh  __attribute__ ((aligned (32)));

struct ehci_qtd * int_qtd=NULL;
struct ehci_qh *int_qh=NULL;

u32 int_toggles=0;

static int interrupt_callback_hand(u32 flags)
{
int n;
int ret=-9; // do nothing
u32 temp;
  
	if(flags & STS_INT) 
		{		
		struct ehci_qtd * qtd;
		ret=0; // done
		
			if(enable_urb_debug)s_printf("interrupt_callback_hand STS_INT\n");

			if(int_qtd)
			{
			for(qtd=int_qtd; qtd; qtd = qtd->next)
				{
				direct_os_sync_before_read((void *) qtd->qtd_dma,sizeof(struct ehci_qtd));
				read_cache_data((void *) qtd->qtd_dma,sizeof(struct ehci_qtd));
				}
			}
			if(int_qh)
				{
				direct_os_sync_before_read((void *) int_qh->qh_dma, 32);
				read_cache_data((void *) int_qh->qh_dma, 32);

				int_qh->hw_qtd_next = /*get_qtd_dummy();*/EHCI_LIST_END();
				int_qh->hw_alt_next = get_qtd_dummy();//EHCI_LIST_END();

				direct_os_sync_after_write((void *) int_qh->qh_dma, 32);
				

				int_toggles=int_qh->hw_token;
				}
			if(qh_end_transfer(int_qtd)!=0)
				{
				ehci->qtd_used = 0;
				ret=-EBADDATA;
			 
				}
			#if 0
		    ehci_stop();
			direct_os_sync_before_read(ehci->async, 32);
			read_cache_data((void *)ehci->async, 32);
			ehci->async->hw_next = QH_NEXT(dummy_qh->qh_dma);
                //ehci->async->hw_next = QH_NEXT(dummy_qh->qh_dma/* ehci->async->qh_dma*/);
			direct_os_sync_after_write(ehci->async, 32);
			ehci_run();
			#endif
      
		}
	
	   if(flags & STS_PCD)
		{
			ret=1;
			if(enable_urb_debug)s_printf("interrupt_callback_hand STS_PCD\n");
	    for(n=0;n<4;n++)
			{
		    temp=ehci_readl(&ehci->regs->port_status[n]); 
			if(n==current_port) 
				{
				if((temp & 1)!=1)  // off
					{
				   
															
						unplug_device=1;
//						*((volatile u32 *)0x0d8000c0) &=~0x20; // LED OFF (you can do it in Interrupt mode)
						ret=-ENODEV;
					
					}
				//if(temp & 2) ehci_writel(0x1001 | PORT_CSC, &ehci->regs->port_status[n]);
				}
			else
			if((temp & 0x2003)==3) // on
				{
				if(n!=current_port) {ehci_writel(PORT_OWNER /*| PORT_CSC*/, &ehci->regs->port_status[n]);}
				}
			}
		}

	  if(ret!=0)
		if(flags & (STS_FATAL | STS_ERR)) ret=-ETRANSERR;

return ret;
}

/*

struct ehci_qtd * update_qtd(struct ehci_urb *urb, void *null)
{
struct ehci_qtd * qtd, *ret;

	qtd= ret=qh_urb_transaction ( urb);

	for(; qtd; qtd = qtd->next)
       direct_os_sync_after_write(qtd,sizeof(struct ehci_qtd));
	
return ret;
}
*/

extern int qtd_alt_mem;
  int ehci_do_urb (
		 struct ehci_device *dev,
	 struct ehci_urb *urb)
 {
	 struct ehci_qh 	 *qh;
	 struct ehci_qtd *qtd;
	 u32 info1 = 0, info2 = 0;
	 int is_input;
	 int maxp = 0;
	 int retval;
 
 
	 
	 
	 //swi_mload_call_func((void *) ehci_wait, (void *) 0, (void *) 0);
	 //disable_OHCI1_IRQ();
	 ehci_wait( 0, (void *) 0);
	 //enable_OHCI1_IRQ();
	 
 
	 
	 
		 if(urb->ep==0) //control message
				 {
				 unplug_device=0;
				 urb->setup_dma = ehci_dma_map_to(urb->setup_buffer,sizeof (usbctrlrequest));
				 }
 
		 if(urb->transfer_buffer_length){
				 if(urb->input)
						 urb->transfer_dma = ehci_dma_map_to(urb->transfer_buffer,urb->transfer_buffer_length);
				 else
						 urb->transfer_dma = ehci_dma_map_from(urb->transfer_buffer,urb->transfer_buffer_length);
		 }
 
		 if(urb->ep==0)
			 qh = ehci->asyncqh;
		 else if(urb->input!=0)
			 qh = in_qh;
		 else
			 qh = out_qh;


		 int_qh=qh;
		 //ehci_dma_unmap_bidir(qh->qh_dma,sizeof(struct ehci_qh));
		 swi_mload_memcpy_from_uncached(&cached_qh, qh, 96/*sizeof(struct ehci_qh)*/);
 
		 //ehci_dma_unmap_bidir(qh->qh_dma,32/*sizeof(struct ehci_qh)*/);
 
		 memset(qh,0,12*4);
		//disable_OHCI1_IRQ();
		 ehci->qtd_used = 0;qtd_alt_mem^=1;
		 qtd =qh_urb_transaction ( urb);
		 //qtd=(void *) swi_mload_call_func ((void *) update_qtd, (void *) urb, NULL);
		 cached_qh.qtd_head = int_qtd=qtd;
		 
 
	 
		 
		 info1 |= ((urb->ep)&0xf)<<8;
		 info1 |= dev->id;
		 is_input = urb->input;
		 maxp = urb->maxpacket;
		 
		 info1 |= (2 << 12); /* EPS "high" */
		 if(urb->ep==0)// control
		 {		 
				 info1 |= (EHCI_TUNE_RL_HS << 28);
				 info1 |= 64 << 16;  /* usb2 fixed maxpacket */
				 info1 |= 1 << 14;	 /* toggle from qtd */
				 info2 |= (EHCI_TUNE_MULT_HS << 30) ;
		 }else//bulk
		 {
				 info1 |= (EHCI_TUNE_RL_HS << 28);
				 /* The USB spec says that high speed bulk endpoints
				  * always use 512 byte maxpacket.	But some device
				  * vendors decided to ignore that, and MSFT is happy
				  * to help them do so.  So now people expect to use
				  * such nonconformant devices with Linux too; sigh.
				  */
				 info1 |= max_packet(maxp) << 16;
				 info2 |= (EHCI_TUNE_MULT_HS << 30);
				 
		 }
		 //ehci_dbg("HW info: %08X\n",info1);
	 cached_qh.hw_info1 = cpu_to_hc32( info1);
	 cached_qh.hw_info2 = cpu_to_hc32( info2);
	 
	 
	 cached_qh.hw_next =QH_NEXT(dummy_qh->qh_dma);
	 cached_qh.hw_qtd_next = QTD_NEXT( qtd->qtd_dma);
	 cached_qh.hw_alt_next =EHCI_LIST_END();// QTD_NEXT(get_qtd_dummy());// 
 
 
		 if(urb->ep!=0){
				 if(get_toggle(dev,urb->ep))
						 cached_qh.hw_token |= cpu_to_hc32(QTD_TOGGLE);
				 else
						 cached_qh.hw_token &= ~cpu_to_hc32( QTD_TOGGLE);
 
				 //ehci_dbg("toggle for ep %x: %d %x\n",urb->ep,get_toggle(dev,urb->ep),qh->hw_token);
		 }
 
		 cached_qh.hw_token &= cpu_to_hc32( QTD_TOGGLE | QTD_STS_PING);
 
       #if 1
         
	    ehci_dma_map_bidir(&cached_qh,sizeof(struct ehci_qh));
		 for(qtd = cached_qh.qtd_head; qtd; qtd = qtd->next)
				 ehci_dma_map_bidir(qtd,sizeof(struct ehci_qtd));
 
		 
		 
		 //enable_OHCI1_IRQ();
		#endif
//		ehci->async->hw_next = QH_NEXT(cached_qh.qh_dma);
//		ehci_dma_map_bidir((void *) ehci->async, 32);

		
		 swi_mload_memcpy(qh, &cached_qh, 96);
 
		 if(enable_urb_debug) s_printf("ehci_int_working_callback_part1, timeout: %u\n",usb_timeout);
		 mode_int=1;
		 int cnt=0;
		 do{
		 cnt++;
		 ehci_int_working_callback_part1(interrupt_callback_hand, usb_timeout);
	 
	 
		 // start (link qh)
 
		 //disable_OHCI1_IRQ();
		 ehci_dma_unmap_bidir((dma_addr_t) ehci->async, 32);
		 ehci->async->hw_next = QH_NEXT(cached_qh.qh_dma);
		 ehci_dma_map_bidir((void *) ehci->async, 32);
		 //enable_OHCI1_IRQ();
 
		 
		 ehci_run(); 
	 
		 
		 
		 
		 retval=ehci_int_working_callback_part2();
		 if(cnt>3) break;
		 if(retval==-9) s_printf("retry\n");
		 }while(retval==-9);
		 mode_int=0;
		 if(enable_urb_debug) s_printf("urb retval: %i\n",retval);
		 if(retval!=0 || unplug_device!=0) 
			 {
			 if((ehci_readl(&ehci->regs->port_status[current_port]) & 5) !=5) unplug_device=1; 
			 //retval=-ETIMEDOUT;
			 }
		 
		 
		 
		 if(retval==0 || retval==-EBADDATA)
			 {
			 
			 ehci_stop();
			 os_sync_before_read(ehci->async, 32);
			 read_cache_data((void *)ehci->async, 32);
			 ehci->async->hw_next = QH_NEXT(dummy_qh->qh_dma);
				 //ehci->async->hw_next = QH_NEXT(dummy_qh->qh_dma/* ehci->async->qh_dma*/);
			 os_sync_after_write(ehci->async, 32);
			 ehci_run();
 
			 if(urb->ep!=0)
					 {
					 set_toggle(dev,urb->ep,(int_toggles /*qh->hw_token*/ & cpu_to_hc32(QTD_TOGGLE))?1:0);
					 }
			 /*
			 if(qh_end_transfer(cached_qh.qtd_head)!=0)
				 {
				 ehci->qtd_used = 0;
				 retval=-EBADDATA;
			  
				 }*/
			 
			 }
		 else
			 {
			 ehci_stop();
			 for(qtd = cached_qh.qtd_head; qtd; qtd = qtd->next)
				 {
				 ehci_dma_unmap_bidir(qtd->qtd_dma,sizeof(struct ehci_qtd));
				 }
 
			 swi_mload_memcpy_from_uncached(&cached_qh, qh, 32);
 
			 if(urb->ep!=0)
					 {
					 set_toggle(dev,urb->ep,(cached_qh.hw_token & cpu_to_hc32(QTD_TOGGLE))?1:0);
					 }
 
			 os_sync_before_read(ehci->async, 32);
			 read_cache_data((void *)ehci->async, 32);
			 ehci->async->hw_next = QH_NEXT(dummy_qh->qh_dma);
			  
			 os_sync_after_write(ehci->async, 32);
 
			 ehci_run();
			 //swi_mload_call_func((void *) ehci_wait, (void *) 1, &cached_qh);
			 //ehci_wait( 1, (void *) &cached_qh);
				 
			 }
		 
			 ehci_wait( 1, (void *) &cached_qh);
			 
	 
			   
 
		 if(urb->transfer_buffer_length){
				 if(urb->input)
						 ehci_dma_unmap_to(urb->transfer_dma,urb->transfer_buffer_length);
				 else
						 ehci_dma_unmap_from(urb->transfer_dma,urb->transfer_buffer_length);
		 }
		 if(urb->ep==0) //control message
				 ehci_dma_unmap_to(urb->setup_dma,sizeof (usbctrlrequest));
		 if(retval==0){
				 
				 return urb->actual_length;
		 }
		
		 return retval;
 }

s32 ehci_control_message(struct ehci_device *dev,u8 bmRequestType,u8 bmRequest,u16 wValue,u16 wIndex,u16 wLength,void *buf)
{
        struct ehci_urb urb;
        usbctrlrequest *req = ehci->ctrl_buffer;
        if(verbose)
          ehci_dbg ( "control msg: rt%02X r%02X v%04X i%04X s%04x %p\n", bmRequestType, bmRequest, wValue, wIndex,wLength,buf);
        req->bRequestType = bmRequestType;
        req->bRequest = bmRequest;
        req->wValue = swab16(wValue);
        req->wIndex = swab16(wIndex);
        req->wLength = swab16(wLength);
        urb.setup_buffer = req;
        urb.ep = 0;
        urb.input = (bmRequestType&USB_CTRLTYPE_DIR_DEVICE2HOST)!=0;
        urb.maxpacket = 64;
        urb.transfer_buffer_length = wLength;
        if (((u32)buf) > 0x13880000){// HW cannot access this buffer, we allow this for convenience
                int ret;
                urb.transfer_buffer = USB_Alloc(wLength);
                if (verbose)
                ehci_dbg("alloc another buffer %p %p\n",buf,urb.transfer_buffer);
                memcpy(urb.transfer_buffer,buf,wLength);
                ret =  ehci_do_urb(dev,&urb);
                memcpy(buf,urb.transfer_buffer,wLength);
                USB_Free(urb.transfer_buffer);
                return ret;
        }
        else{
                urb.transfer_buffer = buf;
                return ehci_do_urb(dev,&urb);
        }
}
s32 ehci_bulk_message(struct ehci_device *dev,u8 bEndpoint,u32 wLength,void *rpData)
{
        struct ehci_urb urb;
        s32 ret;
        urb.setup_buffer = NULL;
        urb.ep = bEndpoint;
        urb.input = (bEndpoint&0x80)!=0;
        urb.maxpacket = 512;
        urb.transfer_buffer_length = wLength;
        urb.transfer_buffer = rpData;
        if(verbose)
                ehci_dbg ( "bulk msg: ep:%02X size:%02X addr:%04X", bEndpoint, wLength, rpData);
        ret= ehci_do_urb(dev,&urb);
        if(verbose)
                ehci_dbg ( "==>%d\n", ret);
        return ret;
}



int ehci_reset_port_old(int port)
{
        u32 __iomem	*status_reg = &ehci->regs->port_status[port];
        struct ehci_device *dev = &ehci->devices[port];
        u32 status ;//= ehci_readl(status_reg);
        int retval = 0,i;
		u32 g_status;
        dev->id = 0;

		g_status=ehci_readl(&ehci->regs->status);
 
	

	    // clear status flags
		
		//ehci_writel( g_status & INTR_MASK,&ehci->regs->status);
		//g_status=ehci_readl (&ehci->regs->command);

		status = ehci_readl(status_reg);

		if ((PORT_OWNER&status) || !(PORT_CONNECT&status))
        {
               // ehci_writel( PORT_OWNER,status_reg);
                ehci_dbg ( "port %d had no usb2 device connected at startup %X \n", port,ehci_readl(status_reg));
                return -ENODEV;// no USB2 device connected
        }
        ehci_dbg ( "port %d has usb2 device connected! reset it...\n", port);


		for(i=0;i<4;i++)  //4 retries
		{ 
		u32 status;
		ehci_writel( 0x1803,status_reg);
        ehci_msleep(10);
        ehci_writel( 0x1903,status_reg);
		ehci_msleep(100);// wait 100ms for the reset sequence
        ehci_writel( 0x1001,status_reg);
		retval = handshake(status_reg, status_reg,
                           PORT_RESET, 0, 2*1000);

		/*	
			for(n=0;n<10;n++)
			{
				ehci_msleep(50);
				status = ehci_readl(status_reg);
				if((status & PORT_PE) || (status & 1)==0) break;
			}*/
		status = ehci_readl(status_reg);
		if ((PORT_OWNER&status) || !(PORT_CONNECT&status) || !(status & PORT_PE) || PORT_USB11(status))
			{
			retval=-1;
			continue;
			}
        //ehci_writel( PORT_OWNER|PORT_POWER|PORT_RESET,status_reg);
    
       
        //ehci_writel( ehci_readl(status_reg)& (~PORT_RESET),status_reg);
       
	
        if (retval == 0) 
			{
			int old_time;
               /* ehci_dbg ( "port %d reset error %d\n",
                          port, retval);*/
               
			ehci_dbg ( "port %d reseted status:%04x...\n", port,ehci_readl(status_reg));
			ehci_msleep(100);
			
			old_time=usb_timeout;
			usb_timeout=400*1000;
			// now the device has the default device id
			retval = ehci_control_message(dev,USB_CTRLTYPE_DIR_DEVICE2HOST,
                             USB_REQ_GETDESCRIPTOR,USB_DT_DEVICE<<8,0,sizeof(dev->desc),&dev->desc);
        
			if (retval >= 0) 
				{
					

				retval = ehci_control_message(dev,USB_CTRLTYPE_DIR_HOST2DEVICE,
										  USB_REQ_SETADDRESS,port+1,0,0,0);
					
				}
			usb_timeout=old_time;
			if(retval>=0)  break;
			}
		}
        

		if (retval < 0) {
           
                return retval;
        }

        dev->toggles = 0;

        dev->id = port+1;
       // ehci_dbg ( "device %d: %X %X...\n", dev->id,le16_to_cpu(dev->desc.idVendor),le16_to_cpu(dev->desc.idProduct));
        return retval;
}

#if 1
void ehci_adquire_port(int port)
{
	u32 __iomem	*status_reg = &ehci->regs->port_status[port];
	u32 status = ehci_readl(status_reg); 

	//change owner, port disabled
	if(!(status & PORT_OWNER))
		status ^= PORT_OWNER;
	status &= ~(PORT_PE | PORT_RWC_BITS);
	ehci_writel(status, status_reg);	
	ehci_mdelay(5);
	status = ehci_readl(status_reg);
	if(status & PORT_OWNER)
		status ^= PORT_OWNER;
	status &= ~(PORT_PE | PORT_RWC_BITS);
	ehci_writel(status, status_reg);	
	ehci_mdelay(5);
	

	//enable port	
	ehci_writel( 0x1801,status_reg);
    ehci_mdelay(5);
}
int ehci_reset_usb_port(int port)
{
    u32 __iomem   *status_reg = &ehci->regs->port_status[port];
    u32 status = ehci_readl(status_reg);
   
    int i, retval = 0;
   
    if ((PORT_OWNER&status) || !(PORT_CONNECT&status))
    {
	if(PORT_OWNER&status)
		{
		ehci_adquire_port(port);
		}
           // ehci_writel( PORT_OWNER,status_reg);
            return -ENODEV;// no USB2 device connected
    }
       

    for(i=0;i<4;i++)  //4 retries
    {       
      status &= ~PORT_PE;
      status |= PORT_RESET | PORT_POWER;       
        ehci_writel( status,status_reg);
        ehci_msleep(60);// wait 60ms for the reset sequence
        status=ehci_readl(status_reg);
        status &= ~(PORT_RWC_BITS | PORT_RESET); /* force reset to complete */
      ehci_writel( status,status_reg);
        ehci_msleep(50);
	
        retval = handshake(status_reg, status_reg,
                           PORT_RESET, 0, 5*1000);
		
        if (retval != 0) {
                status=ehci_readl(status_reg);
				
                return -2000;
        }
        status=ehci_readl(status_reg);
      if (status & PORT_PE) break; //port enabled
   }
    
   if (!(status & PORT_PE)) {
            // that means is low speed device so release
         status |= PORT_OWNER;
         status &= ~PORT_RWC_BITS;
         ehci_writel( status, status_reg);   
         ehci_writel( PORT_OWNER, status_reg);    
           ehci_msleep(10);
           //status = ehci_readl(status_reg);   
           return -1119;
   }
   return retval;   
}


int ehci_init_port(int port)
{	
        struct ehci_device *dev = &ehci->devices[port];
        int retval = 0;
        dev->id = 0;
	int i;
	ehci_msleep(50);
	for(i=0;i<3;i++)
	{
        
        
        s_printf("getting USB_REQ_GETDESCRIPTOR\n");ehci_msleep(50);
        // sdlog("getting USB_REQ_GETDESCRIPTOR\n");   
        // now the device has the default device id
        retval = ehci_control_message(dev,USB_CTRLTYPE_DIR_DEVICE2HOST,
                             USB_REQ_GETDESCRIPTOR,USB_DT_DEVICE<<8,0,sizeof(dev->desc),&dev->desc);
        //retval=-1;
        if (retval < 0) {
        		s_printf("unable to get device desc...\n");ehci_msleep(50);
        		// sdlog("error getting USB_REQ_GETDESCRIPTOR\n");
                //ehci_dbg ( "unable to get device desc...\n");
                retval=-2201;
                ehci_msleep(100);
                //return retval;
        }
        else 
        {
        	if(dev->desc.idProduct==0x2077 && dev->desc.idVendor==0x950b)
			{
			/*
				u32 __iomem   *status_reg = &ehci->regs->port_status[port];
				u32 status=ehci_readl(status_reg);
				//s_printf("usblan detected 0, release\n");
				status |= PORT_OWNER;
				status &= ~PORT_RWC_BITS;
				ehci_writel( status, status_reg);
				*/
				ehci_release_port( port);
				return -1120;
			}
        	break;
        }
    }
    if (retval < 0)
    {
		for(i=0;i<3;i++)
		{
	        
	        if(ehci_reset_usb_port(port)==-1119) return -1119;
	        ehci_msleep(100);
	        //my_sprint("getting USB_REQ_GETDESCRIPTOR",NULL);ehci_msleep(50);
	        s_printf("getting USB_REQ_GETDESCRIPTOR - reset\n");   
	        // now the device has the default device id
	        retval = ehci_control_message(dev,USB_CTRLTYPE_DIR_DEVICE2HOST,
	                             USB_REQ_GETDESCRIPTOR,USB_DT_DEVICE<<8,0,sizeof(dev->desc),&dev->desc);
	        if (retval < 0) {
	        		//my_sprint("unable to get device desc...",NULL);ehci_msleep(50);
	        		s_printf("error getting USB_REQ_GETDESCRIPTOR\n");
	                //ehci_dbg ( "unable to get device desc...\n");
	                retval=-2201;
	                
	                //return retval;
	        }
	         else 
		    {
		    	if(dev->desc.idProduct==0x2077 && dev->desc.idVendor==0x950b)
				{
					/*u32 __iomem   *status_reg = &ehci->regs->port_status[port];
					u32 status=ehci_readl(status_reg);
					//s_printf("usblan detected 0, release\n");
					status |= PORT_OWNER;
					status &= ~PORT_RWC_BITS;
					ehci_writel( status, status_reg);*/
					ehci_release_port( port);
					return -1120;
				}
		    	break;
		    }
	    }
    }
    
    if (retval < 0)
    {
		for(i=0;i<3;i++)
		{
			ehci_adquire_port(port);
	        ehci_msleep(100);
	        if(ehci_reset_usb_port(port)==-1119) return -1119;
	        ehci_msleep(100);
	        //my_sprint("getting USB_REQ_GETDESCRIPTOR",NULL);ehci_msleep(50);
	        s_printf("getting USB_REQ_GETDESCRIPTOR - adquire - reset\n");   
	        // now the device has the default device id
	        retval = ehci_control_message(dev,USB_CTRLTYPE_DIR_DEVICE2HOST,
	                             USB_REQ_GETDESCRIPTOR,USB_DT_DEVICE<<8,0,sizeof(dev->desc),&dev->desc);
	        if (retval < 0) {
	        		//my_sprint("unable to get device desc...",NULL);ehci_msleep(50);
	        		s_printf("error getting USB_REQ_GETDESCRIPTOR\n");
	                //ehci_dbg ( "unable to get device desc...\n");
	                retval=-2201;
	                
	                //return retval;
	        }
	        else break;
	    }
    }

    if (retval < 0) 	return -2201;
    
        s_printf("USB_REQ_GETDESCRIPTOR ok\n");

        int cnt=0;
        do{
	        ehci_msleep(50);
	        s_printf("trying USB_REQ_SETADDRESS: %d\n",cnt);
	        retval = ehci_control_message(dev,USB_CTRLTYPE_DIR_HOST2DEVICE,
	                                      USB_REQ_SETADDRESS,port+1,0,0,0);
	        if (retval < 0) {
	        		//my_sprint("unable to set device addr...",NULL);
	                ehci_dbg ( "unable to set device addr...\n");	                
	                retval=-8000-cnt;
	                s_printf("unable to set device addr: %d\n",cnt);
	                //return retval;
	        cnt++;
			}

	        else  s_printf("USB_REQ_SETADDRESS ok: %d\n",cnt);
	
	        dev->toggles = 0;
	
	        dev->id = port+1;

			if(retval>=0) break;
	
	        USB_ClearHalt(dev, 0);
	        //USB_ClearHalt(dev, 0x80);
			ehci_msleep(50);
	        s_printf("checking USB_REQ_GETDESCRIPTOR\n");
	        retval = ehci_control_message(dev,USB_CTRLTYPE_DIR_DEVICE2HOST,
	                             USB_REQ_GETDESCRIPTOR,USB_DT_DEVICE<<8,0,sizeof(dev->desc),&dev->desc);
	                             
	        
	        if (retval < 0) {
	        		//my_sprint("unable to get device desc...",NULL);
	                ehci_dbg ( "unable to get device desc...\n");
	                s_printf("error checking USB_REQ_GETDESCRIPTOR\n");
	                retval=-2242;
	                dev->id =0;
	                //return retval;
	        }
			
			else s_printf("ok checking USB_REQ_GETDESCRIPTOR\n");
        	cnt++;
        }while(retval<0 && cnt<5);
        
        if(retval>=0)s_printf("init ok\n");
        return retval;
}

#endif
int ehci_reset_port(int port)
{
	int retval;
	//retval=ehci_reset_port_old(port);

	ehci_writel (STS_INT, &ehci->regs->intr_enable);
	
	retval=ehci_reset_usb_port(port);
	if(retval>=0)retval=ehci_init_port(port);
	//if(retval<0) ehci_release_port(port);

	ehci_writel (STS_PCD, &ehci->regs->intr_enable);

	return retval;
}

int ehci_reset_port2(int port)
{
u32 __iomem	*status_reg = &ehci->regs->port_status[port];
//int n;
u32 g_mstatus;	
u32 command;

ehci_writel (STS_INT, &ehci->regs->intr_enable);

int ret=ehci_reset_port_old(port);
if(ret<0/*==-ENODEV || ret==-ETIMEDOUT*/)
	{

	
	g_mstatus=ehci_readl(&ehci->regs->status) & INTR_MASK;
	ehci_writel (g_mstatus, &ehci->regs->status);
	
	
	command=ehci_readl( &ehci->regs->command);
	

	ehci_msleep(10); // power off 
	ehci_writel( 0x1803,status_reg);
    ehci_msleep(50);
    ehci_writel( 0x1903,status_reg);
	ehci_msleep(100);
    ehci_writel( 0x1001,status_reg);
	/*
	for(n=0;n<10;n++)
		{
		u32 status;
		ehci_msleep(50);
		status = ehci_readl(status_reg);
		if((status & PORT_PE) || (status & 1)==0) break;
		}
	*/
	
	}

g_mstatus=ehci_readl(&ehci->regs->status) & INTR_MASK;
ehci_writel (g_mstatus, &ehci->regs->status);
ehci_writel (STS_PCD, &ehci->regs->intr_enable);
command=ehci_readl( &ehci->regs->command);

return ret;
}
        
int ehci_reset_device(struct ehci_device *dev)
{
        return ehci_reset_port(dev->port);
}
#include "usbstorage.h"

int  ehci_adquire_usb_port(int port)
{
	u32 __iomem	*status_reg = &ehci->regs->port_status[port];
	u32 status = ehci_readl(status_reg); 

	//if(!(PORT_CONNECT&status)) return -1; //port not connected

	//change owner, port disabled
	if(!(status & PORT_OWNER))
		status ^= PORT_OWNER;
	status &= ~(PORT_PE | PORT_RWC_BITS);
	ehci_writel(status, status_reg);	
	ehci_mdelay(5);
	status = ehci_readl(status_reg);
	if(status & PORT_OWNER)
		status ^= PORT_OWNER;
	status &= ~(PORT_PE | PORT_RWC_BITS);
	ehci_writel(status, status_reg);	
	ehci_mdelay(5);
	

	//enable port	
	ehci_writel( 0x1801,status_reg);
    ehci_mdelay(60);
    return 1;
}

int ehci_discover(void)
{
        int i,ret,from,to;
		u32 status;
		ret=-1;
		return 0;

		//current_port=use_usb_port1!=0;
		current_port=from=to=0;
		//if(use_usb_port1==0)from=to=0;
		if(use_usb_port1==1)from=to=1;
		else if(use_usb_port1==2)
		{
			from=0;
			to=1;
		}		

        // precondition: the ehci should be halted
		
        for(i = from;i<=to; i++)
        {
		
            struct ehci_device *dev = &ehci->devices[i];
            dev->port = i;

			status = ehci_readl(&ehci->regs->port_status[i]);
			if(!(status & 1)) 
				ehci_adquire_usb_port(i);					
				
			status = ehci_readl(&ehci->regs->port_status[i]);

			
			if(status & 1)
			{
				ret=ehci_reset_port2(i);
				ehci_msleep(20);
				status=ehci_readl(&ehci->regs->port_status[i]);

				if(ret<0 || (status & 0x3905)!=0x1005)
					ret=ehci_reset_port(i);
					
				if(ret==-1)	ret= -101;
			}			

        }        
        return ret;
}
int ehci_release_port(int port)
{
	u32 __iomem	*status_reg = &ehci->regs->port_status[port];
//	u32 status = ehci_readl(status_reg);		 
	ehci_writel( PORT_OWNER,status_reg);
	return 0;
}

/* wii: quickly release non ehci or not connected ports,
 as we can't kick OHCI drivers laters if we discover a device for them.
*/
int ehci_release_ports(void)
{
		int i;
		u32 __iomem *status_reg ;
		for(i = 0;i<2; i++){
		  status_reg = &ehci->regs->port_status[i];
			ehci_writel( PORT_OWNER,status_reg); // release port.
		}
		return 0;
}

#if 0
int ehci_release_ports(void)
{
        int i;
        u32 __iomem	*status_reg = &ehci->regs->port_status[2];
        while(ehci_readl(&ehci->regs->port_status[2]) == 0x1000) ehci_usleep(100);// wait port 2 to init
        ehci_msleep(100);// wait another msec..
        for(i = 0;i<ehci->num_port; i++){
          status_reg = &ehci->regs->port_status[i];
          u32 status = ehci_readl(status_reg);
          if (i==2 || !(PORT_CONNECT&status) || PORT_USB11(status))
            ehci_writel( PORT_OWNER,status_reg); // release port.
        }
        return 0;
}
#endif
int ehci_release_externals_usb_ports(void)
{
        int i;
        u32 __iomem	*status_reg = &ehci->regs->port_status[2];
      
        for(i = 0;i<2; i++){
          status_reg = &ehci->regs->port_status[i];
          u32 status = ehci_readl(status_reg);
		    if(!(status & PORT_OWNER))	ehci_writel( PORT_OWNER,status_reg); // release port.
        }
        return 0;
}

int ehci_open_device(int vid,int pid,int fd)
{
        int i;
       // for(i=0;i<ehci->num_port;i++)
       // {
		
		i=use_usb_port1!=0;
		
                //ehci_dbg("try device: %d\n",i);
                if(ehci->devices[i].fd == 0 &&
                   le16_to_cpu(ehci->devices[i].desc.idVendor) == vid &&
                   le16_to_cpu(ehci->devices[i].desc.idProduct) == pid)
                {
                        //ehci_dbg("found device: %x %x\n",vid,pid);
                        ehci->devices[i].fd = fd;
                        return fd;
                }
        //}
        return -6;
}
int ehci_close_device(struct ehci_device *dev)
{
        if (dev)
                dev->fd = -1;
        return 0;
}

void ehci_close_devices()
{
	struct ehci_device *dev = 
	dev = &ehci->devices[0];
	dev->id=0;
	dev->fd=-1;
	dev = &ehci->devices[1];
	dev->id=0;
	dev->fd=-1;

}

 void * ehci_fd_to_dev(int fd)
{
        int i;
       // for(i=0;i<ehci->num_port;i++)
	    current_port=use_usb_port1!=0;
	    
		i=use_usb_port1!=0;
		
       
		{
		  
                struct ehci_device *dev = &ehci->devices[i];

				return dev; // return always device[0]

				#if 0
                //ehci_dbg ( "device %d:fd:%d %X %X...\n", dev->id,dev->fd,le16_to_cpu(dev->desc.idVendor),le16_to_cpu(dev->desc.idProduct));
                if(dev->fd == fd){
                        return dev;
				
                }
				#endif
        }
        ehci_dbg("unknown fd! %d\n",fd);
        return 0;
}
#define g_ehci #error
int ehci_get_device_list(u8 maxdev,u8 b0,u8*num,u16*buf)
{
        int i,j = 0;
      //  for(i=0;i<ehci->num_port && j<maxdev ;i++)
	    current_port=use_usb_port1!=0;
		i=current_port;
		
        {
                struct ehci_device *dev = &ehci->devices[i];
                if(dev->id != 0){
                        //ehci_dbg ( "device %d: %X %X...\n", dev->id,le16_to_cpu(dev->desc.idVendor),le16_to_cpu(dev->desc.idProduct));
                        buf[j*4] = 0;
                        buf[j*4+1] = 0;
                        buf[j*4+2] = le16_to_cpu(dev->desc.idVendor);
                        buf[j*4+3] = le16_to_cpu(dev->desc.idProduct);
                        j++;
                }
        }
        //ehci_dbg("found %d devices\n",j);
        *num = j;
        return 0;
}



#include "usb.c"
#include "usbstorage.c"
