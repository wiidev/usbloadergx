/**
 * ext2_dir.c - devoptab directory routines for EXT2-based devices.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "ext2_internal.h"
#include "ext2dir.h"
#include <sys/dir.h>

#define STATE(x)    ((ext2_dir_state*)(x)->dirStruct)

void ext2CloseDir (ext2_dir_state *dir)
{
    // Sanity check
    if (!dir || !dir->vd)
        return;

    // Free the directory entries (if any)
    while (dir->first) {
        ext2_dir_entry *next = dir->first->next;
        mem_free(dir->first->name);
        mem_free(dir->first);
        dir->first = next;
    }

    // Close the directory (if open)
    if (dir->ni)
        ext2CloseEntry(dir->vd, dir->ni);

    // Reset the directory state
    dir->ni = NULL;
    dir->first = NULL;
    dir->current = NULL;

    return;
}

int ext2_stat_r (struct _reent *r, const char *path, struct stat *st)
{
    // Short circuit cases were we don't actually have to do anything
    if (!st || !path)
        return 0;

    ext2_log_trace("path %s, st %p\n", path, st);

    ext2_vd *vd = NULL;
    ext2_inode_t *ni = NULL;

    // Get the volume descriptor for this path
    vd = ext2GetVolume(path);
    if (!vd) {
        r->_errno = ENODEV;
        return -1;
    }

    if(strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
    {
        memset(st, 0, sizeof(struct stat));
        st->st_mode = S_IFDIR;
        return 0;
    }

    // Lock
    ext2Lock(vd);

    // Find the entry
    ni = ext2OpenEntry(vd, path);
    if (!ni) {
        r->_errno = errno;
        ext2Unlock(vd);
        return -1;
    }

    // Get the entry stats
    int ret = ext2Stat(vd, ni, st);
    if (ret)
        r->_errno = errno;

    // Close the entry
    ext2CloseEntry(vd, ni);

    ext2Unlock(vd);

    return 0;
}

int ext2_link_r (struct _reent *r, const char *existing, const char *newLink)
{
    ext2_log_trace("existing %s, newLink %s\n", existing, newLink);

    ext2_vd *vd = NULL;
    ext2_inode_t *ni = NULL;

    // Get the volume descriptor for this path
    vd = ext2GetVolume(existing);
    if (!vd) {
        r->_errno = ENODEV;
        return -1;
    }

    // Lock
    ext2Lock(vd);

    // Create a symbolic link between the two paths
    ni = ext2Create(vd, existing, S_IFLNK, newLink);
    if (!ni) {
        ext2Unlock(vd);
        r->_errno = errno;
        return -1;
    }

    // Close the symbolic link
    ext2CloseEntry(vd, ni);

    // Unlock
    ext2Unlock(vd);

    return 0;
}

int ext2_unlink_r (struct _reent *r, const char *name)
{
    ext2_log_trace("name %s\n", name);

    ext2_vd *vd = NULL;

    // Get the volume descriptor for this path
    vd = ext2GetVolume(name);
    if (!vd) {
        r->_errno = ENODEV;
        return -1;
    }

    // Unlink the entry
    int ret = ext2Unlink(vd, name);
    if (ret)
        r->_errno = errno;

    return ret;
}

int ext2_chdir_r (struct _reent *r, const char *name)
{
    ext2_log_trace("name %s\n", name);

    ext2_vd *vd = NULL;
    ext2_inode_t *ni = NULL;

    // Get the volume descriptor for this path
    vd = ext2GetVolume(name);
    if (!vd) {
        r->_errno = ENODEV;
        return -1;
    }

    // Lock
    ext2Lock(vd);

    // Find the directory
    ni = ext2OpenEntry(vd, name);
    if (!ni) {
        ext2Unlock(vd);
        r->_errno = ENOENT;
        return -1;
    }

    // Ensure that this directory is indeed a directory
    if (!LINUX_S_ISDIR(ni->ni.i_mode)) {
        ext2CloseEntry(vd, ni);
        ext2Unlock(vd);
        r->_errno = ENOTDIR;
        return -1;
    }

    // Close the old current directory (if any)
    if (vd->cwd_ni)
        ext2CloseEntry(vd, vd->cwd_ni);

    // Set the new current directory
    vd->cwd_ni = ni;

    // Unlock
    ext2Unlock(vd);

    return 0;
}

int ext2_rename_r (struct _reent *r, const char *oldName, const char *newName)
{
    ext2_log_trace("oldName %s, newName %s\n", oldName, newName);

    ext2_vd *vd = NULL;
    ext2_inode_t *ni = NULL;

    // Get the volume descriptor for this path
    vd = ext2GetVolume(oldName);
    if (!vd) {
        r->_errno = ENODEV;
        return -1;
    }

    // Lock
    ext2Lock(vd);

    // You cannot rename between devices
    if(vd != ext2GetVolume(newName)) {
        ext2Unlock(vd);
        r->_errno = EXDEV;
        return -1;
    }

    // Check that there is no existing entry with the new name
    ni = ext2OpenEntry(vd, newName);
    if (ni) {
        ext2CloseEntry(vd, ni);
        ext2Unlock(vd);
        r->_errno = EEXIST;
        return -1;
    }

    // Link the old entry with the new one
    if (ext2Link(vd, oldName, newName)) {
        ext2Unlock(vd);
        return -1;
    }

    // Unlink the old entry
    if (ext2Unlink(vd, oldName)) {
        if (ext2Unlink(vd, newName)) {
            ext2Unlock(vd);
            return -1;
        }
        ext2Unlock(vd);
        return -1;
    }

    // Unlock
    ext2Unlock(vd);

    return 0;
}

int ext2_mkdir_r (struct _reent *r, const char *path, int mode)
{
    ext2_log_trace("path %s, mode %i\n", path, mode);

    ext2_vd *vd = NULL;
    ext2_inode_t *ni = NULL;

    // Get the volume descriptor for this path
    vd = ext2GetVolume(path);
    if (!vd) {
        r->_errno = ENODEV;
        return -1;
    }

    // Lock
    ext2Lock(vd);

    // Create the directory
    ni = ext2Create(vd, path, S_IFDIR, NULL);
    if (!ni) {
        ext2Unlock(vd);
        r->_errno = errno;
        return -1;
    }

    // Close the directory
    ext2CloseEntry(vd, ni);

    // Unlock
    ext2Unlock(vd);

    return 0;
}

int ext2_statvfs_r (struct _reent *r, const char *path, struct statvfs *buf)
{
    ext2_log_trace("path %s, buf %p\n", path, buf);

    ext2_vd *vd = NULL;

    // Get the volume descriptor for this path
    vd = ext2GetVolume(path);
    if (!vd) {
        r->_errno = ENODEV;
        return -1;
    }

    // Short circuit cases were we don't actually have to do anything
    if (!buf)
        return 0;

    // Lock
    ext2Lock(vd);

    // Zero out the stat buffer
    memset(buf, 0, sizeof(struct statvfs));

    // File system block size
    switch(vd->fs->super->s_log_block_size)
    {
        case 1:
            buf->f_bsize = 2048;
            break;
        case 2:
            buf->f_bsize = 4096;
            break;
        case 3:
            buf->f_bsize = 8192;
            break;
        default:
        case 0:
            buf->f_bsize = 1024;
            break;
    }

    // Fundamental file system block size
    buf->f_frsize = buf->f_bsize;

    // Total number of blocks on file system in units of f_frsize
    buf->f_blocks = vd->fs->super->s_blocks_count | (((u64) vd->fs->super->s_blocks_count_hi) << 32);

    // Free blocks available for all and for non-privileged processes
    buf->f_bfree = vd->fs->super->s_free_blocks_count | (((u64) vd->fs->super->s_free_blocks_hi) << 32);

    // Number of inodes at this point in time
    buf->f_files = vd->fs->super->s_inodes_count;

    // Free inodes available for all and for non-privileged processes
    buf->f_ffree = vd->fs->super->s_free_inodes_count;

    // File system id
    buf->f_fsid = vd->fs->super->s_magic;

    // Bit mask of f_flag values.
    buf->f_flag = vd->fs->super->s_flags;

    // Maximum length of filenames
    buf->f_namemax = EXT2_NAME_LEN;

    // Unlock
    ext2Unlock(vd);

    return 0;
}

/**
 * PRIVATE: Callback for directory walking
 */
