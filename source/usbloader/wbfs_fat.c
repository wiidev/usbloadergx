// WBFS FAT by oggzee

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
#include "settings/cfg.h"
#include "wpad.h"
#include "wbfs_fat.h"

#define MAX_FAT_PATH 1024
#define D_S(A) A, sizeof(A)

char wbfs_fat_drive[16];
char wbfs_fat_dir[16] = "/wbfs";

int  wbfs_fat_vfs_have = 0;
int  wbfs_fat_vfs_lba = 0;
struct statvfs wbfs_fat_vfs;

split_info_t split;

static u32 fat_sector_size = 512;

static struct discHdr *fat_hdr_list = NULL;
static int fat_hdr_count = 0;

void WBFS_Spinner(s32 x, s32 max);
s32 __WBFS_ReadDVD(void *fp, u32 lba, u32 len, void *iobuf);

bool is_gameid(char *id)
{
	int i;
	for (i=0; i<6; i++) {
		if (!isalnum(id[i])) return false;
	}
	return true;
}

s32 _WBFS_FAT_GetHeadersCount()
{
	DIR_ITER *dir_iter;
	char path[MAX_FAT_PATH];
	char fname[MAX_FAT_PATH];
	char fpath[MAX_FAT_PATH];
	struct discHdr tmpHdr;
	struct stat st;
	wbfs_t *part = NULL;
	u8 id[8];
	int ret;
	char *p;
	u32 size;
	int is_dir;

	//dbg_time1();

	if(fat_hdr_list){
		free(fat_hdr_list);
		fat_hdr_list=NULL;
	}

	fat_hdr_count = 0;

	strcpy(path, wbfs_fat_drive);
	strcat(path, wbfs_fat_dir);
	dir_iter = diropen(path);
	if (!dir_iter) return 0;
	
	while (dirnext(dir_iter, fname, &st) == 0) {
		if ((char)fname[0] == '.') continue;
		if (strlen(fname) < 8) continue; // "GAMEID_x"

		memcpy(id, fname, 6);
		id[6] = 0;

		is_dir = S_ISDIR(st.st_mode);
		if (is_dir) {
			// usb:/wbfs/GAMEID_TITLE/GAMEID.wbfs
			if (fname[6] != '_') continue;
			snprintf(fpath, sizeof(fpath), "%s/%s/%s.wbfs", path, fname, id);
			// if more than 50 games, skip second stat to improve speed
			if (fat_hdr_count < 50) {
				do_stat2:
				if (stat(fpath, &st) == -1) continue;
			} else {
				// just check if gameid is valid (alphanum)
				if (!is_gameid((char*)id)) goto do_stat2;
				st.st_size = 1024*1024;
			}
		} else {
			// usb:/wbfs/GAMEID.wbfs
			p = strrchr(fname, '.');
			if (!p) continue;
			if (strcasecmp(p, ".wbfs") != 0) continue;
			if (strlen(fname) != 11) continue; // GAMEID.wbfs
		}

		// size must be at least 1MB to be considered a valid wbfs file
		if (st.st_size < 1024*1024) continue;
		// if we have titles.txt entry use that
		char *title = cfg_get_title(id);
		// if directory, and no titles.txt get title from dir name
		if (!title && is_dir) {
			title = &fname[7];
		}
		if (title) {
			memset(&tmpHdr, 0, sizeof(tmpHdr));
			memcpy(tmpHdr.id, id, 6);
			strncpy(tmpHdr.title, title, sizeof(tmpHdr.title)-1);
			tmpHdr.magic = 0x5D1C9EA3;
			goto add_hdr;
		}

		// else read it from wbfs file directly
 		FILE *fp = fopen(fpath, "rb");
		if (fp != NULL) {
			fseek(fp, 512, SEEK_SET);
			fread(&tmpHdr, sizeof(struct discHdr), 1, fp);
			fclose(fp);
			if ((tmpHdr.magic == 0x5D1C9EA3) && (memcmp(tmpHdr.id, id, 6) == 0)) {
				goto add_hdr;
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
		ret = wbfs_get_disc_info(part, 0, (u8*)&tmpHdr, sizeof(struct discHdr), &size);
		WBFS_FAT_ClosePart(part);
		if (ret) continue;
		// add tmpHdr to list:
		add_hdr:
		fat_hdr_count++;
		fat_hdr_list = realloc(fat_hdr_list, fat_hdr_count * sizeof(struct discHdr));
		memcpy(&fat_hdr_list[fat_hdr_count-1], &tmpHdr, sizeof(struct discHdr));
	}
	dirclose(dir_iter);
	return 0;
}


s32 WBFS_FAT_GetCount(u32 *count)
{
	*count = 0;
	_WBFS_FAT_GetHeadersCount();
	if (fat_hdr_count && fat_hdr_list) {
		// for compacter mem - move up as it will be freed later
		int size = fat_hdr_count * sizeof(struct discHdr);
		struct discHdr *buf = malloc(size);
		if (buf) {
			memcpy(buf, fat_hdr_list, size);
			if (fat_hdr_list) {
				free(fat_hdr_list);
			}
			fat_hdr_list = buf;
		}
	}
	*count = fat_hdr_count;
	return 0;
}

s32 WBFS_FAT_GetHeaders(void *outbuf, u32 cnt, u32 len)
{
	int i;
	int slen = len;
	if (slen > sizeof(struct discHdr)) {
		slen = sizeof(struct discHdr);
	}
	for (i=0; i<cnt && i<fat_hdr_count; i++) {
		memcpy(outbuf + i * len, &fat_hdr_list[i], slen);
	}
	if (fat_hdr_list) {
		free(fat_hdr_list);
		fat_hdr_list = NULL;
	}
	fat_hdr_count = 0;
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

void WBFS_FAT_fname(u8 *id, char *fname, int len, char *path)
{
	if (path == NULL) {
		snprintf(fname, len, "%s%s/%.6s.wbfs", wbfs_fat_drive, wbfs_fat_dir, id);
	} else {
		snprintf(fname, len, "%s/%.6s.wbfs", path, id);
	}
}

void mk_gameid_title(struct discHdr *header, char *name, int re_space)
{
	int i, len;

	memcpy(name, header->id, 6);
	name[6] = 0;
	strcat(name, "_");
	strcat(name, get_title(header));

	// replace silly chars with '_'
	len = strlen(name);
	for (i = 0; i < len; i++) {
		if(strchr("\\/:<>|\"", name[i]) || iscntrl(name[i])) {
			name[i] = '_';
		}
		if(re_space && name[i]==' ') {
			name[i] = '_';
		}
	}
}

void WBFS_FAT_get_dir(struct discHdr *header, char *path)
{
	strcpy(path, wbfs_fat_drive);
	strcat(path, wbfs_fat_dir);
	if (Settings.FatInstallToDir) {
		strcat(path, "/");
		mk_gameid_title(header, path + strlen(path), 0);
	}
}

int WBFS_FAT_find_fname(u8 *id, char *fname, int len)
{
	struct stat st;
	WBFS_FAT_fname(id, fname, len, NULL);
	if (stat(fname, &st) == 0) return 1;
	// direct file not found, check subdirs
	DIR_ITER *dir_iter;
	char path[MAX_FAT_PATH];
	char name[MAX_FAT_PATH];
	strcpy(path, wbfs_fat_drive);
	strcat(path, wbfs_fat_dir);
	dir_iter = diropen(path);
	if (!dir_iter) {
		goto err;
	}
	while (dirnext(dir_iter, name, &st) == 0) {
		if (name[0] == '.') continue;
		if (name[6] != '_') continue;
		if (strncmp(name, (char*)id, 6) != 0) continue;
		if (strlen(name) < 8) continue;
		snprintf(fname, len, "%s/%s/%.6s.wbfs",	path, name, id);
		if (stat(fname, &st) == 0) {
			dirclose(dir_iter);
			return 2;
		}
	}
	dirclose(dir_iter);
	// not found
	err:
	*fname = 0;
	return 0;
}

wbfs_t* WBFS_FAT_OpenPart(u8 *id)
{
	char fname[MAX_FAT_PATH];
	wbfs_t *part = NULL;
	int ret;

	// wbfs 'partition' file
	if ( !WBFS_FAT_find_fname(id, fname, sizeof(fname)) ) return NULL;
	ret = split_open(&split, fname);
	if (ret) return NULL;
	part = wbfs_open_partition(
			split_read_sector,
			nop_write_sector, //readonly //split_write_sector,
			&split, fat_sector_size, split.total_sec, 0, 0);
	if (!part) {
		split_close(&split);
	}
	return part;
}

wbfs_t* WBFS_FAT_CreatePart(u8 *id, char *path)
{
	char fname[MAX_FAT_PATH];
	wbfs_t *part = NULL;
	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / 512;
	int ret;

	snprintf(D_S(fname), "%s%s", wbfs_fat_drive, wbfs_fat_dir);
	mkdir(fname, 0777); // base usb:/wbfs
	mkdir(path, 0777); // game subdir
	WBFS_FAT_fname(id, fname, sizeof(fname), path);
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
	char fname[MAX_FAT_PATH];
	int loc;
	// wbfs 'partition' file
	loc = WBFS_FAT_find_fname(discid, fname, sizeof(fname));
	if ( !loc ) return -1;
	split_create(&split, fname, 0, 0, true);
	split_close(&split);
	if (loc == 1) return 0;

	// game is in subdir
	// remove optional .txt file
	DIR_ITER *dir_iter;
	struct stat st;
	char path[MAX_FAT_PATH];
	char name[MAX_FAT_PATH];
	strncpy(path, fname, sizeof(path));
	char *p = strrchr(path, '/');
	if (p) *p = 0;
	dir_iter = diropen(path);
	if (!dir_iter) return 0;
	while (dirnext(dir_iter, name, &st) == 0) {
		if (name[0] == '.') continue;
		if (name[6] != '_') continue;
		if (strncmp(name, (char*)discid, 6) != 0) continue;
		p = strrchr(name, '.');
		if (!p) continue;
		if (strcasecmp(p, ".txt") != 0) continue;
		snprintf(fname, sizeof(fname), "%s/%s", path, name);
		remove(fname);
		break;
	}
	dirclose(dir_iter);
	// remove game subdir
	//rmdir(path);
	if (unlink(path) == -1) {
		return -1;
	}

	return 0;
}


s32 WBFS_FAT_AddGame(void)
{
	static struct discHdr header ATTRIBUTE_ALIGN(32);
	char path[MAX_FAT_PATH];
	wbfs_t *part = NULL;
	s32 ret;

	// read ID from DVD
	Disc_ReadHeader(&header);
	// path
	WBFS_FAT_get_dir(&header, path);
	// create wbfs 'partition' file
	part = WBFS_FAT_CreatePart(header.id, path);
	if (!part) return -1;
	/* Add game to device */
	partition_selector_t part_sel;
	int copy_1_1 = 0;
	
	if (Settings.fullcopy) {
		part_sel = ALL_PARTITIONS;
		copy_1_1 = 1;
	} else {
		part_sel = Settings.partitions_to_install == install_game_only ? ONLY_GAME_PARTITION : ALL_PARTITIONS;
	}

	extern wbfs_t *hdd;
	wbfs_t *old_hdd = hdd;
	hdd = part; // used by spinner
	ret = wbfs_add_disc(part, __WBFS_ReadDVD, NULL, WBFS_Spinner, part_sel, copy_1_1);
	hdd = old_hdd;
	wbfs_trim(part);
	WBFS_FAT_ClosePart(part);
	if (ret) {
		return -1;
	}
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
	partition_selector_t part_sel;
	if (Settings.fullcopy) {
		part_sel = ALL_PARTITIONS;
	} else {
		part_sel = Settings.partitions_to_install == install_game_only ? ONLY_GAME_PARTITION : ALL_PARTITIONS;
	}

	ret = wbfs_size_disc(part, __WBFS_ReadDVD, NULL, part_sel, &comp_sec, &last_sec);
	wbfs_close(part);
	if (ret < 0)
		return ret;

	*comp_size = (u64)wii_sec_sz * comp_sec;
	*real_size = (u64)wii_sec_sz * last_sec;

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

        WBFS_FAT_fname(discid, fname, sizeof(fname), NULL);
        WBFS_FAT_fname((u8*) newID, fnamenew, sizeof(fnamenew), NULL);

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
