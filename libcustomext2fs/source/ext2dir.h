/**
 * ext2_dir.h - devoptab directory routines for EXT2-based devices.
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

#ifndef _EXT2DIR_H
#define _EXT2DIR_H

#include <sys/reent.h>
#include "ext2_internal.h"

/**
 * ext2_dir_entry - Directory entry
 */
typedef struct _ext2_dir_entry {
    char *name;
    struct _ext2_dir_entry *next;
} ext2_dir_entry;

/**
 * ext2_dir_state - Directory state
 */
typedef struct _ext2_dir_state {
    ext2_vd *vd;                            /* Volume this directory belongs to */
    ext2_inode_t *ni;                       /* Directory descriptor */
    unsigned short length;                  /* Directory length */
    ext2_dir_entry *first;                  /* The first entry in the directory */
    ext2_dir_entry *current;                /* The current entry in the directory */
    struct _ext2_dir_state *prevOpenDir;    /* The previous entry in a double-linked FILO list of open directories */
    struct _ext2_dir_state *nextOpenDir;    /* The next entry in a double-linked FILO list of open directories */
} ext2_dir_state;

/* Directory state routines */
void ext2CloseDir (ext2_dir_state *file);

/* Gekko devoptab directory routines for EXT2-based devices */
extern int ext2_stat_r (struct _reent *r, const char *path, struct stat *st);
extern int ext2_link_r (struct _reent *r, const char *existing, const char *newLink);
extern int ext2_unlink_r (struct _reent *r, const char *name);
extern int ext2_chdir_r (struct _reent *r, const char *name);
extern int ext2_rename_r (struct _reent *r, const char *oldName, const char *newName);
extern int ext2_mkdir_r (struct _reent *r, const char *path, int mode);
extern int ext2_statvfs_r (struct _reent *r, const char *path, struct statvfs *buf);

/* Gekko devoptab directory walking routines for EXT2-based devices */
extern DIR_ITER *ext2_diropen_r (struct _reent *r, DIR_ITER *dirState, const char *path);
extern int ext2_dirreset_r (struct _reent *r, DIR_ITER *dirState);
extern int ext2_dirnext_r (struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
extern int ext2_dirclose_r (struct _reent *r, DIR_ITER *dirState);

#endif /* _EXT2DIR_H */

