#include "ehci_interrupt.h"
#include "swi_mload.h"

#define ehci_readl(a) (*((volatile u32*)(a)))
#define ehci_writel(v,a) do{*((volatile u32*)(a))=(v);}while(0)
#define get_timer()  (*(((volatile u32*)0x0D800010)))

void enable_EHCI_IRQ(void)
{   
	*((volatile u32 *) 0x0d0400cc)|=1<<15;
}

void disable_EHCI_IRQ(void)
{
	*((volatile u32 *) 0x0d0400cc) &=~(1<<15);
}

#if 0

void enable_OHCI0_IRQ(void)
{
	*((volatile u32 *) 0x0d0400cc)|=1<<11;
}

void disable_OHCI0_IRQ(void)
{
	*((volatile u32 *) 0x0d0400cc) &=~(1<<11);
}

void enable_OHCI1_IRQ(void)
{
	*((volatile u32 *) 0x0d0400cc)|=1<<12;
}

void disable_OHCI1_IRQ(void)
{
	*((volatile u32 *) 0x0d0400cc) &=~(1<<12);
}

#endif


int ehci1_queuehandle=-1;


void init_thread_ehci(void)
{
	disable_EHCI_IRQ();


	ehci1_queuehandle= os_message_queue_create( USB_Alloc(4*32)/*os_heap_alloc(heaphandle, 4*32)*/, 32);

	os_unregister_event_handler(DEV_EHCI);
	os_register_event_handler(DEV_EHCI, ehci1_queuehandle, 0); // register interrupt event handler

	enable_EHCI_IRQ();
	os_software_IRQ(DEV_EHCI);

}

static int (*working_callback)(u32 flags)= NULL;

static void (*passive_callback)(u32 flags)= NULL;

static int private_timer_id=-1;
static int remote_message=0;

void ehci_int_working_callback_part1( int (*callback)(u32 flags), u32 timeout)
{

	private_timer_id=os_create_timer(timeout, timeout*10, ehci1_queuehandle, 1);

	swi_mload_set_register(0x0d800038,(1<<DEV_EHCI)); // clear interrupt flag
	swi_mload_clr_register(0x0d80003c,(1<<DEV_EHCI)); // unmask interrupt flag

	working_callback=callback;
	
	ehci_writel (INTR_MASK, &ehci->regs->intr_enable);

	os_software_IRQ(DEV_EHCI); // enable and mask interrupt flag
	
}

int ehci_int_working_callback_part2(void)
{
static int message=0;

	message=-ETIMEDOUT;

	os_message_queue_receive(ehci1_queuehandle, (void*)&message, 0); // waits for interrupt or timeout

	ehci_writel (0, &ehci->regs->intr_enable); // disable interrupts flags
	working_callback=NULL;	// disable callback

	os_stop_timer(private_timer_id); // stops the timeout timer
	os_destroy_timer(private_timer_id);
    private_timer_id=-1;
	

	if(message==0) // build message response
		{
		message=remote_message;
		}
	else message=-ETIMEDOUT;
	
	os_software_IRQ(DEV_EHCI); // enable and mask interrupt flag
 
	return message;
}

void ehci_int_passive_callback( void (*callback)(u32 flags))
{

	passive_callback=callback;
	working_callback=NULL;
    
}




void int_send_device_message(int device);


int ehci_vector(void)
{
int ret=0;
u32 flags;

int message=1;

	*((volatile u32 *)0x0d80003c ) &= ~(1<<DEV_EHCI); // disable EHCI interrupt

	flags=ehci_readl (&ehci->regs->status);
	
	if(working_callback)
		{

		message= working_callback(flags);
		
		if(((int)message)<=0)
			{
			working_callback=NULL;
		
			remote_message=message;
			int_send_device_message(DEV_EHCI);
			//ret=1;

			ehci_writel (flags & INTR_MASK, &ehci->regs->status);
			}
		else
			{
			ehci_writel (flags & INTR_MASK, &ehci->regs->status);	
			//temp=ehci_readl( &ehci->regs->command);
			
			*((volatile u32 *)0x0d80003c ) |= 1<<DEV_EHCI;
			*((volatile u32 *)0x0d800038 ) |= 1<<DEV_EHCI;
			
			}

		}
	else
	if(passive_callback)
		{

		passive_callback(flags);
		ehci_writel (flags & INTR_MASK, &ehci->regs->status);
		*((volatile u32 *)0x0d80003c ) |= 1<<DEV_EHCI;
		*((volatile u32 *)0x0d800038 ) |= 1<<DEV_EHCI;
		//ret=1; // remote int_send_device_message

		}
	else
		{

		ehci_writel (flags & INTR_MASK, &ehci->regs->status);
		*((volatile u32 *)0x0d80003c ) |= 1<<DEV_EHCI;
		*((volatile u32 *)0x0d800038 ) |= 1<<DEV_EHCI;
		//ret=1; // remote int_send_device_message

		}


// ret==1: send message to EHCI queue
return ret;
}


