/*-------------------------------------------------------------

 usbstorage_starlet.c -- USB mass storage support, inside starlet
 Copyright (C) 2011 Dimok
 Copyright (C) 2011 Rodries
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
#include "gecko.h"


/* IOCTL commands */
#define UMS_BASE			(('U'<<24)|('M'<<16)|('S'<<8))
#define USB_IOCTL_UMS_INIT			  (UMS_BASE+0x1)
#define USB_IOCTL_UMS_GET_CAPACITY	  (UMS_BASE+0x2)
#define USB_IOCTL_UMS_READ_SECTORS	  (UMS_BASE+0x3)
#define USB_IOCTL_UMS_WRITE_SECTORS	 (UMS_BASE+0x4)
#define USB_IOCTL_UMS_READ_STRESS	   (UMS_BASE+0x5)
#define USB_IOCTL_UMS_SET_VERBOSE	   (UMS_BASE+0x6)
#define USB_IOCTL_UMS_UMOUNT			(UMS_BASE+0x10)
#define USB_IOCTL_UMS_WATCHDOG		  (UMS_BASE+0x80)

#define USB_IOCTL_UMS_TESTMODE		  (UMS_BASE+0x81)
#define USB_IOCTL_SET_PORT				(UMS_BASE+0x83)

#define WBFS_BASE (('W'<<24)|('F'<<16)|('S'<<8))
#define USB_IOCTL_WBFS_OPEN_DISC			(WBFS_BASE+0x1)
#define USB_IOCTL_WBFS_READ_DISC			(WBFS_BASE+0x2)
#define USB_IOCTL_WBFS_SET_DEVICE		   (WBFS_BASE+0x50)
#define USB_IOCTL_WBFS_SET_FRAGLIST		 (WBFS_BASE+0x51)

#define isMEM2Buffer(p) (((u32) p & 0x10000000) != 0)

#define MAX_SECTOR_SIZE		 4096
#define MAX_BUFFER_SECTORS	  128
#define UMS_HEAPSIZE			2*1024

/* Variables */
static char fs[] ATTRIBUTE_ALIGN(32) = "/dev/usb2";
static char fs2[] ATTRIBUTE_ALIGN(32) = "/dev/usb123";
static char fs3[] ATTRIBUTE_ALIGN(32) = "/dev/usb/ehc";

static u8 * mem2_ptr = NULL;
static s32 hid = -1, fd = -1;
static u32 usb2_port = -1;  //current USB port
bool hddInUse[2] = { false, false };
u32 hdd_sector_size[2] = { 512, 512 };

s32 USBStorage2_Init(u32 port)
{
	if(hddInUse[port])
		return 0;

	/* Create heap */
	if (hid < 0)
	{
		hid = iosCreateHeap(UMS_HEAPSIZE);
		if (hid < 0) return IPC_ENOMEM;
	}

	/* Open USB device */
	if (fd < 0) fd = IOS_Open(fs, 0);
	if (fd < 0) fd = IOS_Open(fs2, 0);
	if (fd < 0) fd = IOS_Open(fs3, 0);
	if (fd < 0) return fd;

	USBStorage2_SetPort(port);

	/* Initialize USB storage */
	IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_INIT, ":");

	/* Get device capacity */
	if (USBStorage2_GetCapacity(port, &hdd_sector_size[port]) == 0)
		return IPC_ENOENT;

	hddInUse[port] = true;

	return 0; // 0->HDD, 1->DVD
}

void USBStorage2_Deinit()
{
	/* Close USB device */
	if (fd >= 0)
	{
		IOS_Close(fd);  // not sure to close the fd is needed
		fd = -1;
	}
}

s32 USBStorage2_SetPort(u32 port)
{
	//! Port = 2 is handle in the loader, no need to handle it in cIOS
	if(port > 1)
		return -1;

	if(port == usb2_port)
		return 0;

	s32 ret = -1;
	usb2_port = port;

	gprintf("Changing USB port to port %i....\n", port);
	//must be called before USBStorage2_Init (default port 0)
	if (fd >= 0)
		ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_SET_PORT, "i:", usb2_port);

	return ret;
}

s32 USBStorage2_GetPort()
{
	return usb2_port;
}

s32 USBStorage2_GetCapacity(u32 port, u32 *_sector_size)
{
	if (fd >= 0)
	{
		s32 ret;
		u32 sector_size = 0;
		USBStorage2_SetPort(port);

		ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_GET_CAPACITY, ":i", &sector_size);

		if (ret && _sector_size) *_sector_size = sector_size;

		return ret;
	}

	return IPC_ENOENT;
}

