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
#include <fat.h>

#include "Controls/DeviceHandler.hpp"
#include "FileOperations/fileops.h"
#include "settings/CSettings.h"
#include "settings/GameTitles.h"
#include "usbloader/disc.h"
#include "usbloader/usbstorage2.h"
#include "language/gettext.h"
#include "libs/libfat/fatfile_frag.h"
#include "utils/ShowError.h"
#include "wbfs_fat.h"
#include "prompts/ProgressWindow.h"
#include "usbloader/wbfs.h"
#include "usbloader/GameList.h"
#include "utils/tools.h"
#include "wbfs_rw.h"

#include "gecko.h"

#define MAX_FAT_PATH 1024
#define TITLE_LEN 64

using namespace std;

static const char wbfs_fat_dir[] = "/wbfs";
static const char invalid_path[] = "/\\:|<>?*\"'";
extern u32 hdd_sector_size[2];
extern int install_abort_signal;

inline bool isGameID(const char *id)
{
	for (int i = 0; i < 6; i++)
		if (!isalnum((int) id[i]))
			return false;

	return true;
}

Wbfs_Fat::Wbfs_Fat(u32 lba, u32 size, u32 part, u32 port) :
	Wbfs(lba, size, part, port), fat_hdr_list(NULL), fat_hdr_count(0)
{
	memset(wbfs_fs_drive, 0, sizeof(wbfs_fs_drive));
}

s32 Wbfs_Fat::Open()
{
	Close();

	if(partition < (u32) DeviceHandler::GetUSBPartitionCount())
	{
		PartitionHandle *usbHandle = DeviceHandler::Instance()->GetUSBHandleFromPartition(partition);
		int portPart = DeviceHandler::PartitionToPortPartition(partition);
		if (lba == usbHandle->GetLBAStart(portPart))
		{
			snprintf(wbfs_fs_drive, sizeof(wbfs_fs_drive), "%s:", usbHandle->MountName(portPart));
			return 0;
		}
	}

	return -1;
}

void Wbfs_Fat::Close()
{
	if (hdd)
	{
		wbfs_close(hdd);
		hdd = NULL;
	}

	memset(wbfs_fs_drive, 0, sizeof(wbfs_fs_drive));
}

wbfs_disc_t* Wbfs_Fat::OpenDisc(u8 *discid)
{
	char fname[MAX_FAT_PATH];

	// wbfs 'partition' file
	if (!FindFilename(discid, fname, sizeof(fname))) return NULL;

	if (strcasecmp(strrchr(fname, '.'), ".iso") == 0)
	{
		// .iso file
		// create a fake wbfs_disc
		int fd;
		fd = open(fname, O_RDONLY);
		if (fd == -1) return NULL;
		wbfs_disc_t *iso_file = (wbfs_disc_t *) calloc(sizeof(wbfs_disc_t), 1);
		if (iso_file == NULL) return NULL;
		// mark with a special wbfs_part
		wbfs_iso_file.wbfs_sec_sz = hdd_sector_size[usbport];
		iso_file->p = &wbfs_iso_file;
		iso_file->header = (wbfs_disc_info_t*) malloc(sizeof(wbfs_disc_info_t));
		if(!iso_file->header)
		{
			free(iso_file);
			return NULL;
		}
		read(fd, iso_file->header, sizeof(wbfs_disc_info_t));
		iso_file->i = fd;
		return iso_file;
	}

	wbfs_t *part = OpenPart(fname);
	if (!part) return NULL;

	wbfs_disc_t *disc = wbfs_open_disc(part, discid);
	if(!disc)
	{
		ClosePart(part);
		return NULL;
	}

	return disc;
}

void Wbfs_Fat::CloseDisc(wbfs_disc_t* disc)
{
	if (!disc) return;
	wbfs_t *part = disc->p;

	// is this really a .iso file?
	if (part == &wbfs_iso_file)
	{
		close(disc->i);
		free(disc->header);
		free(disc);
		return;
	}

	wbfs_close_disc(disc);
	ClosePart(part);
	return;
}

s32 Wbfs_Fat::GetCount(u32 *count)
{
	GetHeadersCount();
	*count = fat_hdr_count;
	return 0;
}

