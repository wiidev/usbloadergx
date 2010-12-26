/**
 * gekko_io.c - Gekko style disk io functions.
 *
 * Copyright (c) 2009 Rhys "Shareese" Koedijk
 * Copyright (c) 2010 Dimok
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <gccore.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include <locale.h>

#include "gekko_io.h"
#include "bitops.h"
#include "ext2_fs.h"
#include "ext2fs.h"
#include "ext2_internal.h"
#include "disc_cache.h"
#include "mem_allocate.h"

#define DEV_FD(dev) ((gekko_fd *) dev->private_data)

/* Prototypes */
static s64 device_gekko_io_readbytes(io_channel dev, s64 offset, s64 count, void *buf);
static bool device_gekko_io_readsectors(io_channel dev, sec_t sector, sec_t numSectors, void* buffer);
static s64 device_gekko_io_writebytes(io_channel dev, s64 offset, s64 count, const void *buf);
static bool device_gekko_io_writesectors(io_channel dev, sec_t sector, sec_t numSectors, const void* buffer);

/**
 *
 */
static errcode_t device_gekko_io_open(const char *name, int flags, io_channel *dev)
{
    // Get the device driver descriptor
    gekko_fd *fd = DEV_FD((*dev));
    if (!fd) {
        errno = EBADF;
        return -1;
    }

    // Get the device interface
    const DISC_INTERFACE* interface = fd->interface;
    if (!interface) {
        errno = ENODEV;
        return -1;
    }

    // Start the device interface and ensure that it is inserted
    if (!interface->startup()) {
        ext2_log_trace("device failed to start\n");
        errno = EIO;
        return -1;
    }
    if (!interface->isInserted()) {
        ext2_log_trace("device media is not inserted\n");
        errno = EIO;
        return -1;
    }

    struct ext2_super_block	* super = (struct ext2_super_block	*) mem_alloc(SUPERBLOCK_SIZE);	//1024 bytes
    if(!super)
    {
        ext2_log_trace("no memory for superblock");
        errno = ENOMEM;
        return -1;
    }

    // Check that there is a valid EXT boot sector at the start of the device
    if (!interface->readSectors(fd->startSector+SUPERBLOCK_OFFSET/BYTES_PER_SECTOR, SUPERBLOCK_SIZE/BYTES_PER_SECTOR, super))
    {
        ext2_log_trace("read failure @ sector %d\n", fd->startSector);
        errno = EROFS;
        mem_free(super);
        return -1;
    }

    if(ext2fs_le16_to_cpu(super->s_magic) != EXT2_SUPER_MAGIC)
    {
        mem_free(super);
        errno = EROFS;
        return -1;
    }

    // Parse the boot sector
    fd->sectorSize = BYTES_PER_SECTOR;
    fd->offset = 0;
    fd->sectorCount = 0;

    switch(ext2fs_le32_to_cpu(super->s_log_block_size))
    {
        case 1:
            fd->sectorCount = (sec_t) ((u64) ext2fs_le32_to_cpu(super->s_blocks_count) * (u64) 2048 / (u64) BYTES_PER_SECTOR);
            break;
        case 2:
            fd->sectorCount = (sec_t) ((u64) ext2fs_le32_to_cpu(super->s_blocks_count) * (u64) 4096 / (u64) BYTES_PER_SECTOR);
            break;
        case 3:
            fd->sectorCount = (sec_t) ((u64) ext2fs_le32_to_cpu(super->s_blocks_count) * (u64) 8192 / (u64) BYTES_PER_SECTOR);
            break;
        default:
        case 0:
            fd->sectorCount = (sec_t) ((u64) ext2fs_le32_to_cpu(super->s_blocks_count) * (u64) 1024 / (u64) BYTES_PER_SECTOR);
            break;
    }

    mem_free(super);

    // Create the cache
    fd->cache = cache_constructor(fd->cachePageCount, fd->cachePageSize, interface, fd->startSector + fd->sectorCount, fd->sectorSize);

    return 0;
}

/**
 *  Flush data out and close volume
 */
static errcode_t device_gekko_io_close(io_channel dev)
{
    // Get the device driver descriptor
    gekko_fd *fd = DEV_FD(dev);
    if (!fd) {
        errno = EBADF;
        return -1;
    }

    if(!(dev->flags & EXT2_FLAG_RW))
        return 0;

    // Flush and destroy the cache (if required)
    if (fd->cache) {
        cache_flush(fd->cache);
        cache_destructor(fd->cache);
    }

    return 0;
}

