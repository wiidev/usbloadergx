/**
 * ext2_internal.c - Internal support routines for EXT2-based devices.
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

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "ext2_internal.h"
#include "ext2dir.h"
#include "ext2file.h"
#include "gekko_io.h"

// EXT2 device driver devoptab
static const devoptab_t devops_ext2 =
{
    NULL, /* Device name */
    sizeof(ext2_file_state),
    ext2_open_r,
    ext2_close_r,
    ext2_write_r,
    ext2_read_r,
    ext2_seek_r,
    ext2_fstat_r,
    ext2_stat_r,
    ext2_link_r,
    ext2_unlink_r,
    ext2_chdir_r,
    ext2_rename_r,
    ext2_mkdir_r,
    sizeof(ext2_dir_state),
    ext2_diropen_r,
    ext2_dirreset_r,
    ext2_dirnext_r,
    ext2_dirclose_r,
    ext2_statvfs_r,
    ext2_ftruncate_r,
    ext2_fsync_r,
    NULL /* Device data */
};


const devoptab_t *ext2GetDevOpTab()
{
    return &devops_ext2;
}

int ext2AddDevice (const char *name, void *deviceData)
{
    const devoptab_t *devoptab_ext2 = ext2GetDevOpTab();
    devoptab_t *dev = NULL;
    char *devname = NULL;
    int i;

    // Sanity check
    if (!name || !deviceData || !devoptab_ext2) {
        errno = EINVAL;
        return -1;
    }

    // Allocate a devoptab for this device
    dev = (devoptab_t *) mem_alloc(sizeof(devoptab_t) + strlen(name) + 1);
    if (!dev) {
        errno = ENOMEM;
        return -1;
    }

    // Use the space allocated at the end of the devoptab for storing the device name
    devname = (char*)(dev + 1);
    strcpy(devname, name);

    // Setup the devoptab
    memcpy(dev, devoptab_ext2, sizeof(devoptab_t));
    dev->name = devname;
    dev->deviceData = deviceData;

    // Add the device to the devoptab table (if there is a free slot)
    for (i = 0; i < STD_MAX; i++) {
        if (devoptab_list[i] == devoptab_list[0] && i != 0) {
            devoptab_list[i] = dev;
            return 0;
        }
    }

    // If we reach here then there are no free slots in the devoptab table for this device
    errno = EADDRNOTAVAIL;
    return -1;
}

void ext2RemoveDevice (const char *path)
{
    const devoptab_t *devoptab = NULL;
    char name[128] = {0};
    int i;

    // Get the device name from the path
    strncpy(name, path, 127);
    strtok(name, ":/");

    // Find and remove the specified device from the devoptab table
    // NOTE: We do this manually due to a 'bug' in RemoveDevice
    //       which ignores names with suffixes
    for (i = 0; i < STD_MAX; i++) {
        devoptab = devoptab_list[i];
        if (devoptab && devoptab->name) {
            if (strcmp(name, devoptab->name) == 0) {
                devoptab_list[i] = devoptab_list[0];
                mem_free((devoptab_t*)devoptab);
                break;
            }
        }
    }

    return;
}

const devoptab_t *ext2GetDevice (const char *path)
{
    const devoptab_t *devoptab = NULL;
    char name[128] = {0};
    int i;

    // Get the device name from the path
    strncpy(name, path, 127);
    strtok(name, ":/");

    // Search the devoptab table for the specified device name
    // NOTE: We do this manually due to a 'bug' in GetDeviceOpTab
    //       which ignores names with suffixes
    for (i = 0; i < STD_MAX; i++) {
        devoptab = devoptab_list[i];
        if (devoptab && devoptab->name) {
            if (strcmp(name, devoptab->name) == 0) {
                return devoptab;
            }
        }
    }

    return NULL;
}

ext2_vd *ext2GetVolume (const char *path)
{
    // Get the volume descriptor from the paths associated devoptab (if found)
    const devoptab_t *devoptab_ext2 = ext2GetDevOpTab();
    const devoptab_t *devoptab = ext2GetDevice(path);
    if (devoptab && devoptab_ext2 && (devoptab->open_r == devoptab_ext2->open_r))
        return (ext2_vd*)devoptab->deviceData;

    return NULL;
}

