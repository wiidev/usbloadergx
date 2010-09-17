#include <ogcsys.h>
#include <unistd.h>
#include <time.h>

#include "usbloader/usbstorage2.h"
#include "fatmounter.h"
#include "wbfs.h"
#include "usbloader/wbfs/wbfs_base.h"
#include "usbloader/wbfs/wbfs_wbfs.h"
#include "usbloader/wbfs/wbfs_fat.h"
#include "usbloader/wbfs/wbfs_ntfs.h"

#include "usbloader/partition_usbloader.h"
#include "usbloader/getentries.h"
#include "gecko.h"

Wbfs *current = NULL;
#define DEBUG_WBFS

/* WBFS device */
s32 wbfsDev = WBFS_MIN_DEVICE;

// partition
char wbfs_fs_drive[16];
int wbfs_part_fs  = PART_FS_WBFS;
u32 wbfs_part_idx = 0;
u32 wbfs_part_lba = 0;

wbfs_disc_t* WBFS_OpenDisc(u8 *discid) {
	return current->OpenDisc(discid);
}

void WBFS_CloseDisc(wbfs_disc_t *disc) {
	current->CloseDisc(disc);
}

wbfs_t *GetHddInfo(void) {
    return current->GetHddInfo();
}

s32 WBFS_Init(u32 device) {
	return Wbfs::Init(device);
}

s32 WBFS_Open(void) {
	WBFS_Close();

	current = new Wbfs_Wbfs(WBFS_DEVICE_USB, 0, 0); // Fix me!

	wbfs_part_fs = wbfs_part_idx = wbfs_part_lba = 0;
	wbfs_part_idx = 1;

	return current->Open();
}

s32 WBFS_OpenPart(u32 part_fs, u32 part_idx, u32 part_lba, u32 part_size, char *partition)
{
	// close
	WBFS_Close();

	if (part_fs == PART_FS_FAT) {
		current = new Wbfs_Fat(wbfsDev, part_lba, part_size);
		strcpy(wbfs_fs_drive, "USB:");
#ifdef DEBUG_WBFS
		gprintf("\n\tCreated WBFS_Fat instance at lba: %d of size %d", part_lba, part_size);
#endif
	    } else if (part_fs == PART_FS_NTFS) {
		current = new Wbfs_Ntfs(wbfsDev, part_lba, part_size);
		strcpy(wbfs_fs_drive, "NTFS:");
#ifdef DEBUG_WBFS
		gprintf("\n\tCreated WBFS_Ntfs instance at lba: %d of size %d", part_lba, part_size);
#endif
	    } else {
		current = new Wbfs_Wbfs(wbfsDev, part_lba, part_size);
#ifdef DEBUG_WBFS
		gprintf("\n\tCreated WBFS_Wbfs instance at lba: %d of size %d", part_lba, part_size);
#endif
	    }
	if (current->Open())
	{
		delete current;
		current = NULL;
		return -1;
	}

	// success
	wbfs_part_fs = part_fs;
	wbfs_part_idx = part_idx;
	wbfs_part_lba = part_lba;

	const char *fs = "WBFS";
	if (wbfs_part_fs == PART_FS_FAT) fs = "FAT";
	if (wbfs_part_fs == PART_FS_NTFS) fs = "NTFS";
	sprintf(partition, "%s%d", fs, wbfs_part_idx);
	return 0;
}

