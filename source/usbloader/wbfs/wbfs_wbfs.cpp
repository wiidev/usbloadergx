#include "wbfs_wbfs.h"
#include "prompts/ProgressWindow.h"
#include "settings/CSettings.h"
#include "usbloader/wbfs.h"
#include "wbfs_rw.h"

extern int wbfs_part_fs;

s32 Wbfs_Wbfs::Open()
{
    wbfs_t *part = NULL;

    u8 buffer[512];
    memset(buffer, 0, sizeof(buffer));

    wbfs_head_t *head = (wbfs_head_t *) buffer;

    if(readCallback(NULL, lba, 1, buffer) < 0)
        return -1;

    if (head->magic != wbfs_htonl(WBFS_MAGIC))
        return -1;

    /* Open partition */
    part = wbfs_open_partition(readCallback, writeCallback, NULL, 512, head->n_hd_sec, lba, 0);
    if (!part) return -1;

    /* Close current hard disk */
    Close();
    hdd = part;

    wbfs_part_fs = PART_FS_WBFS;

    return 0;
}

void Wbfs_Wbfs::Close()
{
    if (hdd)
    {
        wbfs_close(hdd);
        hdd = NULL;
    }

    wbfs_part_fs = -1;
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
    wbfs_t *partition = NULL;

    /* Reset partition */
    partition = wbfs_open_partition(readCallback, writeCallback, NULL, 512, size, lba, 1);
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
    return get_frag_list_for_file((char *) ".iso", id);
}
