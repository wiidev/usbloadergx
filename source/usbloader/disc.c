#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <ogc/lwp_watchdog.h>

#include "patches/gamepatches.h"
#include "patches/wip.h"
#include "apploader.h"
#include "disc.h"
#include "video.h"
#include "wdvd.h"
#include "frag.h"
#include "alternatedol.h"
#include "memory/memory.h"
#include "wbfs.h"
#include "settings/SettingsEnums.h"
#include "GameCube/DML_Config.h"
#include "GameCube/NIN_Config.h"
#include "gecko.h"

// Global app entry point
extern u32 AppEntrypoint;

/* Constants */
#define PTABLE_OFFSET   0x40000
#define WII_MAGIC   0x5D1C9EA3

/* Disc pointers */
static u32 *buffer = (u32 *) 0x93000000;
static u8 *diskid = (u8 *) Disc_ID;
static u32 rmode_reg = 0;
GXRModeObj *rmode = NULL;

void Disc_SetLowMem(void)
{
	/* Setup low memory */
	*Sys_Magic = 0x0D15EA5E;			// Standard Boot Code
	*Sys_Version = 0x00000001;			// Version
	*Arena_L = 0x00000000;				// Arena Low
	*BI2 = 0x817E5480;					// BI2
	*Bus_Speed = 0x0E7BE2C0;			// Console Bus Speed
	*CPU_Speed = 0x2B73A840;			// Console CPU Speed

	/* Setup low memory */
	*Assembler = 0x38A00040;			// Assembler
	*(u32 *) 0x800000E4 = 0x80431A80;	// OS_Thread
	*Dev_Debugger = 0x81800000;			// Dev Debugger Monitor Address
	*Simulated_Mem = 0x01800000;		// Simulated Memory Size
	*(vu32 *) 0xCD00643C = 0x00000000;	// 32Mhz on Bus

	int iosVer = IOS_GetVersion();
	if(iosVer != 222 && iosVer != 223 && iosVer != 224 && iosVer != 225 && IOS_GetRevision() >= 18)
		*GameID_Address = 0x80000000;	// Game ID Address

	/* Copy disc ID */
	memcpy((void *) Online_Check, (void *) Disc_ID, 4);
}

