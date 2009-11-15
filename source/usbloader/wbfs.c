#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <ogcsys.h>
#include <errno.h>

#include "sdhc.h"
#include "usbstorage.h"
#include "utils.h"
#include "video.h"
#include "wdvd.h"
#include "wbfs.h"
#include "wbfs_fat.h"
#include "fatmounter.h"
#include "partition_usbloader.h"

#include "libwbfs/libwbfs.h"

/* Constants */
#define MAX_NB_SECTORS	32

/* WBFS device */
s32 wbfsDev = WBFS_MIN_DEVICE;

// partition
int wbfs_part_fat = 0;
u32 wbfs_part_idx = 0;
u32 wbfs_part_lba = 0;

/* WBFS HDD */
wbfs_t *hdd = NULL;

/* WBFS callbacks */
static rw_sector_callback_t readCallback  = NULL;
static rw_sector_callback_t writeCallback = NULL;
static s32 done = -1, total = -1;
/* Variables */

static u32 nb_sectors, sector_size;

void WBFS_Spinner(s32 x, s32 max) {
    done = x;
    total = max;
}

wbfs_disc_t* WBFS_OpenDisc(u8 *discid)
{
	if (wbfs_part_fat) return WBFS_FAT_OpenDisc(discid);

	/* No device open */
	if (!hdd)
		return NULL;

	/* Open disc */
	return wbfs_open_disc(hdd, discid);
}

void WBFS_CloseDisc(wbfs_disc_t *disc)
{
	if (wbfs_part_fat) {
		WBFS_FAT_CloseDisc(disc);
		return;
	}

	/* No device open */
	if (!hdd || !disc)
		return;

	/* Close disc */
	wbfs_close_disc(disc);
}

void GetProgressValue(s32 * d, s32 * m) {
    *d = done;
    *m = total;
}

wbfs_t *GetHddInfo(void) {
    return hdd;
}

s32 __WBFS_ReadDVD(void *fp, u32 lba, u32 len, void *iobuf) {
    void *buffer = NULL;

    u64 offset;
    u32 mod, size;
    s32 ret;

    /* Calculate offset */
    offset = ((u64)lba) << 2;

    /* Calcualte sizes */
    mod  = len % 32;
    size = len - mod;

    /* Read aligned data */
    if (size) {
        ret = WDVD_UnencryptedRead(iobuf, size, offset);
        if (ret < 0)
            goto out;
    }

    /* Read non-aligned data */
    if (mod) {
        /* Allocate memory */
        buffer = memalign(32, 0x20);
        if (!buffer)
            return -1;

        /* Read data */
        ret = WDVD_UnencryptedRead(buffer, 0x20, offset + size);
        if (ret < 0)
            goto out;

        /* Copy data */
        memcpy(iobuf + size, buffer, mod);
    }

    /* Success */
    ret = 0;

out:
    /* Free memory */
    if (buffer)
        free(buffer);

    return ret;
}

s32 __WBFS_ReadUSB(void *fp, u32 lba, u32 count, void *iobuf) {
    u32 cnt = 0;
    s32 ret;

    /* Do reads */
    while (cnt < count) {
        void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
        u32   sectors = (count - cnt);

        /* Read sectors is too big */
        if (sectors > MAX_NB_SECTORS)
            sectors = MAX_NB_SECTORS;

        /* USB read */
        ret = USBStorage_ReadSectors(lba + cnt, sectors, ptr);
        if (ret < 0)
            return ret;

        /* Increment counter */
        cnt += sectors;
    }

    return 0;
}

s32 __WBFS_WriteUSB(void *fp, u32 lba, u32 count, void *iobuf) {
    u32 cnt = 0;
    s32 ret;

    /* Do writes */
    while (cnt < count) {
        void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
        u32   sectors = (count - cnt);

        /* Write sectors is too big */
        if (sectors > MAX_NB_SECTORS)
            sectors = MAX_NB_SECTORS;

        /* USB write */
        ret = USBStorage_WriteSectors(lba + cnt, sectors, ptr);
        if (ret < 0)
            return ret;

        /* Increment counter */
        cnt += sectors;
    }

    return 0;
}

s32 __WBFS_ReadSDHC(void *fp, u32 lba, u32 count, void *iobuf) {
    u32 cnt = 0;
    s32 ret;

    /* Do reads */
    while (cnt < count) {
        void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
        u32   sectors = (count - cnt);

        /* Read sectors is too big */
        if (sectors > MAX_NB_SECTORS)
            sectors = MAX_NB_SECTORS;

        /* SDHC read */
        ret = SDHC_ReadSectors(lba + cnt, sectors, ptr);
        if (!ret)
            return -1;

        /* Increment counter */
        cnt += sectors;
    }

    return 0;
}

