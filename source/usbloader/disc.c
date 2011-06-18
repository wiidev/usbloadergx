#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <ogc/lwp_watchdog.h>

#include "patches/fst.h"
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
#include "../settings/SettingsEnums.h"
#include "../gecko.h"

/* Constants */
#define PTABLE_OFFSET   0x40000
#define WII_MAGIC   0x5D1C9EA3

/* Disc pointers */
static u32 *buffer = (u32 *) 0x93000000;
static u8 *diskid = (u8 *) Disc_ID;
static GXRModeObj *vmode = NULL;
static u32 vmode_reg = 0;

void Disc_SetLowMem(void)
{
    /* Setup low memory */
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

    int iosVer = IOS_GetVersion();
    if(iosVer != 222 && iosVer != 223 && iosVer != 224 && IOS_GetRevision() >= 18)
        *GameID_Address = 0x80000000; // Game ID Address

    /* Copy disc ID */
    memcpy((void *) Online_Check, (void *) Disc_ID, 4);

    /* Flush cache */
    DCFlushRange((void *) Disc_ID, 0x3F00);
}

void Disc_SelectVMode(u8 videoselected)
{
    vmode = VIDEO_GetPreferredMode(0);

	/* Get video mode configuration */
	bool progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();

	/* Select video mode register */
	switch (CONF_GetVideo())
	{
		case CONF_VIDEO_PAL:
			if (CONF_GetEuRGB60() > 0)
			{
				vmode_reg = VI_EURGB60;
				vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
			}
			else
				vmode_reg = VI_PAL;
			break;

		case CONF_VIDEO_MPAL:
			vmode_reg = VI_MPAL;
			break;

		case CONF_VIDEO_NTSC:
			vmode_reg = VI_NTSC;
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
					if (CONF_GetVideo() != CONF_VIDEO_PAL)
					{
						vmode_reg = VI_PAL;
						vmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
					}
					break;
				// NTSC
				case 'E':
				case 'J':
				default:
					if (CONF_GetVideo() != CONF_VIDEO_NTSC)
					{
						vmode_reg = VI_NTSC;
						vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
					}
					break;
			}
			break;
		case VIDEO_MODE_PAL50: // PAL50
			vmode =  &TVPal528IntDf;
			vmode_reg = vmode->viTVMode >> 2;
			break;
		case VIDEO_MODE_PAL60: // PAL60
			vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
			vmode_reg = progressive ? TVEurgb60Hz480Prog.viTVMode >> 2 : vmode->viTVMode >> 2;
			break;
		case VIDEO_MODE_NTSC: // NTSC
			vmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
			vmode_reg = vmode->viTVMode >> 2;
			break;
        case VIDEO_MODE_PAL480P:
            vmode_reg = TVEurgb60Hz480Prog.viTVMode >> 2;
            vmode = &TVNtsc480Prog;
            break;
        case VIDEO_MODE_NTSC480P:
            vmode_reg = VI_NTSC;
            vmode = &TVNtsc480Prog;
            break;
		case VIDEO_MODE_SYSDEFAULT: // AUTO PATCH TO SYSTEM
			break;
	}
}

void __Disc_SetVMode(void)
{
	/* Set video mode register */
	*Video_Mode = vmode_reg;

	/* Set video mode */
	if (vmode != NULL)
	{
		VIDEO_Configure(vmode);
	}

	/* Setup video  */
	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
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

s32 Disc_JumpToEntrypoint(bool enablecheat, u32 dolparameter)
{
    /* Set an appropiate video mode */
    __Disc_SetVMode();

    /* Set time */
    __Disc_SetTime();

    /* Shutdown IOS subsystems */
    extern void __exception_closeall();
    u32 level = IRQ_Disable();
    __IOS_ShutdownSubsystems();
    __exception_closeall();

     /* Originally from tueidj - taken from NeoGamme (thx) */
	*(vu32*)0xCC003024 = dolparameter != 0 ? dolparameter : 1;

    if (enablecheat)
    {
        __asm__(
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
        __asm__(
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
