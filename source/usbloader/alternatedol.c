#include <ogcsys.h>
#include <stdio.h>
#include <gccore.h>
#include <string.h>
#include <malloc.h>

#include "fatmounter.h"

/** Alternate dolloader made by WiiPower modified by dimok **/

bool Load_Dol(void **buffer, int* dollen, char * filepath)
{
	int ret;
	FILE* file;
	void* dol_buffer;

	char fullpath[200];
	char gameidbuffer6[7];
	memset(gameidbuffer6, 0, 7);
	memcpy(gameidbuffer6, (char*)0x80000000, 6);
	snprintf(fullpath, 200, "%s%s.dol", filepath, gameidbuffer6);

    SDCard_Init();
    USBDevice_Init();

	file = fopen(fullpath, "rb");

	if(file == NULL)
	{
		fclose(file);
	    SDCard_deInit();
	    USBDevice_deInit();
		return false;
	}

	int filesize;
	fseek(file, 0, SEEK_END);
	filesize = ftell(file);
	fseek(file, 0, SEEK_SET);

	dol_buffer = malloc(filesize);
	if (dol_buffer == NULL)
	{
		fclose(file);
	    SDCard_deInit();
	    USBDevice_deInit();
		return false;
	}
	ret = fread( dol_buffer, 1, filesize, file);
	if(ret != filesize)
	{
		free(dol_buffer);
		fclose(file);
	    SDCard_deInit();
	    USBDevice_deInit();
		return false;
	}
	fclose(file);

	SDCard_deInit();
    USBDevice_deInit();
	*buffer = dol_buffer;
	*dollen = filesize;
	return true;
}

bool Remove_001_Protection(void *Address, int Size)
{
	u8 SearchPattern[16] = 	{ 0x40, 0x82, 0x00, 0x0C, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18 };
	u8 PatchData[16] = 		{ 0x40, 0x82, 0x00, 0x04, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18 };

	void *Addr = Address;
	void *Addr_end = Address+Size;

	while(Addr <= Addr_end-sizeof(SearchPattern))
	{
		if(memcmp(Addr, SearchPattern, sizeof(SearchPattern))==0)
		{
			memcpy(Addr,PatchData,sizeof(PatchData));
			return true;
		}
		Addr += 4;
	}
	return false;
}

typedef struct _dolheader {
	u32 text_pos[7];
	u32 data_pos[11];
	u32 text_start[7];
	u32 data_start[11];
	u32 text_size[7];
	u32 data_size[11];
	u32 bss_start;
	u32 bss_size;
	u32 entry_point;
} dolheader;

u32 load_dol_image(void *dolstart) {

	u32 i;
	dolheader *dolfile;

	if (dolstart) {
		dolfile = (dolheader *) dolstart;
		for (i = 0; i < 7; i++) {
			if ((!dolfile->text_size[i]) || (dolfile->text_start[i] < 0x100)) continue;
			VIDEO_WaitVSync();
			ICInvalidateRange ((void *) dolfile->text_start[i],dolfile->text_size[i]);
			memmove ((void *) dolfile->text_start[i],dolstart+dolfile->text_pos[i],dolfile->text_size[i]);
		}

		for(i = 0; i < 11; i++) {
			if ((!dolfile->data_size[i]) || (dolfile->data_start[i] < 0x100)) continue;
			VIDEO_WaitVSync();
			memmove ((void *) dolfile->data_start[i],dolstart+dolfile->data_pos[i],dolfile->data_size[i]);
			DCFlushRangeNoSync ((void *) dolfile->data_start[i],dolfile->data_size[i]);
		}
        /*
		memset ((void *) dolfile->bss_start, 0, dolfile->bss_size);
		DCFlushRange((void *) dolfile->bss_start, dolfile->bss_size);
        */
		return dolfile->entry_point;
	}
	return 0;
}
