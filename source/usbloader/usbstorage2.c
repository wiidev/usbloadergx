/*-------------------------------------------------------------

 usbstorage_starlet.c -- USB mass storage support, inside starlet
 Copyright (C) 2009 Kwiirk

 If this driver is linked before libogc, this will replace the original
 usbstorage driver by svpe from libogc
 This software is provided 'as-is', without any express or implied
 warranty.  In no event will the authors be held liable for any
 damages arising from the use of this software.

 Permission is granted to anyone to use this software for any
 purpose, including commercial applications, and to alter it and
 redistribute it freely, subject to the following restrictions:

 1.  The origin of this software must not be misrepresented; you
 must not claim that you wrote the original software. If you use
 this software in a product, an acknowledgment in the product
 documentation would be appreciated but is not required.

 2.  Altered source versions must be plainly marked as such, and
 must not be misrepresented as being the original software.

 3.  This notice may not be removed or altered from any source
 distribution.

 -------------------------------------------------------------*/

#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "usbstorage2.h"
#include "memory/mem2.h"

#define isMEM2Buffer(p) (((u32) p & 0x10000000) != 0)

#define MAX_BUFFER_SECTORS  256

/* IOCTL commands */
#define UMS_BASE            (('U'<<24)|('M'<<16)|('S'<<8))
#define USB_IOCTL_UMS_INIT              (UMS_BASE+0x1)
#define USB_IOCTL_UMS_GET_CAPACITY      (UMS_BASE+0x2)
#define USB_IOCTL_UMS_READ_SECTORS      (UMS_BASE+0x3)
#define USB_IOCTL_UMS_WRITE_SECTORS     (UMS_BASE+0x4)
#define USB_IOCTL_UMS_READ_STRESS       (UMS_BASE+0x5)
#define USB_IOCTL_UMS_SET_VERBOSE       (UMS_BASE+0x6)
#define USB_IOCTL_UMS_UMOUNT            (UMS_BASE+0x10)
#define USB_IOCTL_UMS_WATCHDOG          (UMS_BASE+0x80)

#define USB_IOCTL_UMS_TESTMODE          (UMS_BASE+0x81)

#define WBFS_BASE (('W'<<24)|('F'<<16)|('S'<<8))
#define USB_IOCTL_WBFS_OPEN_DISC            (WBFS_BASE+0x1)
#define USB_IOCTL_WBFS_READ_DISC            (WBFS_BASE+0x2)
#define USB_IOCTL_WBFS_READ_DIRECT_DISC     (WBFS_BASE+0x3)
#define USB_IOCTL_WBFS_STS_DISC             (WBFS_BASE+0x4)
#define USB_IOCTL_WBFS_SET_DEVICE           (WBFS_BASE+0x50)
#define USB_IOCTL_WBFS_SET_FRAGLIST         (WBFS_BASE+0x51)

#define UMS_HEAPSIZE            0x1000
/* Variables */
static char fs[] ATTRIBUTE_ALIGN( 32 ) = "/dev/usb2";
static char fs2[] ATTRIBUTE_ALIGN( 32 ) = "/dev/usb/ehc";

static u8 * mem2_ptr = NULL;
static s32 hid = -1, fd = -1;
static u32 sector_size = 0;
static int mounted = 0;

s32 USBStorage2_Init(void)
{
    s32 ret, ret2;

    /* Already open */
    if (fd >= 0) return 0;

    /* Create heap */
    if (hid < 0)
    {
        hid = iosCreateHeap(UMS_HEAPSIZE);
        if (hid < 0) return IPC_ENOMEM;
    }

    /* Open USB device */
    fd = IOS_Open(fs, 0);
    if (fd < 0) fd = IOS_Open(fs2, 0);

    if (fd < 0) return fd;

    /* Initialize USB storage */
    ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_INIT, ":");
    if (ret < 0) goto err;
    if (ret > 1) ret = 0;
    ret2 = ret;

    /* Get device capacity */
    ret = USBStorage2_GetCapacity(&sector_size);
    if (!ret)
    {
        ret = -1;
        goto err;
    }
    if (ret2 == 0 && sector_size != 512) // check for HD sector size 512 bytes
    {
        ret = -20001;
        goto err;
    }
    if (ret2 == 1 && sector_size != 2048) // check for DVD sector size 2048 bytes
    {
        ret = -20002;
        goto err;
    }
    mounted = 1;
    return ret2; // 0->HDD, 1->DVD

    err:
    /* Close USB device */
    if (fd >= 0)
    {
        IOS_Close(fd);
        fd = -1;
    }

    return ret;
}

void USBStorage2_Deinit(void)
{
    mounted = 0;

    /* Close USB device */
    if (fd >= 0)
    {
        IOS_Close(fd);
        fd = -1;
    }
}


s32 USBStorage2_Watchdog(u32 on_off)
{
    if (fd >= 0)
    {
        s32 ret;

        ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_WATCHDOG, "i:", on_off);

        return ret;
    }

    return IPC_ENOENT;
}

s32 USBStorage2_GetCapacity(u32 *_sector_size)
{
    if (fd >= 0)
    {
        s32 ret;

        ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_GET_CAPACITY, ":i", &sector_size);

        if (ret && _sector_size) *_sector_size = sector_size;

        return ret;
    }

    return IPC_ENOENT;
}