int ext2InitVolume (ext2_vd *vd)
{
    // Sanity check
    if (!vd) {
        errno = ENODEV;
        return -1;
    }

    // Reset the volumes data
    memset(vd, 0, sizeof(ext2_vd));

    // Initialise the volume lock
    LWP_MutexInit(&vd->lock, false);

    return 0;
}

void ext2DeinitVolume (ext2_vd *vd)
{
    // Sanity check
    if (!vd) {
        errno = ENODEV;
        return;
    }

    // Lock
    ext2Lock(vd);

    // Close any directories which are still open (lazy programmers!)
    ext2_dir_state *nextDir = vd->firstOpenDir;
    while (nextDir) {
        ext2CloseDir(nextDir);
        nextDir = nextDir->nextOpenDir;
    }

    // Close any files which are still open (lazy programmers!)
    ext2_file_state *nextFile = vd->firstOpenFile;
    while (nextFile) {
       ext2CloseFile(nextFile);
       nextFile = nextFile->nextOpenFile;
    }

    // Reset open directory and file stats
    vd->openDirCount = 0;
    vd->openFileCount = 0;
    vd->firstOpenDir = NULL;
    vd->firstOpenFile = NULL;

    // Force the underlying device to sync
    ext2Sync(vd, NULL);

    // Unlock
    ext2Unlock(vd);

    // Deinitialise the volume lock
    LWP_MutexDestroy(vd->lock);
}

static ext2_ino_t ext2PathToInode(ext2_vd *vd, const char * path)
{
    //Sanity check
    if(!vd || !path)
        return 0;

    char filename[EXT2_NAME_LEN];
    errcode_t errorcode = 0;
    ext2_ino_t ino = 0, parent = vd->cwd_ni && *path != '/' && *path != '\0' ? vd->cwd_ni->ino : vd->root;
    const char * ptr = path;
    int i;

    while(*ptr == '/') ++ptr;

    if(*ptr == '\0')
        return parent;

    while(*ptr != '\0')
    {
        for(i = 0; *ptr != '\0' && *ptr != '/' && (i < EXT2_NAME_LEN-1); ++ptr, ++i)
            filename[i] = *ptr;

        filename[i] = '\0';

        errorcode = ext2fs_namei(vd->fs, vd->root, parent, filename, &ino);
        if(errorcode != EXT2_ET_OK)
            return 0;

        parent = ino;

        while(*ptr == '/') ++ptr;

    }


    return ino;
}

ext2_inode_t *ext2OpenEntry (ext2_vd *vd, const char *path)
{
    errcode_t errorcode = 0;
    ext2_inode_t * ni = 0;

    // Sanity check
    if (!vd) {
        errno = ENODEV;
        return NULL;
    }

    // Get the actual path of the entry
    path = ext2RealPath(path);
    if (!path)
    {
        errno = EINVAL;
        return NULL;
    }

    ni = mem_alloc(sizeof(ext2_inode_t));
    if(!ni)
    {
        errno = ENOMEM;
        return NULL;
    }

	memset(ni, 0, sizeof(ext2_inode_t));

    // Find the entry, taking into account our current directory (if any)
    ni->ino = ext2PathToInode(vd, path);
    if(ni->ino == 0)
    {
        mem_free(ni);
        return NULL;
    }

    errorcode = ext2fs_read_inode(vd->fs, ni->ino, &ni->ni);
    if(errorcode)
    {
        mem_free(ni);
        return NULL;
    }

    return ni;
}

void ext2CloseEntry(ext2_vd *vd, ext2_inode_t * ni)
{
    // Sanity check
    if (!vd || !ni) {
        errno = ENODEV;
        return;
    }

    // Lock
    ext2Lock(vd);

    // Sync the entry (if it is dirty)
    if(ni && ni->dirty)
        ext2fs_write_inode(vd->fs, ni->ino, &ni->ni);

    // Close the entry
	if(ni)
		mem_free(ni);

    // Unlock
    ext2Unlock(vd);

    return;
}

