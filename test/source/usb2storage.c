/****************************************************************************
* WiiMC
* usb2storage.c -- USB mass storage support, inside starlet
* Copyright (C) 2008 Kwiirk
* Improved for homebrew by rodries and Tantric
* 
* IOS 202 and the ehcmodule must be loaded before using this!
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any
* damages arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any
* purpose, including commercial applications, and to alter it and
* redistribute it freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you
* must not claim that you wrote the original software. If you use
* this software in a product, an acknowledgment in the product
* documentation would be appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and
* must not be misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source
* distribution.
* 
***************************************************************************/

#include <gccore.h>

#include <ogc/lwp_heap.h>
#include <malloc.h>
#include <ogc/disc_io.h>
#include <ogc/usbstorage.h>
#include <ogc/mutex.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_watchdog.h>


//#define DEBUG_USB2

#ifdef DEBUG_USB2
#define debug_printf(fmt, args...) \
 fprintf(stderr, "%s:%d:" fmt, __FUNCTION__, __LINE__, ##args)
#else
#define debug_printf(fmt, args...)
#endif // DEBUG_USB2

#define UMS_BASE (('U'<<24)|('M'<<16)|('S'<<8))
#define USB_IOCTL_UMS_INIT      (UMS_BASE+0x1)
#define USB_IOCTL_UMS_GET_CAPACITY    (UMS_BASE+0x2)
#define USB_IOCTL_UMS_READ_SECTORS    (UMS_BASE+0x3)
#define USB_IOCTL_UMS_WRITE_SECTORS  (UMS_BASE+0x4)
#define USB_IOCTL_UMS_READ_STRESS  (UMS_BASE+0x5)
#define USB_IOCTL_UMS_SET_VERBOSE  (UMS_BASE+0x6)
#define USB_IOCTL_UMS_IS_INSERTED  (UMS_BASE+0x7)
#define USB_IOCTL_UMS_USB_MODE   (UMS_BASE+0x8)
#define USB_IOCTL_UMS_GET_USBLAN_PORT (UMS_BASE+0x9)

#define USB_IOCTL_UMS_UMOUNT   (UMS_BASE+0x10)
#define USB_IOCTL_UMS_START   (UMS_BASE+0x11)
#define USB_IOCTL_UMS_STOP   (UMS_BASE+0x12)
#define USB_IOCTL_UMS_EXIT   (UMS_BASE+0x16)

#define UMS_HEAPSIZE     2*1024
#define UMS_MAXPATH 16

static s32 hId = -1;
static s32 __usb2fd = -1;
static u32 sector_size;
static mutex_t usb2_mutex = LWP_MUTEX_NULL;
static u8 *fixed_buffer = NULL;

#define ROUNDDOWN32(v)    (((u32)(v)-0x1f)&~0x1f)
#define USB2_BUFFER 128*1024
static heap_cntrl usb2_heap;
static u8 __usb2_heap_created = 0;

static DISC_INTERFACE __io_usb1storage;
static int usb1disc_inited = 0;
extern const DISC_INTERFACE __io_usb2storage;
static int currentMode = 2; // 1 = use USB1 interface, 2 = use USB2 interface

static s32 USB2CreateHeap()
{
    u32 level;
    void *usb2_heap_ptr;

    if (__usb2_heap_created != 0)
        return IPC_OK;

    _CPU_ISR_Disable(level);

    usb2_heap_ptr = (void *) ROUNDDOWN32(((u32)SYS_GetArena2Hi() - (USB2_BUFFER + (4 * 1024))));

    if ((u32) usb2_heap_ptr < (u32) SYS_GetArena2Lo())
    {
        _CPU_ISR_Restore(level);
        return IPC_ENOMEM;
    }

    SYS_SetArena2Hi(usb2_heap_ptr);
    __lwp_heap_init(&usb2_heap, usb2_heap_ptr, (USB2_BUFFER + (4 * 1024)), 32);
    __usb2_heap_created = 1;
    _CPU_ISR_Restore(level);
    return IPC_OK;
}

static s32 USB2Storage_Initialize()
{
    char *devicepath = NULL;

    if (usb2_mutex == LWP_MUTEX_NULL)
        LWP_MutexInit(&usb2_mutex, false);

    if (hId == -1)
        hId = iosCreateHeap(UMS_HEAPSIZE);

    if (hId < 0)
    {
        debug_printf("error IPC_ENOMEM\n");
        return IPC_ENOMEM;
    }

    if (USB2CreateHeap() != IPC_OK)
    {
        debug_printf("error USB2 IPC_ENOMEM\n");
        return IPC_ENOMEM;
    }

    if (fixed_buffer == NULL)
        fixed_buffer = __lwp_heap_allocate(&usb2_heap, USB2_BUFFER);

    LWP_MutexLock(usb2_mutex);

    if (__usb2fd <= 0)
    {
        devicepath = iosAlloc(hId, UMS_MAXPATH);

        if (devicepath == NULL)
        {
            LWP_MutexUnlock(usb2_mutex);
            return IPC_ENOMEM;
        }

        snprintf(devicepath, USB_MAXPATH, "/dev/usb2");
        __usb2fd = IOS_Open(devicepath, 0);
        iosFree(hId, devicepath);
    }

    if (__usb2fd <= 0)
        return -1;

    return 0;
}

static s32 USB2Storage_Open()
{
    s32 ret = USB_OK;
    u32 size = 0;

    if (USB2Storage_Initialize() < 0)
        return -1;

    ret = IOS_IoctlvFormat(hId, __usb2fd, USB_IOCTL_UMS_INIT, ":");

    debug_printf("usb2 init value: %i\n", ret);

    if (ret < 0)
        debug_printf("usb2 error init\n");
    else
        size = IOS_IoctlvFormat(hId, __usb2fd, USB_IOCTL_UMS_GET_CAPACITY, ":i",
                                &sector_size);

    debug_printf("usb2 GET_CAPACITY: %d\n", size);

    if (ret < 0 || size <= 0)
        ret = -2012;
    else
        ret = 1;

    debug_printf("usb2 USB2Storage_Open ret: %d\n", ret);

    return ret;
}

void USB2Storage_Close()
{
    if (__usb2fd <= 0)
        return ;

    IOS_Close(__usb2fd);

    __usb2fd = -1;
}

static inline int is_MEM2_buffer(const void *buffer)
{
    u32 high_addr = ((u32) buffer) >> 24;
    return (high_addr == 0x90) || (high_addr == 0xD0);
}

void USB2Enable(bool enable)
{
    if (!usb1disc_inited)
    {
        usb1disc_inited = 1;
        memcpy(&__io_usb1storage, &__io_usbstorage, sizeof(DISC_INTERFACE));
    }

    if (!enable)
    {
        memcpy(&__io_usbstorage, &__io_usb1storage, sizeof(DISC_INTERFACE));
    }

    else
    {
        USB2Storage_Initialize();
        memcpy(&__io_usbstorage, &__io_usb2storage, sizeof(DISC_INTERFACE));
    }
}

static bool __usb2storage_Startup(void)
{
    s32 ret;

    ret = USB2Storage_Open();

    if (__usb2fd > 0 )
    {
        currentMode = 2;
        return ret > 0;
    }

    if (__io_usb1storage.startup())
    {
        currentMode = 1;
        return true;
    }

    return false;
}

static bool __usb2storage_IsInserted(void)
{
    if (!USB2Storage_Initialize() < 0)
        return false;


    if (usb2_mutex == LWP_MUTEX_NULL)
        return false;

    if (__usb2fd > 0)
    {
        LWP_MutexLock(usb2_mutex);
        int retval = IOS_IoctlvFormat(hId, __usb2fd, USB_IOCTL_UMS_IS_INSERTED, ":");
        LWP_MutexUnlock(usb2_mutex);
        debug_printf("isinserted usb2 retval: %d \n", retval);

        if (retval > 0)
            return true;
    }

    if (__io_usb1storage.isInserted() > 0)
    {
        return true;
    }

    return false;
}

bool __usb2storage_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    s32 ret = 1;
    u32 sectors = 0;
    uint8_t *dest = buffer;

    if (currentMode == 1)
        return __io_usb1storage.readSectors(sector, numSectors, buffer);

    if (__usb2fd < 1 || usb2_mutex == LWP_MUTEX_NULL)
        return false;

    LWP_MutexLock(usb2_mutex);

    while (numSectors > 0)
    {
        if (numSectors * sector_size > USB2_BUFFER)
            sectors = USB2_BUFFER / sector_size;
        else
            sectors = numSectors;

        if (!is_MEM2_buffer(dest)) //libfat is not providing us good buffers :-(
        {
            ret = IOS_IoctlvFormat(hId, __usb2fd, USB_IOCTL_UMS_READ_SECTORS, "ii:d",
                                   sector, sectors, fixed_buffer, sector_size * sectors);
            memcpy(dest, fixed_buffer, sector_size * sectors);
        }
        else
            ret = IOS_IoctlvFormat(hId, __usb2fd, USB_IOCTL_UMS_READ_SECTORS, "ii:d",
                                   sector, sectors, dest, sector_size * sectors);

        dest += sector_size * sectors;

        if (ret < 1) break;

        sector += sectors;

        numSectors -= sectors;
    }

    if (ret < 1) USB2Storage_Close();

    LWP_MutexUnlock(usb2_mutex);

    if (ret < 1) return false;

    return true;
}

