#ifndef PARTITIONS_H_
#define PARTITIONS_H_

#include <gccore.h>
#include "bitops.h"

#define MBR_SIGNATURE                       ext2fs_cpu_to_le16(0xAA55)
#define EBR_SIGNATURE                       ext2fs_cpu_to_le16(0xAA55)

#define PARTITION_STATUS_BOOTABLE           0x80 /* Bootable (active) */

#define PARTITION_TYPE_EMPTY                0x00 /* Empty */
#define PARTITION_TYPE_DOS33_EXTENDED       0x05 /* DOS 3.3+ extended partition */
#define PARTITION_TYPE_WIN95_EXTENDED       0x0F /* Windows 95 extended partition */
#define PARTITION_TYPE_LINUX                0x83 /* EXT2/3/4 */

/**
 * PRIMARY_PARTITION - Block device partition record
 */
typedef struct _PARTITION_RECORD {
    u8 status;                              /* Partition status; see above */
    u8 chs_start[3];                        /* Cylinder-head-sector address to first block of partition */
    u8 type;                                /* Partition type; see above */
    u8 chs_end[3];                          /* Cylinder-head-sector address to last block of partition */
    u32 lba_start;                          /* Local block address to first sector of partition */
    u32 block_count;                        /* Number of blocks in partition */
} __attribute__((__packed__)) PARTITION_RECORD;

/**
 * MASTER_BOOT_RECORD - Block device master boot record
 */
typedef struct _MASTER_BOOT_RECORD {
    u8 code_area[446];                      /* Code area; normally empty */
    PARTITION_RECORD partitions[4];         /* 4 primary partitions */
    u16 signature;                          /* MBR signature; 0xAA55 */
} __attribute__((__packed__)) MASTER_BOOT_RECORD;

/**
 * EXTENDED_PARTITION - Block device extended boot record
 */
typedef struct _EXTENDED_BOOT_RECORD {
    u8 code_area[446];                      /* Code area; normally empty */
    PARTITION_RECORD partition;             /* Primary partition */
    PARTITION_RECORD next_ebr;              /* Next extended boot record in the chain */
    u8 reserved[32];                        /* Normally empty */
    u16 signature;                          /* EBR signature; 0xAA55 */
} __attribute__((__packed__)) EXTENDED_BOOT_RECORD;

#endif
