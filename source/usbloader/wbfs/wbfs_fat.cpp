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

#include "settings/cfg.h"
#include "usbloader/disc.h"
#include "fatmounter.h"
#include "wbfs_fat.h"

#include "gecko.h"

#define MAX_FAT_PATH 1024
#define TITLE_LEN 64

using namespace std;

char Wbfs_Fat::wbfs_fs_drive[16];
char Wbfs_Fat::wbfs_fat_dir[16] = "/wbfs";
char Wbfs_Fat::invalid_path[] = "/\\:|<>?*\"'";
struct discHdr *Wbfs_Fat::fat_hdr_list = NULL;
u32 Wbfs_Fat::fat_hdr_count = 0;

extern "C"
{
	int _FAT_get_fragments (const char *path, _frag_append_t append_fragment, void *callback_data);
	extern FragList *frag_list;
}

u32 Wbfs_Fat::fat_sector_size = 512;

Wbfs_Fat::Wbfs_Fat(u32 device, u32 lba, u32 size) : Wbfs(device, lba, size)
{
}

s32 Wbfs_Fat::Open()
{
	if (device == WBFS_DEVICE_USB && lba == fat_usb_sec) {
		strcpy(wbfs_fs_drive, "USB:");
	} else if (device == WBFS_DEVICE_SDHC && lba == fat_sd_sec) {
		strcpy(wbfs_fs_drive, "SD:");
	} else {
		if (WBFSDevice_Init(lba)) return -1;
		strcpy(wbfs_fs_drive, "WBFS:");
	}
	
	return 0;
}

wbfs_disc_t* Wbfs_Fat::OpenDisc(u8 *discid)
{
	char fname[MAX_FAT_PATH];

	// wbfs 'partition' file
	if (!FindFilename(discid, fname, sizeof(fname)) ) return NULL;

	if (strcasecmp(strrchr(fname,'.'), ".iso") == 0) {
		// .iso file
		// create a fake wbfs_disc
		int fd;
		fd = open(fname, O_RDONLY);
		if (fd == -1) return NULL;
		wbfs_disc_t *iso_file = (wbfs_disc_t *)calloc(sizeof(wbfs_disc_t),1);
		if (iso_file == NULL) return NULL;
		// mark with a special wbfs_part
		wbfs_iso_file.wbfs_sec_sz = 512;
		iso_file->p = &wbfs_iso_file;
		iso_file->header = (wbfs_disc_info_t*)fd;
		return iso_file;
	}

	wbfs_t *part = OpenPart(fname);
	if (!part) return NULL;
	return wbfs_open_disc(part, discid);
}

void Wbfs_Fat::CloseDisc(wbfs_disc_t* disc)
{
	if (!disc) return;
	wbfs_t *part = disc->p;

	// is this really a .iso file?
	if (part == &wbfs_iso_file) {
		close((int)disc->header);
		free(disc);
		return;
	}

	wbfs_close_disc(disc);
	ClosePart(part);
	return;
}

s32 Wbfs_Fat::GetCount(u32 *count)
{
	*count = 0;
	GetHeadersCount();
	if (fat_hdr_count && fat_hdr_list) {
		// for compacter mem - move up as it will be freed later
		int size = fat_hdr_count * sizeof(struct discHdr);
		struct discHdr *buf = (struct discHdr *) malloc(size);
		if (buf) {
			memcpy(buf, fat_hdr_list, size);
			SAFE_FREE(fat_hdr_list);
			fat_hdr_list = buf;
		}
	}
	*count = fat_hdr_count;
	return 0;
}

s32 Wbfs_Fat::GetHeaders(struct discHdr *outbuf, u32 cnt, u32 len)
{
	u32 i;
	if (len > sizeof(struct discHdr)) {
		len = sizeof(struct discHdr);
	}
#ifdef DEBUG_WBFS
	gprintf("\n\tGetHeaders");
#endif
	for (i=0; i<cnt && i<fat_hdr_count; i++) {
		memcpy(&outbuf[i], &fat_hdr_list[i], len);
	}
	SAFE_FREE(fat_hdr_list);
	fat_hdr_count = 0;
#ifdef DEBUG_WBFS
	gprintf("...ok");
#endif
	return 0;
}

