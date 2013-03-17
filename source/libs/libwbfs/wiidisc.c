// Copyright 2009 Kwiirk based on negentig.c:
// Copyright 2007,2008  Segher Boessenkool  <segher@kernel.crashing.org>
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#include "wiidisc.h"

void aes_set_key(u8 *key);
void aes_decrypt(u8 *iv, u8 *inbuf, u8 *outbuf, unsigned long long len);

void _decrypt_title_key(u8 *tik, u8 *title_key)
{
	u8 common_key[16] = { 0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48, 0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7 };
	u8 korean_key[16]={ 0x63, 0xb8, 0x2b, 0xb4, 0xf4, 0x61, 0x4e, 0x2e, 0x13, 0xf2, 0xfe, 0xfb, 0xba, 0x4c, 0x9b, 0x7e }; //korean common key
	u8 iv[16];

	wbfs_memset( iv, 0, sizeof iv );
	wbfs_memcpy( iv, tik + 0x01dc, 8 );

	//check byte 0x1f1 in ticket to determine whether or not to use Korean Common Key.
	//if value = 0x01, use Korean Common Key, else just use regular one             -dmm
	//Also check the GameID region code as some channels are using wrong ticket with 0x01    -Cyan
	if(tik[0x01f1] == 0x01 && (tik[0x01e3] == 'K' || tik[0x01e3] == 'Q' || tik[0x01e3] == 'T')){
			aes_set_key(korean_key);
	} else {
			aes_set_key(common_key);
	}

	aes_decrypt(iv, tik + 0x01bf, title_key, 16);
}

static void disc_read(wiidisc_t *d, u32 offset, u8 *data, u32 len)
{
	if (data)
	{
		int ret = 0;
		if (len == 0) return;
		ret = d->read(d->fp, offset, len, data);
		if (ret)
		wbfs_fatal( "error reading disc" );
	}
	if (d->sector_usage_table)
	{
		u32 blockno = offset >> 13;
		do
		{
			d->sector_usage_table[blockno] = 1;
			blockno += 1;
			if (len > 0x8000) len -= 0x8000;
		} while (len > 0x8000);
	}
}

static void partition_raw_read(wiidisc_t *d, u32 offset, u8 *data, u32 len)
{
	disc_read(d, d->partition_raw_offset + offset, data, len);
}

static void partition_read_block(wiidisc_t *d, u32 blockno, u8 *block)
{
	u8*raw = d->tmp_buffer;
	u8 iv[16];
	u32 offset;
	if (d->sector_usage_table) d->sector_usage_table[d->partition_block + blockno] = 1;
	offset = d->partition_data_offset + ((0x8000 >> 2) * blockno);
	partition_raw_read(d, offset, raw, 0x8000);

	// decrypt data
	memcpy(iv, raw + 0x3d0, 16);
	aes_set_key(d->disc_key);
	aes_decrypt(iv, raw + 0x400, block, 0x7c00);
}

static void partition_read(wiidisc_t *d, u32 offset, u8 *data, u32 len, int fake)
{
	u8 *block = d->tmp_buffer2;
	u32 offset_in_block;
	u32 len_in_block;
	if (fake && d->sector_usage_table == 0) return;

	while (len)
	{
		offset_in_block = offset % (0x7c00 >> 2);
		len_in_block = 0x7c00 - (offset_in_block << 2);
		if (len_in_block > len) len_in_block = len;
		if (!fake)
		{
			partition_read_block(d, offset / (0x7c00 >> 2), block);
			wbfs_memcpy( data, block + ( offset_in_block << 2 ), len_in_block );
		}
		else d->sector_usage_table[d->partition_block + (offset / (0x7c00 >> 2))] = 1;
		data += len_in_block;
		offset += len_in_block >> 2;
		len -= len_in_block;
	}
}