void Disc_SelectVMode(u8 videoselected, bool devolution, u32 *dml_VideoMode, u32 *nin_VideoMode)
{
	rmode = VIDEO_GetPreferredMode(0);

	/* Get video mode configuration */
	bool progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();
	bool PAL60 = CONF_GetEuRGB60() > 0;
	u32 tvmode = CONF_GetVideo();


	/* Select video mode register: GameCube Devolution only */
	if(devolution)
	{
		if (diskid[3] =='E' || diskid[3] =='J')
		{
			if (CONF_GetVideo() == CONF_VIDEO_PAL)
			{
				rmode_reg = VI_EURGB60;
				rmode = &TVEurgb60Hz480IntDf;
			}
			else
			{
				rmode_reg = VI_NTSC;
				rmode = &TVNtsc480IntDf;
			}
		}
		else
		{
			rmode_reg = VI_PAL;
			rmode = &TVPal528IntDf;
		}
		return;
	}

	/* Select video mode register:  Wii and GameCube MIOS */
	switch (tvmode)
	{
		case CONF_VIDEO_PAL:
			rmode_reg = PAL60 ? VI_EURGB60 : VI_PAL;
			rmode = progressive ? &TVEurgb60Hz480Prog : (PAL60 ? &TVEurgb60Hz480IntDf : &TVPal528IntDf);
			if(dml_VideoMode) *dml_VideoMode = progressive ? DML_VID_FORCE_PROG : (PAL60 ? DML_VID_FORCE_PAL60 : DML_VID_FORCE_PAL50);
			if(nin_VideoMode) *nin_VideoMode = progressive ? NIN_VID_FORCE_PAL60 | NIN_VID_PROG : (PAL60 ? NIN_VID_FORCE_PAL60 : NIN_VID_FORCE_PAL50);
			break;

		case CONF_VIDEO_MPAL:
			rmode_reg = VI_MPAL;
			rmode = progressive ? &TVEurgb60Hz480Prog : &TVMpal480IntDf;
			if(nin_VideoMode) *nin_VideoMode = progressive ? NIN_VID_FORCE_MPAL | NIN_VID_PROG : NIN_VID_FORCE_MPAL;
			break;

		case CONF_VIDEO_NTSC:
			rmode_reg = VI_NTSC;
			rmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
			if(dml_VideoMode) *dml_VideoMode = progressive ? DML_VID_FORCE_PROG : DML_VID_FORCE_NTSC;
			if(nin_VideoMode) *nin_VideoMode = progressive ? NIN_VID_FORCE_NTSC | NIN_VID_PROG : NIN_VID_FORCE_NTSC;
			break;
	}

	switch (videoselected)
	{
		default:
		case VIDEO_MODE_DISCDEFAULT: // DEFAULT (DISC/GAME)
			/* Select video mode */
			switch (diskid[3])
			{
				// PAL
				case 'D':
				case 'F':
				case 'P':
				case 'X':
				case 'Y':
					rmode_reg = PAL60 ? VI_EURGB60 : VI_PAL;
					rmode = progressive ? &TVEurgb60Hz480Prog : (PAL60 ? &TVEurgb60Hz480IntDf : &TVPal528IntDf);
					if(dml_VideoMode) *dml_VideoMode = progressive ? DML_VID_FORCE_PROG : (PAL60 ? DML_VID_FORCE_PAL60 : DML_VID_FORCE_PAL50);
					if(nin_VideoMode) *nin_VideoMode = progressive ? NIN_VID_FORCE_PAL60 | NIN_VID_PROG : (PAL60 ? NIN_VID_FORCE_PAL60 : NIN_VID_FORCE_PAL50);
					break;
				// NTSC
				case 'E':
				case 'J':
					rmode_reg = VI_NTSC;
					rmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
					if(dml_VideoMode) *dml_VideoMode = progressive ? DML_VID_FORCE_PROG : DML_VID_FORCE_NTSC;
					if(nin_VideoMode) *nin_VideoMode = progressive ? NIN_VID_FORCE_NTSC | NIN_VID_PROG : NIN_VID_FORCE_NTSC;
					break;
				default:
					if(dml_VideoMode) *dml_VideoMode = DML_VID_DML_AUTO;
					if(nin_VideoMode) *nin_VideoMode = NIN_VID_AUTO;
					break;
			}
			break;
		case VIDEO_MODE_PAL50: // PAL50
			rmode =  &TVPal528IntDf;
			rmode_reg = VI_PAL;
			if(dml_VideoMode) *dml_VideoMode = DML_VID_FORCE_PAL50;
			if(nin_VideoMode) *nin_VideoMode = NIN_VID_FORCE_PAL50;
			break;
		case VIDEO_MODE_PAL60: // PAL60
			rmode = &TVEurgb60Hz480IntDf;
			rmode_reg = VI_EURGB60;
			if(dml_VideoMode) *dml_VideoMode = DML_VID_FORCE_PAL60;
			if(nin_VideoMode) *nin_VideoMode = NIN_VID_FORCE_PAL60;
			break;
		case VIDEO_MODE_NTSC: // NTSC
			rmode = &TVNtsc480IntDf;
			rmode_reg = VI_NTSC;
			if(dml_VideoMode) *dml_VideoMode = DML_VID_FORCE_NTSC;
			if(nin_VideoMode) *nin_VideoMode = NIN_VID_FORCE_NTSC;
			break;
		case VIDEO_MODE_PAL480P:
			rmode = &TVEurgb60Hz480Prog;
			rmode_reg = VI_EURGB60;
			if(dml_VideoMode) *dml_VideoMode = DML_VID_FORCE_PROG | DML_VID_PROG_PATCH;
			if(nin_VideoMode) *nin_VideoMode = NIN_VID_FORCE_PAL60 | NIN_VID_PROG;
			break;
		case VIDEO_MODE_NTSC480P:
			rmode = &TVNtsc480Prog;
			rmode_reg = VI_NTSC;
			if(dml_VideoMode) *dml_VideoMode = DML_VID_FORCE_PROG | DML_VID_PROG_PATCH;
			if(nin_VideoMode) *nin_VideoMode = NIN_VID_FORCE_NTSC | NIN_VID_PROG;
			break;
		case VIDEO_MODE_SYSDEFAULT: // AUTO PATCH TO SYSTEM
			break;
	}
}

void Disc_SetVMode(void)
{
	/* Set video mode register */
	*Video_Mode = rmode_reg;
	DCFlushRange((void *) Video_Mode, 4);

	/* Set video mode */
	if (rmode != NULL)
		VIDEO_Configure(rmode);

	/* Setup video  */
	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else while(VIDEO_GetNextField())
		VIDEO_WaitVSync();
}

