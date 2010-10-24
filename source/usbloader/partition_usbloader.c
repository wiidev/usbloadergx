// Modified by oggzee

#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>

#include "partition_usbloader.h"
#include "sdhc.h"
#include "usbstorage2.h"
#include "utils.h"
#include "wbfs.h"
#include "libs/libwbfs/libwbfs.h"

/* 'partition table' structure */
typedef struct
{
    /* Zero bytes */
    u8 padding[446];

    /* Partition table entries */
    partitionEntry entries[MAX_PARTITIONS];
}ATTRIBUTE_PACKED partitionTable;

s32 Partition_GetEntries(u32 device, partitionEntry *outbuf, u32 *outval)
{
    static partitionTable table ATTRIBUTE_ALIGN( 32 );

    u32 cnt, sector_size;
    s32 ret;

    /* Read from specified device */
    switch (device)
    {
        case WBFS_DEVICE_USB:
        {
            /* Get sector size */
            ret = USBStorage2_GetCapacity(&sector_size);
            if (ret == 0) return -1;

            /* Read partition table */
            ret = USBStorage2_ReadSectors(0, 1, &table);
            if (ret < 0) return ret;

            break;
        }

        case WBFS_DEVICE_SDHC:
        {
            /* SDHC sector size */
            sector_size = SDHC_SECTOR_SIZE;

            /* Read partition table */
            ret = SDHC_ReadSectors(0, 1, &table);
            if (!ret) return -1;

            break;
        }

        default:
            return -1;
    }

    /* Swap endianess */
    for (cnt = 0; cnt < 4; cnt++)
    {
        partitionEntry *entry = &table.entries[cnt];

        entry->sector = swap32(entry->sector);
        entry->size = swap32(entry->size);
    }

    /* Set partition entries */
    memcpy(outbuf, table.entries, sizeof(table.entries));

    /* Set sector size */
    *outval = sector_size;

    return 0;
}

bool Device_ReadSectors(u32 device, u32 sector, u32 count, void *buffer)
{
    s32 ret;

    /* Read from specified device */
    switch (device)
    {
        case WBFS_DEVICE_USB:
            ret = USBStorage2_ReadSectors(sector, count, buffer);
            if (ret < 0) return false;
            return true;

        case WBFS_DEVICE_SDHC:
            return SDHC_ReadSectors(sector, count, buffer);
    }

    return false;
}

bool Device_WriteSectors(u32 device, u32 sector, u32 count, void *buffer)
{
    s32 ret;

    /* Read from specified device */
    switch (device)
    {
        case WBFS_DEVICE_USB:
            ret = USBStorage2_WriteSectors(sector, count, buffer);
            if (ret < 0) return false;
            return true;

        case WBFS_DEVICE_SDHC:
            return SDHC_WriteSectors(sector, count, buffer);
    }

    return false;
}

s32 Partition_GetEntriesEx(u32 device, partitionEntry *outbuf, u32 *psect_size, u8 *num)
{
    static u8 Buffer[sizeof(partitionTable)] ATTRIBUTE_ALIGN( 32 );
    partitionTable *table = (partitionTable *) Buffer;
    partitionEntry *entry;

    u32 i, sector_size;
    s32 ret;
    int maxpart = *num;

    // Get sector size
    switch (device)
    {
        case WBFS_DEVICE_USB:
            ret = USBStorage2_GetCapacity(&sector_size);
            if (ret == 0) return -1;
            break;
        case WBFS_DEVICE_SDHC:
            sector_size = SDHC_SECTOR_SIZE;
            break;
        default:
            return -1;
    }
    /* Set sector size */
    *psect_size = sector_size;

    u32 ext = 0;
    u32 next = 0;

    // Read partition table
    ret = Device_ReadSectors(device, 0, 1, table);
    if (!ret) return -1;
    // Check if it's a RAW WBFS disc, without partition table
    if (get_fs_type((u8 *) table) == FS_TYPE_WBFS)
    {
        memset(outbuf, 0, sizeof(table->entries));
        wbfs_head_t * head = (wbfs_head_t *) Buffer;
        outbuf->size = wbfs_ntohl( head->n_hd_sec );
        *num = 1;
        return 0;
    }
    /* Swap endianess */
    for (i = 0; i < 4; i++)
    {
        entry = &table->entries[i];
        entry->sector = swap32(entry->sector);
        entry->size = swap32(entry->size);
        if (!ext && part_is_extended(entry->type))
        {
            ext = entry->sector;
        }
    }
    /* Set partition entries */
    memcpy(outbuf, table->entries, sizeof(table->entries));
    // num primary
    *num = 4;

    if (!ext) return 0;

    next = ext;
    // scan extended partition for logical
    for (i = 0; i < maxpart - 4; i++)
    {
        ret = Device_ReadSectors(device, next, 1, table);
        if (!ret) break;
        if (i == 0)
        {
            // handle the invalid scenario where wbfs is on an EXTENDED
            // partition instead of on the Logical inside Extended.
            if (get_fs_type((u8 *) table) == FS_TYPE_WBFS) break;
        }
        entry = &table->entries[0];
        entry->sector = swap32(entry->sector);
        entry->size = swap32(entry->size);
        if (entry->type && entry->size && entry->sector)
        {
            // rebase to abolute address
            entry->sector += next;
            // add logical
            memcpy(&outbuf[*num], entry, sizeof(*entry));
            (*num)++;
            // get next
            entry++;
            if (entry->type && entry->size && entry->sector)
            {
                next = ext + swap32(entry->sector);
            }
            else
            {
                break;
            }
        }
    }

    return 0;
}