s32 __WBFS_WriteSDHC(void *fp, u32 lba, u32 count, void *iobuf) {
    u32 cnt = 0;
    s32 ret;

    /* Do writes */
    while (cnt < count) {
        void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
        u32   sectors = (count - cnt);

        /* Write sectors is too big */
        if (sectors > MAX_NB_SECTORS)
            sectors = MAX_NB_SECTORS;

        /* SDHC write */
        ret = SDHC_WriteSectors(lba + cnt, sectors, ptr);
        if (!ret)
            return -1;

        /* Increment counter */
        cnt += sectors;
    }

    return 0;
}

s32 WBFS_Init(u32 device) {
    s32 ret;

    switch (device) {
    case WBFS_DEVICE_USB:
        /* Initialize USB storage */
        ret = USBStorage_Init();
        if (ret >= 0) {
            /* Setup callbacks */
            readCallback = __WBFS_ReadUSB;
            writeCallback = __WBFS_WriteUSB;
            /* Device info */
            /* Get USB capacity */
            nb_sectors = USBStorage_GetCapacity(&sector_size);
            if (!nb_sectors)
                return -1;
        } else
            return ret;
        break;
    case WBFS_DEVICE_SDHC:
        /* Initialize SDHC */
        ret = SDHC_Init();

        if (ret) {
            /* Setup callbacks */
            readCallback  = __WBFS_ReadSDHC;
            writeCallback = __WBFS_WriteSDHC;

            /* Device info */
            nb_sectors  = 0;
            sector_size = SDHC_SECTOR_SIZE;
        } else
            return -1;
        break;
    }

    return 0;
}

s32 WBFS_Open(void) {
    /* Close hard disk */
    if (hdd)
        wbfs_close(hdd);

    /* Open hard disk */
	wbfs_part_fat = wbfs_part_idx = wbfs_part_lba = 0;
    hdd = wbfs_open_hd(readCallback, writeCallback, NULL, sector_size, nb_sectors, 0);
    if (!hdd)
        return -1;

	// Save the new sector size, so it will be used in read and write calls
	sector_size = 1 << hdd->head->hd_sec_sz_s;

	wbfs_part_idx = 1;
    return 0;
}

s32 WBFS_OpenPart(u32 part_fat, u32 part_idx, u32 part_lba, u32 part_size, char *partition)
{
	// close
	WBFS_Close();

	if (part_fat) {
		if (wbfsDev != WBFS_DEVICE_USB) return -1;
		if (part_lba == fat_usb_sec) {
			strcpy(wbfs_fat_drive, "USB:");
		} else {
			if (WBFSDevice_Init(part_lba)) return -1;
			strcpy(wbfs_fat_drive, "WBFS:");
		}
	} else {
		if (WBFS_OpenLBA(part_lba, part_size)) return -3;
	}

	// success
	wbfs_part_fat = part_fat;
	wbfs_part_idx = part_idx;
	wbfs_part_lba = part_lba;
	
	sprintf(partition, "%s%d", wbfs_part_fat ? "FAT" : "WBFS", wbfs_part_idx);
	return 0;
}

s32 WBFS_OpenNamed(char *partition)
{
	int i;
	u32 part_idx = 0;
	u32 part_fat = 0;
	u32 part_lba = 0;
	s32 ret = 0;
	PartList plist;

	// close
	WBFS_Close();

	// parse partition option
	if (strncasecmp(partition, "WBFS", 4) == 0) {
		i = atoi(partition+4);
		if (i < 1 || i > 4) goto err;
		part_idx = i;
	} else if (strncasecmp(partition, "FAT", 3) == 0) {
		if (wbfsDev != WBFS_DEVICE_USB) goto err;
		i = atoi(partition+3);
		if (i < 1 || i > 9) goto err;
		part_idx = i;
		part_fat = 1;
	} else {
		goto err;
	}

	// Get partition entries
	ret = Partition_GetList(&plist);
	if (ret || plist.num == 0) return -1;

	if (part_fat) {
		if (part_idx > plist.fat_n) goto err;
		for (i=0; i<plist.num; i++) {
			if (plist.pinfo[i].fat_i == part_idx) break;
		}
	} else {
		if (part_idx > plist.wbfs_n) goto err;
		for (i=0; i<plist.num; i++) {
			if (plist.pinfo[i].wbfs_i == part_idx) break;
		}
	}
	if (i >= plist.num) goto err;
	// set partition lba sector
	part_lba = plist.pentry[i].sector;

	if (WBFS_OpenPart(part_fat, part_idx, part_lba, plist.pentry[i].size, partition)) {
		goto err;
	}
	// success
	return 0;
err:
	return -1;
}

s32 WBFS_OpenLBA(u32 lba, u32 size)
{
	wbfs_t *part = NULL;

	/* Open partition */
	part = wbfs_open_partition(readCallback, writeCallback, NULL, sector_size, size, lba, 0);
	if (!part) return -1;

	/* Close current hard disk */
	if (hdd) wbfs_close(hdd);
	hdd = part;

	return 0;
}

