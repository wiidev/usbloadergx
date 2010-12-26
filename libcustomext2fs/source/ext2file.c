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

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "ext2_internal.h"
#include "ext2file.h"

#define STATE(x)    ((ext2_file_state*)x)

void ext2CloseFile (ext2_file_state *file)
{
    // Sanity check
    if (!file || !file->vd)
        return;

    ext2fs_file_close(file->fd);

    // Sync the file (and its attributes) to disc
    if(file->write)
    {
        // Read in node changes before writing them
        ext2fs_read_inode(file->vd->fs, file->ni->ino, &file->ni->ni);
        ext2UpdateTimes(file->vd, file->ni, EXT2_UPDATE_ACTIME);
    }

    if (file->read)
        ext2UpdateTimes(file->vd, file->ni, EXT2_UPDATE_ATIME);

    ext2Sync(file->vd, file->ni);

    // Close the file (if open)
    if (file->ni)
        ext2CloseEntry(file->vd, file->ni);

    // Reset the file state
    file->ni = NULL;
    file->fd = NULL;
    file->flags = 0;
    file->read = false;
    file->write = false;
    file->append = false;

    return;
}

int ext2_open_r (struct _reent *r, void *fileStruct, const char *path, int flags, int mode)
{
    ext2_log_trace("fileStruct %p, path %s, flags %i, mode %i\n", fileStruct, path, flags, mode);

    ext2_file_state* file = STATE(fileStruct);

    // Get the volume descriptor for this path
    file->vd = ext2GetVolume(path);
    if (!file->vd) {
        r->_errno = ENODEV;
        return -1;
    }

    // Lock
    ext2Lock(file->vd);

    // Determine which mode the file is opened for
    file->flags = flags;
    if ((flags & 0x03) == O_RDONLY) {
        file->read = true;
        file->write = false;
        file->append = false;
    } else if ((flags & 0x03) == O_WRONLY) {
        file->read = false;
        file->write = true;
        file->append = (flags & O_APPEND);
    } else if ((flags & 0x03) == O_RDWR) {
        file->read = true;
        file->write = true;
        file->append = (flags & O_APPEND);
    } else {
        r->_errno = EACCES;
        ext2Unlock(file->vd);
        return -1;
    }

    // Try and find the file and (if found) ensure that it is not a directory
    file->ni = ext2OpenEntry(file->vd, path);
    if (file->ni && LINUX_S_ISDIR(file->ni->ni.i_mode))
    {
        ext2CloseEntry(file->vd, file->ni);
        ext2Unlock(file->vd);
        r->_errno = EISDIR;
        return -1;
    }

    // Are we creating this file?
    if ((flags & O_CREAT) && !file->ni)
        // Create the file
        file->ni = ext2Create(file->vd, path, S_IFREG, NULL);

    // Sanity check, the file should be open by now
    if (!file->ni) {
        ext2Unlock(file->vd);
        r->_errno = ENOENT;
        return -1;
    }

    // Make sure we aren't trying to write to a read-only file
    if (!(file->vd->fs->flags & EXT2_FLAG_RW) && file->write)
    {
        ext2CloseEntry(file->vd, file->ni);
        ext2Unlock(file->vd);
        r->_errno = EROFS;
        return -1;
    }

    errcode_t err = ext2fs_file_open2(file->vd->fs, file->ni->ino, &file->ni->ni,
                                      file->write ? EXT2_FLAG_RW : 0, &file->fd);
    if(err != 0)
    {
        ext2CloseEntry(file->vd, file->ni);
        ext2Unlock(file->vd);
        r->_errno = ENOENT;
        return -1;
    }


    // Truncate the file if requested
    if ((flags & O_TRUNC) && file->write) {
        if (ext2fs_file_set_size2(file->fd, 0) != 0) {
            ext2CloseEntry(file->vd, file->ni);
            ext2Unlock(file->vd);
            r->_errno = errno;
            return -1;
        }
        file->ni->ni.i_size = file->ni->ni.i_size_high = 0;
    }

    // Set the files current position
	ext2fs_file_llseek(file->fd, file->append ? EXT2_I_SIZE(&file->ni->ni) : 0, SEEK_SET, 0);

    ext2_log_trace("file->len %lld\n", EXT2_I_SIZE(&file->ni->ni));

    // Update file times
    ext2UpdateTimes(file->vd, file->ni, EXT2_UPDATE_ATIME);

    // Insert the file into the double-linked FILO list of open files
    if (file->vd->firstOpenFile) {
        file->nextOpenFile = file->vd->firstOpenFile;
        file->vd->firstOpenFile->prevOpenFile = file;
    } else {
        file->nextOpenFile = NULL;
    }
    file->prevOpenFile = NULL;
    file->vd->firstOpenFile = file;
    file->vd->openFileCount++;

    // Sync access time
    ext2Sync(file->vd, file->ni);

    // Unlock
    ext2Unlock(file->vd);

    return (int)fileStruct;
}