static bool __usb2storage_WriteSectors(u32 sector, u32 numSectors, const void *buffer)
{
    s32 ret = 1;
    u32 sectors = 0;
    uint8_t *dest = (uint8_t *) buffer;

    if (currentMode == 1)
        return __io_usb1storage.writeSectors(sector, numSectors, buffer);

    if (__usb2fd < 1 || usb2_mutex == LWP_MUTEX_NULL)
        return false;

    LWP_MutexLock(usb2_mutex);

    while (numSectors > 0 && ret > 0)
    {
        if (numSectors * sector_size > USB2_BUFFER)
            sectors = USB2_BUFFER / sector_size;
        else
            sectors = numSectors;

        numSectors -= sectors;

        if (!is_MEM2_buffer(dest)) // libfat is not providing us good buffers :-(
        {
            memcpy(fixed_buffer, dest, sector_size * sectors);
            ret = IOS_IoctlvFormat(hId, __usb2fd, USB_IOCTL_UMS_WRITE_SECTORS,
                                   "ii:d", sector, sectors, fixed_buffer, sector_size
                                   * sectors);
        }
        else
            ret = IOS_IoctlvFormat(hId, __usb2fd, USB_IOCTL_UMS_WRITE_SECTORS,
                                   "ii:d", sector, sectors, dest, sector_size * sectors);

        if (ret < 1)break;

        dest += sector_size * sectors;

        sector += sectors;
    }

    LWP_MutexUnlock(usb2_mutex);

    if (ret < 1 ) return false;

    return true;
}