static int DirIterateCallback(struct ext2_dir_entry *dirent, int offset, int blocksize, char *buf, void *dirState)
{
    // Sanity check
    if(!dirent)
    {
        errno = EINVAL;
        return -1;
    }

    ext2_dir_state* dir = STATE(((DIR_ITER *) dirState));

    // Sanity check
    if (!dir || !dir->vd) {
        errno = EINVAL;
        return -1;
    }

    //skip ".." on root directory
    if(dir->ni->ino == dir->vd->root && strcmp(dirent->name, "..") == 0)
    {
        return EXT2_ET_OK;
    }

    // Allocate a new directory entry
    ext2_dir_entry *entry = (ext2_dir_entry *) mem_alloc(sizeof(ext2_dir_entry));
    if (!entry)
    {
        errno = ENOMEM;
        return -1;
    }

    memset(entry, 0, sizeof(ext2_dir_entry));

    int stringlen = dirent->name_len & 0xFF;

    entry->name = mem_alloc(stringlen+1);
    if(!entry->name)
    {
        mem_free(entry);
        errno = ENOMEM;
        return -1;
    }

    // The null termination is not necessarily there in the fs, we gotta do it
    int i;
    for(i = 0; i < stringlen; ++i)
        entry->name[i] = dirent->name[i];
    entry->name[i] = '\0';

    // Link the entry to the directory
    if (!dir->first) {
        dir->first = entry;
		dir->length = dirent->rec_len;
    } else {
        ext2_dir_entry *last = dir->first;
        while (last->next) last = last->next;
        last->next = entry;
    }

    return EXT2_ET_OK;
}

