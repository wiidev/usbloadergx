/*****************************************
* This code is from Mighty Channel 11
* which is based on the TriiForce source.
* Modifications by Dimok.
*****************************************/
#include <stdio.h>
#include <ogcsys.h>
#include <malloc.h>
#include <string.h>

#include "system/IosLoader.h"
#include "utils/tools.h"
#include "gecko.h"
#include "NandEmu.h"

/* 'NAND Device' structure */
typedef struct {
	/* Device name */
	const char *name;

	/* Mode value */
	u32 mode;

	/* Un/mount command */
	u32 mountCmd;
	u32 umountCmd;
} nandDevice;

static nandDevice ndevList[] =
{
	{ "Disable",						0,	0x00,	0x00 },
	{ "SD/SDHC Card",					1,	0xF0,	0xF1 },
	{ "USB 2.0 Mass Storage Device",	2,	0xF2,	0xF3 },
};

/* Buffer */
static const char fs[] ATTRIBUTE_ALIGN(32) = "/dev/fs";
static const char fat[] ATTRIBUTE_ALIGN(32) = "fat";
static u32 inbuf[8] ATTRIBUTE_ALIGN(32);
static u32 partition ATTRIBUTE_ALIGN(32) = 0;
static u32 mode ATTRIBUTE_ALIGN(32) = 0;
static char path[32] ATTRIBUTE_ALIGN(32) = "\0";
static int fullmode = 0;
static int mounted = 0;

static s32 Nand_Mount(nandDevice *dev)
{
	s32 fd, ret;
	u32 inlen = 0;
	ioctlv *vector = NULL;
	int rev = IOS_GetRevision();

	/* Open FAT module */
	fd = IOS_Open(fat, 0);
	if (fd < 0)
		return fd;


	if(rev >= 21 && rev != 65280) // stub version from nintendo
	{
		// NOTE:
		// The official cIOSX rev21 by Waninkoko ignores the partition argument
		// and the nand is always expected to be on the 1st partition.
		// However this way earlier d2x betas having revision 21 take in
		// consideration the partition argument.
		inlen = 1;

		/* Allocate memory */
		vector = (ioctlv *) memalign(32, sizeof(ioctlv));
		if(vector == NULL)
		{
			/* Close FAT module */
			IOS_Close(fd);
			return -2;
		}

		vector[0].data = &partition;
		vector[0].len = sizeof(u32);
	}
	/* Mount device */
	ret = IOS_Ioctlv(fd, dev->mountCmd, inlen, 0, vector);

	/* Close FAT module */
	IOS_Close(fd);

	/* Free memory */
	free(vector);

	return ret;
}

static s32 Nand_Unmount(nandDevice *dev)
{
	s32 fd, ret;

	// Open FAT module
	fd = IOS_Open(fat, 0);
	if (fd < 0)
		return fd;

	// Unmount device
	ret = IOS_Ioctlv(fd, dev->umountCmd, 0, 0, NULL);

	// Close FAT module
	IOS_Close(fd);

	return ret;
}

static s32 Nand_Enable(nandDevice *dev)
{
	s32 fd, ret;
	ioctlv *vector = NULL;
	int rev = IOS_GetRevision();

	// Open /dev/fs
	fd = IOS_Open(fs, 0);
	if (fd < 0)
		return fd;

	if (IOS_GetRevision() >= 18)
	{
		//FULL NAND emulation since rev18
		//needed for reading images on triiforce mrc folder using ISFS commands
		mode = dev->mode | fullmode;
	}
	else
	{
		mode = dev->mode;
	}

	if(rev >= 21 && rev != 65280) // stub version from nintendo
	{
		// NOTE:
		// The official cIOSX rev21 by Waninkoko provides an undocumented feature
		// to set nand path when mounting the device.
		// This feature has been discovered during d2x development.

		/* Allocate memory */
		vector = (ioctlv *)memalign(32, sizeof(ioctlv)*2);
		if(vector == NULL)
		{
			/* Close FAT module */
			IOS_Close(fd);
			return -2;
		}

		vector[0].data = &mode;
		vector[0].len = sizeof(u32);
		vector[1].data = path;
		vector[1].len = sizeof(path);

		ret = IOS_Ioctlv(fd, 100, 2, 0, vector);

		/* Free memory */
		free(vector);
	}
	else
	{
		memset(inbuf, 0, sizeof(inbuf));
		inbuf[0] = dev->mode;
		ret = IOS_Ioctl(fd, 100, inbuf, sizeof(inbuf), NULL, 0);
	}

	// Close /dev/fs
	IOS_Close(fd);

	return ret;
}

static s32 Nand_Disable(void)
{
	s32 fd, ret;

	// Open /dev/fs
	fd = IOS_Open(fs, 0);
	if (fd < 0)
		return fd;

	// Set input buffer
	mode = 0;

	// Disable NAND emulator
	ret = IOS_Ioctl(fd, 100, &mode, sizeof(mode), NULL, 0);

	// Close /dev/fs
	IOS_Close(fd);

	return ret;
}

s32 Enable_Emu(int selection)
{
	if(!IosLoader::IsD2X() && IOS_GetRevision() < 17)
		return -1;

	if(mounted != 0)
		return -1;

	s32 ret;
	nandDevice *ndev = NULL;
	ndev = &ndevList[selection];

	ret = Nand_Mount(ndev);
	if (ret < 0)
	{
		gprintf(" ERROR Mount! (ret = %d)\n", ret);
		return ret;

	}

	ret = Nand_Enable(ndev);
	if (ret < 0)
	{
		gprintf(" ERROR Enable! (ret = %d)\n", ret);
		return ret;
	}
	mounted = selection;
	return 0;
}

s32 Disable_Emu()
{
	if(!IosLoader::IsD2X() && IOS_GetRevision() < 17)
		return -1;

	if(mounted==0)
		return 0;

	nandDevice *ndev = NULL;
	ndev = &ndevList[mounted];

	Nand_Disable();
	Nand_Unmount(ndev);

	mounted = 0;

	return 0;
}

void Set_Partition(int p)
{
	partition = p;
}

void Set_Path(const char* p)
{
	int i = 0;

	while(p[i] != '\0' && i < 31)
	{
		path[i] = p[i];
		i++;
	}
	while(path[i-1] == '/')
	{
		path[i-1] = '\0';
		--i;
	}
	path[i] = '\0';
}

void Set_FullMode(int fm)
{
	fullmode = fm ? 0x100 : 0;
}

const char* Get_Path(void)
{
	return path;
}