s32 Wbfs_Fat::AddGame(void)
{
	static struct discHdr header ATTRIBUTE_ALIGN(32);
	char path[MAX_FAT_PATH];
	wbfs_t *part = NULL;
	s32 ret;

	// read ID from DVD
	Disc_ReadHeader(&header);
	// path
	GetDir(&header, path);
	// create wbfs 'partition' file
	part = CreatePart(header.id, path);
	if (!part) return -1;
	/* Add game to device */
	partition_selector_t part_sel = ALL_PARTITIONS;
	int copy_1_1 = Settings.fullcopy;
	switch (Settings.partitions_to_install) {
		case install_game_only:
			part_sel = ONLY_GAME_PARTITION;
			break;
		case install_all:
			part_sel = ALL_PARTITIONS;
			break;
		case install_all_but_update:
			part_sel = REMOVE_UPDATE_PARTITION;
			break;
	}
	if (copy_1_1) {
		part_sel = ALL_PARTITIONS;
	}
	wbfs_t *old_hdd = hdd;
	hdd = part; // used by spinner
	ret = wbfs_add_disc(part, __ReadDVD, NULL, Spinner, part_sel, copy_1_1);
	hdd = old_hdd;
	wbfs_trim(part);
	ClosePart(part);

	if (ret < 0) return ret;
	mk_title_txt(&header, path);

	return 0;
}

s32 Wbfs_Fat::RemoveGame(u8 *discid)
{
	char fname[MAX_FAT_PATH];
	int loc;
	// wbfs 'partition' file
	loc = FindFilename(discid, fname, sizeof(fname));
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
	unlink(path);

	return 0;
}

s32 Wbfs_Fat::DiskSpace(f32 *used, f32 *free)
{
	f32 size;
	int ret;
	struct statvfs wbfs_fat_vfs;

	*used = 0;
	*free = 0;
	ret = statvfs(wbfs_fs_drive, &wbfs_fat_vfs);
	if (ret) return -1;

	/* FS size in GB */
	size = (f32)wbfs_fat_vfs.f_frsize * (f32)wbfs_fat_vfs.f_blocks / GB_SIZE;
	*free = (f32)wbfs_fat_vfs.f_frsize * (f32)wbfs_fat_vfs.f_bfree / GB_SIZE;
	*used = size - *free;

	return 0;
}

s32 Wbfs_Fat::RenameGame(u8 *discid, const void *newname)
{
	wbfs_t *part = OpenPart((char *) discid);
	if (!part)
	return -1;

    s32 ret = wbfs_ren_disc(part, discid,(u8*)newname);

	ClosePart(part);

	return ret;
}