DIR_ITER *ext2_diropen_r (struct _reent *r, DIR_ITER *dirState, const char *path)
{
    ext2_log_trace("dirState %p, path %s\n", dirState, path);

    if(!dirState)
    {
        r->_errno = EINVAL;
        return NULL;
    }

    ext2_dir_state* dir = STATE(dirState);

    if(!dir)
    {
        r->_errno = EINVAL;
        return NULL;
    }

    // Get the volume descriptor for this path
    dir->vd = ext2GetVolume(path);
    if (!dir->vd) {
        r->_errno = ENODEV;
        return NULL;
    }

    // Lock
    ext2Lock(dir->vd);

    // Find the directory
    dir->ni = ext2OpenEntry(dir->vd, path);
    if (!dir->ni) {
        ext2Unlock(dir->vd);
        r->_errno = ENOENT;
        return NULL;
    }

    // Ensure that this directory is indeed a directory
    if (!LINUX_S_ISDIR(dir->ni->ni.i_mode)) {
        ext2CloseEntry(dir->vd, dir->ni);
        ext2Unlock(dir->vd);
        r->_errno = ENOTDIR;
        return NULL;
    }

    // Read the directory
    dir->first = dir->current = NULL;
    if (ext2fs_dir_iterate(dir->vd->fs, dir->ni->ino, 0, 0, DirIterateCallback, dirState) != EXT2_ET_OK) {
        ext2CloseDir(dir);
        ext2Unlock(dir->vd);
        r->_errno = errno;
        return NULL;
    }

    // Move to the first entry in the directory
    dir->current = dir->first;

    // Update directory times
    ext2UpdateTimes(dir->vd, dir->ni, EXT2_UPDATE_ATIME);

    // Insert the directory into the double-linked FILO list of open directories
    if (dir->vd->firstOpenDir) {
        dir->nextOpenDir = dir->vd->firstOpenDir;
        dir->vd->firstOpenDir->prevOpenDir = dir;
    } else {
        dir->nextOpenDir = NULL;
    }
    dir->prevOpenDir = NULL;
    dir->vd->cwd_ni = dir->ni;
    dir->vd->firstOpenDir = dir;
    dir->vd->openDirCount++;

    // Unlock
    ext2Unlock(dir->vd);

    return dirState;
}

