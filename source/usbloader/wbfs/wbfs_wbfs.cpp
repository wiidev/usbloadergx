#include "wbfs_wbfs.h"
#include "Controls/DeviceHandler.hpp"
#include "prompts/ProgressWindow.h"
#include "settings/CSettings.h"
#include "usbloader/wbfs.h"
#include "usbloader/usbstorage2.h"
#include "utils/tools.h"
#include "wbfs_rw.h"

#define MAX_WBFS_SECTORSIZE	 4096

extern u32 hdd_sector_size[2];

s32 Wbfs_Wbfs::Open()
{
	wbfs_t *part = NULL;

	PartInfo.wbfs_sector_size = MAX_WBFS_SECTORSIZE;
	PartInfo.hdd_sector_size = hdd_sector_size[usbport];
	PartInfo.partition_lba = lba;
	PartInfo.partition_num_sec = size;
	PartInfo.handle = (usbport == 0) ? DeviceHandler::GetUSB0Interface() : DeviceHandler::GetUSB1Interface();

	u8 * buffer = (u8 *) malloc(MAX_WBFS_SECTORSIZE);
	memset(buffer, 0, MAX_WBFS_SECTORSIZE);

	if(readCallback(&PartInfo, lba, 1, buffer) < 0)
	{
		free(buffer);
		return -1;
	}

	wbfs_head_t head;
	memcpy(&head, buffer, sizeof(wbfs_head_t));
	free(buffer);

	if (head.magic != wbfs_htonl(WBFS_MAGIC))
		return -1;

	/* Set correct sector values for wbfs read/write */
	PartInfo.wbfs_sector_size = 1 << head.hd_sec_sz_s;
	PartInfo.partition_num_sec = head.n_hd_sec;

	/* Open partition */
	part = wbfs_open_partition(readCallback, writeCallback, &PartInfo,
							   PartInfo.wbfs_sector_size, PartInfo.partition_num_sec,
							   lba, 0);
	if (!part) return -1;

	/* Close current hard disk */
	Close();
	hdd = part;

	return 0;
}

void Wbfs_Wbfs::Close()
{
	if (hdd)
	{
		wbfs_close(hdd);
		hdd = NULL;
	}
}

wbfs_disc_t* Wbfs_Wbfs::OpenDisc(u8 *discid)
{
	/* No device open */
	if (!hdd) return NULL;

	/* Open disc */
	return wbfs_open_disc(hdd, discid);
}

void Wbfs_Wbfs::CloseDisc(wbfs_disc_t *disc)
{
	/* No device open */
	if (!hdd || !disc) return;

	/* Close disc */
	wbfs_close_disc(disc);
}

s32 Wbfs_Wbfs::Format()
{
	WBFS_PartInfo HDD_Inf;
	HDD_Inf.wbfs_sector_size = hdd_sector_size[usbport];
	HDD_Inf.hdd_sector_size = hdd_sector_size[usbport];
	HDD_Inf.partition_lba = lba;
	HDD_Inf.partition_num_sec = size;
	HDD_Inf.handle = (usbport == 0) ? DeviceHandler::GetUSB0Interface() : DeviceHandler::GetUSB1Interface();

	//! If size is over 500GB in sectors and sector size is 512
	//! set 2048 as hdd sector size
	if(size > 1048576000 && hdd_sector_size[usbport] == 512)
	{
		HDD_Inf.wbfs_sector_size = 2048;
		HDD_Inf.partition_num_sec = size/(2048/hdd_sector_size[usbport]);
	}

	wbfs_t *partition = NULL;

	/* Reset partition */
	partition = wbfs_open_partition(readCallback, writeCallback, &HDD_Inf, HDD_Inf.wbfs_sector_size, HDD_Inf.partition_num_sec, lba, 1);
	if (!partition) return -1;

	/* Free memory */
	wbfs_close(partition);

	return 0;
}

s32 Wbfs_Wbfs::GetCount(u32 *count)
{
	/* No device open */
	if (!hdd) return -1;

	/* Get list length */
	*count = wbfs_count_discs(hdd);

	return 0;
}

s32 Wbfs_Wbfs::GetHeaders(struct discHdr *outbuf, u32 cnt, u32 len)
{
	u32 idx, size;
	s32 ret;

	/* No device open */
	if (!hdd) return -1;

	for (idx = 0; idx < cnt; idx++)
	{
		u8 *ptr = ((u8 *) outbuf) + (idx * len);

		/* Get header */
		ret = wbfs_get_disc_info(hdd, idx, ptr, len, &size);
		if (ret < 0) return ret;
	}

	return 0;
}

s32 Wbfs_Wbfs::AddGame()
{
	s32 ret;

	/* No device open */
	if (!hdd) return -1;

	partition_selector_t part_sel = (partition_selector_t) Settings.InstallPartitions;

	/* Add game to device */
	ret = wbfs_add_disc(hdd, __ReadDVD, NULL, ShowProgress, part_sel, 0);
	if (ret < 0) return ret;

	return 0;
}

s32 Wbfs_Wbfs::RemoveGame(u8 *discid)
{
	s32 ret;

	/* No device open */
	if (!hdd) return -1;

	/* Remove game from USB device */
	ret = wbfs_rm_disc(hdd, discid);
	if (ret < 0) return ret;

	return 0;
}

s32 Wbfs_Wbfs::DiskSpace(f32 *used, f32 *free)
{
	f32 ssize;
	u32 cnt;

	/* No device open */
	if (!hdd) return -1;

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
	if (!hdd) return -1;
	ret = wbfs_ren_disc(hdd, discid, (u8*) newname);
	if (ret < 0) return ret;

	return 0;
}

s32 Wbfs_Wbfs::ReIDGame(u8 *discid, const void *newID)
{
	s32 ret;

	/* No USB device open */
	if (!hdd) return -1;
	ret = wbfs_rID_disc(hdd, discid, (u8*) newID);
	if (ret < 0) return ret;

	return 0;
}

u64 Wbfs_Wbfs::EstimateGameSize()
{
	partition_selector_t part_sel = (partition_selector_t) Settings.InstallPartitions;
	return wbfs_estimate_disc(hdd, __ReadDVD, NULL, part_sel);
}

s32 Wbfs_Wbfs::GetFragList(u8 *id)
{
	//! Doesn't have to be called ".iso" just something different than .wbfs but with a dot.
	//! So that the code doesn't fail.
	return get_frag_list_for_file((char *) ".iso", id, GetFSType(), lba, hdd_sector_size[usbport]);
}
