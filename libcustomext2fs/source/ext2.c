/**
 * ext2file.c - devoptab file routines for EXT2-based devices.
 *
 * Copyright (c) 2006 Michael "Chishm" Chisholm
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
#include <errno.h>
#include <string.h>
#include "ext2_fs.h"
#include "ext2fs.h"
#include "ext2_internal.h"
#include "gekko_io.h"
#include "mem_allocate.h"
#include "partitions.h"

bool ext2Mount(const char *name, const DISC_INTERFACE *interface, sec_t startSector, u32 cachePageCount, u32 cachePageSize, u32 flags)
{
    errcode_t retval = -1;
    ext2_filsys fs = NULL;
    io_channel io_chan = NULL;
    gekko_fd *fd = NULL;
    ext2_vd * vd = NULL;

    // Sanity check
    if (!name || !interface)
    {
        errno = EINVAL;
        return false;
    }

    // Allocate the device driver descriptor
    fd = (gekko_fd*) mem_alloc(sizeof(gekko_fd));
    if (!fd)
		goto cleanup;

    memset(fd, 0, sizeof(gekko_fd));

    // Setup the device driver descriptor
    fd->interface = interface;
    fd->startSector = startSector;
    fd->sectorSize = 0;
    fd->sectorCount = 0;
    fd->cachePageCount = cachePageCount;
    fd->cachePageSize = cachePageSize;

    fs = mem_alloc(sizeof(struct struct_ext2_filsys));
	if (!fs)
	{
        ext2_log_trace("no memory for fs\n");
        errno = ENOMEM;
        goto cleanup;
    }

	memset(fs, 0, sizeof(struct struct_ext2_filsys));

	io_chan = mem_alloc(sizeof(struct struct_io_channel));
	if (!io_chan)
	{
        ext2_log_trace("no memory for io_chan\n");
        errno = ENOMEM;
        goto cleanup;
    }

	memset(io_chan, 0, sizeof(struct struct_io_channel));

    io_chan->magic = EXT2_ET_MAGIC_IO_CHANNEL;
	io_chan->manager = gekko_io_manager;
	io_chan->name = strdup(name);
	if(!io_chan->name) goto cleanup;
	io_chan->block_size = 1024;
	io_chan->read_error = 0;
	io_chan->write_error = 0;
	io_chan->refcount = 1;
    io_chan->private_data = fd;
	io_chan->flags = flags;

    retval = ext2fs_open2(io_chan->name, 0, io_chan->flags, 0, 0, &io_chan, &fs);
    if(retval)
    {
        ext2_log_trace("error mounting %i\n", (int) retval);
		goto cleanup;
    }

    vd = mem_alloc(sizeof(ext2_vd));
    if(!vd)
    {
        ext2_log_trace("no memory for vd\n");
		goto cleanup;
    }

    // Initialise the volume descriptor
    ext2InitVolume(vd);
    vd->fs = fs;
    vd->io = io_chan;
    vd->root = EXT2_ROOT_INO;

    // Add the device to the devoptab table
    if (ext2AddDevice(name, vd)) {
        ext2DeinitVolume(vd);
        goto cleanup;
    }

    return true;

cleanup:
	if(fd)
		mem_free(fd);
	if(io_chan)
		mem_free(io_chan);
    if(vd)
        mem_free(vd);
	if(fs)
	{
        ext2fs_close(fs);
		ext2fs_free(fs);
	}

    return false;
}

void ext2Unmount(const char *name)
{
    ext2_vd *vd = NULL;

    // Get the devices volume descriptor
    vd = ext2GetVolume(name);
    if (!vd)
        return;

    // Remove the device from the devoptab table
    ext2RemoveDevice(name);

    // Deinitialise the volume descriptor
    ext2DeinitVolume(vd);

    // Unmount the volume
    ext2fs_close(vd->fs);
    ext2fs_free(vd->fs);

    //Free the io manager
    mem_free(vd->io->private_data);
    mem_free(vd->io);

    // Free the volume descriptor
    mem_free(vd);

    return;
}


const char *ext2GetVolumeName (const char *name)
{
    if (!name) {
        errno = EINVAL;
        return NULL;
    }

    // Get the devices volume descriptor
    ext2_vd *vd = ext2GetVolume(name);
    if (!vd) {
        errno = ENODEV;
        return NULL;
    }

    return vd->fs->super->s_volume_name;
}

bool ext2SetVolumeName (const char *name, const char *volumeName)
{
    // Sanity check
    if (!name || !volumeName) {
        errno = EINVAL;
        return false;
    }

    // Get the devices volume descriptor
    ext2_vd *vd = ext2GetVolume(name);
    if (!vd) {
        errno = ENODEV;
        return false;
    }

    // Lock
    ext2Lock(vd);
    int i;
    for(i = 0; i < 15 && *volumeName != 0; ++i, volumeName++)
        vd->fs->super->s_volume_name[i] = *volumeName;

    vd->fs->super->s_volume_name[i] = '\0';

    ext2fs_mark_super_dirty(vd->fs);

    ext2Sync(vd, NULL);

    // Unlock
    ext2Unlock(vd);

    return true;
}

int ext2FindPartitions (const DISC_INTERFACE *interface, sec_t **out_partitions)
{
    MASTER_BOOT_RECORD mbr;
    PARTITION_RECORD *partition = NULL;
    int partition_count = 0, ret = -1;
    sec_t part_lba = 0;
    sec_t * partitions = NULL;
    int i;

    union {
        u8 buffer[512];
        MASTER_BOOT_RECORD mbr;
        EXTENDED_BOOT_RECORD ebr;
    } sector;

    // Sanity check
    if (!interface) {
        errno = EINVAL;
        return -1;
    }

    if(!out_partitions) {
        errno = EINVAL;
        return -1;
    }

    // Start the device and check that it is inserted
    if (!interface->startup()) {
        errno = EIO;
        return -1;
    }
    if (!interface->isInserted()) {
        errno = EIO;
        return 0;
    }

    struct ext2_super_block	* super = (struct ext2_super_block	*) malloc(SUPERBLOCK_SIZE);	//1024 bytes
    if(!super)
    {
        ext2_log_trace("no memory for superblock");
        errno = ENOMEM;
        return -1;
    }

    partitions = (sec_t *) malloc(sizeof(sec_t));
    if(!partitions)
    {
        ext2_log_trace("no memory for partitions");
        errno = ENOMEM;
        mem_free(super);
        return -1;
    }
    // Read the first sector on the device
    if (!interface->readSectors(0, 1, &sector.buffer)) {
        errno = EIO;
        mem_free(partitions);
        mem_free(super);
        return -1;
    }

    // If this is the devices master boot record
    if (sector.mbr.signature == MBR_SIGNATURE)
    {
        memcpy(&mbr, &sector, sizeof(MASTER_BOOT_RECORD));

        // Search the partition table for all EXT2/3/4 partitions (max. 4 primary partitions)
        for (i = 0; i < 4; i++)
        {
            partition = &mbr.partitions[i];
            part_lba = ext2fs_le32_to_cpu(mbr.partitions[i].lba_start);

            // Figure out what type of partition this is
            switch (partition->type)
            {
                // Ignore empty partitions
                case PARTITION_TYPE_EMPTY:
                    continue;

                // EXT2/3/4 partition
                case PARTITION_TYPE_LINUX:

                    // Read and validate the EXT partition
                    if (interface->readSectors(part_lba+SUPERBLOCK_OFFSET/BYTES_PER_SECTOR, SUPERBLOCK_SIZE/BYTES_PER_SECTOR, super))
                    {
                        if (ext2fs_le16_to_cpu(super->s_magic) == EXT2_SUPER_MAGIC)
                        {
                            partition_count++;
                            sec_t * tmp = (sec_t *) realloc(partitions, partition_count*sizeof(sec_t));
                            if(!tmp) goto cleanup;
                            partitions = tmp;
                            partitions[partition_count-1] = part_lba;
                        }
                    }

                    break;

                // DOS 3.3+ or Windows 95 extended partition
                case PARTITION_TYPE_DOS33_EXTENDED:
                case PARTITION_TYPE_WIN95_EXTENDED:
                {
                    ext2_log_trace("Partition %i: Claims to be Extended\n", i + 1);

                    // Walk the extended partition chain, finding all EXT partitions within it
                    sec_t ebr_lba = part_lba;
                    sec_t next_erb_lba = 0;
                    do {
                        // Read and validate the extended boot record
                        if (interface->readSectors(ebr_lba + next_erb_lba, 1, &sector))
                        {
                            if (sector.ebr.signature == EBR_SIGNATURE)
                            {
                                ext2_log_trace("Logical Partition @ %d: %s type 0x%x\n", ebr_lba + next_erb_lba,
                                               sector.ebr.partition.status == PARTITION_STATUS_BOOTABLE ? "bootable (active)" : "non-bootable",
                                               sector.ebr.partition.type);

                                // Get the start sector of the current partition
                                // and the next extended boot record in the chain
                                part_lba = ebr_lba + next_erb_lba + ext2fs_le32_to_cpu(sector.ebr.partition.lba_start);
                                next_erb_lba = ext2fs_le32_to_cpu(sector.ebr.next_ebr.lba_start);

                                // Check if this partition has a valid EXT boot record
                                if (interface->readSectors(part_lba+SUPERBLOCK_OFFSET/BYTES_PER_SECTOR, SUPERBLOCK_SIZE/BYTES_PER_SECTOR, super))
                                {
                                    if (ext2fs_le16_to_cpu(super->s_magic) == EXT2_SUPER_MAGIC)
                                    {
                                        partition_count++;
                                        sec_t * tmp = (sec_t *) realloc(partitions, partition_count*sizeof(sec_t));
                                        if(!tmp) goto cleanup;
                                        partitions = tmp;
                                        partitions[partition_count-1] = part_lba;
                                    }
                                }
                            }
                            else
                                next_erb_lba = 0;
                        }

                    } while (next_erb_lba);

                    break;

                }

                // Unknown or unsupported partition type
                default:
                {
                    // Check if this partition has a valid EXT boot record anyway,
                    // it might be misrepresented due to a lazy partition editor
                    if (interface->readSectors(part_lba+SUPERBLOCK_OFFSET/BYTES_PER_SECTOR, SUPERBLOCK_SIZE/BYTES_PER_SECTOR, super))
                    {
                        if (ext2fs_le16_to_cpu(super->s_magic) == EXT2_SUPER_MAGIC)
                        {
                            partition_count++;
                            sec_t * tmp = (sec_t *) realloc(partitions, partition_count*sizeof(sec_t));
                            if(!tmp) goto cleanup;
                            partitions = tmp;
                            partitions[partition_count-1] = part_lba;
                        }
                    }
                    break;
                }
            }
        }

    // Else it is assumed this device has no master boot record
    }
    else
    {
        ext2_log_trace("No Master Boot Record was found!\n");

        // As a last-ditched effort, search the first 64 sectors of the device for stray EXT partitions
        for (i = 1; i < 64; i++)
        {
            if (interface->readSectors(i+SUPERBLOCK_OFFSET/BYTES_PER_SECTOR, SUPERBLOCK_SIZE/BYTES_PER_SECTOR, super))
            {
                if (ext2fs_le16_to_cpu(super->s_magic) == EXT2_SUPER_MAGIC)
                {
                    partition_count++;
                    sec_t * tmp = (sec_t *) realloc(partitions, partition_count*sizeof(sec_t));
                    if(!tmp) goto cleanup;
                    partitions = tmp;
                    partitions[partition_count-1] = i;
                }
            }
        }

    }

    // Return the found partitions (if any)
    if (partition_count > 0)
    {
        *out_partitions = partitions;
        ret = partition_count;
    }

cleanup:

    if(partitions && partition_count == 0)
        mem_free(partitions);
    if(super)
        mem_free(super);

    return ret;
}