int ext2_dirreset_r (struct _reent *r, DIR_ITER *dirState)
{
    ext2_log_trace("dirState %p\n", dirState);

    if(!dirState)
    {
        r->_errno = EINVAL;
        return -1;
    }

    ext2_dir_state* dir = STATE(dirState);

    // Sanity check
    if (!dir || !dir->vd || !dir->ni) {
        r->_errno = EBADF;
        return -1;
    }

    // Lock
    ext2Lock(dir->vd);

    // Move to the first entry in the directory
    dir->current = dir->first;

    // Update directory times
    ext2UpdateTimes(dir->vd, dir->ni, EXT2_UPDATE_ATIME);

    // Unlock
    ext2Unlock(dir->vd);

    return 0;
}

int ext2_dirnext_r (struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat)
{
    ext2_log_trace("dirState %p, filename %p, filestat %p\n", dirState, filename, filestat);

    if(!dirState)
    {
        r->_errno = EINVAL;
        return -1;
    }

    ext2_dir_state* dir = STATE(dirState);
    ext2_inode_t *ni = NULL;

    // Sanity check
    if (!dir || !dir->vd || !dir->ni) {
        r->_errno = EBADF;
        return -1;
    }

    // Lock
    ext2Lock(dir->vd);

    // Check that there is a entry waiting to be fetched
    if (!dir->current) {
        ext2Unlock(dir->vd);
        r->_errno = ENOENT;
        return -1;
    }

    // Fetch the current entry
    strcpy(filename, dir->current->name);
    if(filestat != NULL)
    {
        if(strcmp(dir->current->name, ".") == 0 || strcmp(dir->current->name, "..") == 0)
        {
            memset(filestat, 0, sizeof(struct stat));
            filestat->st_mode = S_IFDIR;
        }
        else
        {
            ni = ext2OpenEntry(dir->vd, dir->current->name);
            if (ni) {
                ext2Stat(dir->vd, ni, filestat);
                ext2CloseEntry(dir->vd, ni);
            }
        }
    }

    // Move to the next entry in the directory
    dir->current = dir->current->next;

    // Update directory times
    ext2UpdateTimes(dir->vd, dir->ni, EXT2_UPDATE_ATIME);

    // Unlock
    ext2Unlock(dir->vd);

    return 0;
}

int ext2_dirclose_r (struct _reent *r, DIR_ITER *dirState)
{
    ext2_log_trace("dirState %p\n", dirState);

    if(!dirState)
    {
        r->_errno = EINVAL;
        return -1;
    }

    ext2_dir_state* dir = STATE(dirState);

    // Sanity check
    if (!dir || !dir->vd) {
        r->_errno = EBADF;
        return -1;
    }

    // Lock
    ext2Lock(dir->vd);

    // Close the directory
    ext2CloseDir(dir);

    // Remove the directory from the double-linked FILO list of open directories
    dir->vd->openDirCount--;
    if (dir->nextOpenDir)
        dir->nextOpenDir->prevOpenDir = dir->prevOpenDir;
    if (dir->prevOpenDir)
        dir->prevOpenDir->nextOpenDir = dir->nextOpenDir;
    else
        dir->vd->firstOpenDir = dir->nextOpenDir;

    // Unlock
    ext2Unlock(dir->vd);

    return 0;
}
