#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <sdcard/wiisd_io.h>

#include "sdhc.h"

/* IOCTL comamnds */
#define IOCTL_SDHC_INIT	 0x01
#define IOCTL_SDHC_READ	 0x02
#define IOCTL_SDHC_WRITE	0x03
#define IOCTL_SDHC_ISINSERTED   0x04

#define SDHC_HEAPSIZE	   0x8000
#define SDHC_MEM2_SIZE	  0x10000

int sdhc_mode_sd = 0;

/* Variables */
static char fs[] ATTRIBUTE_ALIGN( 32 ) = "/dev/sdio/sdhc";

static s32 hid = -1, fd = -1;
static u32 sector_size = SDHC_SECTOR_SIZE;
static void *sdhc_buf2;

extern void* SYS_AllocArena2MemLo(u32 size, u32 align);

bool SDHC_Init(void)
{
	s32 ret;

	if (sdhc_mode_sd)
	{
		return __io_wiisd.startup();
	}

	/* Already open */
	if (fd >= 0) return true;

	/* Create heap */
	if (hid < 0)
	{
		hid = iosCreateHeap(SDHC_HEAPSIZE);
		if (hid < 0) goto err;
	}

	// allocate buf2
	if (sdhc_buf2 == NULL)
	{
		sdhc_buf2 = SYS_AllocArena2MemLo(SDHC_MEM2_SIZE, 32);
	}

	/* Open SDHC device */
	fd = IOS_Open(fs, 0);
	if (fd < 0) goto err;

	/* Initialize SDHC */
	ret = IOS_IoctlvFormat(hid, fd, IOCTL_SDHC_INIT, ":");
	if (ret) goto err;

	return true;

	err:
	/* Close SDHC device */
	if (fd >= 0)
	{
		IOS_Close(fd);
		fd = -1;
	}

	return false;
}

bool SDHC_Close(void)
{
	if (sdhc_mode_sd)
	{
		return __io_wiisd.shutdown();
	}

	/* Close SDHC device */
	if (fd >= 0)
	{
		IOS_Close(fd);
		fd = -1;
	}

	/*if (hid > 0) {
	 iosDestroyHeap(hid);
	 hid = -1;
	 }*/

	return true;
}

bool SDHC_IsInserted(void)
{
	s32 ret;
	if (sdhc_mode_sd)
	{
		return __io_wiisd.isInserted();
	}

	/* Check if SD card is inserted */
	ret = IOS_IoctlvFormat(hid, fd, IOCTL_SDHC_ISINSERTED, ":");

	return (!ret) ? true : false;
}

bool SDHC_ReadSectors(u32 sector, u32 count, void *buffer)
{
	//printf("SD-R(%u %u)\n", sector, count);
	if (sdhc_mode_sd)
	{
		return __io_wiisd.readSectors(sector, count, buffer);
	}

	void *buf = (void *) buffer;
	u32 len = (sector_size * count);

	s32 ret;

	/* Device not opened */
	if (fd < 0) return false;

	/* Buffer not aligned */
	if ((u32) buffer & 0x1F)
	{
		/* Allocate memory */
		//buf = iosAlloc(hid, len);
		buf = sdhc_buf2;
		if (!buf) return false;
	}

	/* Read data */
	ret = IOS_IoctlvFormat(hid, fd, IOCTL_SDHC_READ, "ii:d", sector, count, buf, len);

	/* Copy data */
	if (buf != buffer)
	{
		memcpy(buffer, buf, len);
		//iosFree(hid, buf);
	}

	return (!ret) ? true : false;
}

bool SDHC_WriteSectors(u32 sector, u32 count, void *buffer)
{
	if (sdhc_mode_sd)
	{
		return __io_wiisd.writeSectors(sector, count, buffer);
	}

	void *buf = (void *) buffer;
	u32 len = (sector_size * count);

	s32 ret;

	/* Device not opened */
	if (fd < 0) return false;

	/* Buffer not aligned */
	if ((u32) buffer & 0x1F)
	{
		/* Allocate memory */
		//buf = iosAlloc(hid, len);
		buf = sdhc_buf2;
		if (!buf) return false;

		/* Copy data */
		memcpy(buf, buffer, len);
	}

	/* Read data */
	ret = IOS_IoctlvFormat(hid, fd, IOCTL_SDHC_WRITE, "ii:d", sector, count, buf, len);

	/* Free memory */
	//if (buf != buffer)
	//  iosFree(hid, buf);

	return (!ret) ? true : false;
}

bool SDHC_ClearStatus(void)
{
	return true;
}

bool __io_SDHC_Close(void)
{
	// do nothing.
	return true;
}

bool __io_SDHC_NOP(void)
{
	// do nothing.
	return true;
}

const DISC_INTERFACE __io_sdhc = { DEVICE_TYPE_WII_SD, FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE
		| FEATURE_WII_SD, (FN_MEDIUM_STARTUP) &SDHC_Init, (FN_MEDIUM_ISINSERTED) &SDHC_IsInserted,
		(FN_MEDIUM_READSECTORS) &SDHC_ReadSectors, (FN_MEDIUM_WRITESECTORS) &SDHC_WriteSectors,
		(FN_MEDIUM_CLEARSTATUS) &SDHC_ClearStatus,
		//(FN_MEDIUM_SHUTDOWN)&SDHC_Close
		(FN_MEDIUM_SHUTDOWN) &__io_SDHC_Close };

const DISC_INTERFACE __io_sdhc_ro = { DEVICE_TYPE_WII_SD, FEATURE_MEDIUM_CANREAD | FEATURE_WII_SD,
		(FN_MEDIUM_STARTUP) &SDHC_Init, (FN_MEDIUM_ISINSERTED) &SDHC_IsInserted,
		(FN_MEDIUM_READSECTORS) &SDHC_ReadSectors, (FN_MEDIUM_WRITESECTORS) &__io_SDHC_NOP, // &SDHC_WriteSectors,
		(FN_MEDIUM_CLEARSTATUS) &SDHC_ClearStatus,
		//(FN_MEDIUM_SHUTDOWN)&SDHC_Close
		(FN_MEDIUM_SHUTDOWN) &__io_SDHC_Close };