s32 USBStorage2_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    u8 *buf = (u8 *) buffer;
    u32 len = (sector_size * numSectors);

    s32 ret = -1;

    /* Device not opened */
    if (fd < 0) return fd;

    if (!mem2_ptr)
        mem2_ptr = (u8 *) MEM2_alloc(sector_size * MAX_BUFFER_SECTORS);

    // Do not read more than MAX_BUFFER_SECTORS sectors at once and create a mem overflow!
    if (!isMEM2Buffer(buffer))
    {
        buf = mem2_ptr;
        s32 read_size;
        s32 sectors_done = 0;

        while(sectors_done < numSectors)
        {
            read_size = numSectors-sectors_done < MAX_BUFFER_SECTORS ? numSectors-sectors_done : MAX_BUFFER_SECTORS;

            ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_READ_SECTORS, "ii:d", sector+sectors_done, read_size, buf, read_size*sector_size);
            if(ret < 0)
                return ret;

            memcpy(((u8 *) buffer) + sectors_done*sector_size, buf, read_size*sector_size);

            sectors_done += read_size;
        }
    }
    else
    {
        /* Read data */
        ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_READ_SECTORS, "ii:d", sector, numSectors, buf, len);
    }

    /* Copy data */

    return ret;
}

s32 USBStorage2_WriteSectors(u32 sector, u32 numSectors, const void *buffer)
{
    u8 *buf = (u8 *) buffer;
    u32 len = (sector_size * numSectors);

    s32 ret = -1;

    /* Device not opened */
    if (fd < 0) return fd;

    /* Device not opened */
    if (!mem2_ptr)
        mem2_ptr = (u8 *) MEM2_alloc(sector_size * MAX_BUFFER_SECTORS);

    /* MEM1 buffer */
    if (!isMEM2Buffer(buffer))
    {
        // Do not read more than MAX_BUFFER_SECTORS sectors at once and create a mem overflow!
        buf = mem2_ptr;
        s32 write_size;
        s32 sectors_done = 0;

        while(sectors_done < numSectors)
        {
            write_size = numSectors-sectors_done < MAX_BUFFER_SECTORS ? numSectors-sectors_done : MAX_BUFFER_SECTORS;

            memcpy(buf, ((u8 *) buffer) + sectors_done*sector_size, write_size*sector_size);

            ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_WRITE_SECTORS, "ii:d", sector+sectors_done, write_size, buf, write_size*sector_size);
            if(ret < 0)
                return ret;

            sectors_done += write_size;
        }
    }
    else
    {
        /* Write data */
        ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_WRITE_SECTORS, "ii:d", sector, numSectors, buf, len);
    }

    return ret;
}

static bool __usbstorage_Startup(void)
{
    return USBStorage2_Init() == 0;
}

static bool __usbstorage_IsInserted(void)
{
    return (USBStorage2_GetCapacity(NULL) >= 0);
}

static bool __usbstorage_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return (USBStorage2_ReadSectors(sector, numSectors, buffer) >= 0);
}

static bool __usbstorage_WriteSectors(u32 sector, u32 numSectors, const void *buffer)
{
    return (USBStorage2_WriteSectors(sector, numSectors, buffer) >= 0);
}

static bool __usbstorage_ClearStatus(void)
{
    return true;
}

static bool __usbstorage_Shutdown(void)
{
    USBStorage2_Deinit();
    return true;
}

const DISC_INTERFACE __io_usbstorage2 = {
    DEVICE_TYPE_WII_UMS, FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
    (FN_MEDIUM_STARTUP) &__usbstorage_Startup,
    (FN_MEDIUM_ISINSERTED) &__usbstorage_IsInserted,
    (FN_MEDIUM_READSECTORS) &__usbstorage_ReadSectors,
    (FN_MEDIUM_WRITESECTORS) &__usbstorage_WriteSectors,
    (FN_MEDIUM_CLEARSTATUS) &__usbstorage_ClearStatus,
    (FN_MEDIUM_SHUTDOWN) &__usbstorage_Shutdown
};

// woffset is in 32bit words, len is in bytes
s32 USBStorage_WBFS_Read(u32 woffset, u32 len, void *buffer)
{
    s32 ret;

    USBStorage2_Init();
    /* Device not opened */
    if (fd < 0) return fd;

    /* Read data */
    ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_WBFS_READ_DISC, "ii:d", woffset, len, buffer, len);

    return ret;
}

s32 USBStorage_WBFS_SetFragList(void *p, int size)
{
    s32 ret;
    USBStorage2_Init();
    // Device not opened
    if (fd < 0) return fd;
    // ioctl
    ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_WBFS_SET_FRAGLIST, "d:", p, size);
    return ret;
}

// Not used currently
#if 0

s32 USBStorage_WBFS_Open(char *buffer)
{
    u32 len = 8;

    s32 ret;

    /* Device not opened */
    if (fd < 0) return fd;

    extern u32 wbfs_part_lba;
    u32 part = wbfs_part_lba;

    /* Read data */
    ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_WBFS_OPEN_DISC, "dd:", buffer, len, &part, 4);

    return ret;
}

s32 USBStorage_WBFS_SetDevice(int dev)
{
    s32 ret, retval = 0;
    USBStorage2_Init();
    // Device not opened
    if (fd < 0) return fd;
    // ioctl
    ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_WBFS_SET_DEVICE, "i:i", dev, &retval);
    if (retval) return retval;
    return ret;
}

#endif