s32 Wbfs_Fat::GetHeaders(struct discHdr *outbuf, u32 cnt, u32 len)
{
	if(cnt*len > fat_hdr_count*sizeof(struct discHdr))
		return -1;

	memcpy(outbuf, fat_hdr_list, cnt*len);

	if(fat_hdr_list)
		free(fat_hdr_list);
	fat_hdr_list = NULL;
	fat_hdr_count = 0;

	return 0;
}

s32 Wbfs_Fat::AddGame(void)
{
	static struct discHdr header ATTRIBUTE_ALIGN( 32 );
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
	partition_selector_t part_sel = (partition_selector_t) Settings.InstallPartitions;

	ret = wbfs_add_disc(part, __ReadDVD, NULL, ShowProgress, part_sel, 0);
	wbfs_trim(part);
	ClosePart(part);

	if(install_abort_signal)
		RemoveGame(header.id);
	if (ret < 0) return ret;

	return 0;
}

s32 Wbfs_Fat::RemoveGame(u8 *discid)
{
	char fname[MAX_FAT_PATH];
	int loc;
	// wbfs 'partition' file
	loc = FindFilename(discid, fname, sizeof(fname));
	if (!loc) return -1;
	split_create(&split, fname, 0, 0, true);
	split_close(&split);
	if (loc == 1) return 0;

	// game is in subdir
	// remove optional .txt file
	DIR *dir = NULL;
	struct dirent *dirent = NULL;
	char path[MAX_FAT_PATH];
	char name[MAX_FAT_PATH];
	strncpy(path, fname, sizeof(path));
	char *p = strrchr(path, '/');
	if (p) *p = 0;
	dir = opendir(path);
	if (!dir) return 0;
	while ((dirent = readdir(dir)) != 0)
	{
		snprintf(name, sizeof(name), dirent->d_name);
		if (name[0] == '.') continue;
		if (name[6] != '_') continue;
		if (strncasecmp(name, (char*) discid, 6) != 0) continue;
		p = strrchr(name, '.');
		if (!p) continue;
		if (strcasecmp(p, ".txt") != 0) continue;
		snprintf(fname, sizeof(fname), "%s/%s", path, name);
		remove(fname);
		break;
	}
	closedir(dir);
	// remove game subdir
	remove(path);

	return 0;
}

s32 Wbfs_Fat::DiskSpace(f32 *used, f32 *free)
{
	static f32 used_cached = 0.0;
	static f32 free_cached = 0.0;
	static int game_count = 0;

	//! Since it's freaken slow, only refresh on new gamecount
	if(used_cached == 0.0 || game_count != gameList.GameCount())
	{
		game_count = gameList.GameCount();
	}
	else
	{
		*used = used_cached;
		*free = free_cached;
		return 0;
	}

	f32 size;
	int ret;
	struct statvfs wbfs_fat_vfs;

	*used = used_cached = 0.0;
	*free = free_cached = 0.0;
	ret = statvfs(wbfs_fs_drive, &wbfs_fat_vfs);
	if (ret) return -1;

	/* FS size in GB */
	size = (f32) wbfs_fat_vfs.f_frsize * (f32) wbfs_fat_vfs.f_blocks / GB_SIZE;
	*free = free_cached = (f32) wbfs_fat_vfs.f_frsize * (f32) wbfs_fat_vfs.f_bfree / GB_SIZE;
	*used = used_cached = size - *free;

	return 0;
}

s32 Wbfs_Fat::RenameGame(u8 *discid, const void *newname)
{
	wbfs_t *part = OpenPart((char *) discid);
	if (!part) return -1;

	s32 ret = wbfs_ren_disc(part, discid, (u8*) newname);

	ClosePart(part);

	return ret;
}

s32 Wbfs_Fat::ReIDGame(u8 *discid, const void *newID)
{
	wbfs_t *part = OpenPart((char *) discid);
	if (!part) return -1;

	s32 ret = wbfs_rID_disc(part, discid, (u8*) newID);

	ClosePart(part);

	if (ret == 0)
	{
		char fname[100];
		char fnamenew[100];
		s32 cnt = 0x31;

		Filename(discid, fname, sizeof(fname), NULL);
		Filename((u8*) newID, fnamenew, sizeof(fnamenew), NULL);

		int stringlength = strlen(fname);

		while (rename(fname, fnamenew) == 0)
		{
			fname[stringlength] = cnt;
			fname[stringlength + 1] = 0;
			fnamenew[stringlength] = cnt;
			fnamenew[stringlength + 1] = 0;
			cnt++;
		}
	}

	return ret;
}