s32 Wbfs_Fat::ReIDGame(u8 *discid, const void *newID)
{
	wbfs_t *part = OpenPart((char *) discid);
	if (!part)
	return -1;
	
    s32 ret = wbfs_rID_disc(part, discid,(u8*)newID);

	ClosePart(part);

	if(ret == 0)
	{
	    char fname[100];
	    char fnamenew[100];
	    s32 cnt = 0x31;

		Filename(discid, fname, sizeof(fname), NULL);
		Filename((u8*) newID, fnamenew, sizeof(fnamenew), NULL);

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

f32 Wbfs_Fat::EstimateGameSize() 
{
	wbfs_t *part = NULL;
	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / fat_sector_size;
	u32 wii_sec_sz;

	// init a temporary dummy part
	// as a placeholder for wbfs_size_disc
	part = wbfs_open_partition(
			nop_rw_sector, nop_rw_sector,
			NULL, fat_sector_size, n_sector, 0, 1);
	if (!part) return -1;
	wii_sec_sz = part->wii_sec_sz;

	partition_selector_t part_sel;
	if (Settings.fullcopy) {
		part_sel = ALL_PARTITIONS;
	} else {
		part_sel = Settings.partitions_to_install == install_game_only ? ONLY_GAME_PARTITION : ALL_PARTITIONS;
	}
    return wbfs_estimate_disc(part, __ReadDVD, NULL, part_sel);
}

// TITLE [GAMEID]
bool Wbfs_Fat::CheckLayoutB(char *fname, int len, u8* id, char *fname_title)
{
	if (len <= 8) return false;
	if (fname[len-8] != '[' || fname[len-1] != ']') return false;
	if (!is_gameid(&fname[len-7])) return false;
	strncpy(fname_title, fname, TITLE_LEN);
	// cut at '['
	fname_title[len-8] = 0;
	int n = strlen(fname_title);
	if (n == 0) return false; 
	// cut trailing _ or ' '
	if (fname_title[n - 1] == ' ' || fname_title[n - 1] == '_' ) {
		fname_title[n - 1] = 0;
	}
	if (strlen(fname_title) == 0) return false;
	if (id) {
		memcpy(id, &fname[len-7], 6);
		id[6] = 0;
	}
	return true;
}

s32 Wbfs_Fat::GetHeadersCount()
{
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
	int len;
	char dir_title[65];
	char fname_title[TITLE_LEN];
	char *title;
	DIR_ITER *dir_iter;

	//dbg_time1();

	SAFE_FREE(fat_hdr_list);
	fat_hdr_count = 0;

	strcpy(path, wbfs_fs_drive);
	strcat(path, wbfs_fat_dir);

	dir_iter = diropen(path);
	if (!dir_iter) return 0;

	dir_iter = diropen(path);
	if (!dir_iter) return 0;

	while (dirnext(dir_iter, fname, &st) == 0) {
		//printf("found: %s\n", fname); Wpad_WaitButtonsCommon();
		if ((char)fname[0] == '.') continue;
		len = strlen(fname);
		if (len < 8) continue; // "GAMEID_x"

		memcpy(id, fname, 6);
		id[6] = 0;
		*fname_title = 0;		

		is_dir = S_ISDIR(st.st_mode);
		//printf("mode: %d %d %x\n", is_dir, st.st_mode, st.st_mode);
		if (is_dir) {
			int lay_a = 0;
			int lay_b = 0;
			if (fname[6] == '_' && is_gameid((char*)id)) {
				// usb:/wbfs/GAMEID_TITLE/GAMEID.wbfs
				lay_a = 1;
			}
			if (CheckLayoutB(fname, len, NULL, fname_title)) {
				// usb:/wbfs/TITLE[GAMEID]/GAMEID.wbfs
				lay_b = 1;
			}
			if (!lay_a && !lay_b) continue;
			if (lay_a) {
				strncpy(dir_title, &fname[7], sizeof(dir_title));
			} else {
				try_lay_b:
				if (!CheckLayoutB(fname, len, id, fname_title)) continue;
			}
			snprintf(fpath, sizeof(fpath), "%s/%s/%s.wbfs", path, fname, id);
			//printf("path2: %s\n", fpath);
			// if more than 50 games, skip second stat to improve speed
			// but if ambiguous layout check anyway
			if (fat_hdr_count < 50 || (lay_a && lay_b)) {
				if (stat(fpath, &st) == -1) {
					//printf("missing: %s\n", fpath);
					// try .iso
					strcpy(strrchr(fpath, '.'), ".iso"); // replace .wbfs with .iso
					if (stat(fpath, &st) == -1) {
						//printf("missing: %s\n", fpath);
						if (lay_a && lay_b == 1) {
							// mark lay_b so that the stat check is still done,
							// but lay_b is not re-tried again
							lay_b = 2;
							// retry with layout b
							goto try_lay_b;
						}
						continue;
					}
				}
			} else {
				st.st_size = 1024*1024;
			}
		} else {
			// usb:/wbfs/GAMEID.wbfs
			// or usb:/wbfs/GAMEID.iso
			p = strrchr(fname, '.');
			if (!p) continue;
			if ( (strcasecmp(p, ".wbfs") != 0)
				&& (strcasecmp(p, ".iso") != 0) ) continue;
			int n = p - fname; // length withouth .wbfs
			if (n != 6) {
				// TITLE [GAMEID].wbfs
				if (!CheckLayoutB(fname, n, id, fname_title)) continue;
			}
			snprintf(fpath, sizeof(fpath), "%s/%s", path, fname);
		}

		//printf("found: %s %d MB\n", fpath, (int)(st.st_size/1024/1024));
		// size must be at least 1MB to be considered a valid wbfs file
		if (st.st_size < 1024*1024) continue;
		// if we have titles.txt entry use that
		title = cfg_get_title(id);
		// if no titles.txt get title from dir or file name
		if (!title && *fname_title) {
			title = fname_title;
		}
		if (title) {
			memset(&tmpHdr, 0, sizeof(tmpHdr));
			memcpy(tmpHdr.id, id, 6);
			strncpy(tmpHdr.title, title, sizeof(tmpHdr.title)-1);
			tmpHdr.magic = 0x5D1C9EA3;
			goto add_hdr;
		}

		// else read it from file directly
		if (strcasecmp(strrchr(fpath,'.'), ".wbfs") == 0) {
			// wbfs file directly
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
			part = OpenPart(fpath);
			if (!part) {
				continue;
			}
			// Get header 
			ret = wbfs_get_disc_info(part, 0, (u8*)&tmpHdr,
					sizeof(struct discHdr), &size);
			ClosePart(part);
			if (ret == 0) {
				goto add_hdr;
			}
		} else if (strcasecmp(strrchr(fpath,'.'), ".iso") == 0) {
			// iso file
			FILE *fp = fopen(fpath, "rb");
			if (fp != NULL) {
				fseek(fp, 0, SEEK_SET);
				fread(&tmpHdr, sizeof(struct discHdr), 1, fp);
				fclose(fp);
				if ((tmpHdr.magic == 0x5D1C9EA3) && (memcmp(tmpHdr.id, id, 6) == 0)) {
					goto add_hdr;
				}
			}
		}
		// fail:
		continue;

		// succes: add tmpHdr to list:
		add_hdr:
		memset(&st, 0, sizeof(st));
		//printf("added: %.6s %.20s\n", tmpHdr.id, tmpHdr.title); Wpad_WaitButtons();
		fat_hdr_count++;
		fat_hdr_list = (struct discHdr *) realloc(fat_hdr_list, fat_hdr_count * sizeof(struct discHdr));
		memcpy(&fat_hdr_list[fat_hdr_count-1], &tmpHdr, sizeof(struct discHdr));
	}
	dirclose(dir_iter);
	//dbg_time2("\nFAT_GetCount"); Wpad_WaitButtonsCommon();
	
	return 0;
}

int Wbfs_Fat::FindFilename(u8 *id, char *fname, int len)
{
	struct stat st;
	// look for direct .wbfs file
	Filename(id, fname, len, NULL);
	if (stat(fname, &st) == 0) return 1;
	// look for direct .iso file
	strcpy(strrchr(fname, '.'), ".iso"); // replace .wbfs with .iso
	if (stat(fname, &st) == 0) return 1;
	// direct file not found, check subdirs
	*fname = 0;
	DIR_ITER *dir_iter;
	char path[MAX_FAT_PATH];
	char name[MAX_FAT_PATH];
	strcpy(path, wbfs_fs_drive);
	strcat(path, wbfs_fat_dir);
	dir_iter = diropen(path);
	//printf("dir: %s %p\n", path, dir); Wpad_WaitButtons();
	if (!dir_iter) {
		return 0;
	}
	while (dirnext(dir_iter, name, &st) == 0) {
		//dbg_printf("name:%s\n", name);
		if (name[0] == '.') continue;
		int n = strlen(name);
		if (n < 8) continue;
		if (S_ISDIR(st.st_mode)) {
			if (name[6] == '_') {
				// GAMEID_TITLE
				if (strncmp(name, (char*)id, 6) != 0) goto try_alter;
			} else {
				try_alter:
				// TITLE [GAMEID]
				if (name[n-8] != '[' || name[n-1] != ']') continue;
				if (strncmp(&name[n-7], (char*)id, 6) != 0) continue;
			}
			// look for .wbfs file
			snprintf(fname, len, "%s/%s/%.6s.wbfs", path, name, id);
			if (stat(fname, &st) == 0) break;
			// look for .iso file
			snprintf(fname, len, "%s/%s/%.6s.iso", path, name, id);
		} else {
			// TITLE [GAMEID].wbfs
			char fn_title[TITLE_LEN];
			u8 fn_id[8];
			char *p = strrchr(name, '.');
			if (!p) continue;
			if ( (strcasecmp(p, ".wbfs") != 0)
				&& (strcasecmp(p, ".iso") != 0) ) continue;
			int n = p - name; // length withouth .wbfs
			if (!CheckLayoutB(name, n, fn_id, fn_title)) continue;
			if (strncmp((char*)fn_id, (char*)id, 6) != 0) continue;
			snprintf(fname, len, "%s/%s", path, name);
		}
		if (stat(fname, &st) == 0) break;
		*fname = 0;
	}
	dirclose(dir_iter);
	if (*fname) {
		// found
		//printf("found:%s\n", fname);
		return 2;
	}
	// not found
	return 0;
}

wbfs_t* Wbfs_Fat::OpenPart(char *fname)
{
	wbfs_t *part = NULL;
	int ret;

	// wbfs 'partition' file
	ret = split_open(&split, fname);
	if (ret) return NULL;
	part = wbfs_open_partition(
			split_read_sector,
			nop_rw_sector, //readonly //split_write_sector,
			&split, fat_sector_size, split.total_sec, 0, 0);
	if (!part) {
		split_close(&split);
	}
	return part;
}

void Wbfs_Fat::ClosePart(wbfs_t* part)
{
	if (!part) return;
	split_info_t *s = (split_info_t*)part->callback_data;
	wbfs_close(part);
	if (s) split_close(s);
}

void Wbfs_Fat::Filename(u8 *id, char *fname, int len, char *path)
{
	if (path == NULL) {
		snprintf(fname, len, "%s%s/%.6s.wbfs", wbfs_fs_drive, wbfs_fat_dir, id);
	} else {
		snprintf(fname, len, "%s/%.6s.wbfs", path, id);
	}
}

void Wbfs_Fat::GetDir(struct discHdr *header, char *path)
{
	strcpy(path, wbfs_fs_drive);
	strcat(path, wbfs_fat_dir);
	if (Settings.FatInstallToDir) {
		strcat(path, "/");
		int layout = 0;
		if (Settings.FatInstallToDir == 2) layout = 1;
		mk_gameid_title(header, path + strlen(path), 0, layout);
	}
}

wbfs_t* Wbfs_Fat::CreatePart(u8 *id, char *path)
{
	char fname[MAX_FAT_PATH];
	wbfs_t *part = NULL;
	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / 512;
	int ret;

	//printf("CREATE PART %s %lld %d\n", id, size, n_sector);
	snprintf(fname, sizeof(fname), "%s%s", wbfs_fs_drive, wbfs_fat_dir);
	mkdir(fname, 0777); // base usb:/wbfs
	mkdir(path, 0777); // game subdir
	Filename(id, fname, sizeof(fname), path);
	printf("Writing to %s\n", fname);
	ret = split_create(&split, fname, OPT_split_size, size, true);
	if (ret) return NULL;

	// force create first file
	u32 scnt = 0;
	int fd = split_get_file(&split, 0, &scnt, 0);
	if (fd<0) {
		printf("ERROR creating file\n");
		sleep(2);
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

void Wbfs_Fat::mk_title_txt(struct discHdr *header, char *path)
{
	char fname[MAX_FAT_PATH];
	FILE *f;

	strcpy(fname, path);
	strcat(fname, "/");
	mk_gameid_title(header, fname+strlen(fname), 1, 0);
	strcat(fname, ".txt");

	f = fopen(fname, "wb");
	if (!f) return;
	fprintf(f, "%.6s = %.64s\n", header->id, get_title(header));
	fclose(f);
	printf("Info file: %s\n", fname);
}

void Wbfs_Fat::mk_gameid_title(struct discHdr *header, char *name, int re_space, int layout)
{
	int i, len;
	char title[65];
	char id[8];

	memcpy(name, header->id, 6);
	name[6] = 0;
	strncpy(title, get_title(header), sizeof(title));
	title_filename(title);

	if (layout == 0) {
		sprintf(name, "%s_%s", id, title);
	} else {
		sprintf(name, "%s [%s]", title, id);
	}

	// replace space with '_'
	if (re_space) {
		len = strlen(name);
		for (i = 0; i < len; i++) {
			if(name[i]==' ') name[i] = '_';
		}
	}
}

void Wbfs_Fat::title_filename(char *title)
{
    int i, len;
    // trim leading space
	len = strlen(title);
	while (*title == ' ') {
		memmove(title, title+1, len);
		len--;
	}
    // trim trailing space - not allowed on windows directories
    while (len && title[len-1] == ' ') {
		title[len-1] = 0;
		len--;
    }
    // replace silly chars with '_'
    for (i=0; i<len; i++) {
		if(strchr(invalid_path, title[i]) || iscntrl((int) title[i])) {
			title[i] = '_';
		}
    }
}

bool Wbfs_Fat::is_gameid(char *id)
{
	int i;
	for (i=0; i<6; i++) {
		if (!isalnum((u32) id[i])) return false;
	}
	return true;
}

int Wbfs_Fat::GetFragList(u8 *id)
{
	char fname[1024];
	char fname1[1024];
	struct stat st;
	FragList *fs = NULL;
	FragList *fa = NULL;
	FragList *fw = NULL;
	int ret;
	int i;
	int is_wbfs = 0;
	int ret_val = -1;
	
	ret = FindFilename(id, fname, sizeof(fname));
	if (!ret) return -1;
	
	if (strcasecmp(strrchr(fname,'.'), ".wbfs") == 0) {
		is_wbfs = 1;
	}

	fs = (FragList *) malloc(sizeof(FragList));
	fa = (FragList *) malloc(sizeof(FragList));
	fw = (FragList *) malloc(sizeof(FragList));

	frag_init(fa, MAX_FRAG);

	for (i=0; i<10; i++) {
		frag_init(fs, MAX_FRAG);
		if (i > 0) {
			fname[strlen(fname)-1] = '0' + i;
			if (stat(fname, &st) == -1) break;
		}
		strcpy(fname1, fname);
		if ((ret = GetFragList((char *) &fname, &_frag_append, fs)))
		{
			ret_val = ret;
			goto out;
		}
		frag_concat(fa, fs);
	}
	frag_list = (FragList *) malloc(sizeof(FragList));
	frag_init(frag_list, MAX_FRAG);
	if (is_wbfs) {
		// if wbfs file format, remap.
		//printf("=====\n");
		wbfs_disc_t *d = OpenDisc(id);
		if (!d) goto out;
		frag_init(fw, MAX_FRAG);
		ret = wbfs_get_fragments(d, &_frag_append, fw);
		if (ret) goto out;
		CloseDisc(d);
		// DEBUG: frag_list->num = MAX_FRAG-10; // stress test
		ret = frag_remap(frag_list, fw, fa);
		if (ret) goto out;
	} else {
		// .iso does not need remap just copy
		//printf("fa:\n");
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

int Wbfs_Fat::GetFragList(char *filename, _frag_append_t append_fragment, FragList *fs)
{
	return _FAT_get_fragments(filename, append_fragment, fs);	
}

bool Wbfs_Fat::ShowFreeSpace(void)
{
	return false;
}