bool part_is_extended(int type)
{
    if (type == 0x05) return true;
    if (type == 0x0f) return true;
    return false;
}

bool part_is_data(int type)
{
    if (type && !part_is_extended(type)) return true;
    return false;
}

bool part_valid_data(partitionEntry *entry)
{
    if (entry->size && entry->type && entry->sector)
    {
        return part_is_data(entry->type);
    }
    return false;
}

char* part_type_data(int type)
{
    switch (type)
    {
        case 0x01:
            return "FAT12";
        case 0x04:
            return "FAT16";
        case 0x06:
            return "FAT16"; //+
        case 0x07:
            return "NTFS";
        case 0x0b:
            return "FAT32";
        case 0x0c:
            return "FAT32";
        case 0x0e:
            return "FAT16";
        case 0x82:
            return "LxSWP";
        case 0x83:
            return "LINUX";
        case 0x8e:
            return "LxLVM";
        case 0xa8:
            return "OSX";
        case 0xab:
            return "OSXBT";
        case 0xaf:
            return "OSXHF";
        case 0xe8:
            return "LUKS";
    }
    return NULL;
}

char *part_type_name(int type)
{
    static char unk[8];
    if (type == 0) return "UNUSED";
    if (part_is_extended(type)) return "EXTEND";
    char *p = part_type_data(type);
    if (p) return p;
    sprintf(unk, "UNK-%02x", type);
    return unk;
}

int get_fs_type(u8 *buff)
{
    // WBFS
    wbfs_head_t *head = (wbfs_head_t *) buff;
    if (head->magic == wbfs_htonl( WBFS_MAGIC )) return FS_TYPE_WBFS;
    // 55AA
    if(*((u16 *) (buff + 0x1FE)) == 0x55AA)
    {
        // FAT
        if (memcmp(buff + 0x36, "FAT", 3) == 0) return FS_TYPE_FAT16;
        if (memcmp(buff + 0x52, "FAT", 3) == 0) return FS_TYPE_FAT32;
        // NTFS
        if (memcmp(buff + 0x03, "NTFS", 4) == 0) return FS_TYPE_NTFS;
    }
    return FS_TYPE_UNK;
}

int get_part_fs(int fs_type)
{
    switch (fs_type)
    {
        case FS_TYPE_FAT32:
            return PART_FS_FAT;
        case FS_TYPE_NTFS:
            return PART_FS_NTFS;
        case FS_TYPE_WBFS:
            return PART_FS_WBFS;
        default:
            return -1;
    }
}

bool is_type_fat(int type)
{
    return (type == FS_TYPE_FAT16 || type == FS_TYPE_FAT32);
}

char *get_fs_name(int i)
{
    switch (i)
    {
        case FS_TYPE_FAT16:
            return "FAT16";
        case FS_TYPE_FAT32:
            return "FAT32";
        case FS_TYPE_NTFS:
            return "NTFS";
        case FS_TYPE_WBFS:
            return "WBFS";
    }
    return "";
}

s32 Partition_GetList(u32 device, PartList *plist)
{
    partitionEntry *entry = NULL;
    PartInfo *pinfo = NULL;
    int i, ret;

    memset(plist, 0, sizeof(PartList));

    // Get partition entries
    plist->num = MAX_PARTITIONS_EX;
    ret = Partition_GetEntriesEx(device, plist->pentry, &plist->sector_size, &plist->num);
    if (ret < 0)
    {
        return -1;
    }
    // check for RAW WBFS disc
    if (plist->num == 1)
    {
        pinfo = &plist->pinfo[0];
        entry = &plist->pentry[0];
        plist->wbfs_n = 1;
        pinfo->wbfs_i = pinfo->index = 1;
        return 0;
    }

    char buf[plist->sector_size];

    // scan partitions for filesystem type
    for (i = 0; i < plist->num; i++)
    {
        pinfo = &plist->pinfo[i];
        entry = &plist->pentry[i];
        if (!entry->size) continue;
        if (!entry->type) continue;
        if (!entry->sector) continue;
        // even though wrong, it's possible WBFS is on an extended part.
        //if (!part_is_data(entry->type)) continue;
        if (!Device_ReadSectors(device, entry->sector, 1, buf)) continue;
        pinfo->fs_type = get_fs_type((u8 *) buf);
        if (pinfo->fs_type == FS_TYPE_WBFS)
        {
            // multiple wbfs on sdhc not supported
            if (device == WBFS_DEVICE_SDHC && (plist->wbfs_n > 1 || i > 4)) continue;
            plist->wbfs_n++;
            pinfo->wbfs_i = pinfo->index = plist->wbfs_n;
        }
        else if (is_type_fat(pinfo->fs_type))
        {
            plist->fat_n++;
            pinfo->fat_i = pinfo->index = plist->fat_n;
        }
        else if (pinfo->fs_type == FS_TYPE_NTFS)
        {
            plist->ntfs_n++;
            pinfo->ntfs_i = pinfo->index = plist->ntfs_n;
        }
        pinfo->part_fs = get_part_fs(pinfo->fs_type);
    }
    return 0;
}

int Partition_FixEXT(u32 device, u8 part)
{
    static partitionTable table ATTRIBUTE_ALIGN( 32 );
    int ret;

    if (part > 3) return -1;
    // Read partition table
    ret = Device_ReadSectors(device, 0, 1, &table);
    if (!ret) return -1;
    if (part_is_extended(table.entries[part].type))
    {
        table.entries[part].type = 0x0b; // FAT32
        ret = Device_WriteSectors(device, 0, 1, &table);
        if (!ret) return -1;
        return 0;
    }
    return -1;
}
