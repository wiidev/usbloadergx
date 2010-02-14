#ifndef _PARTITION_H_
#define _PARTITION_H_

#ifdef __cplusplus
extern "C" {
#endif

/* 'partition entry' structure */
typedef struct {
	/* Boot indicator */
	u8 boot;

	/* Starting CHS */
	u8 start[3];

	/* Partition type */
	u8 type;

	/* Ending CHS */
	u8 end[3];

	/* Partition sector */
	u32 sector;

	/* Partition size */
	u32 size;
} ATTRIBUTE_PACKED partitionEntry;

/* Constants */
#define MAX_PARTITIONS		4
#define MAX_PARTITIONS_EX	10

#define FS_TYPE_UNK   0
#define FS_TYPE_FAT16 1
#define FS_TYPE_FAT32 2
#define FS_TYPE_NTFS  3
#define FS_TYPE_WBFS  4

typedef struct
{
	u8 fs_type;
	u8 part_fs;
	u8 wbfs_i;  // seq wbfs part index
	u8 fat_i;   // seq fat part index
	u8 ntfs_i;  // seq ntfs part index
	u8 index;
} PartInfo;

typedef struct
{
	u8 num;
	u32 sector_size;
	partitionEntry pentry[MAX_PARTITIONS_EX];
	u8 wbfs_n;
	u8 fat_n;
	u8 ntfs_n;
	PartInfo pinfo[MAX_PARTITIONS_EX];
} PartList;

/* Prototypes */
s32 Partition_GetEntries(u32 device, partitionEntry *outbuf, u32 *outval);
s32 Partition_GetEntriesEx(u32 device, partitionEntry *outbuf, u32 *outval, u8 *num);
bool Device_ReadSectors(u32 device, u32 sector, u32 count, void *buffer);
bool Device_WriteSectors(u32 device, u32 sector, u32 count, void *buffer);
s32 Partition_GetList(u32 device, PartList *plist);
int Partition_FixEXT(u32 device, u8 part);

bool  part_is_extended(int type);
bool  part_is_data(int type);
char* part_type_data(int type);
char* part_type_name(int type);
bool  part_valid_data(partitionEntry *entry);
int   get_fs_type(void *buf);
bool  is_type_fat(int type);
char* get_fs_name(int i);

#ifdef __cplusplus
}
#endif

#endif
