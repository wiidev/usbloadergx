#include <ogcsys.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "libs/libwbfs/libwbfs.h"
#include "libs/libfat/fatfile_frag.h"
#include "libs/libntfs/ntfsfile_frag.h"
#include "libs/libext2fs/ext2_frag.h"

#include "usbloader/wbfs.h"
#include "usbloader/wdvd.h"
#include "usbloader/usbstorage2.h"
#include "frag.h"
#include "sys.h"
#include "gecko.h"

#define SAFE_FREE(x) if(x) { free(x); x = NULL; }

static FragList *frag_list = NULL;

void frag_init(FragList *ff, int maxnum)
{
	memset(ff, 0, sizeof(Fragment) * (maxnum+1));
	ff->maxnum = maxnum;
}

void frag_dump(FragList *ff)
{
	u32 i;
	gprintf("frag list: %d %d 0x%x\n", ff->num, ff->size, ff->size);
	for (i=0; i<ff->num; i++) {
		if (i>10) {
			gprintf("...\n");
			break;
		}
		gprintf(" %d : %8x %8x %8x\n", i,
				ff->frag[i].offset,
				ff->frag[i].count,
				ff->frag[i].sector);
	}
}

int frag_append(void *f, u32 offset, u32 sector, u32 count)
{
	FragList *ff = (FragList *) f;
	int n;
	if (count) {
		n = ff->num - 1;
		if (ff->num > 0
			&& ff->frag[n].offset + ff->frag[n].count == offset
			&& ff->frag[n].sector + ff->frag[n].count == sector)
		{
			// merge
			ff->frag[n].count += count;
		}
		else
		{
			// add
			if (ff->num >= ff->maxnum) {
				// too many fragments
				return -500;
			}
			n = ff->num;
			ff->frag[n].offset = offset;
			ff->frag[n].sector = sector;
			ff->frag[n].count  = count;
			ff->num++;
		}
	}
	ff->size = offset + count;
	return 0;
}

int frag_concat(FragList *ff, FragList *src)
{
	u32 i;
	int ret;
	u32 size = ff->size;
	//printf("concat: %d %d <- %d %d\n", ff->num, ff->size, src->num, src->size);
	for (i=0; i<src->num; i++) {
		ret = frag_append(ff, size + src->frag[i].offset,
				src->frag[i].sector, src->frag[i].count);
		if (ret) return ret;
	}
	ff->size = size + src->size;
	//printf("concat: -> %d %d\n", ff->num, ff->size);
	return 0;
}

// in case a sparse block is requested,
// the returned poffset might not be equal to requested offset
// the difference should be filled with 0
int frag_get(FragList *ff, u32 offset, u32 count, u32 *poffset, u32 *psector, u32 *pcount)
{
	u32 i;
	u32 delta;
	//printf("frag_get(%u %u)\n", offset, count);
	for (i=0; i<ff->num; i++) {
		if (ff->frag[i].offset <= offset && ff->frag[i].offset + ff->frag[i].count > offset)
		{
			delta = offset - ff->frag[i].offset;
			*poffset = offset;
			*psector = ff->frag[i].sector + delta;
			*pcount = ff->frag[i].count - delta;
			if (*pcount > count)
				*pcount = count;
			return 0;
		}
		if (ff->frag[i].offset > offset
			&& ff->frag[i].offset < offset + count)
		{
			delta = ff->frag[i].offset - offset;
			*poffset = ff->frag[i].offset;
			*psector = ff->frag[i].sector;
			*pcount = ff->frag[i].count;
			count -= delta;
			if (*pcount > count)
				*pcount = count;
			return 0;
		}
	}
	// not found
	if (offset + count > ff->size) {
		// error: out of range!
		return -1;
	}
	// if inside range, then it must be just sparse, zero filled
	// return empty block at the end of requested
	*poffset = offset + count;
	*psector = 0;
	*pcount = 0;

	return 0;
}

int frag_remap(FragList *ff, FragList *log, FragList *phy)
{
	u32 i;
	int ret;
	u32 offset;
	u32 sector;
	u32 count;
	u32 delta;
	for (i=0; i<log->num; i++) {
		delta = 0;
		count = 0;
		do {
			ret = frag_get(phy,
					log->frag[i].sector + delta + count,
					log->frag[i].count - delta - count,
					&offset, &sector, &count);
			if (ret) return ret; // error
			delta = offset - log->frag[i].sector;
			ret = frag_append(ff, log->frag[i].offset + delta, sector, count);
			if (ret) return ret; // error
		} while (count + delta < log->frag[i].count);
	}
	return 0;
}

