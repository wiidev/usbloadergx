/**
 * attrib.c - Attribute handling code. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2005 Anton Altaparmakov
 * Copyright (c) 2002-2005 Richard Russon
 * Copyright (c) 2002-2008 Szabolcs Szakacsits
 * Copyright (c) 2004-2007 Yura Pakhuchiy
 * Copyright (c) 2007-2009 Jean-Pierre Andre
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
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include "compat.h"
#include "attrib.h"
#include "attrlist.h"
#include "device.h"
#include "mft.h"
#include "debug.h"
#include "mst.h"
#include "volume.h"
#include "types.h"
#include "layout.h"
#include "inode.h"
#include "runlist.h"
#include "lcnalloc.h"
#include "dir.h"
#include "compress.h"
#include "bitmap.h"
#include "logging.h"
#include "misc.h"
#include "efs.h"
#include "ntfs.h"
#include "ntfsfile_frag.h"

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

/**
 * ntfs_attr_pread_i - see description at ntfs_attr_pread()
 */
static s64 ntfs_attr_getfragments_i(ntfs_attr *na, const s64 pos, s64 count, u64 offset,
        _ntfs_frag_append_t append_fragment, void *callback_data)
{
	u64 b = offset;
	s64 br, to_read, ofs, total, total2, max_read, max_init;
	ntfs_volume *vol;
	runlist_element *rl;
	//u16 efs_padding_length;

	/* Sanity checking arguments is done in ntfs_attr_pread(). */

	if ((na->data_flags & ATTR_COMPRESSION_MASK) && NAttrNonResident(na))
	{
		//return -1; // no compressed files
		return -31;
		/*
		if ((na->data_flags & ATTR_COMPRESSION_MASK)
		    == ATTR_IS_COMPRESSED)
			return ntfs_compressed_attr_pread(na, pos, count, b);
		else {
				// compression mode not supported
			errno = EOPNOTSUPP;
			return -1;
		}
		*/
	}
	/*
	 * Encrypted non-resident attributes are not supported.  We return
	 * access denied, which is what Windows NT4 does, too.
	 * However, allow if mounted with efs_raw option
	 */
	vol = na->ni->vol;
	if (!vol->efs_raw && NAttrEncrypted(na) && NAttrNonResident(na)) {
		errno = EACCES;
		//return -1;
		return -32;
	}

	if (!count)
		return 0;
		/*
		 * Truncate reads beyond end of attribute,
		 * but round to next 512 byte boundary for encrypted
		 * attributes with efs_raw mount option
		 */
	max_read = na->data_size;
	max_init = na->initialized_size;
	if (na->ni->vol->efs_raw
	    && (na->data_flags & ATTR_IS_ENCRYPTED)
	    && NAttrNonResident(na)) {
		if (na->data_size != na->initialized_size) {
			ntfs_log_error("uninitialized encrypted file not supported\n");
			errno = EINVAL;
			//return -1;
			return -33;
		}
		max_init = max_read = ((na->data_size + 511) & ~511) + 2;
	}
	if (pos + count > max_read) {
		if (pos >= max_read)
			return 0;
		count = max_read - pos;
	}
	/* If it is a resident attribute, get the value from the mft record. */
	if (!NAttrNonResident(na))
	{
		return -34; // No resident files
		/*
		ntfs_attr_search_ctx *ctx;
		char *val;

		ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
		if (!ctx)
			return -1;
		if (ntfs_attr_lookup(na->type, na->name, na->name_len, 0,
				0, NULL, 0, ctx)) {
res_err_out:
			ntfs_attr_put_search_ctx(ctx);
			return -1;
		}
		val = (char*)ctx->attr + le16_to_cpu(ctx->attr->value_offset);
		if (val < (char*)ctx->attr || val +
				le32_to_cpu(ctx->attr->value_length) >
				(char*)ctx->mrec + vol->mft_record_size) {
			errno = EIO;
			ntfs_log_perror("%s: Sanity check failed", __FUNCTION__);
			goto res_err_out;
		}
		memcpy(b, val + pos, count);
		ntfs_attr_put_search_ctx(ctx);
		return count;
		*/
	}
	total = total2 = 0;
	/* Zero out reads beyond initialized size. */
	if (pos + count > max_init) {
		if (pos >= max_init) {
			//memset(b, 0, count);
			return count;
		}
		total2 = pos + count - max_init;
		count -= total2;
		//memset((u8*)b + count, 0, total2);
	}
		/*
		 * for encrypted non-resident attributes with efs_raw set
		 * the last two bytes aren't read from disk but contain
		 * the number of padding bytes so original size can be
		 * restored
		 */
	if (na->ni->vol->efs_raw &&
			(na->data_flags & ATTR_IS_ENCRYPTED) &&
			((pos + count) > max_init-2))
	{
		return -35; //No encrypted files
		/*
		efs_padding_length = 511 - ((na->data_size - 1) & 511);
		if (pos+count == max_init) {
			if (count == 1) {
				*((u8*)b+count-1) = (u8)(efs_padding_length >> 8);
				count--;
				total2++;
			} else {
				*(u16*)((u8*)b+count-2) = cpu_to_le16(efs_padding_length);
				count -= 2;
				total2 +=2;
			}
		} else {
			*((u8*)b+count-1) = (u8)(efs_padding_length & 0xff);
			count--;
			total2++;
		}
		*/
	}

	/* Find the runlist element containing the vcn. */
	rl = ntfs_attr_find_vcn(na, pos >> vol->cluster_size_bits);
	if (!rl) {
		/*
		 * If the vcn is not present it is an out of bounds read.
		 * However, we already truncated the read to the data_size,
		 * so getting this here is an error.
		 */
		if (errno == ENOENT) {
			errno = EIO;
			ntfs_log_perror("%s: Failed to find VCN #1", __FUNCTION__);
		}
		//return -1;
		return -36;
	}
	/*
	 * Gather the requested data into the linear destination buffer. Note,
	 * a partial final vcn is taken care of by the @count capping of read
	 * length.
	 */
	ofs = pos - (rl->vcn << vol->cluster_size_bits);
	for (; count; rl++, ofs = 0) {
		if (rl->lcn == LCN_RL_NOT_MAPPED) {
			rl = ntfs_attr_find_vcn(na, rl->vcn);
			if (!rl) {
				if (errno == ENOENT) {
					errno = EIO;
					ntfs_log_perror("%s: Failed to find VCN #2",
							__FUNCTION__);
				}
				goto rl_err_out;
			}
			/* Needed for case when runs merged. */
			ofs = pos + total - (rl->vcn << vol->cluster_size_bits);
		}
		if (!rl->length) {
			errno = EIO;
			ntfs_log_perror("%s: Zero run length", __FUNCTION__);
			goto rl_err_out;
		}
		if (rl->lcn < (LCN)0) {
			if (rl->lcn != (LCN)LCN_HOLE) {
				ntfs_log_perror("%s: Bad run (%lld)",
						__FUNCTION__,
						(long long)rl->lcn);
				goto rl_err_out;
			}
			/* It is a hole, just zero the matching @b range. */
			to_read = min(count, (rl->length <<
					vol->cluster_size_bits) - ofs);
			//memset(b, 0, to_read);
			/* Update progress counters. */
			total += to_read;
			count -= to_read;
			b = b + to_read;
			continue;
		}
		/* It is a real lcn, read it into @dst. */
		to_read = min(count, (rl->length << vol->cluster_size_bits) -
				ofs);
retry:
		ntfs_log_trace("Reading %lld bytes from vcn %lld, lcn %lld, ofs"
				" %lld.\n", (long long)to_read, (long long)rl->vcn,
			       (long long )rl->lcn, (long long)ofs);
		/*
		br = ntfs_pread(vol->dev, (rl->lcn << vol->cluster_size_bits) +
				ofs, to_read, b);
		*/
		br = to_read;
		// convert to sectors unit
		u32 shift   = size_to_shift(na->ni->vol->sector_size);
		u32 off_sec   = b >> shift;
		u32 sector    = ((rl->lcn << vol->cluster_size_bits) + ofs) >> shift;
		u32 count_sec = to_read >> shift;
		int ret;
		ret = append_fragment(callback_data, off_sec, sector, count_sec);
		if (ret) {
			if (ret < 0) return ret;
			return -50;
		}
		/* If everything ok, update progress counters and continue. */
		if (br > 0) {
			total += br;
			count -= br;
			b = b + br;
		}
		if (br == to_read)
			continue;
		/* If the syscall was interrupted, try again. */
		if (br == (s64)-1 && errno == EINTR)
			goto retry;
		if (total)
			return total;
		if (!br)
			errno = EIO;
		ntfs_log_perror("%s: ntfs_pread failed", __FUNCTION__);
		//return -1;
		return -38;
	}
	/* Finally, return the number of bytes read. */
	return total + total2;
rl_err_out:
	if (total)
		return total;
	errno = EIO;
	//return -1;
	return -39;
}


