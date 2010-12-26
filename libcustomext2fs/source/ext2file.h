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

#ifndef _EXT2FILE_H
#define _EXT2FILE_H

#include <sys/reent.h>
#include "ext2_internal.h"

/**
 * ext2_file_state - File state
 */
typedef struct _ext2_file_state {
    ext2_vd *vd;                            /* Volume this file belongs to */
    ext2_inode_t *ni;                       /* File inode */
    ext2_file_t fd;                         /* File descriptor */
    int flags;                              /* Opening flags */
    bool read;                              /* True if allowed to read from file */
    bool write;                             /* True if allowed to write to file */
    bool append;                            /* True if allowed to append to file */
//    bool compressed;                        /* True if file data is compressed */
//    bool encrypted;                         /* True if file data is encryted */
    struct _ext2_file_state *prevOpenFile;  /* The previous entry in a double-linked FILO list of open files */
    struct _ext2_file_state *nextOpenFile;  /* The next entry in a double-linked FILO list of open files */
} ext2_file_state;

/* File state routines */
void ext2CloseFile (ext2_file_state *file);

/* Gekko devoptab file routines for EXT2-based devices */
extern int ext2_open_r (struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
extern int ext2_close_r (struct _reent *r, int fd);
extern ssize_t ext2_write_r (struct _reent *r, int fd, const char *ptr, size_t len);
extern ssize_t ext2_read_r (struct _reent *r, int fd, char *ptr, size_t len);
extern off_t ext2_seek_r (struct _reent *r, int fd, off_t pos, int dir);
extern int ext2_fstat_r (struct _reent *r, int fd, struct stat *st);
extern int ext2_ftruncate_r (struct _reent *r, int fd, off_t len);
extern int ext2_fsync_r (struct _reent *r, int fd);

#endif /* _EXT2FILE_H */