bool WBFS_Close(void)
{
    /* Close hard disk */
    if (hdd) {
        wbfs_close(hdd);
		hdd = NULL;
	}

	WBFSDevice_deInit();
	wbfs_part_fat = 0;
	wbfs_part_idx = 0;
	wbfs_part_lba = 0;
	wbfs_fat_drive[0] = '\0';

    return 0;
}

bool WBFS_Mounted()
{
	return (hdd != NULL);
}

bool WBFS_Selected()
{
	if (wbfs_part_fat && wbfs_part_lba && *wbfs_fat_drive) return true;
	return WBFS_Mounted();
}

s32 WBFS_Format(u32 lba, u32 size) {
    wbfs_t *partition = NULL;

    /* Reset partition */
    partition = wbfs_open_partition(readCallback, writeCallback, NULL, sector_size, size, lba, 1);
    if (!partition)
        return -1;

    /* Free memory */
    wbfs_close(partition);

    return 0;
}

s32 WBFS_GetCount(u32 *count) {
	if (wbfs_part_fat) return WBFS_FAT_GetCount(count);

    /* No device open */
    if (!hdd)
        return -1;

    /* Get list length */
    *count = wbfs_count_discs(hdd);

    return 0;
}

s32 WBFS_GetHeaders(void *outbuf, u32 cnt, u32 len) {
	if (wbfs_part_fat) return WBFS_FAT_GetHeaders(outbuf, cnt, len);

    u32 idx, size;
    s32 ret;

    /* No device open */
    if (!hdd)
        return -1;

    for (idx = 0; idx < cnt; idx++) {
        u8 *ptr = ((u8 *)outbuf) + (idx * len);

        /* Get header */
        ret = wbfs_get_disc_info(hdd, idx, ptr, len, &size);
        if (ret < 0)
            return ret;
    }

    return 0;
}

s32 WBFS_CheckGame(u8 *discid) {
    wbfs_disc_t *disc = NULL;

    /* Try to open game disc */
    disc = WBFS_OpenDisc(discid);
    if (disc) {
        /* Close disc */
        WBFS_CloseDisc(disc);

        return 1;
    }

    return 0;
}

s32 WBFS_AddGame(void) {
	if (wbfs_part_fat) return WBFS_FAT_AddGame();

    s32 ret;

    /* No device open */
    if (!hdd)
        return -1;

    /* Add game to device */
    ret = wbfs_add_disc(hdd, __WBFS_ReadDVD, NULL, WBFS_Spinner, ONLY_GAME_PARTITION, 0);
    if (ret < 0)
        return ret;

    return 0;
}

s32 WBFS_RemoveGame(u8 *discid) {
	if (wbfs_part_fat) return WBFS_FAT_RemoveGame(discid);

    s32 ret;

    /* No device open */
    if (!hdd)
        return -1;

    /* Remove game from USB device */
    ret = wbfs_rm_disc(hdd, discid);
    if (ret < 0)
        return ret;

    return 0;
}

s32 WBFS_GameSize(u8 *discid, f32 *size) {
    wbfs_disc_t *disc = NULL;

    u32 sectors;

    /* Open disc */
	disc = WBFS_OpenDisc(discid);
    if (!disc)
        return -2;

    /* Get game size in sectors */
	sectors = wbfs_sector_used(disc->p, disc->header);

    /* Copy value */
	*size = (disc->p->wbfs_sec_sz / GB_SIZE) * sectors;

    /* Close disc */
	WBFS_CloseDisc(disc);

    return 0;
}

s32 WBFS_DiskSpace(f32 *used, f32 *free) {
	if (wbfs_part_fat) return WBFS_FAT_DiskSpace(used, free);

    f32 ssize;
    u32 cnt;

    /* No device open */
    if (!hdd)
        return -1;

    /* Count used blocks */
    cnt = wbfs_count_usedblocks(hdd);

    /* Sector size in GB */
    ssize = hdd->wbfs_sec_sz / GB_SIZE;

    /* Copy values */
    *free = ssize * cnt;
    *used = ssize * (hdd->n_wbfs_sec - cnt);

    return 0;
}

s32 WBFS_RenameGame(u8 *discid, const void *newname) {
	if (wbfs_part_fat) return -1;

    s32 ret;

    /* No USB device open */
    if (!hdd)
        return -1;
    ret = wbfs_ren_disc(hdd, discid,(u8*)newname);
    if (ret < 0)
        return ret;

    return 0;
}

s32 WBFS_ReIDGame(u8 *discid, const void *newID) {
	if (wbfs_part_fat) return -1;

    s32 ret;

    /* No USB device open */
    if (!hdd)
        return -1;
    ret = wbfs_rID_disc(hdd, discid,(u8*)newID);
    if (ret < 0)
        return ret;

    return 0;
}

f32 WBFS_EstimeGameSize(void) {
	if (wbfs_part_fat) {
		u64 comp;
		WBFS_FAT_DVD_Size(&comp, NULL);
		return comp;
	}

    return wbfs_estimate_disc(hdd, __WBFS_ReadDVD, NULL, ONLY_GAME_PARTITION);
}