u64 Wbfs_Fat::EstimateGameSize()
{
	wbfs_t *part = NULL;
	u64 size = (u64) 143432 * 2 * 0x8000ULL;
	u32 n_sector = size / hdd_sector_size[usbport];

	// init a temporary dummy part
	// as a placeholder for wbfs_size_disc
	wbfs_set_force_mode(1);
	part = wbfs_open_partition(nop_rw_sector, nop_rw_sector, NULL, hdd_sector_size[usbport], n_sector, 0, 1);
	wbfs_set_force_mode(0);
	if (!part) return -1;

	partition_selector_t part_sel = (partition_selector_t) Settings.InstallPartitions;

	u64 estimated_size = wbfs_estimate_disc(part, __ReadDVD, NULL, part_sel);

	wbfs_close(part);

	return estimated_size;
}

// TITLE [GAMEID]
bool Wbfs_Fat::CheckLayoutB(char *fname, int len, u8* id, char *fname_title)
{
	if (len <= 8) return false;
	if (fname[len - 8] != '[' || fname[len - 1] != ']') return false;
	if (!isGameID(&fname[len - 7])) return false;
	strncpy(fname_title, fname, TITLE_LEN);
	// cut at '['
	fname_title[len - 8] = 0;
	int n = strlen(fname_title);
	if (n == 0) return false;
	// cut trailing _ or ' '
	if (fname_title[n - 1] == ' ' || fname_title[n - 1] == '_')
	{
		fname_title[n - 1] = 0;
	}
	if (strlen(fname_title) == 0) return false;
	if (id)
	{
		memcpy(id, &fname[len - 7], 6);
		id[6] = 0;
	}
	return true;
}

void Wbfs_Fat::AddHeader(struct discHdr *discHeader)
{
	//! First allocate before reallocating
	if(!fat_hdr_list)
		fat_hdr_list = (struct discHdr *) malloc(sizeof(struct discHdr));

	struct discHdr *tmpList = (struct discHdr *) realloc(fat_hdr_list, (fat_hdr_count+1) * sizeof(struct discHdr));
	if(!tmpList)
		return; //out of memory, keep the list until now and stop

	for(int j = 0; j < 6; ++j)
		discHeader->id[j] = toupper((int) discHeader->id[j]);

	fat_hdr_list = tmpList;
	memcpy(&fat_hdr_list[fat_hdr_count], discHeader, sizeof(struct discHdr));
	GameTitles.SetGameTitle(discHeader->id, discHeader->title);
	fat_hdr_count++;
}

