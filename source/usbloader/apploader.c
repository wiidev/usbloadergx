#include <stdio.h>
#include <ogcsys.h>
#include <string.h>
#include <malloc.h>

#include "patches/patchcode.h"
#include "apploader.h"
#include "wdvd.h"
#include "wpad.h"
#include "disc.h"
#include "alternatedol.h"
#include "fstfile.h"
#include "gecko.h"
#include "patches/wip.h"
#include "patches/dolpatcher.h"
#include "patches/gamepatches.h"

extern bool geckoinit;

/* Apploader function pointers */
typedef int (*app_main)(void **dst, int *size, int *offset);
typedef void (*app_init)(void(*report)(const char *fmt, ...));
typedef void *(*app_final)();
typedef void (*app_entry)(void(**init)(void(*report)(const char *fmt, ...)), int(**main)(), void *(**final)());

/* Apploader pointers */
static u8 *appldr = (u8 *) 0x81200000;

/* Constants */
#define APPLDR_OFFSET   0x2440

/* Variables */
static u32 buffer[0x20] ATTRIBUTE_ALIGN( 32 );

void gamepatches( u8 * dst, int len, u8 videoSelected, u8 languageChoice, u8 patchcountrystring, u8 vipatch, u8 cheat, u32 returnTo, u8 fix002 )
{
    VideoModePatcher( dst, len, videoSelected );

    if ( cheat )
        dogamehooks( dst, len );

    if ( vipatch )
        vidolpatcher( dst, len );

    /*LANGUAGE PATCH - FISHEARS*/
    langpatcher( dst, len, languageChoice );

    /*Thanks to WiiPower*/
    if ( patchcountrystring == 1 )
        PatchCountryStrings( dst, len );

    do_wip_code( ( u8 * ) dst, len );

    if ( fix002 == 2 )
        Anti_002_fix( dst, len );

    PatchReturnTo( dst, len, returnTo );
}

s32 Apploader_Run(entry_point *entry, char * dolpath, u8 cheat, u8 videoSelected, u8 languageChoice, u8 vipatch, u8 patchcountrystring,
	u8 alternatedol, u32 alternatedoloffset, u32 returnTo, u8 fix002)
{
    app_entry appldr_entry;
    app_init appldr_init;
    app_main appldr_main;
    app_final appldr_final;

    u32 appldr_len;
    s32 ret;
    gprintf("\nApploader_Run() started\n");

    /* Read apploader header */
    ret = WDVD_Read(buffer, 0x20, APPLDR_OFFSET);
    if (ret < 0) return ret;

    /* Calculate apploader length */
    appldr_len = buffer[5] + buffer[6];

    /* Read apploader code */
    ret = WDVD_Read(appldr, appldr_len, APPLDR_OFFSET + 0x20);
    if (ret < 0) return ret;

    /* Set apploader entry function */
    appldr_entry = (app_entry) buffer[4];

    /* Call apploader entry */
    appldr_entry(&appldr_init, &appldr_main, &appldr_final);

    /* Initialize apploader */
    appldr_init(gprintf);

    if (fix002 != 0)
    {
        /* ERROR 002 fix (thanks to WiiPower for sharing this)*/
        *(u32 *) 0x80003188 = *(u32 *) 0x80003140;
    }

    for (;;)
    {
        void *dst = NULL;
        int len = 0, offset = 0;

        /* Run apploader main function */
        ret = appldr_main(&dst, &len, &offset);
        if (!ret) break;

        /* Read data from DVD */
        WDVD_Read(dst, len, (u64) (offset << 2));

        if( !alternatedol )
            gamepatches(dst, len, videoSelected, languageChoice, patchcountrystring, vipatch, cheat, returnTo, fix002 );

        DCFlushRange(dst, len);
    }

    *entry = appldr_final();

    /** Load alternate dol if set **/
    if (alternatedol == 2)
    {
        wip_reset_counter();
        void *dolbuffer = NULL;
        int dollen = 0;

        bool dolloaded = Load_Dol(&dolbuffer, &dollen, dolpath, videoSelected, languageChoice, patchcountrystring, vipatch, cheat, returnTo);
        if (dolloaded)
        {
            *entry = (entry_point) load_dol_image(dolbuffer);
        }

        if (dolbuffer) free(dolbuffer);
    }
    else if ((alternatedol == 1 || alternatedol == 3) && alternatedoloffset != 0)
    {
        wip_reset_counter();
        FST_ENTRY *fst = (FST_ENTRY *) *(u32 *) 0x80000038;

        *entry = (entry_point) Load_Dol_from_disc(fst[alternatedoloffset].fileoffset, videoSelected, languageChoice,
		patchcountrystring, vipatch, cheat, returnTo);
    }

    return 0;
}