int ext2_close_r (struct _reent *r, int fd)
{
    ext2_log_trace("fd %p\n", (void *) fd);

    ext2_file_state* file = STATE(fd);

    // Sanity check
    if (!file || !file->vd) {
        r->_errno = EBADF;
        return -1;
    }

    // Lock
    ext2Lock(file->vd);

    // Close the file
    ext2CloseFile(file);

    // Remove the file from the double-linked FILO list of open files
    file->vd->openFileCount--;
    if (file->nextOpenFile)
        file->nextOpenFile->prevOpenFile = file->prevOpenFile;
    if (file->prevOpenFile)
        file->prevOpenFile->nextOpenFile = file->nextOpenFile;
    else
        file->vd->firstOpenFile = file->nextOpenFile;

    // Unlock
    ext2Unlock(file->vd);

    return 0;
}

ssize_t ext2_write_r (struct _reent *r, int fd, const char *ptr, size_t len)
{
    ext2_log_trace("fd %p, ptr %p, len %i\n", (void *) fd, ptr, len);

    ext2_file_state* file = STATE(fd);

    // Sanity check
    if (!file || !file->vd || !file->fd) {
        r->_errno = EINVAL;
        return -1;
    }

    // Short circuit cases where we don't actually have to do anything
    if (!ptr || len <= 0) {
        return 0;
    }

    // Check that we are allowed to write to this file
    if (!file->write) {
        r->_errno = EACCES;
        return -1;
    }

    // Lock
    ext2Lock(file->vd);

    u32 writen = 0;

    // Write to the files data atrribute
    errcode_t err = ext2fs_file_write(file->fd, ptr, len, &writen);
    if (writen <= 0 || err) {
        ext2Unlock(file->vd);
        r->_errno = errno;
        return (err ? err : -1);
    }

    // Unlock
    ext2Unlock(file->vd);

    return (writen == 0 ? -1 : writen);
}

ssize_t ext2_read_r (struct _reent *r, int fd, char *ptr, size_t len)
{
    ext2_log_trace("fd %p, ptr %p, len %i\n", (void *) fd, ptr, len);

    ext2_file_state* file = STATE(fd);

    // Sanity check
    if (!file || !file->vd || !file->fd) {
        r->_errno = EINVAL;
        return -1;
    }

    // Short circuit cases where we don't actually have to do anything
    if (!ptr || len <= 0) {
        return 0;
    }

    // Lock
    ext2Lock(file->vd);

    // Check that we are allowed to read from this file
    if (!file->read) {
        ext2Unlock(file->vd);
        r->_errno = EACCES;
        return -1;
    }

    u32 read = 0;
    errcode_t err = 0;

    // Read from the files data attribute
    err = ext2fs_file_read(file->fd, ptr, len, &read);
    if (err || read <= 0 || read > len) {
        ext2Unlock(file->vd);
        r->_errno = errno;
        return err ? err : -1;
    }

    // Unlock
    ext2Unlock(file->vd);

    return (read == 0) ? -1 : read;
}

off_t ext2_seek_r (struct _reent *r, int fd, off_t pos, int dir)
{
    ext2_log_trace("fd %p, pos %lli, dir %i\n", (void *) fd, pos, dir);

    ext2_file_state* file = STATE(fd);

    // Sanity check
    if (!file || !file->fd) {
        r->_errno = EINVAL;
        return -1;
    }

    u64 pos_loaded = 0;

    ext2fs_file_llseek(file->fd, pos, dir, &pos_loaded);

    return (off_t) pos_loaded;
}

int ext2_fstat_r (struct _reent *r, int fd, struct stat *st)
{
    ext2_log_trace("fd %p\n", (void *) fd);

    ext2_file_state* file = STATE(fd);
    int ret = 0;

    // Sanity check
    if (!file || !file->vd || !file->ni || !file->fd) {
        r->_errno = EINVAL;
        return -1;
    }

    // Short circuit cases were we don't actually have to do anything
    if (!st)
        return 0;

    // Get the file stats
    ret = ext2Stat(file->vd, file->ni, st);
    if (ret)
        r->_errno = errno;

    return ret;
}

int ext2_ftruncate_r (struct _reent *r, int fd, off_t len)
{
    ext2_log_trace("fd %p, len %Li\n", (void *) fd, len);

    ext2_file_state* file = STATE(fd);
    errcode_t err = 0;

    // Sanity check
    if (!file || !file->vd || !file->ni || !file->fd) {
        r->_errno = EINVAL;
        return -1;
    }

    // Lock
    ext2Lock(file->vd);

    // Check that we are allowed to write to this file
    if (!file->write) {
        ext2Unlock(file->vd);
        r->_errno = EACCES;
        return -1;
    }

    err = ext2fs_file_set_size2(file->fd, len);

    // Sync the file (and its attributes) to disc
    if(!err)
        ext2Sync(file->vd, file->ni);

    // update times
    ext2UpdateTimes(file->vd, file->ni, EXT2_UPDATE_AMTIME);

    // Unlock
    ext2Unlock(file->vd);

    return err;
}

int ext2_fsync_r (struct _reent *r, int fd)
{
    ext2_log_trace("fd %p\n", (void *) fd);

    ext2_file_state* file = STATE(fd);
    int ret = 0;

    // Sanity check
    if (!file || !file->fd) {
        r->_errno = EINVAL;
        return -1;
    }

    // Lock
    ext2Lock(file->vd);

    // Sync the file (and its attributes) to disc
    ret = ext2fs_file_flush(file->fd);
    if (ret)
        r->_errno = ret;

    // Unlock
    ext2Unlock(file->vd);

    return ret;
}