s32 Wbfs_Fat::GetHeadersCount()
{
	char path[MAX_FAT_PATH];
	char fname[MAX_FAT_PATH];
	char fpath[MAX_FAT_PATH];
	char fname_title[TITLE_LEN];
	struct discHdr tmpHdr;
	struct stat st;
	int is_dir;
	int len;
	u8 id[8];
	memset(id, 0, sizeof(id));
	const char *title;
	DIR *dir_iter;
	struct dirent *dirent;

	if(fat_hdr_list)
		free(fat_hdr_list);
	fat_hdr_list = NULL;
	fat_hdr_count = 0;

	strcpy(path, wbfs_fs_drive);
	strcat(path, wbfs_fat_dir);

	dir_iter = opendir(path);
	if (!dir_iter) return 0;

	while ((dirent = readdir(dir_iter)) != 0)
	{
		if (dirent->d_name[0] == '.') continue;

		snprintf(fname, sizeof(fname), "%s", dirent->d_name);

		// reset id and title
		memset(id, 0, sizeof(id));
		*fname_title = 0;

		const char * fileext = strrchr(fname, '.');
		if(fileext && (strcasecmp(fileext, ".wbfs") == 0 ||
		   strcasecmp(fileext, ".iso") == 0 || strcasecmp(fileext, ".ciso") == 0))
		{
			// usb:/wbfs/GAMEID.wbfs
			// or usb:/wbfs/GAMEID.iso
			// or usb:/wbfs/GAMEID.ciso
			int n = fileext - fname; // length withouth .wbfs
			memcpy(id, fname, 6);
			if (n != 6)
			{
				// TITLE [GAMEID].wbfs
				if (!CheckLayoutB(fname, n, id, fname_title)) continue;
			}
			snprintf(fpath, sizeof(fpath), "%s/%s", path, fname);
			is_dir = 0;
		}
		else
		{
			snprintf(fname, sizeof(fname), "%s/%s", path, dirent->d_name);

			if(stat(fname, &st) != 0)
				continue;

			is_dir = S_ISDIR( st.st_mode );
			if(!is_dir) continue;

			snprintf(fname, sizeof(fname), "%s", dirent->d_name);

			len = strlen(fname);
			if (len < 8) continue; // "GAMEID_x"

			int lay_a = 0;
			int lay_b = 0;
			if (CheckLayoutB(fname, len, id, fname_title))
			{
				// usb:/wbfs/TITLE[GAMEID]/GAMEID.wbfs
				lay_b = 1;
			}
			else if (fname[6] == '_')
			{
				// usb:/wbfs/GAMEID_TITLE/GAMEID.wbfs
				memcpy(id, fname, 6);

				if(isGameID((char*) id))
				{
					lay_a = 1;
					snprintf(fname_title, sizeof(fname_title), &fname[7]);
				}
			}

			if (!lay_a && !lay_b) continue;

			// check ahead, make sure it succeeds
			snprintf(fpath, sizeof(fpath), "%s/%s/%.6s.wbfs", path, dirent->d_name, (char *) id);
		}

		// if we have titles.txt entry use that
		title = GameTitles.GetTitle(id);
		// if no titles.txt get title from dir or file name
		if (strlen(title) == 0 && !Settings.ForceDiscTitles && strlen(fname_title) > 0)
			title = fname_title;

		if (strlen(title) > 0)
		{
			memset(&tmpHdr, 0, sizeof(tmpHdr));
			memcpy(tmpHdr.id, id, 6);
			strncpy(tmpHdr.title, title, sizeof(tmpHdr.title)-1);
			tmpHdr.magic = 0x5D1C9EA3;
			AddHeader(&tmpHdr);
			continue;
		}

		// Check for existing wbfs/iso/ciso file in the directory
		if(is_dir)
		{
			if (stat(fpath, &st) != 0)
			{
				// look for direct .iso file
				strcpy(strrchr(fpath, '.'), ".iso"); // replace .wbfs with .iso
				if (stat(fpath, &st) != 0)
				{
					// look for direct .ciso file
					strcpy(strrchr(fpath, '.'), ".ciso"); // replace .iso with .ciso
					if (stat(fpath, &st) != 0) continue;
				}
			}
		}

		fileext = strrchr(fpath, '.');
		// Sanity check
		if(!fileext)
			continue;

		// else read it from file directly
		if (strcasecmp(fileext, ".wbfs") == 0)
		{
			// wbfs file directly
			FILE *fp = fopen(fpath, "rb");
			if (fp != NULL)
			{
				fseek(fp, 512, SEEK_SET);
				fread(&tmpHdr, sizeof(struct discHdr), 1, fp);
				fclose(fp);
				tmpHdr.is_ciso = 0;
				if ((tmpHdr.magic == 0x5D1C9EA3) && (memcmp(tmpHdr.id, id, 6) == 0))
				{
					AddHeader(&tmpHdr);
					continue;
				}
			}
			// no title found, read it from wbfs file
			// but this is a little bit slower
			// open 'partition' file
			wbfs_t *part = OpenPart(fpath);
			if (!part)
				continue;

			u32 size;
			// Get header
			int ret = wbfs_get_disc_info(part, 0, (u8*) &tmpHdr, sizeof(struct discHdr), &size);
			ClosePart(part);
			if (ret == 0)
			{
				AddHeader(&tmpHdr);
				continue;
			}

		}
		else if (strcasecmp(fileext, ".iso") == 0)
		{
			// iso file
			FILE *fp = fopen(fpath, "rb");
			if (fp != NULL)
			{
				fseek(fp, 0, SEEK_SET);
				fread(&tmpHdr, sizeof(struct discHdr), 1, fp);
				fclose(fp);
				tmpHdr.is_ciso = 0;
				if ((tmpHdr.magic == 0x5D1C9EA3) && (memcmp(tmpHdr.id, id, 6) == 0))
				{
					AddHeader(&tmpHdr);
					continue;
				}
			}
		}
		else if (strcasecmp(fileext, ".ciso") == 0)
		{
			// ciso file
			FILE *fp = fopen(fpath, "rb");
			if (fp != NULL)
			{
				fseek(fp, 0x8000, SEEK_SET);
				fread(&tmpHdr, sizeof(struct discHdr), 1, fp);
				fclose(fp);
				tmpHdr.is_ciso = 1;
				if ((tmpHdr.magic == 0x5D1C9EA3) && (memcmp(tmpHdr.id, id, 6) == 0))
				{
					AddHeader(&tmpHdr);
					continue;
				}
			}
		}
	}

	closedir(dir_iter);

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
	// look for direct .ciso file
	strcpy(strrchr(fname, '.'), ".ciso"); // replace .iso with .ciso
	if (stat(fname, &st) == 0) return 1;

	// direct file not found, check subdirs
	*fname = 0;
	DIR *dir_iter;
	struct dirent *dirent;
	char gameID[7];
	snprintf(gameID, sizeof(gameID), (char *) id);
	char path[MAX_FAT_PATH];
	strcpy(path, wbfs_fs_drive);
	strcat(path, wbfs_fat_dir);

	dir_iter = opendir(path);
	if (!dir_iter)
		return 0;

	while ((dirent = readdir(dir_iter)) != 0)
	{
		if(strcasestr(dirent->d_name, gameID) == NULL) continue;

		if (dirent->d_name[0] == '.') continue;
		int n = strlen(dirent->d_name);
		if (n < 8) continue;

		const char *fileext = strrchr(dirent->d_name, '.');
		if(fileext && (strcasecmp(fileext, ".wbfs") == 0 ||
		   strcasecmp(fileext, ".iso") == 0 || strcasecmp(fileext, ".ciso") == 0))
		{
			// TITLE [GAMEID].wbfs
			char fn_title[TITLE_LEN];
			u8 fn_id[8];
			int n = fileext - dirent->d_name; // length withouth .wbfs
			if (!CheckLayoutB(dirent->d_name, n, fn_id, fn_title)) continue;
			if (strncasecmp((char*) fn_id, gameID, 6) != 0) continue;
			snprintf(fname, len, "%s/%s", path, dirent->d_name);
			if (stat(fname, &st) == 0) break;
		}

		snprintf(fname, len, "%s/%s", path, dirent->d_name);

		if(stat(fname, &st) != 0)
		{
			*fname = 0;
			continue;
		}

		if (S_ISDIR( st.st_mode ))
		{
			// look for .wbfs file
			snprintf(fname, len, "%s/%s/%.6s.wbfs", path, dirent->d_name, gameID);
			if (stat(fname, &st) == 0) break;
			// look for .iso file
			snprintf(fname, len, "%s/%s/%.6s.iso", path, dirent->d_name, gameID);
			if (stat(fname, &st) == 0) break;
			// look for .ciso file
			snprintf(fname, len, "%s/%s/%.6s.ciso", path, dirent->d_name, gameID);
			if (stat(fname, &st) == 0) break;
		}

		*fname = 0;
	}
	closedir(dir_iter);

	if (*fname)
		return 2;

	return 0;
}

