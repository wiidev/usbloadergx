#include <stdio.h>
#include <string.h>
#include <ogcsys.h>

#include "partition.h"
#include "usbstorage.h"
#include "sdhc.h"
#include "utils.h"
#include "libwbfs/libwbfs.h"
#include "wbfs.h"

/* 'partition table' structure */
typedef struct {
    /* Zero bytes */
    u8 padding[446];

    /* Partition table entries */
    partitionEntry entries[MAX_PARTITIONS];
} ATTRIBUTE_PACKED partitionTable;

s32 Partition_GetEntries(partitionEntry *outbuf, u32 *outval) {
    static partitionTable table ATTRIBUTE_ALIGN(32);

    u32 cnt, sector_size;
    s32 ret;

    /* Get sector size */
    ret = USBStorage_GetCapacity(&sector_size);
    if (ret < 0)
        return ret;

    /* Read partition table */
    ret = USBStorage_ReadSectors(0, 1, &table);
    if (ret < 0)
        return ret;

    /* Swap endianess */
    for (cnt = 0; cnt < 4; cnt++) {
        partitionEntry *entry = &table.entries[cnt];

        entry->sector = swap32(entry->sector);
        entry->size   = swap32(entry->size);
    }

    /* Set partition entries */
    memcpy(outbuf, table.entries, sizeof(table.entries));

    /* Set sector size */
    *outval = sector_size;

    return 0;
}

s32 Partition_GetEntriesEx(partitionEntry *outbuf, u32 *outval, int *num)
{
	static partitionTable table ATTRIBUTE_ALIGN(32);
	partitionEntry *entry;

	u32 i, sector_size;
	s32 ret;
	int maxpart = *num;

	// Get sector size
	ret = USBStorage_GetCapacity(&sector_size);
	if (ret == 0) return -1;

	u32 ext = 0;
	u32 next = 0;

	// Read partition table
	ret = USBStorage_ReadSectors(0, 1, &table);
	if (!ret) return -1;
	/* Swap endianess */
	for (i = 0; i < 4; i++) {
		entry = &table.entries[i];
		entry->sector = swap32(entry->sector);
		entry->size   = swap32(entry->size);
		if (!ext && entry->type == 0x0f) ext = entry->sector;
	}
	/* Set partition entries */
	memcpy(outbuf, table.entries, sizeof(table.entries));
	/* Set sector size */
	*outval = sector_size;
	// num primary
	*num = 4;

	next = ext;
	// scan extended partition for logical
	if (ext) for(i=0; i<maxpart-4; i++) {
		ret = USBStorage_ReadSectors(next, 1, &table);
		if (!ret) break;
		entry = &table.entries[0];
		entry->sector = swap32(entry->sector);
		entry->size   = swap32(entry->size);
		if (entry->type && entry->size && entry->sector) {
			// rebase to abolute address
			entry->sector += next;
			// add logical
			memcpy(&outbuf[*num], entry, sizeof(*entry));
			(*num)++;
			// get next
			entry++;
			if (entry->type && entry->size && entry->sector) {
				next = ext + swap32(entry->sector);
			} else {
				break;
			}
		}

	}

	return 0;
}

char* part_type_data(int type)
{
	switch (type) {
		case 0x01: return "FAT12";
		case 0x04: return "FAT16";
		case 0x06: return "FAT16"; //+
		case 0x07: return "NTFS";
		case 0x0b: return "FAT32";
		case 0x0c: return "FAT32";
		case 0x0e: return "FAT16";
		case 0x82: return "LxSWP";
		case 0x83: return "LINUX";
		case 0x8e: return "LxLVM";
		case 0xa8: return "OSX";
		case 0xab: return "OSXBT";
		case 0xaf: return "OSXHF";
		case 0xe8: return "LUKS";
	}
	return NULL;
}

int get_fs_type(char *buf)
{
	// WBFS
	wbfs_head_t *head = (wbfs_head_t *)buf;
	if (head->magic == wbfs_htonl(WBFS_MAGIC)) return FS_TYPE_WBFS;
	// 55AA
	if (buf[0x1FE] == 0x55 && buf[0x1FF] == 0xAA) {
		// FAT
		if (memcmp(buf+0x36,"FAT",3) == 0) return FS_TYPE_FAT16;
		if (memcmp(buf+0x52,"FAT",3) == 0) return FS_TYPE_FAT32;
		// NTFS
		if (memcmp(buf+0x03,"NTFS",4) == 0) return FS_TYPE_NTFS;
	}
	return FS_TYPE_UNK;
}

bool is_type_fat(int type)
{
	return (type == FS_TYPE_FAT16 || type == FS_TYPE_FAT32);
}

s32 Partition_GetList(PartList *plist)
{
	partitionEntry *entry = NULL;
	PartInfo *pinfo = NULL;
	int i, ret;

	memset(plist, 0, sizeof(PartList));

	// Get partition entries
	plist->num = MAX_PARTITIONS_EX;
	ret = Partition_GetEntriesEx(plist->pentry, &plist->sector_size, &plist->num);
	if (ret < 0) {
		return -1;
	}
	char buf[plist->sector_size];

	// scan partitions for filesystem type
	for (i = 0; i < plist->num; i++) {
		pinfo = &plist->pinfo[i];
		entry = &plist->pentry[i];
		if (!entry->size) continue;
		if (!part_type_data(entry->type)) continue;
		if (!USBStorage_ReadSectors(entry->sector, 1, buf)) continue;
		pinfo->fs_type = get_fs_type(buf);
		if (pinfo->fs_type == FS_TYPE_WBFS) {
			plist->wbfs_n++;
			pinfo->wbfs_i = plist->wbfs_n;
		} else if (is_type_fat(pinfo->fs_type)) {
			plist->fat_n++;
			pinfo->fat_i = plist->fat_n;
		}
	}
	return 0;
}
