#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>


#include "apploader.h"
#include "disc.h"
#include "video.h"
#include "wdvd.h"
#include "fst.h"

/* Constants */
#define PTABLE_OFFSET	0x40000
#define WII_MAGIC	0x5D1C9EA3

/* Disc pointers */
static u32 *buffer = (u32 *)0x93000000;
static u8  *diskid = (u8  *)0x80000000;
static char gameid[8];


void __Disc_SetLowMem(void)
{
	/* Setup low memory */
	*(vu32 *)0x80000030 = 0x00000000;
	*(vu32 *)0x80000060 = 0x38A00040;
	*(vu32 *)0x800000E4 = 0x80431A80;
	*(vu32 *)0x800000EC = 0x81800000;
	*(vu32 *)0x800000F4 = 0x817E5480;
	*(vu32 *)0x800000F8 = 0x0E7BE2C0;
	*(vu32 *)0x800000FC = 0x2B73A840;

	/* Copy disc ID */
	memcpy((void *)0x80003180, (void *)0x80000000, 4);

	/* Flush cache */
	DCFlushRange((void *)0x80000000, 0x3F00);
}

void __Disc_SetVMode(u8 videoselected)
{
	GXRModeObj *vmode = NULL;

	u32 progressive, tvmode, vmode_reg = 0;

	/* Get video mode configuration */
	progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();
	tvmode      =  CONF_GetVideo();

	/* Select video mode register */
	switch (tvmode) {
	case CONF_VIDEO_PAL:
		vmode_reg = (CONF_GetEuRGB60() > 0) ? 5 : 1;
		break;

	case CONF_VIDEO_MPAL:
		vmode_reg = 4;
		break;

	case CONF_VIDEO_NTSC:
		vmode_reg = 0;
		break;
	}

    switch (videoselected) {
    case 0:

	/* Select video mode */
	switch(diskid[3]) {
	/* PAL */
	case 'D':
	case 'F':
	case 'P':
	case 'X':
	case 'Y':
		if (tvmode != CONF_VIDEO_PAL) {
			vmode_reg = 5;
			vmode     = (progressive) ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
		}

		break;

	/* NTSC or unknown */
	case 'E':
	case 'J':
		if (tvmode != CONF_VIDEO_NTSC) {
			vmode_reg = 0;
			vmode     = (progressive) ? &TVNtsc480Prog : &TVNtsc480IntDf;
		}

		break;
	}
	break;

	case 1:
        vmode_reg = 1;
        progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();
        vmode     = (progressive) ? &TVEurgb60Hz480Prog : &TVPal528IntDf;
        break;
    case 2:
        vmode_reg = 5;
        progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();
        vmode     = (progressive) ? &TVEurgb60Hz480Prog : &TVEurgb60Hz480IntDf;
        break;
    case 3:
        vmode_reg = 0;
        progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();
        vmode     = (progressive) ? &TVNtsc480Prog : &TVNtsc480IntDf;
        break;
    case 4:
 //       vmode     = VIDEO_GetPreferredMode(NULL);
        break;
    }

	/* Set video mode register */
	*(vu32 *)0x800000CC = vmode_reg;

	/* Set video mode */
	if (vmode) {

	VIDEO_Configure(vmode);

	/* Setup video */
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	}
}

void __Disc_SetTime(void)
{
	/* Extern */
	extern void settime(long long);

	/* Set proper time */
	settime(secs_to_ticks(time(NULL) - 946684800));
}

s32 __Disc_FindPartition(u64 *outbuf)
{
	u64 offset = 0, table_offset = 0;

	u32 cnt, nb_partitions;
	s32 ret;

	/* Read partition info */
	ret = WDVD_UnencryptedRead(buffer, 0x20, PTABLE_OFFSET);
	if (ret < 0)
		return ret;

	/* Get data */
	nb_partitions = buffer[0];
	table_offset  = buffer[1] << 2;

	/* Read partition table */
	ret = WDVD_UnencryptedRead(buffer, 0x20, table_offset);
	if (ret < 0)
		return ret;

	/* Find game partition */
	for (cnt = 0; cnt < nb_partitions; cnt++) {
		u32 type = buffer[cnt * 2 + 1];

		/* Game partition */
		if(!type)
			offset = buffer[cnt * 2] << 2;
	}

	/* No game partition found */
	if (!offset)
		return -1;

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
	if (ret < 0)
		return ret;

	/* Read disc ID */
	return WDVD_ReadDiskId(diskid);
}

s32 Disc_Wait(void)
{
	u32 cover = 0;
	s32 ret;

	/* Wait for disc */
	while (!(cover & 0x2)) {
		/* Get cover status */
		ret = WDVD_GetCoverStatus(&cover);
		if (ret < 0)
			return ret;
	}

	return 0;
}

s32 Disc_SetUSB(u8 *id, int ios222)
{
	/* Set USB mode */
	return WDVD_SetUSBMode(id, ios222);
}

s32 Disc_ReadHeader(void *outbuf)
{
	/* Read disc header */
	return WDVD_UnencryptedRead(outbuf, sizeof(struct discHdr), 0);
}

s32 Disc_IsWii(void)
{
	struct discHdr *header = (struct discHdr *)buffer;

	s32 ret;

	/* Read disc header */
	ret = Disc_ReadHeader(header);
	if (ret < 0)
		return ret;

	/* Check magic word */
	if (header->magic != WII_MAGIC)
		return -1;

	return 0;
}

s32 Disc_BootPartition(u64 offset, u8 videoselected, u8 cheat, u8 vipatch)
{
	entry_point p_entry;

	s32 ret;

	/* Open specified partition */
	ret = WDVD_OpenPartition(offset);
	if (ret < 0)
		return ret;

	/* Run apploader */
	ret = Apploader_Run(&p_entry, cheat, videoselected, vipatch);
	if (ret < 0)
		return ret;

	/* Setup low memory */
	__Disc_SetLowMem();

	/* Set an appropiate video mode */
	__Disc_SetVMode(videoselected);

	/* Set time */
	__Disc_SetTime();

	if (cheat == 1) {
    /* OCARINA STUFF - FISHEARS*/
    memset(gameid, 0, 8);
	memcpy(gameid, (char*)0x80000000, 6);
	do_sd_code(gameid);
    /* OCARINA STUFF - FISHEARS*/
	}

	/* Disconnect Wiimote */
    WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();

	/* Shutdown IOS subsystems */
 	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

	/* Jump to entry point */
	p_entry();

	return 0;
}

s32 Disc_WiiBoot(u8 videoselected, u8 cheat, u8 vipatch)
{
	u64 offset;
	s32 ret;

	/* Find game partition offset */
	ret = __Disc_FindPartition(&offset);
	if (ret < 0)
		return ret;

	/* Boot partition */
	return Disc_BootPartition(offset, videoselected, cheat, vipatch);
}
