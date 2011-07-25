#include <stdio.h>
#include <ogcsys.h>
#include <string.h>
#include <malloc.h>

#include "apploader.h"
#include "wdvd.h"
#include "wpad.h"
#include "disc.h"
#include "alternatedol.h"
#include "fstfile.h"
#include "gecko.h"
#include "patches/gamepatches.h"
#include "patches/wip.h"
#include "settings/SettingsEnums.h"

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

s32 Apploader_Run(entry_point *entry, char * dolpath, u8 alternatedol, u32 alternatedoloffset)
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

	for (;;)
	{
		void *dst = NULL;
		int len = 0, offset = 0;

		/* Run apploader main function */
		ret = appldr_main(&dst, &len, &offset);
		if (!ret) break;

		/* Read data from DVD */
		WDVD_Read(dst, len, (u64) (offset << 2));

		RegisterDOL((u8 *) dst, len);

		DCFlushRange(dst, len);
	}

	*entry = appldr_final();

	/** Load alternate dol if set **/
	if (alternatedol == ALT_DOL_FROM_SD_USB)
	{
		ClearDOLList();
		wip_reset_counter();
		void *dolbuffer = NULL;
		int dollen = 0;

		bool dolloaded = Load_Dol(&dolbuffer, &dollen, dolpath);
		if (dolloaded)
			*entry = (entry_point) load_dol_image(dolbuffer);

		if (dolbuffer) free(dolbuffer);
	}
	else if (alternatedol == ALT_DOL_FROM_GAME && alternatedoloffset != 0)
	{
		ClearDOLList();
		wip_reset_counter();
		FST_ENTRY *fst = (FST_ENTRY *) *(u32 *) 0x80000038;

		//! Check if it's inside the limits
		if(alternatedoloffset >= fst[0].filelen)
			return 0;

		*entry = (entry_point) Load_Dol_from_disc(fst[alternatedoloffset].fileoffset);
	}

	return 0;
}