wbfs_t* Wbfs_Fat::OpenPart(char *fname)
{
	wbfs_t *part = NULL;
	int ret;

	// wbfs 'partition' file
	ret = split_open(&split, fname);
	if (ret) return NULL;

	wbfs_set_force_mode(1);

	part = wbfs_open_partition(split_read_sector, nop_rw_sector, //readonly //split_write_sector,
			&split, hdd_sector_size[usbport], split.total_sec, 0, 0);

	wbfs_set_force_mode(0);

	if (!part)
		split_close(&split);

	return part;
}

void Wbfs_Fat::ClosePart(wbfs_t* part)
{
	if (!part) return;
	split_info_t *s = (split_info_t*) part->callback_data;
	wbfs_close(part);
	if (s) split_close(s);
}

void Wbfs_Fat::Filename(u8 *id, char *fname, int len, char *path)
{
	if (path == NULL)
	{
		snprintf(fname, len, "%s%s/%.6s.wbfs", wbfs_fs_drive, wbfs_fat_dir, id);
	}
	else
	{
		snprintf(fname, len, "%s/%.6s.wbfs", path, id);
	}
}

void Wbfs_Fat::GetDir(struct discHdr *header, char *path)
{
	strcpy(path, wbfs_fs_drive);
	strcat(path, wbfs_fat_dir);
	if (Settings.InstallToDir)
	{
		strcat(path, "/");
		int layout = 0;
		if (Settings.InstallToDir == 2) layout = 1;
		mk_gameid_title(header, path + strlen(path), 0, layout);
	}
}

