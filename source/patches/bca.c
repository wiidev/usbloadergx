
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
	if (!BCAFilepath) return 0;

	if (IOS_GetVersion() == 222 || IOS_GetVersion() == 223)
	{
		FILE *fp;
		u32 filesize;
		char filepath[150];
		memset(filepath, 0, 150);
		u8 bcaCode[64] ATTRIBUTE_ALIGN( 32 );

		sprintf(filepath, "%s%6s", BCAFilepath, gameid);
		filepath[strlen(BCAFilepath) + 6] = '.';
		filepath[strlen(BCAFilepath) + 7] = 'b';
		filepath[strlen(BCAFilepath) + 8] = 'c';
		filepath[strlen(BCAFilepath) + 9] = 'a';

		fp = fopen(filepath, "rb");
		if (!fp)
		{
			memset(filepath, 0, 150);
			sprintf(filepath, "%s%3s", BCAFilepath, gameid + 1);
			filepath[strlen(BCAFilepath) + 3] = '.';
			filepath[strlen(BCAFilepath) + 4] = 'b';
			filepath[strlen(BCAFilepath) + 5] = 'c';
			filepath[strlen(BCAFilepath) + 6] = 'a';
			fp = fopen(filepath, "rb");

			if (!fp)
			{
				// Set default bcaCode
				memset(bcaCode, 0, 64);
				bcaCode[0x33] = 1;
			}
		}

		if (fp)
		{
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
				// Set default bcaCode
				memset(bcaCode, 0, 64);
				bcaCode[0x33] = 1;
			}
		}

		Set_DIP_BCA_Datas(bcaCode);
	}
	return 0;
}
