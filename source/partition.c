#include <stdio.h>
#include <string.h>
#include <ogcsys.h>

#include "partition.h"
#include "usbstorage.h"
#include "utils.h"

/* 'partition table' structure */
typedef struct {
	/* Zero bytes */
	u8 padding[446];

	/* Partition table entries */
	partitionEntry entries[MAX_PARTITIONS];
} ATTRIBUTE_PACKED partitionTable;


s32 Partition_GetEntries(partitionEntry *outbuf, u32 *outval)
{
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