s32 WBFS_OpenNamed(char *partition)
{
	u32 i;
	u32 part_fs  = PART_FS_WBFS;
	u32 part_idx = 0;
	u32 part_lba = 0;
	s32 ret = 0;
	PartList plist;

	// close
	WBFS_Close();

	// parse partition option
	if (strncasecmp(partition, "WBFS", 4) == 0) {
		i = atoi(partition+4);
		if (i < 1 || i > 4) goto err;
		part_fs  = PART_FS_WBFS;
		part_idx = i;
	} else if (strncasecmp(partition, "FAT", 3) == 0) {
		if (wbfsDev != WBFS_DEVICE_USB) goto err;
		i = atoi(partition+3);
		if (i < 1 || i > 9) goto err;
		part_fs  = PART_FS_FAT;
		part_idx = i;
	} else if (strncasecmp(partition, "NTFS", 4) == 0) {
		i = atoi(partition+4);
		if (i < 1 || i > 9) goto err;
		part_fs  = PART_FS_NTFS;
		part_idx = i;
	} else {
		goto err;
	}

	// Get partition entries
	ret = Partition_GetList(wbfsDev, &plist);
	if (ret || plist.num == 0) return -1;

	if (part_fs == PART_FS_WBFS) {
		if (part_idx > plist.wbfs_n) goto err;
		for (i=0; i<plist.num; i++) {
			if (plist.pinfo[i].wbfs_i == part_idx) break;
		}
	} else if (part_fs == PART_FS_FAT) {
		if (part_idx > plist.fat_n) goto err;
		for (i=0; i<plist.num; i++) {
			if (plist.pinfo[i].fat_i == part_idx) break;
		}
	} else if (part_fs == PART_FS_NTFS) {
		if (part_idx > plist.ntfs_n) goto err;
		for (i=0; i<plist.num; i++) {
			if (plist.pinfo[i].ntfs_i == part_idx) break;
		}
	}
	if (i >= plist.num) goto err;
	// set partition lba sector
	part_lba = plist.pentry[i].sector;

	if (WBFS_OpenPart(part_fs, part_idx, part_lba, plist.pentry[i].size, partition)) {
		goto err;
	}
	// success
	return 0;
err:
	return -1;
}

s32 WBFS_OpenLBA(u32 lba, u32 size)
{
	Wbfs *part = new Wbfs_Wbfs(wbfsDev, lba, size);
	if (part->Open() != 0)
	{
		delete part;
		return -1;
	}

	WBFS_Close();
	current = part;
	return 0;
}

bool WBFS_Close(void)
{
	if (current != NULL) {
		current->Close();
		delete current;
		current = NULL;
	}

	wbfs_part_fs = 0;
	wbfs_part_idx = 0;
	wbfs_part_lba = 0;
	wbfs_fs_drive[0] = '\0';

	ResetGamelist();

    return 0;
}

bool WBFS_Mounted()
{
	return (current != NULL && current->Mounted());
}

s32 WBFS_Format(u32 lba, u32 size) {
	return current->Format();
}

s32 WBFS_GetCount(u32 *count) {
	return current->GetCount(count);
}

s32 WBFS_GetHeaders(struct discHdr *outbuf, u32 cnt, u32 len) {
	return current->GetHeaders(outbuf, cnt, len);
}

s32 WBFS_CheckGame(u8 *discid) {
	return current->CheckGame(discid);
}

s32 WBFS_AddGame(void) {
	s32 retval = current->AddGame();
	if (retval == 0) {
		ResetGamelist();
	}
	return retval;
}

s32 WBFS_RemoveGame(u8 *discid) {
	s32 retval = current->RemoveGame(discid);
	if (retval == 0) {
		ResetGamelist();
	}
	return retval;
}

s32 WBFS_GameSize(u8 *discid, f32 *size) {
    return current->GameSize(discid, size);
}

s32 WBFS_DiskSpace(f32 *used, f32 *free) {
	return current->DiskSpace(used, free);
}

s32 WBFS_RenameGame(u8 *discid, const void *newname) {
	s32 retval = current->RenameGame(discid, newname);
	if (retval == 0) {
		ResetGamelist();
	}
	return retval;
}

s32 WBFS_ReIDGame(u8 *discid, const void *newID) {
	s32 retval = current->ReIDGame(discid, newID);
	if (retval == 0) {
		ResetGamelist();
	}
	return retval;
}

f32 WBFS_EstimeGameSize(void) {
	return current->EstimateGameSize();
}

int WBFS_GetFragList(u8 *id) {
	return current->GetFragList(id);
}

bool WBFS_ShowFreeSpace(void) {
	return current->ShowFreeSpace();
}

int MountWBFS()
{
    int ret = -1;
    time_t currTime = time(0);

    while(time(0)-currTime < 15)
    {
        USBDevice_deInit();
        USBStorage2_Deinit();
        USBDevice_Init();
        ret = WBFS_Init(WBFS_DEVICE_USB);
        printf("%i...", int(time(0)-currTime));
        if(ret < 0)
            sleep(1);
        else
            break;
    }

    printf("\n");

    return ret;
}
