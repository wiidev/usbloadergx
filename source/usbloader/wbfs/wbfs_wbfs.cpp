#include "wbfs_wbfs.h"
#include "prompts/ProgressWindow.h"
#include "settings/cfg.h"
#include "wbfs_rw.h"

extern u32 sector_size;

s32 Wbfs_Wbfs::Open()
{
	wbfs_t *part = NULL;

	/* Open partition */
	part = wbfs_open_partition(readCallback, writeCallback, NULL, sector_size, size, lba, 0);
	if (!part) return -1;

	/* Close current hard disk */
	Close();
	hdd = part;

	// Save the new sector size, so it will be used in read and write calls
	sector_size = 1 << hdd->head->hd_sec_sz_s;

	return 0;
}

wbfs_disc_t* Wbfs_Wbfs::OpenDisc(u8 *discid)
{
	/* No device open */
	if (!hdd)
		return NULL;

	/* Open disc */
	return wbfs_open_disc(hdd, discid);
}

void Wbfs_Wbfs::CloseDisc(wbfs_disc_t *disc)
{
	/* No device open */
	if (!hdd || !disc)
		return;

	/* Close disc */
	wbfs_close_disc(disc);
}

s32 Wbfs_Wbfs::Format()
{
    wbfs_t *partition = NULL;

    /* Reset partition */
    partition = wbfs_open_partition(readCallback, writeCallback, NULL, sector_size, size, lba, 1);
    if (!partition)
        return -1;

    /* Free memory */
    wbfs_close(partition);

    return 0;
}

s32 Wbfs_Wbfs::GetCount(u32 *count)
{
    /* No device open */
    if (!hdd)
        return -1;

    /* Get list length */
    *count = wbfs_count_discs(hdd);

    return 0;
}

s32 Wbfs_Wbfs::GetHeaders(struct discHdr *outbuf, u32 cnt, u32 len)
{
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

s32 Wbfs_Wbfs::AddGame()
{
    s32 ret;

    /* No device open */
    if (!hdd)
        return -1;

    /* Add game to device */
	partition_selector_t part_sel;
	int copy_1_1 = 0;

	if (Settings.fullcopy) {
		part_sel = ALL_PARTITIONS;
		copy_1_1 = 1;
	} else {
		part_sel = Settings.partitions_to_install == install_game_only ? ONLY_GAME_PARTITION : ALL_PARTITIONS;
	}

    ret = wbfs_add_disc(hdd, __ReadDVD, NULL, ProgressCallback, part_sel, copy_1_1);
    if (ret < 0)
        return ret;

    return 0;
}

s32 Wbfs_Wbfs::RemoveGame(u8 *discid)
{
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

s32 Wbfs_Wbfs::DiskSpace(f32 *used, f32 *free)
{
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

s32 Wbfs_Wbfs::RenameGame(u8 *discid, const void *newname)
{
    s32 ret;

    /* No USB device open */
    if (!hdd)
        return -1;
    ret = wbfs_ren_disc(hdd, discid,(u8*)newname);
    if (ret < 0)
        return ret;

    return 0;
}

s32 Wbfs_Wbfs::ReIDGame(u8 *discid, const void *newID)
{
    s32 ret;

    /* No USB device open */
    if (!hdd)
        return -1;
    ret = wbfs_rID_disc(hdd, discid,(u8*)newID);
    if (ret < 0)
        return ret;

    return 0;
}

f32 Wbfs_Wbfs::EstimateGameSize()
{
	partition_selector_t part_sel = ONLY_GAME_PARTITION;
	if (Settings.fullcopy) {
		part_sel = ALL_PARTITIONS;
	} else {
		switch(Settings.partitions_to_install)
		{
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
	}
    return wbfs_estimate_disc(hdd, __ReadDVD, NULL, part_sel);
}
