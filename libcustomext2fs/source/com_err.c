/*
 * Copyright 1987, 1988 by MIT Student Information Processing Board.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose is hereby granted, provided that
 * the names of M.I.T. and the M.I.T. S.I.P.B. not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  M.I.T. and the
 * M.I.T. S.I.P.B. make no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "ext2_fs.h"
#include "ext2fs.h"
#include "ext2_internal.h"

void com_err (const char *whoami,
	      errcode_t code,
	      const char *fmt, ...)
{
    if(whoami)
        ext2_log_trace("%s: ", whoami);

    ext2_log_trace("error code: %i ", (int) code);

    if(fmt)
        ext2_log_trace(fmt);

    ext2_log_trace("\n");
}