/**
 * ntfs_attr_pread - read from an attribute specified by an ntfs_attr structure
 * @na:		ntfs attribute to read from
 * @pos:	byte position in the attribute to begin reading from
 * @count:	number of bytes to read
 * @b:		output data buffer
 *
 * This function will read @count bytes starting at offset @pos from the ntfs
 * attribute @na into the data buffer @b.
 *
 * On success, return the number of successfully read bytes. If this number is
 * lower than @count this means that the read reached end of file or that an
 * error was encountered during the read so that the read is partial. 0 means
 * end of file or nothing was read (also return 0 when @count is 0).
 *
 * On error and nothing has been read, return -1 with errno set appropriately
 * to the return code of ntfs_pread(), or to EINVAL in case of invalid
 * arguments.
 */
s64 ntfs_attr_getfragments(ntfs_attr *na, const s64 pos, s64 count, u64 offset,
        _ntfs_frag_append_t append_fragment, void *callback_data)
{
	s64 ret;

	if (!na || !na->ni || !na->ni->vol || !callback_data || pos < 0 || count < 0) {
		errno = EINVAL;
		ntfs_log_perror("%s: na=%p  b=%p  pos=%lld  count=%lld",
				__FUNCTION__, na, callback_data, (long long)pos,
				(long long)count);
		//return -1;
		return -21;
	}

	/*
	ntfs_log_enter("Entering for inode %lld attr 0x%x pos %lld count "
			"%lld\n", (unsigned long long)na->ni->mft_no,
			na->type, (long long)pos, (long long)count);
	*/

	ret = ntfs_attr_getfragments_i(na, pos, count, offset,
			append_fragment, callback_data);

	//ntfs_log_leave("\n");
	return ret;
}

