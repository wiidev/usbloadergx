#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>

#include "patches/fst.h"
#include "patches/gamepatches.h"
#include "patches/wip.h"
#include "apploader.h"
#include "disc.h"
#include "video.h"
#include "wdvd.h"
#include "alternatedol.h"
#include "memory/memory.h"
#include "wbfs.h"
#include "../settings/SettingsEnums.h"
#include "../gecko.h"
#include "../fatmounter.h"

/* Constants */
#define PTABLE_OFFSET   0x40000
#define WII_MAGIC   0x5D1C9EA3

/* Disc pointers */
static u32 *buffer = (u32 *) 0x93000000;
static u8 *diskid = (u8 *) Disc_ID;

void __Disc_SetLowMem(void)
{

    *Sys_Magic = 0x0D15EA5E; // Standard Boot Code
    *Version = 0x00000001; // Version
    *Arena_L = 0x00000000; // Arena Low
    *BI2 = 0x817E5480; // BI2
    *Bus_Speed = 0x0E7BE2C0; // Console Bus Speed
    *CPU_Speed = 0x2B73A840; // Console CPU Speed

    /* Setup low memory */
    *Assembler = 0x38A00040; // Assembler
    *(u32 *) 0x800000E4 = 0x80431A80;
    *Dev_Debugger = 0x81800000; // Dev Debugger Monitor Address
    *Simulated_Mem = 0x01800000; // Simulated Memory Size
    *(vu32 *) 0xCD00643C = 0x00000000; // 32Mhz on Bus

    //If the game is sam & max: season 1  put this shit in
    if ((strncmp((char*) Disc_ID, "R3XE6U", 6) == 0) || (strncmp((char*) Disc_ID, "R3XP6V", 6) == 0))
    {
        *GameID_Address = 0x80000000; // Game ID Address
    }

    /* Copy disc ID */
    memcpy((void *) Online_Check, (void *) Disc_ID, 4);

    /* Flush cache */
    DCFlushRange((void *) Disc_ID, 0x3F00);
}

void __Disc_SetVMode(u8 videoselected)
{
    GXRModeObj *vmode = NULL;

    u32 progressive, tvmode, vmode_reg = 0;

    /* Get video mode configuration */
    progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();
    tvmode = CONF_GetVideo();

    /* Select video mode register */
    switch (tvmode)
    {
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

    switch (videoselected)
    {
	case VIDEO_MODE_PAL50:
            vmode = &TVPal528IntDf;
            vmode_reg = (vmode->viTVMode) >> 2;
            break;
	case VIDEO_MODE_PAL60:
            vmode = (progressive) ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
            vmode_reg = (vmode->viTVMode) >> 2;
            break;
	case VIDEO_MODE_NTSC:
            vmode = (progressive) ? &TVNtsc480Prog : &TVNtsc480IntDf;
            vmode_reg = (vmode->viTVMode) >> 2;
            break;
	case VIDEO_MODE_SYSDEFAULT:
            //       vmode     = VIDEO_GetPreferredMode(NULL);
            break;
	case VIDEO_MODE_DISCDEFAULT:
	default:
	    /* Select video mode */
	    switch (diskid[3])
	    {
		/* PAL */
		case 'P':
		case 'D':
		case 'F':
		case 'I':
		case 'S':
		case 'H':
		case 'X':
		case 'Y':
		case 'Z':
		    if (tvmode != CONF_VIDEO_PAL)
		    {
			vmode_reg = 5;
			vmode = (progressive) ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
		    }

		    break;

		    /* NTSC or unknown */
		case 'E':
		case 'J':
		case 'K':
		case 'W':
		    if (tvmode != CONF_VIDEO_NTSC)
		    {
			vmode_reg = 0;
			vmode = (progressive) ? &TVNtsc480Prog : &TVNtsc480IntDf;
		    }

		    break;
	    }
	    break;
	}

    /* Set video mode register */
    *Video_Mode = vmode_reg;

    /* Set video mode */
    if (vmode)
    {

        VIDEO_Configure(vmode);

        /* Setup video */
        VIDEO_SetBlack(FALSE);
        VIDEO_Flush();
        VIDEO_WaitVSync();

        if (vmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
    }
    gprintf("Video mode - %s\n", ((progressive) ? "progressive" : "interlaced"));

}

void __Disc_SetTime(void)
{
    /* Extern */
    extern void settime(u64);

    /* Set proper time */
    settime(secs_to_ticks( time( NULL ) - 946684800 ));
}

s32 __Disc_FindPartition(u64 *outbuf)
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
    u32 part = 0;
    if (wbfs_part_fs)
    {
        part = wbfs_part_lba;
    }
    else
    {
        part = wbfs_part_idx ? wbfs_part_idx - 1 : 0;
    }

    /* Set USB mode */
    return WDVD_SetUSBMode((u8  *) id, part);
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

s32 Disc_JumpToEntrypoint(u8 videoselected, bool enablecheat)
{
    /* Set an appropiate video mode */
    __Disc_SetVMode(videoselected);

    /* Set time */
    __Disc_SetTime();

    // Anti-green screen fix
    VIDEO_SetBlack(TRUE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
	VIDEO_WaitVSync();
    gprintf("USB Loader GX is done.\n");

    /* Shutdown IOS subsystems */
    // fix for PeppaPig (from WiiFlow)
	u8 temp_data[4];
	memcpy(temp_data, (u8 *) 0x800000F4, 4);
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	memcpy((u8 *) 0x800000F4, temp_data, 4);

    if (enablecheat)
    {
        __asm__(
                "lis %r3, AppEntrypoint@h\n"
                "ori %r3, %r3, AppEntrypoint@l\n"
                "lwz %r3, 0(%r3)\n"
                "mtlr %r3\n"
                "lis %r3, 0x8000\n"
                "ori %r3, %r3, 0x18A8\n"
                "mtctr %r3\n"
                "bctr\n"
        );
    }
    else
    {
        __asm__(
                "lis %r3, AppEntrypoint@h\n"
                "ori %r3, %r3, AppEntrypoint@l\n"
                "lwz %r3, 0(%r3)\n"
                "mtlr %r3\n"
                "blr\n"
        );
    }

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