static ext2_ino_t ext2CreateSymlink(ext2_vd *vd, const char *path, const char * targetdir, const char * name, mode_t type)
{
    ext2_inode_t *target_ni = NULL;
    ext2_ino_t newentry = 0;
    ext2_ino_t ino = 0;

    // Check if it does exist
    target_ni = ext2OpenEntry(vd, targetdir);
    if (!target_ni)
        goto cleanup;

    int err = ext2fs_new_inode(vd->fs, target_ni->ino, type, 0, &ino);
    if (err)
        goto cleanup;

    do
    {
        err = ext2fs_link(vd->fs, target_ni->ino, name, ino, EXT2_FT_SYMLINK);
        if (err == EXT2_ET_DIR_NO_SPACE)
        {
            err = ext2fs_expand_dir(vd->fs, target_ni->ino);
            if (err)
                goto cleanup;
        }
    }
    while(err == EXT2_ET_DIR_NO_SPACE);

    ext2fs_inode_alloc_stats2(vd->fs, ino, +1, 0);

    struct ext2_inode inode;
    memset(&inode, 0, sizeof(inode));
    inode.i_mode = type;
    inode.i_atime = inode.i_ctime = inode.i_mtime = time(NULL);
    inode.i_links_count = 1;
    inode.i_size = strlen(path); //initial size of file
    inode.i_uid = target_ni->ni.i_uid;
    inode.i_gid = target_ni->ni.i_gid;

    if (strlen(path) <= sizeof(inode.i_block))
    {
        /* fast symlink */
        strncpy((char *)&(inode.i_block[0]),path,sizeof(inode.i_blocks));
    }
    else
    {
        /* slow symlink */
        char * buffer = mem_alloc(vd->fs->blocksize);
        if (buffer)
        {
            blk_t blk;
            strncpy(buffer, path, vd->fs->blocksize);
            err = ext2fs_new_block(vd->fs, 0, 0, &blk);
            if (!err)
            {
                inode.i_block[0] = blk;
                inode.i_blocks = vd->fs->blocksize / BYTES_PER_SECTOR;
                vd->fs->io->manager->write_blk(vd->fs->io, blk, 1, buffer);
                ext2fs_block_alloc_stats(vd->fs, blk, +1);
            }
            mem_free(buffer);
        }
    }

    if(ext2fs_write_new_inode(vd->fs, ino, &inode) != 0)
        newentry = ino;

cleanup:

    if(target_ni)
        ext2CloseEntry(vd, target_ni);

    return newentry;
}

static ext2_ino_t ext2CreateMkDir(ext2_vd *vd, ext2_inode_t * parent, int type, const char * name)
{
    ext2_ino_t newentry = 0;
    ext2_ino_t existing;

    if(ext2fs_namei(vd->fs, vd->root, parent->ino, name, &existing) == 0)
        return 0;

    errcode_t err = ext2fs_new_inode(vd->fs, parent->ino, type, 0, &newentry);
    if(err != EXT2_ET_OK)
        return 0;

    do
    {
        err = ext2fs_mkdir(vd->fs, parent->ino, newentry, name);
        if(err == EXT2_ET_DIR_NO_SPACE)
        {
            if(ext2fs_expand_dir(vd->fs, parent->ino) != 0)
                return 0;
        }
    }
    while(err == EXT2_ET_DIR_NO_SPACE);

    if(err != EXT2_ET_OK)
        return 0;

    struct ext2_inode inode;
    if(ext2fs_read_inode(vd->fs, newentry, &inode) == EXT2_ET_OK)
    {
        inode.i_mode = type;
        inode.i_uid = parent->ni.i_uid;
        inode.i_gid = parent->ni.i_gid;
        ext2fs_write_new_inode(vd->fs, newentry, &inode);
    }

    return newentry;
}