wbfs_t* Wbfs_Fat::CreatePart(u8 *id, char *path)
{
	char fname[MAX_FAT_PATH];
	wbfs_t *part = NULL;
	u64 size = (u64) 143432 * 2 * 0x8000ULL;
	u32 n_sector = size / 512;
	int ret;

	if(!CreateSubfolder(path)) // game subdir
	{
		ProgressStop();
		ShowError(tr("Error creating path: %s"), path);
		return NULL;
	}

	// 1 cluster less than 4gb
	u64 OPT_split_size = 4LL * 1024 * 1024 * 1024 - 32 * 1024;

	if(Settings.GameSplit == GAMESPLIT_NONE && DeviceHandler::GetFilesystemType(USB1+Settings.partition) != PART_FS_FAT)
		OPT_split_size = (u64) 100LL * 1024 * 1024 * 1024 - 32 * 1024;

	else if(Settings.GameSplit == GAMESPLIT_2GB)
		// 1 cluster less than 2gb
		OPT_split_size = (u64)2LL * 1024 * 1024 * 1024 - 32 * 1024;

	Filename(id, fname, sizeof(fname), path);
	printf("Writing to %s\n", fname);
	ret = split_create(&split, fname, OPT_split_size, size, true);
	if (ret) return NULL;

	// force create first file
	u32 scnt = 0;
	int fd = split_get_file(&split, 0, &scnt, 0);
	if (fd < 0)
	{
		printf("ERROR creating file\n");
		sleep(2);
		split_close(&split);
		return NULL;
	}

	wbfs_set_force_mode(1);

	part = wbfs_open_partition(split_read_sector, split_write_sector, &split, hdd_sector_size[usbport], n_sector, 0, 1);

	wbfs_set_force_mode(0);

	if (!part)
		split_close(&split);

	return part;
}

void Wbfs_Fat::mk_gameid_title(struct discHdr *header, char *name, int re_space, int layout)
{
	int i, len;
	char title[100];
	char id[7];

	snprintf(id, sizeof(id), (char *) header->id);
	snprintf(title, sizeof(title), header->title);
	CleanTitleCharacters(title);

	if (layout == 0)
	{
		sprintf(name, "%s_%s", id, title);
	}
	else
	{
		sprintf(name, "%s [%s]", title, id);
	}

	// replace space with '_'
	if (re_space)
	{
		len = strlen(name);
		for (i = 0; i < len; i++)
		{
			if (name[i] == ' ') name[i] = '_';
		}
	}
}

void Wbfs_Fat::CleanTitleCharacters(char *title)
{
	int i, len;
	// trim leading space
	len = strlen(title);
	while (*title == ' ')
	{
		memmove(title, title + 1, len);
		len--;
	}
	// trim trailing space - not allowed on windows directories
	while (len && title[len - 1] == ' ')
	{
		title[len - 1] = 0;
		len--;
	}
	// replace silly chars with '_'
	for (i = 0; i < len; i++)
	{
		if (strchr(invalid_path, title[i]) || iscntrl((int) title[i]))
		{
			title[i] = '_';
		}
	}
}

int Wbfs_Fat::GetFragList(u8 *id)
{
	char fname[1024];

	int ret = FindFilename(id, fname, sizeof(fname));
	if (!ret) return -1;

	return get_frag_list_for_file(fname, id, GetFSType(), lba, hdd_sector_size[usbport]);
}
