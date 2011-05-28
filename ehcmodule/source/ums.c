#ifndef HOMEBREW
#include "syscalls.h"
#include "ios_usbstorage.h"
#include <stdarg.h>

#define UMS_BASE (('U'<<24)|('M'<<16)|('S'<<8))
#define USB_IOCTL_UMS_INIT          (UMS_BASE+0x1)
#define USB_IOCTL_UMS_GET_CAPACITY       (UMS_BASE+0x2)
#define USB_IOCTL_UMS_READ_SECTORS       (UMS_BASE+0x3)
#define USB_IOCTL_UMS_WRITE_SECTORS  (UMS_BASE+0x4)
#define USB_IOCTL_UMS_READ_STRESS  (UMS_BASE+0x5)
#define USB_IOCTL_UMS_SET_VERBOSE  (UMS_BASE+0x6)

static int fd;
static u32 sector_size;
static u32 num_sector;
static int heap;
/* */
void ums_init(void)
{
    ioctlv *vec;
    u32 *p_sector_size;
    fd = os_open("/dev/usb2", 1);

    if (fd < 0)
    {
        debug_printf("unable to open /dev/usb2 %d\n", fd);
        return ;
    }

    os_ioctlv(fd, USB_IOCTL_UMS_INIT, 0, 0, 0);

    heap = os_heap_create((void*)0x13898000, 0x8000);
    vec = os_heap_alloc(heap, sizeof(ioctlv));
    p_sector_size = os_heap_alloc(heap, sizeof(u32));

    vec[0].data = p_sector_size;
    vec[0].len = 4;
    num_sector = os_ioctlv(fd, USB_IOCTL_UMS_GET_CAPACITY, 0, 1, vec);
    sector_size = *p_sector_size;
    debug_printf("found device %d %d: %dkB\n", sector_size, num_sector, sector_size*(num_sector / 1024));
    os_heap_free(heap, vec);
    os_heap_free(heap, p_sector_size);
}

void *ums_alloc(int size)
{
    return os_heap_alloc(heap, size);
}

void ums_free(void *ptr)
{
    os_heap_free(heap, ptr);
}

s32 ums_read_sectors(u32 sector, u32 numSectors, void *buffer)
{
    ioctlv *vec;
    u32 *p_sector;
    u32 *p_nsector;
    s32 ret;
    vec = os_heap_alloc(heap, sizeof(ioctlv) * 3);
    p_sector = os_heap_alloc(heap, sizeof(u32));
    p_nsector = os_heap_alloc(heap, sizeof(u32));
    *p_nsector = numSectors;
    *p_sector = sector;

    vec[0].data = p_sector;
    vec[0].len = 4;
    vec[1].data = p_nsector;
    vec[1].len = 4;
    vec[2].data = buffer;
    vec[2].len = sector_size * numSectors;
    ret = os_ioctlv(fd, USB_IOCTL_UMS_READ_SECTORS, 2, 1, vec);
    // no need to flush cache, this is done by ehc..
    os_heap_free(heap, vec);
    os_heap_free(heap, p_sector);
    os_heap_free(heap, p_nsector);
    return ret;
}

void ums_close(void)
{
    os_close(fd);
    os_heap_destroy(heap);
    fd = -1;
    heap = -1;
}

#endif