static ext2_ino_t ext2CreateFile(ext2_vd *vd, ext2_inode_t * parent, int type, const char * name)
{
    errcode_t retval = -1;
    ext2_ino_t newfile = 0;
    ext2_ino_t existing;

    if(ext2fs_namei(vd->fs, vd->root, parent->ino, name, &existing) == 0)
        return 0;

	retval = ext2fs_new_inode(vd->fs, parent->ino, type, 0, &newfile);
	if (retval)
        return 0;

    do
    {
        retval = ext2fs_link(vd->fs, parent->ino, name, newfile, EXT2_FT_REG_FILE);
        if (retval == EXT2_ET_DIR_NO_SPACE)
        {
            if (ext2fs_expand_dir(vd->fs, parent->ino) != 0)
                return 0;
        }
    }
    while(retval == EXT2_ET_DIR_NO_SPACE);

    if (retval)
        return 0;

	ext2fs_inode_alloc_stats2(vd->fs, newfile, +1, 0);

	struct ext2_inode inode;
	memset(&inode, 0, sizeof(inode));
	inode.i_mode = type;
	inode.i_atime = inode.i_ctime = inode.i_mtime = time(0);
	inode.i_links_count = 1;
	inode.i_size = 0;
    inode.i_uid = parent->ni.i_uid;
    inode.i_gid = parent->ni.i_gid;

	if (ext2fs_write_new_inode(vd->fs, newfile, &inode) != 0)
        return 0;

    return newfile;
}

ext2_inode_t *ext2Create(ext2_vd *vd, const char *path, mode_t type, const char *target)
{
    ext2_inode_t *dir_ni = NULL, *ni = NULL;
    char *dir = NULL;
    char *targetdir = NULL;
    char *name = NULL;
    ext2_ino_t newentry = 0;

    // Sanity check
    if (!vd || !vd->fs) {
        errno = ENODEV;
        return NULL;
    }

    if(!(vd->fs->flags & EXT2_FLAG_RW))
        return NULL;

    // You cannot link between devices
    if(target) {
        if(vd != ext2GetVolume(target)) {
            errno = EXDEV;
            return NULL;
        }
        // Check if existing
        dir_ni = ext2OpenEntry(vd, target);
        if (dir_ni) {
            goto cleanup;
        }
        ext2CloseEntry(vd, dir_ni);
        dir_ni = NULL;
        targetdir = strdup(target);
        if (!targetdir) {
            errno = EINVAL;
            goto cleanup;
        }
    }

    // Get the actual paths of the entry
    path = ext2RealPath(path);
    target = ext2RealPath(target);
    if (!path) {
        errno = EINVAL;
        return NULL;
    }

    // Lock
    ext2Lock(vd);

    // Clean me
    // NOTE: this looks horrible right now and need a cleanup
    dir = strdup(path);
    if (!dir) {
        errno = EINVAL;
        goto cleanup;
    }

    char * tmp_path = (targetdir && (type == S_IFLNK)) ? targetdir : dir;
    if (strrchr(tmp_path, '/') != NULL)
    {
        char * ptr = strrchr(tmp_path, '/');
        name = strdup(ptr+1);
        *ptr = '\0';
    }
    else
        name = strdup(tmp_path);

    // Open the entries parent directory
    dir_ni = ext2OpenEntry(vd, dir);
    if (!dir_ni) {
        goto cleanup;
    }

    // If not yet read, read the inode and block bitmap
    if(!vd->fs->inode_map || !vd->fs->block_map)
        ext2fs_read_bitmaps(vd->fs);

    // Symbolic link
    if(type == S_IFLNK)
    {
        if (!target) {
            errno = EINVAL;
            goto cleanup;
        }

        newentry = ext2CreateSymlink(vd, path, targetdir, name, type);
    }
    // Directory
    else if(type == S_IFDIR)
    {
        newentry = ext2CreateMkDir(vd, dir_ni, LINUX_S_IFDIR | (0755 & ~vd->fs->umask), name);
    }
    // File
    else if(type == S_IFREG)
    {
        newentry = ext2CreateFile(vd, dir_ni, LINUX_S_IFREG | (0755 & ~vd->fs->umask), name);
    }

    // If the entry was created
    if (newentry != 0)
    {
        // Sync the entry to disc
        ext2Sync(vd, NULL);

        ni = ext2OpenEntry(vd, target ? target : path);
    }

cleanup:

    if(dir_ni)
        ext2CloseEntry(vd, dir_ni);

    if(name)
        mem_free(name);

    if(dir)
        mem_free(dir);

    if(targetdir)
        mem_free(targetdir);

    // Unlock
    ext2Unlock(vd);

    return ni;
}

