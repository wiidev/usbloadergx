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
	int fs_type;
	int part_fs;
	int wbfs_i;  // seq wbfs part index
	int fat_i;   // seq fat part index
	int ntfs_i;  // seq ntfs part index
	int index;
} PartInfo;

typedef struct
{
	int num;
	u32 sector_size;
	partitionEntry pentry[MAX_PARTITIONS_EX];
	int wbfs_n;
	int fat_n;
	int ntfs_n;
	PartInfo pinfo[MAX_PARTITIONS_EX];
} PartList;

/* Prototypes */
s32 Partition_GetEntries(u32 device, partitionEntry *outbuf, u32 *outval);
s32 Partition_GetEntriesEx(u32 device, partitionEntry *outbuf, u32 *outval, int *num);
bool Device_ReadSectors(u32 device, u32 sector, u32 count, void *buffer);
bool Device_WriteSectors(u32 device, u32 sector, u32 count, void *buffer);
s32 Partition_GetList(u32 device, PartList *plist);
int Partition_FixEXT(u32 device, int part);

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