void __Disc_SetTime(void)
{
	/* Extern */
	extern void settime(u64);

	/* Set proper time */
	settime(secs_to_ticks( time( NULL ) - 946684800 ));
}

s32 Disc_FindPartition(u64 *outbuf)
{
	u64 offset = 0, table_offset = 0;

	u32 cnt, nb_partitions;
	s32 ret;

	/* Read partition info */
	ret = WDVD_UnencryptedRead(buffer, 0x20, PTABLE_OFFSET);
	if (ret < 0) return ret;

	/* Get data */
	nb_partitions = buffer[0];
	table_offset = buffer[1] << 2;

	/* Read partition table */
	ret = WDVD_UnencryptedRead(buffer, 0x20, table_offset);
	if (ret < 0) return ret;

	/* Find game partition */
	for (cnt = 0; cnt < nb_partitions; cnt++)
	{
		u32 type = buffer[cnt * 2 + 1];

		/* Game partition */
		if (!type) offset = buffer[cnt * 2] << 2;
	}

	/* No game partition found */
	if (!offset) return -1;

	/* Set output buffer */
	*outbuf = offset;

	return 0;
}

s32 Disc_Init(void)
{
	/* Init DVD subsystem */
	return WDVD_Init();
}

s32 Disc_Open(void)
{
	s32 ret;

	/* Reset drive */
	ret = WDVD_Reset();
	if (ret < 0) return ret;

	/* Read disc ID */
	return WDVD_ReadDiskId(diskid);
}

s32 Disc_Wait(void)
{
	u32 cover = 0;
	s32 ret;

	/* Wait for disc */
	while (!(cover & 0x2))
	{
		/* Get cover status */
		ret = WDVD_GetCoverStatus(&cover);
		if (ret < 0) return ret;
	}

	return 0;
}

s32 Disc_SetUSB(const u8 *id)
{
	/* Set USB mode */
	return WDVD_SetUSBMode((u8  *) id, -1);
}

s32 Disc_ReadHeader(void *outbuf)
{
	/* Read disc header */
	return WDVD_UnencryptedRead(outbuf, sizeof(struct discHdr), 0);
}

s32 Disc_IsWii(void)
{
	struct discHdr *header = (struct discHdr *) buffer;

	s32 ret;

	/* Read disc header */
	ret = Disc_ReadHeader(header);
	if (ret < 0) return ret;

	/* Check magic word */
	if (header->magic != WII_MAGIC) return -1;

	return 0;
}

s32 Disc_Mount(struct discHdr *header)
{
	if(!header)
		return -1;

	gprintf("\nDiscMount() ");
	s32 ret;

	u8 tmpBuff[0x60];
	memcpy(tmpBuff, diskid, 0x60); // Make a backup of the first 96 bytes at 0x80000000

	Disc_SetUSB(NULL);

	ret = WDVD_Reset();
	if(ret < 0)
		return ret;

	ret = WDVD_ReadDiskId(diskid);
	if(ret < 0)
		return ret;

	ret = WDVD_UnencryptedRead(diskid, 0x60, 0x00);
	if(ret < 0)
		return ret;

	memcpy(header, diskid, sizeof(struct discHdr));
	memcpy(diskid, tmpBuff, 0x60); // Put the backup back, or games won't load

	if(header->magic == 0x5D1C9EA3)
	{
		header->type = TYPE_GAME_WII_DISC;
		return 0;
	}

	if(header->gc_magic == 0xC2339F3D)
	{
		header->type = TYPE_GAME_GC_DISC;
		return 0;
	}

	return -1;
}