/*
 * Given a mode, return the ext2 file type
 */
static int ext2_file_type(unsigned int mode)
{
      if (LINUX_S_ISREG(mode))
            return EXT2_FT_REG_FILE;

      if (LINUX_S_ISDIR(mode))
            return EXT2_FT_DIR;

      if (LINUX_S_ISCHR(mode))
            return EXT2_FT_CHRDEV;

      if (LINUX_S_ISBLK(mode))
            return EXT2_FT_BLKDEV;

      if (LINUX_S_ISLNK(mode))
            return EXT2_FT_SYMLINK;

      if (LINUX_S_ISFIFO(mode))
            return EXT2_FT_FIFO;

      if (LINUX_S_ISSOCK(mode))
            return EXT2_FT_SOCK;

      return 0;
}

int ext2Link(ext2_vd *vd, const char *old_path, const char *new_path)
{
    ext2_inode_t *dir_ni = NULL, *ni = NULL;
    char *dir = NULL;
    char *name = NULL;
    errcode_t err = 0;

    // Sanity check
    if (!vd || !vd->fs) {
        errno = ENODEV;
        return -1;
    }

    if(!(vd->fs->flags & EXT2_FLAG_RW))
        return -1;

    // You cannot link between devices
    if(vd != ext2GetVolume(new_path)) {
        errno = EXDEV;
        return -1;
    }

    // Get the actual paths of the entry
    old_path = ext2RealPath(old_path);
    new_path = ext2RealPath(new_path);
    if (!old_path || !new_path) {
        errno = EINVAL;
        return -1;
    }

    // Lock
    ext2Lock(vd);

    //check for existing in new path
    ni = ext2OpenEntry(vd, new_path);
    if (ni) {
        ext2CloseEntry(vd, ni);
        ni = NULL;
        errno = EINVAL;
        return -1;
    }

    dir = strdup(new_path);
    if (!dir) {
        errno = EINVAL;
        err = -1;
        goto cleanup;
    }
    char * ptr = strrchr(dir, '/');
    if (ptr)
    {
        name = strdup(ptr+1);
        *ptr = 0;
    }
    else
        name = strdup(dir);

    // Find the entry
    ni = ext2OpenEntry(vd, old_path);
    if (!ni) {
        errno = ENOENT;
        err = -1;
        goto cleanup;
    }

    // Open the entries new parent directory
    dir_ni = ext2OpenEntry(vd, dir);
    if (!dir_ni) {
        errno = ENOENT;
        err = -1;
        goto cleanup;
    }

    do
    {
        // Link the entry to its new parent
        err = ext2fs_link(vd->fs, dir_ni->ino, name, ni->ino, ext2_file_type(ni->ni.i_mode));
        if (err == EXT2_ET_DIR_NO_SPACE)
        {
            if (ext2fs_expand_dir(vd->fs, dir_ni->ino) != 0)
                goto cleanup;
        }
        else if(err != 0)
        {
            errno = ENOMEM;
            goto cleanup;
        }
    }
    while(err == EXT2_ET_DIR_NO_SPACE);

    ni->ni.i_links_count++;

    // Update entry times
    ext2UpdateTimes(vd, ni, EXT2_UPDATE_MCTIME);

    // Sync the entry to disc
    ext2Sync(vd, ni);

cleanup:

    if(dir_ni)
        ext2CloseEntry(vd, dir_ni);

    if(ni)
        ext2CloseEntry(vd, ni);

    if(dir)
        mem_free(dir);

    if(name)
        mem_free(name);

    // Unlock
    ext2Unlock(vd);

    return err;
}

typedef struct _rd_struct
{
	ext2_ino_t	parent;
	int		empty;
} rd_struct;

