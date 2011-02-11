#include "syscalls.h"
#include <string.h>
#include "ehci_types.h"
#include "usb.h"
#include "ehci.h"

static  int heap=-1;

void ehci_usleep(int usec);
void ehci_msleep(int msec);

extern u8 heap_space2[0xe000];

int usb_os_init(void)
{
        heap = os_heap_create(heap_space2, 0xe000);
		//heap = os_heap_create((void*)0x13890000, 0x8000);
        if(heap<0)
        {
          return -1;
        }
        return 0;
}

void read_cache_data(char *in, int len)
{
int n;
char t;

	for(n=0;n<len;n++) t=*in++;
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
		read_cache_data((void *) buf, len);
}
void ehci_dma_unmap_from(dma_addr_t buf,size_t len)
{
        os_sync_before_read((void*)buf, len);
		read_cache_data((void *) buf, len);
}
void ehci_dma_unmap_bidir(dma_addr_t buf,size_t len)
{
        os_sync_before_read((void*)buf, len);
		read_cache_data((void *) buf, len);
}



void *USB_Alloc(int size)
{
  void * ret = 0;
  ret= os_heap_alloc_aligned(heap, size, 32);
 // ret= os_heap_alloc(heap, size);
  if(ret==0)
	{
    os_puts("USB Alloc: not enough memory!\n");
     while(1) {swi_mload_led_on();ehci_msleep(200);swi_mload_led_off();ehci_msleep(200);}
	}
  return ret;
}
void USB_Free(void *ptr)
{
        return os_heap_free(heap, ptr);
}

