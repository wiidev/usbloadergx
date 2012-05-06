
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <malloc.h>
#include <sys/unistd.h>
#include <ogc/ipc.h>

#include "mload/mload.h"
#include "mload/mload_modules.h"

u32 do_bca_code(const char *BCAFilepath, u8 *gameid)
{
	if (!BCAFilepath || !gameid) return 0;

	if (IOS_GetVersion() == 222 || IOS_GetVersion() == 223)
	{
		FILE *fp;
		u32 filesize;
		char filepath[150];
		memset(filepath, 0, 150);
		u8 bcaCode[64] ATTRIBUTE_ALIGN( 32 );

		// Attempt to open the BCA file using the full game ID.
		snprintf(filepath, sizeof(filepath), "%s%.6s.bca", BCAFilepath, gameid);

		fp = fopen(filepath, "rb");
		if (!fp)
		{
			// Not found. Try again without the system code or company code.
			snprintf(filepath, sizeof(filepath), "%s%.3s.bca", BCAFilepath, gameid+1);

			fp = fopen(filepath, "rb");
			if (!fp)
			{
				// Not found. Use the default BCA.
				memset(bcaCode, 0, 64);
				bcaCode[0x33] = 1;
			}
		}

		if (fp)
		{
			// BCA file opened.
			u32 ret = 0;

			fseek(fp, 0, SEEK_END);
			filesize = ftell(fp);

			if (filesize == 64)
			{
				fseek(fp, 0, SEEK_SET);
				ret = fread(bcaCode, 1, 64, fp);
			}
			fclose(fp);

			if (ret != 64)
			{
				// Error reading the BCA file.
				// Use the default BCA.
				memset(bcaCode, 0, 64);
				bcaCode[0x33] = 1;
			}
		}

		Set_DIP_BCA_Datas(bcaCode);
	}
	return 0;
}