s32 Disc_JumpToEntrypoint(s32 hooktype, u32 dolparameter)
{
	/* Set an appropiate video mode */
	Disc_SetVMode();

	/* Set time */
	__Disc_SetTime();

	/* Shutdown IOS subsystems */
	extern void __exception_closeall();
	u32 level = IRQ_Disable();
	__IOS_ShutdownSubsystems();
	__exception_closeall();

	 /* Originally from tueidj - taken from NeoGamme (thx) */
	*(vu32*)0xCC003024 = dolparameter != 0 ? dolparameter : 1;

 	if(AppEntrypoint == 0x3400)
	{
 		if(hooktype)
 		{
			asm volatile (
				"lis %r3, returnpoint@h\n"
				"ori %r3, %r3, returnpoint@l\n"
				"mtlr %r3\n"
				"lis %r3, 0x8000\n"
				"ori %r3, %r3, 0x18A8\n"
				"nop\n"
				"mtctr %r3\n"
				"bctr\n"
				"returnpoint:\n"
				"bl DCDisable\n"
				"bl ICDisable\n"
				"li %r3, 0\n"
				"mtsrr1 %r3\n"
				"lis %r4, AppEntrypoint@h\n"
				"ori %r4,%r4,AppEntrypoint@l\n"
				"lwz %r4, 0(%r4)\n"
				"mtsrr0 %r4\n"
				"rfi\n"
			);
 		}
 		else
 		{
 			asm volatile (
 				"isync\n"
				"lis %r3, AppEntrypoint@h\n"
				"ori %r3, %r3, AppEntrypoint@l\n"
 				"lwz %r3, 0(%r3)\n"
 				"mtsrr0 %r3\n"
 				"mfmsr %r3\n"
 				"li %r4, 0x30\n"
 				"andc %r3, %r3, %r4\n"
 				"mtsrr1 %r3\n"
 				"rfi\n"
 			);
 		}
	}
 	else if (hooktype)
	{
		asm volatile (
				"lis %r3, AppEntrypoint@h\n"
				"ori %r3, %r3, AppEntrypoint@l\n"
				"lwz %r3, 0(%r3)\n"
				"mtlr %r3\n"
				"lis %r3, 0x8000\n"
				"ori %r3, %r3, 0x18A8\n"
				"nop\n"
				"mtctr %r3\n"
				"bctr\n"
		);
	}
	else
	{
		asm volatile (
				"lis %r3, AppEntrypoint@h\n"
				"ori %r3, %r3, AppEntrypoint@l\n"
				"lwz %r3, 0(%r3)\n"
				"mtlr %r3\n"
				"blr\n"
		);
	}

	IRQ_Restore(level);

	return 0;
}

void PatchCountryStrings(void *Address, int Size)
{
	u8 SearchPattern[4] = { 0x00, 0x00, 0x00, 0x00 };
	u8 PatchData[4] = { 0x00, 0x00, 0x00, 0x00 };
	u8 *Addr = (u8*) Address;

	int wiiregion = CONF_GetRegion();

	switch (wiiregion)
	{
		case CONF_REGION_JP:
			SearchPattern[0] = 0x00;
			SearchPattern[1] = 0x4A; // J
			SearchPattern[2] = 0x50; // P
			break;
		case CONF_REGION_EU:
			SearchPattern[0] = 0x02;
			SearchPattern[1] = 0x45; // E
			SearchPattern[2] = 0x55; // U
			break;
		case CONF_REGION_KR:
			SearchPattern[0] = 0x04;
			SearchPattern[1] = 0x4B; // K
			SearchPattern[2] = 0x52; // R
			break;
		case CONF_REGION_CN:
			SearchPattern[0] = 0x05;
			SearchPattern[1] = 0x43; // C
			SearchPattern[2] = 0x4E; // N
			break;
		case CONF_REGION_US:
		default:
			SearchPattern[0] = 0x01;
			SearchPattern[1] = 0x55; // U
			SearchPattern[2] = 0x53; // S
			break;
	}

	switch (diskid[3])
	{
		case 'J':
			PatchData[1] = 0x4A; // J
			PatchData[2] = 0x50; // P
			break;

		case 'D':
		case 'F':
		case 'P':
		case 'X':
		case 'Y':
			PatchData[1] = 0x45; // E
			PatchData[2] = 0x55; // U
			break;

		case 'E':
		default:
			PatchData[1] = 0x55; // U
			PatchData[2] = 0x53; // S
			break;
	}

	while (Size >= 4)
	{
		if (Addr[0] == SearchPattern[0] && Addr[1] == SearchPattern[1] && Addr[2] == SearchPattern[2] && Addr[3]
				== SearchPattern[3])
		{
			//*Addr = PatchData[0];
			Addr += 1;
			*Addr = PatchData[1];
			Addr += 1;
			*Addr = PatchData[2];
			Addr += 1;
			//*Addr = PatchData[3];
			Addr += 1;
			Size -= 4;
		}
		else
		{
			Addr += 4;
			Size -= 4;
		}
	}
}
