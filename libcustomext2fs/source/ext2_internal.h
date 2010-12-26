/**
 * ext2_internal.h
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
#ifndef EXT2_INTERNAL_H_
#define EXT2_INTERNAL_H_

#include <gccore.h>
#include <ogc/disc_io.h>
#include <sys/iosupport.h>
#include "ext2fs.h"
#include "ext2_fs.h"
#include "mem_allocate.h"

#ifdef DEBUG_GEKKO
#define ext2_log_trace  printf
#else
#define ext2_log_trace(...)
#endif

typedef struct _ext2_inode_t
{
    struct ext2_inode ni;
    ext2_ino_t ino;
    bool dirty;
} ext2_inode_t;

/**
 * ext2_vd - EXT2 volume descriptor
 */
typedef struct _ext2_vd
{
	io_channel io;                          /* EXT device handle */
    ext2_filsys fs;                         /* EXT volume handle */
    mutex_t lock;                           /* Volume lock mutex */
    ext2_inode_t *cwd_ni;                   /* Current directory */
    struct _ext2_dir_state *firstOpenDir;   /* The start of a FILO linked list of currently opened directories */
    struct _ext2_file_state *firstOpenFile; /* The start of a FILO linked list of currently opened files */
    u16 openDirCount;                       /* The total number of directories currently open in this volume */
    u16 openFileCount;                      /* The total number of files currently open in this volume */
    ext2_ino_t root;                        /* Root node */
} ext2_vd;

typedef enum {
	EXT2_UPDATE_ATIME = 0x01,
	EXT2_UPDATE_MTIME = 0x02,
	EXT2_UPDATE_CTIME = 0x04,
	EXT2_UPDATE_AMTIME = EXT2_UPDATE_ATIME | EXT2_UPDATE_MTIME,
	EXT2_UPDATE_ACTIME = EXT2_UPDATE_ATIME | EXT2_UPDATE_CTIME,
	EXT2_UPDATE_MCTIME = EXT2_UPDATE_MTIME | EXT2_UPDATE_CTIME,
	EXT2_UPDATE_AMCTIME = EXT2_UPDATE_ATIME | EXT2_UPDATE_MTIME | EXT2_UPDATE_CTIME,
} ext2_time_update_flags;

/* Lock volume */
static inline void ext2Lock (ext2_vd *vd)
{
    LWP_MutexLock(vd->lock);
}

/* Unlock volume */
static inline void ext2Unlock (ext2_vd *vd)
{
    LWP_MutexUnlock(vd->lock);
}

const char *ext2RealPath (const char *path);
int ext2InitVolume (ext2_vd *vd);
void ext2DeinitVolume (ext2_vd *vd);
ext2_vd *ext2GetVolume (const char *path);

int ext2AddDevice (const char *name, void *deviceData);
void ext2RemoveDevice (const char *path);
const devoptab_t *ext2GetDevice (const char *path);

ext2_inode_t *ext2OpenEntry (ext2_vd *vd, const char *path);
void ext2CloseEntry (ext2_vd *vd, ext2_inode_t * ni);
int ext2Stat (ext2_vd *vd, ext2_inode_t * ni, struct stat *st);
int ext2Sync (ext2_vd *vd, ext2_inode_t * ni);

ext2_inode_t *ext2Create (ext2_vd *vd, const char *path, mode_t type, const char *target);
int ext2Link (ext2_vd *vd, const char *old_path, const char *new_path);
int ext2Unlink (ext2_vd *vd, const char *path);

void ext2UpdateTimes(ext2_vd *vd, ext2_inode_t *ni, ext2_time_update_flags mask);

#endif