int get_frag_list_for_file(char *fname, u8 *id, const u8 wbfs_part_fs, const u32 lba_offset, const u32 hdd_sector_size)
{
	struct stat st;
	FragList *fs = NULL;
	FragList *fa = NULL;
	FragList *fw = NULL;
	int ret;
	u32 i, j;
	int is_wbfs = 0;
	int ret_val = -1;

	if (strcasecmp(strrchr(fname,'.'), ".wbfs") == 0) {
		is_wbfs = 1;
	}

	fs = malloc(sizeof(FragList));
	fa = malloc(sizeof(FragList));
	fw = malloc(sizeof(FragList));

	frag_init(fa, MAX_FRAG);

	for (i=0; i<10; i++) {
		frag_init(fs, MAX_FRAG);
		if (i > 0) {
			fname[strlen(fname)-1] = '0' + i;
			if (stat(fname, &st) == -1) break;
		}
		if (wbfs_part_fs == PART_FS_FAT) {
			ret = _FAT_get_fragments(fname, &frag_append, fs);
			if (ret) {
				// don't return failure, let it fallback to old method
				//ret_val = ret;
				ret_val = 0;
				goto out;
			}
		} else if (wbfs_part_fs == PART_FS_NTFS) {
			ret = _NTFS_get_fragments(fname, &frag_append, fs);
			if (ret) {
				ret_val = ret;
				goto out;
			}
			// offset to start of partition
			for (j=0; j<fs->num; j++) {
				fs->frag[j].sector += lba_offset;
			}
		} else if (wbfs_part_fs == PART_FS_EXT) {
			ret = _EXT2_get_fragments(fname, &frag_append, fs);
			if (ret) {
				ret_val = ret;
				goto out;
			}
			// offset to start of partition
			for (j=0; j<fs->num; j++) {
				fs->frag[j].sector += lba_offset;
			}
		} else if (wbfs_part_fs == PART_FS_WBFS) {
			// if wbfs file format, remap.
			wbfs_disc_t *d = WBFS_OpenDisc(id);
			if (!d) { ret_val = -4; WBFS_CloseDisc(d); goto out; }
			ret = wbfs_get_fragments(d, &frag_append, fs, hdd_sector_size);
			WBFS_CloseDisc(d);
			if (ret) { ret_val = -5; goto out; }
		}
		frag_concat(fa, fs);
	}

	frag_list = memalign(32, (sizeof(FragList)+31)&(~31));
	frag_init(frag_list, MAX_FRAG);
	if (is_wbfs) {
		// if wbfs file format, remap.
		wbfs_disc_t *d = WBFS_OpenDisc(id);
		if (!d) { ret_val = -4; goto out; }
		frag_init(fw, MAX_FRAG);
		ret = wbfs_get_fragments(d, &frag_append, fw, hdd_sector_size);
		if (ret) { ret_val = -5; goto out; }
		WBFS_CloseDisc(d);
		// DEBUG: frag_list->num = MAX_FRAG-10; // stress test
		ret = frag_remap(frag_list, fw, fa);
		if (ret) { ret_val = -6; goto out; }
	} else {
		// .iso does not need remap just copy
		memcpy(frag_list, fa, sizeof(FragList));
	}

	ret_val = 0;

out:
	if (ret_val) {
		// error
		SAFE_FREE(frag_list);
	}
	SAFE_FREE(fs);
	SAFE_FREE(fa);
	SAFE_FREE(fw);

	return ret_val;
}

int get_frag_list(u8 *id)
{
	return WBFS_GetFragList(id);
}

int set_frag_list(u8 *id)
{
	if (frag_list == NULL) {
		return -2;
	}

	// (+1 for header which is same size as fragment)
	int size = sizeof(Fragment) * (frag_list->num + 1);

	gprintf("Calling WDVD_SetFragList, frag list size %d\n", size);
	int ret = WDVD_SetFragList(WBFS_MIN_DEVICE, frag_list, size);

	free(frag_list);
	frag_list = NULL;

	if (ret)
		return ret;

	// verify id matches
	char discid[32] ATTRIBUTE_ALIGN(32);
	memset(discid, 0, sizeof(discid));
	ret = WDVD_UnencryptedRead(discid, 32, 0);
	gprintf("Reading ID after setting fraglist: %s (expected: %s)\n", discid, id);
	return (strncasecmp((char *) id, discid, 6) != 0) ? -3 : 0;
}
