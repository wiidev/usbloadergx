#include "fatfile.h"

#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#include "cache.h"
#include "file_allocation_table.h"
#include "bit_ops.h"
#include "filetime.h"
#include "lock.h"
#include "fatfile_frag.h"

int _FAT_get_fragments (const char *path, _fat_frag_append_t append_fragment, void *callback_data)
{
	struct _reent r;
	FILE_STRUCT file;
	PARTITION* partition;
	u32 cluster;
	u32 sector;
	u32 offset; // in sectors
	u32 size;   // in sectors
	int ret = -1;
	int fd;

	fd = _FAT_open_r (&r, &file, path, O_RDONLY, 0);
	if (fd == -1) return -1;
	if (fd != (int)&file) return -1;

	partition = file.partition;
	_FAT_lock(&partition->lock);

	size = file.filesize / BYTES_PER_READ;
	cluster = file.startCluster;
	offset = 0;

	do {
		if (!_FAT_fat_isValidCluster(partition, cluster)) {
			// invalid cluster
			goto out;
		}
		// add cluster to fileinfo
		sector = _FAT_fat_clusterToSector(partition, cluster);
		if (append_fragment(callback_data, offset, sector, partition->sectorsPerCluster)) {
			// too many fragments
			goto out;
		}
		offset += partition->sectorsPerCluster;
		cluster = _FAT_fat_nextCluster (partition, cluster);
	} while (offset < size);

	// set size
	append_fragment(callback_data, size, 0, 0);
	// success
	ret = 0;

	out:
	_FAT_unlock(&partition->lock);
	_FAT_close_r(&r, fd);
	return ret;
}
