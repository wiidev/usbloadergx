// Copyright 2012 Dimok based on wiidisc.c:
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#include "gcdisc.h"

static void disc_read(gcdisc_t *d, u32 offset, u8 *data, u32 len)
{
	if (data)
	{
		int ret = 0;
		if (len == 0) return;
		ret = d->read(d->fp, offset, len, data);
		if (ret < 0)
		wbfs_fatal( "error reading disc" );
	}
}

static u32 do_fst(gcdisc_t *d, u8 *fst, const char *names, u32 i)
{
	u32 offset;
	u32 size;
	const char *name;
	u32 j;

	name = names + (wbfs_be32(fst + 12 * i) & 0x00ffffff);
	size = wbfs_be32(fst + 12 * i + 8);

	if (i == 0)
	{
		for (j = 1; j < size && !d->extracted_buffer;)
		{
			j = do_fst(d, fst, names, j);
		}
		return size;
	}

	if (fst[12 * i])
	{

		for (j = i + 1; j < size && !d->extracted_buffer;)
			j = do_fst(d, fst, names, j);

		return size;
	}
	else
	{
		offset = wbfs_be32(fst + 12 * i + 4);

		if (d->extract_pathname && strcasecmp(name, d->extract_pathname) == 0)
		{
			d->extracted_buffer = wbfs_ioalloc( size );
			d->extracted_size = size;
			disc_read(d, offset, d->extracted_buffer, size);
		}
		return i + 1;
	}
}

static void do_files(gcdisc_t*d)
{
	u8 *b = wbfs_ioalloc( 0x480 ); // XXX: determine actual header size
	//u32 dol_offset;
	u32 fst_offset;
	u32 fst_size;
	u8 *fst;
	u32 n_files;
	disc_read(d, 0, b, 0x480);

	//dol_offset = wbfs_be32(b + 0x0420);
	fst_offset = wbfs_be32(b + 0x0424);
	fst_size = wbfs_be32(b + 0x0428);

	if (fst_size)
	{
		fst = wbfs_ioalloc( fst_size );
		if (fst == 0)
		wbfs_fatal( "malloc fst" );
		disc_read(d, fst_offset, fst, fst_size);
		n_files = wbfs_be32(fst + 8);


		if (d->extract_pathname && strcmp(d->extract_pathname, "FST") == 0)
		{
			// if empty pathname requested return fst
			d->extracted_buffer = fst;
			d->extracted_size = fst_size;
			d->extract_pathname = NULL;
			// skip do_fst if only fst requested
			n_files = 0;
		}

		if (12 * n_files <= fst_size)
		{
			if (n_files > 1) do_fst(d, fst, (char *) fst + 12 * n_files, 0);
		}

		if (fst != d->extracted_buffer) wbfs_iofree( fst );
	}
	wbfs_iofree( b );
}

static void do_disc(gcdisc_t*d)
{
	u8 *b = wbfs_ioalloc( 0x100 );
	u32 magic;
	disc_read(d, 0, b, 0x100);
	magic = wbfs_be32(b + 28);
	if (magic != 0xC2339F3D)
	{
		wbfs_iofree( b );
		wbfs_error( "not a gc disc" );
		return;
	}
	wbfs_iofree( b );

	do_files(d);
}

gcdisc_t *gc_open_disc(read_wiidisc_callback_t read, void*fp)
{
	gcdisc_t *d = wbfs_malloc( sizeof( gcdisc_t ) );
	if (!d) return 0;
	wbfs_memset( d, 0, sizeof( gcdisc_t ) );
	d->read = read;
	d->fp = fp;

	return d;
}
void gc_close_disc(gcdisc_t *d)
{
	wbfs_free( d );
}

u8 * gc_extract_file(gcdisc_t *d, const char *pathname)
{
	u8 *retval = 0;
	d->extract_pathname = pathname;
	d->extracted_buffer = 0;
	do_disc(d);
	d->extract_pathname = 0;
	retval = d->extracted_buffer;
	return retval;
}