/**
 *
 */
static s64 device_gekko_io_readbytes(io_channel dev, s64 offset, s64 count, void *buf)
{
    ext2_log_trace("dev %p, offset %lli, count %lli\n", dev, offset, count);
    // Get the device driver descriptor
    gekko_fd *fd = DEV_FD(dev);
    if (!fd) {
        errno = EBADF;
        return -1;
    }

    // Get the device interface
    const DISC_INTERFACE* interface = fd->interface;
    if (!interface) {
        errno = ENODEV;
        return -1;
    }

    if(offset < 0)
    {
        errno = EROFS;
        return -1;
    }

    if(!count)
        return 0;

    sec_t sec_start = (sec_t) fd->startSector;
    sec_t sec_count = 1;
    u32 buffer_offset = (u32) (offset % fd->sectorSize);
    u8 *buffer = NULL;

    // Determine the range of sectors required for this read
    if (offset > 0) {
        sec_start += (sec_t) floor((f64) offset / (f64) fd->sectorSize);
    }
    if (buffer_offset+count > fd->sectorSize) {
        sec_count = (sec_t) ceil((f64) (buffer_offset+count) / (f64) fd->sectorSize);
    }

    // Don't read over the partitions limit
    if(sec_start+sec_count > fd->startSector+fd->sectorCount)
    {
        ext2_log_trace("Error: read requested up to sector %lli while partition goes up to %lli\n", (s64) (sec_start+sec_count), (s64) (fd->startSector+fd->sectorCount));
        errno = EROFS;
        return -1;
    }

    // If this read happens to be on the sector boundaries then do the read straight into the destination buffer

    if((buffer_offset == 0) && (count % fd->sectorSize == 0))
    {
        // Read from the device
        ext2_log_trace("direct read from sector %d (%d sector(s) long)\n", sec_start, sec_count);
        if (!device_gekko_io_readsectors(dev, sec_start, sec_count, buf))
        {
            ext2_log_trace("direct read failure @ sector %d (%d sector(s) long)\n", sec_start, sec_count);
            errno = EIO;
            return -1;
        }
    // Else read into a buffer and copy over only what was requested
    }
    else
	{

        // Allocate a buffer to hold the read data
        buffer = (u8*)mem_alloc(sec_count * fd->sectorSize);
        if (!buffer) {
            errno = ENOMEM;
            return -1;
        }

        // Read from the device
        ext2_log_trace("buffered read from sector %d (%d sector(s) long)\n", sec_start, sec_count);
        ext2_log_trace("count: %d  sec_count:%d  fd->sectorSize: %d )\n", (u32)count, (u32)sec_count,(u32)fd->sectorSize);
        if (!device_gekko_io_readsectors(dev, sec_start, sec_count, buffer)) {
            ext2_log_trace("buffered read failure @ sector %d (%d sector(s) long)\n", sec_start, sec_count);
            mem_free(buffer);
            errno = EIO;
            return -1;
        }

        // Copy what was requested to the destination buffer
        memcpy(buf, buffer + buffer_offset, count);
        mem_free(buffer);

    }

    return count;
}

/**
 *
 */
