#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <ogcsys.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <ctype.h>

#include "libwbfs/libwbfs.h"
#include "sdhc.h"
#include "usbstorage.h"
#include "utils.h"
#include "video.h"
#include "wbfs.h"
#include "wdvd.h"
#include "splits.h"
#include "fat.h"
#include "partition_usbloader.h"
#include "wpad.h"
#include "wbfs_fat.h"
#include "disc.h"
#include "settings/cfg.h"

// WBFS FAT by oggzee

#define D_S(A) A, sizeof(A)

char wbfs_fat_drive[16];
char wbfs_fat_dir[16] = "/wbfs";

int  wbfs_fat_vfs_have = 0;
int  wbfs_fat_vfs_lba = 0;
struct statvfs wbfs_fat_vfs;

split_info_t split;

static u32 fat_sector_size = 512;

void WBFS_Spinner(s32 x, s32 max);
s32 __WBFS_ReadDVD(void *fp, u32 lba, u32 len, void *iobuf);

s32 _WBFS_FAT_GetHeadersCount(void *outbuf, u32 *count, u32 len)
{
	DIR *dir;
	struct dirent *dent;
	char *p;
	int ret, cnt = 0;
	char path[100];
	wbfs_t *part = NULL;
	u32 size;
	u8 *ptr;
	struct discHdr tmpHdr;
	int hdrsize;
	struct stat st;
	//dbg_time1();

	strcpy(path, wbfs_fat_drive);
	strcat(path, wbfs_fat_dir);
	dir = opendir(path);
	if (!dir) {
		*count = 0;
		return 0;
	}
	while ((dent = readdir(dir)) != NULL) {
		if (outbuf && cnt >= *count) break;
		if ((char)dent->d_name[0] == '.') continue;
		p = strrchr(dent->d_name, '.');
		if (!p) continue;
		if (strcasecmp(p, ".wbfs") != 0) continue;
		if (strlen(dent->d_name) != 11) continue; // GAMEID.wbfs
		u8 id[8];
		memcpy(id, dent->d_name, 6);
		id[6] = 0;

		strcpy(path, wbfs_fat_drive);
		strcat(path, wbfs_fat_dir);
		strcat(path, "/");
		strcat(path, dent->d_name);
		stat(path, &st);
		// size must be at least 1MB to be considered a valid wbfs file
		if (st.st_size < 1024*1024) continue;
		if (!outbuf) {
			// just counting
			cnt++;
			continue;
		}
		ptr = ((u8 *)outbuf) + (cnt * len);
		hdrsize = len;

		char *title = cfg_get_title(id);
		if (title) {
			memset(&tmpHdr, 0, sizeof(tmpHdr));
			memcpy(tmpHdr.id, id, 6);
			strncpy(tmpHdr.title, title, sizeof(tmpHdr.title));
			tmpHdr.magic = 0x5D1C9EA3;
			memcpy(ptr, &tmpHdr, hdrsize);
			cnt++;
			continue;
		}

		// no title found, read it from wbfs file directly
 		FILE *fp = fopen(path, "rb");
		if (fp != NULL) {
			fseek(fp, 512, SEEK_SET);
			fread(&tmpHdr, sizeof(struct discHdr), 1, fp);
			fclose(fp);

			if (tmpHdr.magic == 0x5D1C9EA3 && (memcmp(tmpHdr.id, id, 6) == 0)) {
				memcpy(ptr, &tmpHdr, hdrsize);
				cnt++;
				continue;
			}
		}

		// no title found, read it from wbfs file
		// but this is a little bit slower
		// open 'partition' file
		part = WBFS_FAT_OpenPart(id);
		if (!part) {
			continue;
		}

		/* Get header */
		ret = wbfs_get_disc_info(part, 0, ptr, hdrsize, &size);
		if (ret == 0) cnt++;
		WBFS_FAT_ClosePart(part);
	}
	*count = cnt;
	closedir(dir);
	//dbg_time2("\nFAT HDRS");
	//Wpad_WaitButtons();
	return 0;
}


s32 WBFS_FAT_GetCount(u32 *count)
{
	*count = 0;
	_WBFS_FAT_GetHeadersCount(NULL, count, 0);
	return 0;
}

s32 WBFS_FAT_GetHeaders(void *outbuf, u32 cnt, u32 len)
{
	_WBFS_FAT_GetHeadersCount(outbuf, &cnt, len);
	return 0;
}

wbfs_disc_t* WBFS_FAT_OpenDisc(u8 *discid)
{
	wbfs_t *part = WBFS_FAT_OpenPart(discid);
	if (!part) return NULL;
	return wbfs_open_disc(part, discid);
}

void WBFS_FAT_CloseDisc(wbfs_disc_t* disc)
{
	if (!disc) return;
	wbfs_t *part = disc->p;
	wbfs_close_disc(disc);
	WBFS_FAT_ClosePart(part);
	return;
}

s32 WBFS_FAT_DiskSpace(f32 *used, f32 *free)
{
	f32 size;
	int ret;

	*used = 0;
	*free = 0;
	// statvfs is slow, so cache values
	if (!wbfs_fat_vfs_have || wbfs_fat_vfs_lba != wbfs_part_lba) {
		ret = statvfs(wbfs_fat_drive, &wbfs_fat_vfs);

		if (ret) return 0;
		wbfs_fat_vfs_have = 1;
		wbfs_fat_vfs_lba = wbfs_part_lba;
	}

	/* FS size in GB */
	size = (f32)wbfs_fat_vfs.f_frsize * (f32)wbfs_fat_vfs.f_blocks / GB_SIZE;
	*free = (f32)wbfs_fat_vfs.f_frsize * (f32)wbfs_fat_vfs.f_bfree / GB_SIZE;
	*used = size - *free;


	return 0;
}

