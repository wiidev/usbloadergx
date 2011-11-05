/*
 * inline.c --- Includes the inlined functions defined in the header
 * 	files as standalone functions, in case the application program
 * 	is compiled with inlining turned off.
 *
 * Copyright (C) 1993, 1994 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */


#include "config.h"
#include <stdio.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#include <time.h>
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "ext2_fs.h"
#define INCLUDE_INLINE_FUNCS
#include "ext2fs.h"

#include "mem_allocate.h"

/*
 * We used to define this as an inline, but since we are now using
 * autoconf-defined #ifdef's, we need to export this as a
 * library-provided function exclusively.
 */
errcode_t ext2fs_get_memalign(unsigned long size,
			      unsigned long align, void *ptr)
{
	errcode_t retval;

	if (align == 0)
		align = 8;
#ifdef HAVE_POSIX_MEMALIGN
	retval = posix_memalign((void **) ptr, align, size);
	if (retval) {
		if (retval == ENOMEM)
			return EXT2_ET_NO_MEMORY;
		return retval;
	}
#else
#ifdef HAVE_MEMALIGN
	void *pp;
	pp = mem_align(align, size);
	if (pp == NULL) {
		if (errno)
			return errno;
		else
			return EXT2_ET_NO_MEMORY;
	}
	memcpy(ptr, &pp, sizeof (pp));
#else
#error memalign or posix_memalign must be defined!
#endif
#endif
	return 0;
}