static s64 device_gekko_io_writebytes(io_channel dev, s64 offset, s64 count, const void *buf)
{
    ext2_log_trace("dev %p, offset %lli, count %lli\n", dev, offset, count);

    // Get the device driver descriptor
    gekko_fd *fd = DEV_FD(dev);
    if (!fd) {
        errno = EBADF;
        return -1;
    }

    if(!(dev->flags & EXT2_FLAG_RW))
        return -1;

    // Get the device interface
    const DISC_INTERFACE* interface = fd->interface;
    if (!interface) {
        errno = ENODEV;
        return -1;
    }

    if(count < 0 || offset < 0) {
        errno = EROFS;
        return -1;
    }

    if(count == 0)
        return 0;

    sec_t sec_start = (sec_t) fd->startSector;
    sec_t sec_count = 1;
    u32 buffer_offset = (u32) (offset % fd->sectorSize);
    u8 *buffer = NULL;

    // Determine the range of sectors required for this write
    if (offset > 0) {
        sec_start += (sec_t) floor((f64) offset / (f64) fd->sectorSize);
    }
    if ((buffer_offset+count) > fd->sectorSize) {
        sec_count = (sec_t) ceil((f64) (buffer_offset+count) / (f64) fd->sectorSize);
    }

    // Don't write over the partitions limit
    if(sec_start+sec_count > fd->startSector+fd->sectorCount)
    {
        ext2_log_trace("Error: write requested up to sector %lli while partition goes up to %lli\n", (s64) (sec_start+sec_count), (s64) (fd->startSector+fd->sectorCount));
        errno = EROFS;
        return -1;
    }

    // If this write happens to be on the sector boundaries then do the write straight to disc
    if((buffer_offset == 0) && (count % fd->sectorSize == 0))
    {
        // Write to the device
        ext2_log_trace("direct write to sector %d (%d sector(s) long)\n", sec_start, sec_count);
        if (!device_gekko_io_writesectors(dev, sec_start, sec_count, buf)) {
            ext2_log_trace("direct write failure @ sector %d (%d sector(s) long)\n", sec_start, sec_count);
            errno = EIO;
            return -1;
        }
    // Else write from a buffer aligned to the sector boundaries
    }
    else
    {
        // Allocate a buffer to hold the write data
        buffer = (u8 *) mem_alloc(sec_count * fd->sectorSize);
        if (!buffer) {
            errno = ENOMEM;
            return -1;
        }
        // Read the first and last sectors of the buffer from disc (if required)
        // NOTE: This is done because the data does not line up with the sector boundaries,
        //       we just read in the buffer edges where the data overlaps with the rest of the disc
        if(buffer_offset != 0)
        {
            if (!device_gekko_io_readsectors(dev, sec_start, 1, buffer)) {
                ext2_log_trace("read failure @ sector %d\n", sec_start);
                mem_free(buffer);
                errno = EIO;
                return -1;
            }
        }
        if((buffer_offset+count) % fd->sectorSize != 0)
        {
            if (!device_gekko_io_readsectors(dev, sec_start + sec_count - 1, 1, buffer + ((sec_count-1) * fd->sectorSize))) {
                ext2_log_trace("read failure @ sector %d\n", sec_start + sec_count - 1);
                mem_free(buffer);
                errno = EIO;
                return -1;
            }
        }

        // Copy the data into the write buffer
        memcpy(buffer + buffer_offset, buf, count);

        // Write to the device
        ext2_log_trace("buffered write to sector %d (%d sector(s) long)\n", sec_start, sec_count);
        if (!device_gekko_io_writesectors(dev, sec_start, sec_count, buffer)) {
            ext2_log_trace("buffered write failure @ sector %d\n", sec_start);
            mem_free(buffer);
            errno = EIO;
            return -1;
        }

        // Free the buffer
        mem_free(buffer);
    }

    return count;
}


/**
 *  Read function wrap for I/O manager
 */
static errcode_t device_gekko_io_read64(io_channel dev, unsigned long long block, int count, void *buf)
{
    gekko_fd *fd = DEV_FD(dev);
    s64 size = (count < 0) ? -count : count * dev->block_size;
	fd->io_stats.bytes_read += size;
	ext2_loff_t location = ((ext2_loff_t) block * dev->block_size) + fd->offset;

    s64 read = device_gekko_io_readbytes(dev, location, size, buf);
    if(read != size)
        return EXT2_ET_SHORT_READ;
    else if(read < 0)
        return EXT2_ET_BLOCK_BITMAP_READ;

    return EXT2_ET_OK;
}

static errcode_t device_gekko_io_read(io_channel dev, unsigned long block, int count, void *buf)
{
    return device_gekko_io_read64(dev, block, count, buf);
}

/**
 *  Write function wrap for I/O manager
 */
static errcode_t device_gekko_io_write64(io_channel dev, unsigned long long block, int count, const void *buf)
{
    gekko_fd *fd = DEV_FD(dev);
    s64 size = (count < 0) ? -count : count * dev->block_size;
	fd->io_stats.bytes_written += size;

	ext2_loff_t location = ((ext2_loff_t) block * dev->block_size) + fd->offset;

    s64 writen = device_gekko_io_writebytes(dev, location, size, buf);
    if(writen != size)
        return EXT2_ET_SHORT_WRITE;
    else if(writen < 0)
        return EXT2_ET_BLOCK_BITMAP_WRITE;

    return EXT2_ET_OK;
}