static u32 do_fst(wiidisc_t *d, u8 *fst, const char *names, u32 i)
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
	//printf("name	%s\n",name);

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
			partition_read(d, offset, d->extracted_buffer, size, 0);
		}
		else partition_read(d, offset, 0, size, 1);
		return i + 1;
	}
}

static void do_files(wiidisc_t*d)
{
	u8 *b = wbfs_ioalloc( 0x480 ); // XXX: determine actual header size
	u32 dol_offset;
	u32 fst_offset;
	u32 fst_size;
	u32 apl_offset;
	u32 apl_size;
	u8 *apl_header = wbfs_ioalloc( 0x20 );
	u8 *fst;
	u32 n_files;
	partition_read(d, 0, b, 0x480, 0);

	dol_offset = wbfs_be32(b + 0x0420);
	fst_offset = wbfs_be32(b + 0x0424);
	fst_size = wbfs_be32(b + 0x0428) << 2;

	apl_offset = 0x2440 >> 2;
	partition_read(d, apl_offset, apl_header, 0x20, 0);
	apl_size = 0x20 + wbfs_be32(apl_header + 0x14) + wbfs_be32(apl_header + 0x18);
	// fake read dol and partition
	if (apl_size) partition_read(d, apl_offset, 0, apl_size, 1);
	partition_read(d, dol_offset, 0, (fst_offset - dol_offset) << 2, 1);

	if (fst_size)
	{
		fst = wbfs_ioalloc( fst_size );
		if (fst == 0)
		wbfs_fatal( "malloc fst" );
		partition_read(d, fst_offset, fst, fst_size, 0);
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
	wbfs_iofree( apl_header );
}

static void do_partition(wiidisc_t*d)
{
	u8 *tik = wbfs_ioalloc( 0x2a4 );
	u8 *b = wbfs_ioalloc( 0x1c );
	u64 tmd_offset;
	u32 tmd_size;
	u8 *tmd;
	u64 cert_offset;
	u32 cert_size;
	u8 *cert;
	u64 h3_offset;

	// read ticket, and read some offsets and sizes
	partition_raw_read(d, 0, tik, 0x2a4);
	partition_raw_read(d, 0x2a4 >> 2, b, 0x1c);

	tmd_size = wbfs_be32(b);
	tmd_offset = wbfs_be32(b + 4);
	cert_size = wbfs_be32(b + 8);
	cert_offset = wbfs_be32(b + 0x0c);
	h3_offset = wbfs_be32(b + 0x10);
	d->partition_data_offset = wbfs_be32(b + 0x14);
	d->partition_block = (d->partition_raw_offset + d->partition_data_offset) >> 13;
	tmd = wbfs_ioalloc( tmd_size );
	if (tmd == 0)
	wbfs_fatal( "malloc tmd" );
	partition_raw_read(d, tmd_offset, tmd, tmd_size);

	if(d->extract_pathname && strcmp(d->extract_pathname, "TMD") == 0 && !d->extracted_buffer)
	{
		d->extracted_buffer = tmd;
		d->extracted_size = tmd_size;
	}

	cert = wbfs_ioalloc( cert_size );
	if (cert == 0)
	wbfs_fatal( "malloc cert" );
	partition_raw_read(d, cert_offset, cert, cert_size);

	_decrypt_title_key(tik, d->disc_key);

	partition_raw_read(d, h3_offset, 0, 0x18000);

	wbfs_iofree( b );
	wbfs_iofree( tik );
	wbfs_iofree( cert );
	if(tmd != d->extracted_buffer)
		wbfs_iofree( tmd );

	do_files(d);

}
static int test_parition_skip(u32 partition_type, partition_selector_t part_sel)
{
	switch (part_sel)
	{
		case ALL_PARTITIONS:
			return 0;
		case REMOVE_UPDATE_PARTITION:
			return (partition_type == 1);
		case ONLY_GAME_PARTITION:
			return (partition_type != 0);
		default:
			return (partition_type != part_sel);
	}
}
static void do_disc(wiidisc_t*d)
{
	u8 *b = wbfs_ioalloc( 0x100 );
	u64 partition_offset[32]; // XXX: don't know the real maximum
	u64 partition_type[32]; // XXX: don't know the real maximum
	u32 n_partitions;
	u32 magic;
	u32 i;
	disc_read(d, 0, b, 0x100);
	magic = wbfs_be32(b + 24);
	if (magic != 0x5D1C9EA3)
	{
		wbfs_iofree( b );
		wbfs_error( "not a wii disc" );
		return;
	}
	disc_read(d, 0x40000 >> 2, b, 0x100);
	n_partitions = wbfs_be32(b);
	disc_read(d, wbfs_be32(b + 4), b, 0x100);
	for (i = 0; i < n_partitions; i++)
	{
		partition_offset[i] = wbfs_be32(b + 8 * i);
		partition_type[i] = wbfs_be32(b + 8 * i + 4);
	}
	for (i = 0; i < n_partitions; i++)
	{
		d->partition_raw_offset = partition_offset[i];
		if (!test_parition_skip(partition_type[i], d->part_sel)) do_partition(d);
	}
	wbfs_iofree( b );
}

wiidisc_t *wd_open_disc(read_wiidisc_callback_t read, void*fp)
{
	wiidisc_t *d = wbfs_malloc( sizeof( wiidisc_t ) );
	if (!d) return 0;
	wbfs_memset( d, 0, sizeof( wiidisc_t ) );
	d->read = read;
	d->fp = fp;
	d->part_sel = ALL_PARTITIONS;
	d->tmp_buffer = wbfs_ioalloc( 0x8000 );
	d->tmp_buffer2 = wbfs_malloc( 0x8000 );

	return d;
}
void wd_close_disc(wiidisc_t *d)
{
	wbfs_iofree( d->tmp_buffer );
	wbfs_free( d->tmp_buffer2 );
	wbfs_free( d );
}
// returns a buffer allocated with wbfs_ioalloc() or NULL if not found of alloc error
// XXX pathname not implemented. files are extracted by their name.
// first file found with that name is returned.
u8 * wd_extract_file(wiidisc_t *d, partition_selector_t partition_type, char *pathname)
{
	u8 *retval = 0;
	d->extract_pathname = pathname;
	d->extracted_buffer = 0;
	d->part_sel = partition_type;
	do_disc(d);
	d->extract_pathname = 0;
	d->part_sel = ALL_PARTITIONS;
	retval = d->extracted_buffer;
	return retval;
}

void wd_build_disc_usage(wiidisc_t *d, partition_selector_t selector, u8* usage_table)
{
	d->sector_usage_table = usage_table;
	wbfs_memset( usage_table, 0, 143432*2 );
	d->part_sel = selector;
	do_disc(d);
	d->part_sel = ALL_PARTITIONS;
	d->sector_usage_table = 0;
}

void wd_fix_partition_table(wiidisc_t *d, partition_selector_t selector, u8* partition_table)
{
	u8 *b = partition_table;
	u32 partition_offset;
	u32 partition_type;
	u32 n_partitions, i, j;
	u32 *b32;
	if (selector == ALL_PARTITIONS) return;
	n_partitions = wbfs_be32(b);
	if (wbfs_be32(b + 4) - (0x40000 >> 2) > 0x50)
	wbfs_fatal( "cannot modify this partition table. Please report the bug." );

	b += (wbfs_be32(b + 4) - (0x40000 >> 2)) * 4;
	j = 0;
	for (i = 0; i < n_partitions; i++)
	{
		partition_offset = wbfs_be32(b + 8 * i);
		partition_type = wbfs_be32(b + 8 * i + 4);
		if (!test_parition_skip(partition_type, selector))
		{
			b32 = (u32*) (b + 8 * j);
			b32[0] = wbfs_htonl( partition_offset );
			b32[1] = wbfs_htonl( partition_type );
			j++;
		}
	}
	b32 = (u32*) (partition_table);
	*b32 = wbfs_htonl( j );
}