static bool __usb2storage_ClearStatus(void)
{
    if (currentMode == 1)
        return __io_usb1storage.clearStatus();

    return true;
}

static bool __usb2storage_Shutdown(void)
{
    if (currentMode == 1)
        return __io_usb1storage.shutdown();

    return true;
}

void SetUSB2Mode(int mode)
{
    USB2Storage_Initialize();
    IOS_IoctlvFormat(hId, __usb2fd, USB_IOCTL_UMS_USB_MODE, "b:", (u8)mode);
}

int GetUSB2LanPort(void)
{
    USB2Storage_Initialize();
    return IOS_IoctlvFormat(hId, __usb2fd, USB_IOCTL_UMS_GET_USBLAN_PORT, ":");
}

const DISC_INTERFACE __io_usb2storage =
    {
        DEVICE_TYPE_WII_USB,
        FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
        (FN_MEDIUM_STARTUP) & __usb2storage_Startup,
        (FN_MEDIUM_ISINSERTED) & __usb2storage_IsInserted,
        (FN_MEDIUM_READSECTORS) & __usb2storage_ReadSectors,
        (FN_MEDIUM_WRITESECTORS) & __usb2storage_WriteSectors,
        (FN_MEDIUM_CLEARSTATUS) & __usb2storage_ClearStatus,
        (FN_MEDIUM_SHUTDOWN) & __usb2storage_Shutdown
    };