static int release_blocks_proc(ext2_filsys fs, blk_t *blocknr, int blockcnt EXT2FS_ATTR((unused)),  void *private EXT2FS_ATTR((unused)))
{
	blk_t	block;

	block = *blocknr;
	ext2fs_block_alloc_stats(fs, block, -1);
	*blocknr = 0;
	return 0;
}

static int unlink_proc(ext2_ino_t dir EXT2FS_ATTR((unused)), int	entry EXT2FS_ATTR((unused)),
                        struct ext2_dir_entry *dirent, int	offset EXT2FS_ATTR((unused)),
                        int	blocksize EXT2FS_ATTR((unused)), char	*buf EXT2FS_ATTR((unused)),
                        void	*private_data)
{
    rd_struct *rds = (rd_struct *) private_data;

	if (dirent->inode == 0)
		return 0;
	if (((dirent->name_len & 0xFF) == 1) && (dirent->name[0] == '.'))
		return 0;
	if (((dirent->name_len & 0xFF) == 2) && (dirent->name[0] == '.') &&
	    (dirent->name[1] == '.')) {
		rds->parent = dirent->inode;
		return 0;
	}

	rds->empty = 0;
	return 0;
}

int ext2Unlink (ext2_vd *vd, const char *path)
{
    ext2_inode_t *dir_ni = NULL, *ni = NULL;
    char *dir = NULL;
    char *name = NULL;
    errcode_t err = -1;

    // Sanity check
    if (!vd || !vd->fs) {
        errno = ENODEV;
        return -1;
    }

    if(!(vd->fs->flags & EXT2_FLAG_RW))
        return -1;

    // Get the actual path of the entry
    path = ext2RealPath(path);
    if (!path) {
        errno = EINVAL;
        return -1;
    }

    // Lock
    ext2Lock(vd);

    dir = strdup(path);
    if (!dir) {
        errno = EINVAL;
        goto cleanup;
    }
    char * ptr = strrchr(dir, '/');
    if (ptr)
    {
        name = strdup(ptr+1);
        *ptr = 0;
    }
    else
        name = dir;

    // Find the entry
    ni = ext2OpenEntry(vd, path);
    if (!ni) {
        errno = ENOENT;
        goto cleanup;
    }

    // Open the entries parent directory
    dir_ni = ext2OpenEntry(vd, dir);
    if (!dir_ni) {
        errno = ENOENT;
        goto cleanup;
    }

    // Directory
    if(LINUX_S_ISDIR(ni->ni.i_mode))
    {
        rd_struct rds;
        rds.parent = 0;
        rds.empty = 1;

        if (ext2fs_dir_iterate2(vd->fs, ni->ino, 0, 0, unlink_proc, &rds) != 0)
            goto cleanup;

        if(!rds.empty)
            goto cleanup;

        if (rds.parent)
        {
            struct ext2_inode inode;
            if (ext2fs_read_inode(vd->fs, rds.parent, &inode) == 0)
            {
                if(inode.i_links_count > 1)
                    inode.i_links_count--;
                ext2fs_write_inode(vd->fs, rds.parent, &inode);
            }
        }

        // set link count 0
        ni->ni.i_links_count = 0;
    }
    // File
    else
    {
        ni->ni.i_links_count--;
    }

    if(ni->ni.i_links_count <= 0)
    {
        ni->ni.i_size = 0;
        ni->ni.i_size_high = 0;
        ni->ni.i_links_count = 0;
        ni->ni.i_dtime = (u32) time(0);
    }

    ext2fs_write_inode(vd->fs, ni->ino, &ni->ni);

    // Unlink the entry from its parent
    if(ext2fs_unlink(vd->fs, dir_ni->ino, name, 0, 0) != 0)
        goto cleanup;

    if (ext2fs_inode_has_valid_blocks(&ni->ni))
    {
        ext2fs_block_iterate(vd->fs, ni->ino, 0, NULL, release_blocks_proc, NULL);
        ext2fs_inode_alloc_stats2(vd->fs, ni->ino, -1, LINUX_S_ISDIR(ni->ni.i_mode));
    }

    if(ni->ni.i_links_count == 0)
    {
        // It's odd that i have to do this on my own and the lib is not doing that for me
        blk64_t truncate_block = ((vd->fs->blocksize - 1) >> EXT2_BLOCK_SIZE_BITS(vd->fs->super)) + 1;
        ext2fs_punch(vd->fs, ni->ino, &ni->ni, 0, truncate_block, ~0ULL);
    }

    // Sync the entry to disc
    ext2Sync(vd, NULL);

    err = 0;

cleanup:

    if(dir_ni)
        ext2CloseEntry(vd, dir_ni);

    if(ni)
        ext2CloseEntry(vd, ni);

    if(name)
        mem_free(name);

    if(dir)
        mem_free(dir);

    // Unlock
    ext2Unlock(vd);

    return err;
}


