/**
 * ntfsfile.c - devoptab file routines for NTFS-based devices.
 * Copyright (c) 2010 Miigotu
 * Copyright (c) 2009 Rhys "Shareese" Koedijk
 * Copyright (c) 2006 Michael "Chishm" Chisholm
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
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "ntfsinternal.h"
#include "ntfsfile.h"
#include "ntfs.h"
#include "ntfsfile_frag.h"

s64 ntfs_attr_getfragments(ntfs_attr *na, const s64 pos, s64 count, u64 offset,
        _ntfs_frag_append_t append_fragment, void *callback_data);

static inline u8 size_to_shift(u32 size)
{
    u8 ret = 0;
    while (size)
    {
        ret++;
        size >>= 1;
    }
    return ret - 1;
}

int _NTFS_get_fragments (const char *path,
        _ntfs_frag_append_t append_fragment, void *callback_data)
{
    struct _reent r;
    ntfs_file_state file_st, *file = &file_st;
    ssize_t read = 0;
    int ret_val = -11;

    // Open File
    r._errno = 0;
    int fd = ntfs_open_r(&r, file, path, O_RDONLY, 0);
    if (fd != (int)file) return -12;


    // Sanity check
    if (!file || !file->vd || !file->ni || !file->data_na) {
        //r->_errno = EINVAL;
        return -13;
    }

    // Lock
    ntfsLock(file->vd);


    u64 offset = 0;
    u64 len = file->len;

    // Read from the files data attribute
    while (len) {
        s64 ret = ntfs_attr_getfragments(file->data_na, file->pos,
                len, offset, append_fragment, callback_data);
        if (ret <= 0 || ret > len) {
            ntfsUnlock(file->vd);
            //r->_errno = errno;
            ret_val = -14;
			if (ret < 0) ret_val = ret;
            goto out;
        }
        offset += ret;
        len -= ret;
        file->pos += ret;
        read += ret;
    }

    u32 shift = size_to_shift(file->ni->vol->sector_size);

    // set file size
    append_fragment(callback_data, file->len >> shift, 0, 0);
    // success
    ret_val = 0;


out:
    // Unlock
    ntfsUnlock(file->vd);
    // Close the file
    ntfs_close_r (&r, fd);

    return ret_val;
}

