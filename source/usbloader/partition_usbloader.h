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
		int wbfs_i;  // seq wbfs part index
		int fat_i;   // seq fat part index
	} PartInfo;

	typedef struct
	{
		int num;
		u32 sector_size;
		partitionEntry pentry[MAX_PARTITIONS_EX];
		int wbfs_n;
		int fat_n;
		PartInfo pinfo[MAX_PARTITIONS_EX];
	} PartList;

    /* Prototypes */
    s32 Partition_GetEntries(partitionEntry *, u32 *);
	s32 Partition_GetEntriesEx(partitionEntry *, u32 *, int *);
	s32 Partition_GetList(PartList *plist);

#ifdef __cplusplus
}
#endif

#endif