s32 USBStorage2_ReadSectors(u32 port, u32 sector, u32 numSectors, void *buffer)
{
	u8 *buf = (u8 *) buffer;
	s32 ret = -1;

	/* Device not opened */
	if (fd < 0) return fd;

	if (!mem2_ptr)
		mem2_ptr = (u8 *) MEM2_alloc(MAX_SECTOR_SIZE * MAX_BUFFER_SECTORS);

	USBStorage2_SetPort(port);

	s32 read_secs, read_size;

	while(numSectors > 0)
	{
		read_secs = numSectors > MAX_BUFFER_SECTORS ? MAX_BUFFER_SECTORS : numSectors;
		read_size = read_secs*hdd_sector_size[port];

		// Do not read more than MAX_BUFFER_SECTORS sectors at once and create a mem overflow!
		if (!isMEM2Buffer(buffer))
		{
			ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_READ_SECTORS, "ii:d", sector, read_secs, mem2_ptr, read_size);
			if(ret < 0)
				return ret;

			memcpy(buf, mem2_ptr, read_size);
		}
		else
		{
			/* Read data */
			ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_READ_SECTORS, "ii:d", sector, read_secs, buf, read_size);
			if(ret < 0)
				return ret;
		}

		sector += read_secs;
		numSectors -= read_secs;
		buf += read_size;
	}

	return ret;
}

s32 USBStorage2_WriteSectors(u32 port, u32 sector, u32 numSectors, const void *buffer)
{
	u8 *buf = (u8 *) buffer;
	s32 ret = -1;

	/* Device not opened */
	if (fd < 0) return fd;

	/* Device not opened */
	if (!mem2_ptr)
		mem2_ptr = (u8 *) MEM2_alloc(MAX_SECTOR_SIZE * MAX_BUFFER_SECTORS);

	USBStorage2_SetPort(port);

	s32 write_size, write_secs;

	while(numSectors > 0)
	{
		write_secs = numSectors > MAX_BUFFER_SECTORS ? MAX_BUFFER_SECTORS : numSectors;
		write_size = write_secs*hdd_sector_size[port];

		/* MEM1 buffer */
		if (!isMEM2Buffer(buffer))
		{
			// Do not read more than MAX_BUFFER_SECTORS sectors at once and create a mem overflow!
			memcpy(mem2_ptr, buf, write_size);

			ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_WRITE_SECTORS, "ii:d", sector, write_secs, mem2_ptr, write_size);
			if(ret < 0)
				return ret;
		}
		else
		{
			/* Write data */
			ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_WRITE_SECTORS, "ii:d", sector, write_secs, buf, write_size);
			if(ret < 0)
				return ret;
		}

		sector += write_secs;
		numSectors -= write_secs;
		buf += write_size;
	}

	return ret;
}

static bool __usbstorage_Startup(void)
{
	return USBStorage2_Init(0) >= 0;
}

static bool __usbstorage_IsInserted(void)
{
	return (USBStorage2_GetCapacity(0, NULL) != 0);
}

static bool __usbstorage_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
	return (USBStorage2_ReadSectors(0, sector, numSectors, buffer) >= 0);
}

static bool __usbstorage_WriteSectors(u32 sector, u32 numSectors, const void *buffer)
{
	return (USBStorage2_WriteSectors(0, sector, numSectors, buffer) >= 0);
}

static bool __usbstorage_ClearStatus(void)
{
	return true;
}

static bool __usbstorage_Shutdown(void)
{
	hddInUse[0] = false;
	hdd_sector_size[0] = 512;
	return true;
}

const DISC_INTERFACE __io_usbstorage2_port0 = {
	DEVICE_TYPE_WII_UMS, FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
	(FN_MEDIUM_STARTUP) &__usbstorage_Startup,
	(FN_MEDIUM_ISINSERTED) &__usbstorage_IsInserted,
	(FN_MEDIUM_READSECTORS) &__usbstorage_ReadSectors,
	(FN_MEDIUM_WRITESECTORS) &__usbstorage_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS) &__usbstorage_ClearStatus,
	(FN_MEDIUM_SHUTDOWN) &__usbstorage_Shutdown
};

static bool __usbstorage_Startup2(void)
{
	return USBStorage2_Init(1) >= 0;
}

static bool __usbstorage_IsInserted2(void)
{
	return (USBStorage2_GetCapacity(1, NULL) != 0);
}

static bool __usbstorage_ReadSectors2(u32 sector, u32 numSectors, void *buffer)
{
	return (USBStorage2_ReadSectors(1, sector, numSectors, buffer) >= 0);
}

static bool __usbstorage_WriteSectors2(u32 sector, u32 numSectors, const void *buffer)
{
	return (USBStorage2_WriteSectors(1, sector, numSectors, buffer) >= 0);
}

static bool __usbstorage_Shutdown2(void)
{
	hddInUse[1] = false;
	hdd_sector_size[1] = 512;
	return true;
}

const DISC_INTERFACE __io_usbstorage2_port1 = {
	DEVICE_TYPE_WII_UMS, FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
	(FN_MEDIUM_STARTUP) &__usbstorage_Startup2,
	(FN_MEDIUM_ISINSERTED) &__usbstorage_IsInserted2,
	(FN_MEDIUM_READSECTORS) &__usbstorage_ReadSectors2,
	(FN_MEDIUM_WRITESECTORS) &__usbstorage_WriteSectors2,
	(FN_MEDIUM_CLEARSTATUS) &__usbstorage_ClearStatus,
	(FN_MEDIUM_SHUTDOWN) &__usbstorage_Shutdown2
};
