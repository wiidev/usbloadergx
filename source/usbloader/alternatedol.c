#include <ogcsys.h>
#include <stdio.h>
#include <gccore.h>
#include <string.h>
#include <malloc.h>

#include "patches/gamepatches.h"
#include "apploader.h"
#include "wdvd.h"
#include "fstfile.h"

typedef struct _dolheader
{
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

static bool Remove_001_Protection(void *Address, int Size)
{
	u8 SearchPattern[16] = { 0x40, 0x82, 0x00, 0x0C, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18 };
	u8 PatchData[16] = { 0x40, 0x82, 0x00, 0x04, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18 };

	void *Addr = Address;
	void *Addr_end = Address + Size;

	while (Addr <= Addr_end - sizeof(SearchPattern))
	{
		if (memcmp(Addr, SearchPattern, sizeof(SearchPattern)) == 0)
		{
			memcpy(Addr, PatchData, sizeof(PatchData));
			return true;
		}
		Addr += 4;
	}
	return false;
}

bool Load_Dol(void **buffer, int* dollen, const char * filepath)
{
	int ret;
	FILE* file;
	void* dol_buffer;

	char fullpath[200];
	char gameidbuffer6[7];
	memset(gameidbuffer6, 0, 7);
	memcpy(gameidbuffer6, (char*) 0x80000000, 6);
	snprintf(fullpath, 200, "%s%s.dol", filepath, gameidbuffer6);

	file = fopen(fullpath, "rb");
	if (file == NULL)
	{
		fclose(file);
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
		return false;
	}
	ret = fread(dol_buffer, 1, filesize, file);
	if (ret != filesize)
	{
		free(dol_buffer);
		fclose(file);
		return false;
	}
	fclose(file);

	*buffer = dol_buffer;
	*dollen = filesize;
	return true;
}

u32 load_dol_image(void *dolstart)
{
	if (!dolstart)
		return 0;

	u32 i;
	dolheader *dolfile = (dolheader *) dolstart;

	for (i = 0; i < 7; i++)
	{
		if ((!dolfile->text_size[i]) || (dolfile->text_start[i] < 0x100)) continue;

		memmove((void *) dolfile->text_start[i], dolstart + dolfile->text_pos[i], dolfile->text_size[i]);
		RegisterDOL((u8 *) dolfile->text_start[i], dolfile->text_size[i]);
		Remove_001_Protection((void *) dolfile->data_start[i], dolfile->data_size[i]);
		DCFlushRange((void *) dolfile->data_start[i], dolfile->data_size[i]);
		ICInvalidateRange((void *) dolfile->text_start[i], dolfile->text_size[i]);
	}

	for (i = 0; i < 11; i++)
	{
		if ((!dolfile->data_size[i]) || (dolfile->data_start[i] < 0x100)) continue;

		memmove((void *) dolfile->data_start[i], dolstart + dolfile->data_pos[i], dolfile->data_size[i]);
		RegisterDOL((u8 *) dolfile->data_start[i], dolfile->data_size[i]);
		Remove_001_Protection((void *) dolfile->data_start[i], dolfile->data_size[i]);
		DCFlushRange((void *) dolfile->data_start[i], dolfile->data_size[i]);
	}

	return dolfile->entry_point;
}

u32 Load_Dol_from_disc(u32 offset)
{
	s32 ret;
	dolheader * dolfile;
	u8 * buffer;
	u32 pos, size;
	u32 i;
	u32 entrypoint;
	u64 doloffset = ((u64) offset) << 2;

	dolfile = (dolheader *) memalign(32, sizeof(dolheader));
	if (dolfile == NULL)
		return 0;

	memset(dolfile, 0, sizeof(dolheader));

	ret = WDVD_Read(dolfile, sizeof(dolheader), doloffset);
	if(ret < 0)
	{
		free(dolfile);
		return 0;
	}

	entrypoint = dolfile->entry_point;
	if (entrypoint == 0)
	{
		free(dolfile);
		return 0;
	}

	for (i = 0; i < 7; ++i)
	{
		if ((!dolfile->text_size[i]) || (dolfile->text_start[i] < 0x100)) continue;

		buffer = (u8 *) dolfile->text_start[i];
		size = dolfile->text_size[i];
		pos = dolfile->text_pos[i];

		ret = WDVD_Read(buffer, size, doloffset+pos);
		if(ret < 0)
		{
			free(dolfile);
			return 0;
		}

		RegisterDOL((u8 *) buffer, size);
		Remove_001_Protection(buffer, size);
		DCFlushRange(buffer, size);
		ICInvalidateRange(buffer, size);
	}

	for (i = 0; i < 11; ++i)
	{
		if ((!dolfile->data_size[i]) || (dolfile->data_start[i] < 0x100)) continue;

		buffer = (u8 *) dolfile->data_start[i];
		size = dolfile->data_size[i];
		pos = dolfile->data_pos[i];

		ret = WDVD_Read(buffer, size, doloffset+pos);
		if(ret < 0)
		{
			free(dolfile);
			return 0;
		}

		RegisterDOL((u8 *) buffer, size);
		Remove_001_Protection(buffer, size);
		DCFlushRange(buffer, size);
	}

	free(dolfile);

	return entrypoint;

}