int ext2Sync(ext2_vd *vd, ext2_inode_t *ni)
{
    errcode_t res = 0;

    // Sanity check
    if (!vd || !vd->fs) {
        errno = ENODEV;
        return -1;
    }

    if(!(vd->fs->flags & EXT2_FLAG_RW))
        return -1;

    // Lock
    ext2Lock(vd);

    if(ni && ni->dirty)
    {
        ext2fs_write_inode(vd->fs, ni->ino, &ni->ni);
        ni->dirty = false;
    }

    // Sync the entry
    res = ext2fs_flush(vd->fs);

    // Force the underlying device to sync
    vd->io->manager->flush(vd->io);

    // Unlock
    ext2Unlock(vd);

    return res;

}

int ext2Stat (ext2_vd *vd, ext2_inode_t *ni_main, struct stat *st)
{
    int res = 0;

    // Sanity check
    if (!vd) {
        errno = ENODEV;
        return -1;
    }

    struct ext2_inode * ni = ni_main ? &ni_main->ni : 0;

    // Sanity check
    if (!ni) {
        errno = ENOENT;
        return -1;
    }

    // Short circuit cases were we don't actually have to do anything
    if (!st)
        return 0;

    // Lock
    ext2Lock(vd);

    // Zero out the stat buffer
    memset(st, 0, sizeof(struct stat));

    if(LINUX_S_ISDIR(ni->i_mode))
    {
        st->st_nlink = 1;
        st->st_size = ni->i_size;
    }
    else
    {
        st->st_nlink = ni->i_links_count;
        st->st_size = EXT2_I_SIZE(ni);
    }

    st->st_mode = ni->i_mode;
    st->st_blocks = ni->i_blocks;
    st->st_blksize = vd->fs->blocksize;

    // Fill in the generic entry stats
    st->st_dev = (dev_t) ((long) vd->fs);
    st->st_uid = ni->i_uid | (((u32) ni->osd2.linux2.l_i_uid_high) << 16);
    st->st_gid = ni->i_gid | (((u32) ni->osd2.linux2.l_i_gid_high) << 16);
    st->st_ino = ni_main->ino;
    st->st_atime = ni->i_atime;
    st->st_ctime = ni->i_ctime;
    st->st_mtime = ni->i_mtime;

    // Update entry times
    ext2UpdateTimes(vd, ni_main, EXT2_UPDATE_ATIME);

    // Unlock
    ext2Unlock(vd);

    return res;
}

void ext2UpdateTimes(ext2_vd *vd, ext2_inode_t *ni, ext2_time_update_flags mask)
{
    // Sanity check
    if(!ni || !mask)
        return;

    if(!(vd->fs->flags & EXT2_FLAG_RW))
        return;

    u32 now = (u32) time(0);

    if(mask & EXT2_UPDATE_ATIME)
        ni->ni.i_atime = now;
    if(mask & EXT2_UPDATE_MTIME)
        ni->ni.i_mtime = now;
    if(mask & EXT2_UPDATE_CTIME)
        ni->ni.i_ctime = now;

    ni->dirty = true;
}

const char *ext2RealPath (const char *path)
{
    // Sanity check
    if (!path)
        return NULL;

    // Move the path pointer to the start of the actual path
    if (strchr(path, ':') != NULL) {
        path = strchr(path, ':')+1;
    }
    if (strchr(path, ':') != NULL) {
        return NULL;
    }

    return path;
}