static errcode_t device_gekko_io_write(io_channel dev, unsigned long block, int count, const void *buf)
{
    return device_gekko_io_write64(dev, block, count, buf);
}


static bool device_gekko_io_readsectors(io_channel dev, sec_t sector, sec_t numSectors, void* buffer)
{
    // Get the device driver descriptor
    gekko_fd *fd = DEV_FD(dev);
    if (!fd) {
        errno = EBADF;
        return false;
    }
    // Read the sectors from disc (or cache, if enabled)
    if (fd->cache)
        return cache_readSectors(fd->cache, sector, numSectors, buffer);
    else
        return fd->interface->readSectors(sector, numSectors, buffer);

    return false;
}

static bool device_gekko_io_writesectors(io_channel dev, sec_t sector, sec_t numSectors, const void* buffer)
{
    // Get the device driver descriptor
    gekko_fd *fd = DEV_FD(dev);
    if (!fd) {
        errno = EBADF;
        return false;
    }

    // Write the sectors to disc (or cache, if enabled)
    if (fd->cache)
        return cache_writeSectors(fd->cache, sector, numSectors, buffer);
    else
        return fd->interface->writeSectors(sector, numSectors, buffer);

    return false;
}

/**
 *
 */
static errcode_t device_gekko_io_sync(io_channel dev)
{
	gekko_fd *fd = DEV_FD(dev);
    ext2_log_trace("dev %p\n", dev);

    // Check that the device can be written to
    if(!(dev->flags & EXT2_FLAG_RW))
        return -1;

    // Flush any sectors in the disc cache (if required)
    if (fd->cache) {
        if (!cache_flush(fd->cache)) {
            errno = EIO;
            return EXT2_ET_BLOCK_BITMAP_WRITE;
        }
    }

    return EXT2_ET_OK;
}

/**
 *
 */
static errcode_t device_gekko_io_stat(io_channel dev, io_stats *stats)
{
	EXT2_CHECK_MAGIC(dev, EXT2_ET_MAGIC_IO_CHANNEL);
	gekko_fd *fd = DEV_FD(dev);

	if (stats)
		*stats = &fd->io_stats;

	return EXT2_ET_OK;
}

static errcode_t device_gekko_set_blksize(io_channel dev, int blksize)
{
	EXT2_CHECK_MAGIC(dev, EXT2_ET_MAGIC_IO_CHANNEL);

	if (dev->block_size != blksize)
	{
		dev->block_size = blksize;

        return device_gekko_io_sync(dev);
	}

	return EXT2_ET_OK;
}

/**
 * Set options.
 */
static errcode_t device_gekko_set_option(io_channel dev, const char *option, const char *arg)
{
	unsigned long long tmp;
	char *end;

    gekko_fd *fd = DEV_FD(dev);
    if (!fd) {
        errno = EBADF;
        return -1;
    }

	EXT2_CHECK_MAGIC(dev, EXT2_ET_MAGIC_IO_CHANNEL);

	if (!strcmp(option, "offset")) {
		if (!arg)
			return EXT2_ET_INVALID_ARGUMENT;

		tmp = strtoull(arg, &end, 0);
		if (*end)
			return EXT2_ET_INVALID_ARGUMENT;
		fd->offset = tmp;
		if (fd->offset < 0)
			return EXT2_ET_INVALID_ARGUMENT;
		return 0;
	}
	return EXT2_ET_INVALID_ARGUMENT;
}

static errcode_t device_gekko_discard(io_channel channel, unsigned long long block, unsigned long long count)
{
    //!TODO as soon as it is implemented in the official lib
    return 0;
}

/**
 * Device operations for working with gekko style devices and files.
 */
const struct struct_io_manager struct_gekko_io_manager =
{
	EXT2_ET_MAGIC_IO_MANAGER,
	"Wii/GC I/O Manager",
	device_gekko_io_open,
	device_gekko_io_close,
	device_gekko_set_blksize,
	device_gekko_io_read,
	device_gekko_io_write,
	device_gekko_io_sync,
	0,
	device_gekko_set_option,
	device_gekko_io_stat,
	device_gekko_io_read64,
	device_gekko_io_write64,
	device_gekko_discard,
};

io_manager gekko_io_manager = (io_manager) &struct_gekko_io_manager;