static int nop_read_sector(void *_fp,u32 lba,u32 count,void*buf)
{
	return 0;
}

static int nop_write_sector(void *_fp,u32 lba,u32 count,void*buf)
{
	return 0;
}

void WBFS_FAT_fname(u8 *id, char *fname, int len)
{
	snprintf(fname, len, "%s%s/%.6s.wbfs", wbfs_fat_drive, wbfs_fat_dir, id);
}

wbfs_t* WBFS_FAT_OpenPart(u8 *id)
{
	char fname[100];
	wbfs_t *part = NULL;
	int ret;

	// wbfs 'partition' file
	WBFS_FAT_fname(id, fname, sizeof(fname));
	ret = split_open(&split, fname);
	if (ret) return NULL;
	part = wbfs_open_partition(
			split_read_sector,
			split_write_sector, //readonly //split_write_sector,
			&split, fat_sector_size, split.total_sec, 0, 0);
	if (!part) {
		split_close(&split);
	}
	return part;
}

wbfs_t* WBFS_FAT_CreatePart(u8 *id)
{
	char fname[100];
	wbfs_t *part = NULL;
	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / 512;
	int ret;

	snprintf(D_S(fname), "%s%s", wbfs_fat_drive, wbfs_fat_dir);
	mkdir(fname, 0777);
	WBFS_FAT_fname(id, fname, sizeof(fname));
	ret = split_create(&split, fname, OPT_split_size, size, true);
	if (ret) return NULL;

	// force create first file
	u32 scnt = 0;
	int fd = split_get_file(&split, 0, &scnt, 0);
	if (fd<0) {
		split_close(&split);
		return NULL;
	}

	part = wbfs_open_partition(
			split_read_sector,
			split_write_sector,
			&split, fat_sector_size, n_sector, 0, 1);
	if (!part) {
		split_close(&split);
	}
	return part;
}

void WBFS_FAT_ClosePart(wbfs_t* part)
{
	if (!part) return;
	split_info_t *s = (split_info_t*)part->callback_data;
	wbfs_close(part);
	if (s) split_close(s);
}

s32 WBFS_FAT_RemoveGame(u8 *discid)
{
	char fname[100];
	// wbfs 'partition' file
	WBFS_FAT_fname(discid, fname, sizeof(fname));
	split_create(&split, fname, 0, 0, true);
	split_close(&split);

	// Reset FAT stats
	wbfs_fat_vfs_have = 0;
	return 0;
}

s32 WBFS_FAT_AddGame(void)
{
	static struct discHdr header ATTRIBUTE_ALIGN(32);
	s32 ret;
	wbfs_t *part = NULL;

	//write_test();	return -1;

	// read ID from DVD
	Disc_ReadHeader(&header);
	// create wbfs 'partition' file
	part = WBFS_FAT_CreatePart(header.id);
	if (!part) return -1;
	/* Add game to device */
	extern wbfs_t *hdd;
	wbfs_t *old_hdd = hdd;
	hdd = part; // used by spinner
	ret = wbfs_add_disc(part, __WBFS_ReadDVD, NULL, WBFS_Spinner, ONLY_GAME_PARTITION, 0);
	hdd = old_hdd;
	wbfs_trim(part);
	WBFS_FAT_ClosePart(part);

	// Reset FAT stats
	wbfs_fat_vfs_have = 0;
	if (ret < 0) return ret;

	return 0;
}

s32 WBFS_FAT_DVD_Size(u64 *comp_size, u64 *real_size)
{
	s32 ret;
	u32 comp_sec = 0, last_sec = 0;

	wbfs_t *part = NULL;
	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / fat_sector_size;
	u32 wii_sec_sz;

	// init a temporary dummy part
	// as a placeholder for wbfs_size_disc
	part = wbfs_open_partition(
			nop_read_sector, nop_write_sector,
			NULL, fat_sector_size, n_sector, 0, 1);
	if (!part) return -1;
	wii_sec_sz = part->wii_sec_sz;

	/* Add game to device */
	ret = wbfs_size_disc(part, __WBFS_ReadDVD, NULL, ONLY_GAME_PARTITION, &comp_sec, &last_sec);
	wbfs_close(part);
	if (ret < 0)
		return ret;

	if (comp_size != NULL) 	*comp_size = (u64)wii_sec_sz * comp_sec;
	if (real_size != NULL) *real_size = (u64)wii_sec_sz * last_sec;

	return 0;
}

s32 WBFS_FAT_RenameGame(u8 *discid, const void *newname)
{
	wbfs_t *part = WBFS_FAT_OpenPart(discid);
	if (!part)
        return -1;

    s32 ret = wbfs_ren_disc(part, discid,(u8*)newname);

	WBFS_FAT_ClosePart(part);

	return ret;
}

s32 WBFS_FAT_ReIDGame(u8 *discid, const void *newID)
{
	wbfs_t *part = WBFS_FAT_OpenPart(discid);
	if (!part)
        return -1;

    s32 ret = wbfs_rID_disc(part, discid,(u8*)newID);

	WBFS_FAT_ClosePart(part);

	if(ret == 0)
	{
	    char fname[100];
	    char fnamenew[100];
	    s32 cnt = 0x31;

        WBFS_FAT_fname(discid, fname, sizeof(fname));
        WBFS_FAT_fname((u8*) newID, fnamenew, sizeof(fnamenew));

        int stringlength = strlen(fname);

        while(rename(fname, fnamenew) == 0)
        {
            fname[stringlength] = cnt;
            fname[stringlength+1] = 0;
            fnamenew[stringlength] = cnt;
            fnamenew[stringlength+1] = 0;
            cnt++;
        }
	}

	return ret;
}

