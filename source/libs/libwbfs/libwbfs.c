// Copyright 2009 Kwiirk
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

// Modified by oggzee

#include "libwbfs.h"

#define likely(x)	   __builtin_expect(!!(x), 1)
#define unlikely(x)	 __builtin_expect(!!(x), 0)

#define ERROR(x) do {wbfs_error(x);goto error;}while(0)
#define ALIGN_LBA(x) (((x)+p->hd_sec_sz-1)&(~(p->hd_sec_sz-1)))

wbfs_t wbfs_iso_file;

static int force_mode = 0;

void wbfs_set_force_mode(int force)
{
	force_mode = force;
}

static u8 size_to_shift(u32 size)
{
	u8 ret = 0;
	while (size)
	{
		ret++;
		size >>= 1;
	}
	return ret - 1;
}
#define read_le32_unaligned(x) ((x)[0]|((x)[1]<<8)|((x)[2]<<16)|((x)[3]<<24))

wbfs_t*wbfs_open_hd(rw_sector_callback_t read_hdsector, rw_sector_callback_t write_hdsector, void *callback_data,
		int hd_sector_size, int num_hd_sector __attribute( ( unused ) ), int reset)
{
	int i = num_hd_sector, ret;
	u8 *ptr, *tmp_buffer = wbfs_ioalloc( hd_sector_size );
	u8 part_table[16 * 4];
	ret = read_hdsector(callback_data, 0, 1, tmp_buffer);
	if (ret) return 0;
	//find wbfs partition
	wbfs_memcpy( part_table, tmp_buffer + 0x1be, 16*4 );
	ptr = part_table;
	for (i = 0; i < 4; i++, ptr += 16)
	{
		u32 part_lba = read_le32_unaligned( ptr + 0x8 );
		wbfs_head_t *head = (wbfs_head_t *) tmp_buffer;
		ret = read_hdsector(callback_data, part_lba, 1, tmp_buffer);
		// verify there is the magic.
		if (head->magic == wbfs_htonl( WBFS_MAGIC ))
		{
			wbfs_t*p = wbfs_open_partition(read_hdsector, write_hdsector, callback_data, hd_sector_size, 0, part_lba,
					reset);
			wbfs_iofree( tmp_buffer );
			return p;
		}
	}
	wbfs_iofree( tmp_buffer );
	if (reset)// XXX make a empty hd partition..
	{
	}
	return 0;
}
wbfs_t*wbfs_open_partition(rw_sector_callback_t read_hdsector, rw_sector_callback_t write_hdsector,
		void *callback_data, int hd_sector_size, int num_hd_sector, u32 part_lba, int reset)
{
	wbfs_t *p = wbfs_malloc( sizeof( wbfs_t ) );

	wbfs_head_t *head = wbfs_ioalloc( hd_sector_size ? hd_sector_size : 512 );

	//constants, but put here for consistancy
	p->wii_sec_sz = 0x8000;
	p->wii_sec_sz_s = size_to_shift(0x8000);
	p->n_wii_sec = (num_hd_sector / 0x8000) * hd_sector_size;
	p->n_wii_sec_per_disc = 143432 * 2;//support for double layers discs..
	p->head = head;
	p->part_lba = part_lba;
	// init the partition
	if (reset)
	{
		u8 sz_s;
		wbfs_memset( head, 0, hd_sector_size );
		head->magic = wbfs_htonl( WBFS_MAGIC );
		head->hd_sec_sz_s = size_to_shift(hd_sector_size);
		head->n_hd_sec = wbfs_htonl( num_hd_sector );
		// choose minimum wblk_sz that fits this partition size
		for (sz_s = 6; sz_s < 11; sz_s++)
		{
			// ensure that wbfs_sec_sz is big enough to address every blocks using 16 bits
			if (p->n_wii_sec < ((1U << 16) * (1 << sz_s))) break;
		}
		head->wbfs_sec_sz_s = sz_s + p->wii_sec_sz_s;
	}
	else read_hdsector(callback_data, p->part_lba, 1, head);
	if (head->magic != wbfs_htonl( WBFS_MAGIC ))
	ERROR( "bad magic" );
	if (!force_mode && hd_sector_size && head->hd_sec_sz_s != size_to_shift(hd_sector_size))
	ERROR( "hd sector size doesn't match" );
	if (!force_mode && num_hd_sector && head->n_hd_sec != (u32) wbfs_htonl( num_hd_sector ))
	ERROR( "hd num sector doesn't match" );
	p->hd_sec_sz = 1 << head->hd_sec_sz_s;
	p->hd_sec_sz_s = head->hd_sec_sz_s;
	p->n_hd_sec = wbfs_ntohl( head->n_hd_sec );

	p->n_wii_sec = (p->n_hd_sec / p->wii_sec_sz) * (p->hd_sec_sz);

	p->wbfs_sec_sz_s = head->wbfs_sec_sz_s;
	p->wbfs_sec_sz = 1 << p->wbfs_sec_sz_s;
	p->n_wbfs_sec = p->n_wii_sec >> (p->wbfs_sec_sz_s - p->wii_sec_sz_s);
	p->n_wbfs_sec_per_disc = p->n_wii_sec_per_disc >> (p->wbfs_sec_sz_s - p->wii_sec_sz_s);
	p->disc_info_sz = ALIGN_LBA( sizeof( wbfs_disc_info_t ) + p->n_wbfs_sec_per_disc * 2 );

	//printf("hd_sector_size %X wii_sector size %X wbfs sector_size %X\n",p->hd_sec_sz,p->wii_sec_sz,p->wbfs_sec_sz);
	p->read_hdsector = read_hdsector;
	p->write_hdsector = write_hdsector;
	p->callback_data = callback_data;

	p->freeblks_lba = (p->wbfs_sec_sz - p->n_wbfs_sec / 8) >> p->hd_sec_sz_s;

	if (!reset)
		p->freeblks = 0; // will alloc and read only if needed
	else
	{
		// init with all free blocks
		p->freeblks = wbfs_ioalloc( ALIGN_LBA( p->n_wbfs_sec / 8 ) );
		wbfs_memset( p->freeblks, 0xff, p->n_wbfs_sec / 8 );
	}
	p->max_disc = (p->freeblks_lba - 1) / (p->disc_info_sz >> p->hd_sec_sz_s);
	if (p->max_disc > p->hd_sec_sz - sizeof(wbfs_head_t)) p->max_disc = p->hd_sec_sz - sizeof(wbfs_head_t);

	p->tmp_buffer = wbfs_ioalloc( p->hd_sec_sz );
	p->n_disc_open = 0;
	return p;
	error: wbfs_free( p );
	wbfs_iofree( head );
	return 0;

}

void wbfs_sync(wbfs_t*p)
{
	// copy back descriptors
	if (p->write_hdsector)
	{
		p->write_hdsector(p->callback_data, p->part_lba + 0, 1, p->head);

		if (p->freeblks) p->write_hdsector(p->callback_data, p->part_lba + p->freeblks_lba,
				ALIGN_LBA( p->n_wbfs_sec / 8 ) >> p->hd_sec_sz_s, p->freeblks);
	}
}

void wbfs_close(wbfs_t*p)
{
	wbfs_sync(p);

	if (p->n_disc_open)
	ERROR( "trying to close wbfs while discs still open" );

	wbfs_iofree( p->head );
	wbfs_iofree( p->tmp_buffer );
	if (p->freeblks) wbfs_iofree( p->freeblks );

	wbfs_free( p );

	error: return;
}

wbfs_disc_t *wbfs_open_disc(wbfs_t* p, u8 *discid)
{
	u32 i;
	int disc_info_sz_lba = p->disc_info_sz >> p->hd_sec_sz_s;
	wbfs_disc_t *d = 0;
	for (i = 0; i < p->max_disc; i++)
	{
		if (p->head->disc_table[i])
		{
			p->read_hdsector(p->callback_data, p->part_lba + 1 + i * disc_info_sz_lba, 1, p->tmp_buffer);
			if (wbfs_memcmp( discid, p->tmp_buffer, 6 ) == 0)
			{
				d = wbfs_malloc( sizeof( *d ) );
				if (!d)
				ERROR( "allocating memory" );
				d->p = p;
				d->i = i;
				d->header = wbfs_ioalloc( p->disc_info_sz );
				if (!d->header)
				ERROR( "allocating memory" );
				p->read_hdsector(p->callback_data, p->part_lba + 1 + i * disc_info_sz_lba, disc_info_sz_lba, d->header);
				p->n_disc_open++;
				//								for(i=0;i<p->n_wbfs_sec_per_disc;i++)
				//										printf("%d,",wbfs_ntohs(d->header->wlba_table[i]));
				return d;
			}
		}
	}
	return 0;
	error: if (d) wbfs_iofree( d );
	return 0;

}
void wbfs_close_disc(wbfs_disc_t*d)
{
	d->p->n_disc_open--;
	wbfs_iofree( d->header );
	wbfs_free( d );
}
// offset is pointing 32bit words to address the whole dvd, although len is in bytes
int wbfs_disc_read(wbfs_disc_t*d, u32 offset, u32 len, u8 *data)
{
	if (d->p == &wbfs_iso_file)
	{
		return wbfs_iso_file_read(d, offset, data, len);
	}

	wbfs_t *p = d->p;
	u16 wlba = offset >> (p->wbfs_sec_sz_s - 2);
	u32 iwlba_shift = p->wbfs_sec_sz_s - p->hd_sec_sz_s;
	u32 lba_mask = (p->wbfs_sec_sz - 1) >> (p->hd_sec_sz_s);
	u32 lba = (offset >> (p->hd_sec_sz_s - 2)) & lba_mask;
	u32 off = offset & ((p->hd_sec_sz >> 2) - 1);
	u16 iwlba = wbfs_ntohs( d->header->wlba_table[wlba] );
	u32 len_copied;
	int err = 0;
	u8 *ptr = data;
	if (unlikely( iwlba == 0 )) return 1;
	if (unlikely( off ))
	{
		off *= 4;
		err = p->read_hdsector(p->callback_data, p->part_lba + (iwlba << iwlba_shift) + lba, 1, p->tmp_buffer);
		if (err) return err;
		len_copied = p->hd_sec_sz - off;
		if (likely( len < len_copied )) len_copied = len;
		wbfs_memcpy( ptr, p->tmp_buffer + off, len_copied );
		len -= len_copied;
		ptr += len_copied;
		lba++;
		if (unlikely( lba > lba_mask && len ))
		{
			lba = 0;
			iwlba = wbfs_ntohs( d->header->wlba_table[++wlba] );
			if (unlikely( iwlba == 0 )) return 1;
		}
	}
	while (likely( len >= p->hd_sec_sz ))
	{
		u32 nlb = len >> (p->hd_sec_sz_s);

		if (unlikely( lba + nlb > p->wbfs_sec_sz )) // dont cross wbfs sectors..
		nlb = p->wbfs_sec_sz - lba;
		err = p->read_hdsector(p->callback_data, p->part_lba + (iwlba << iwlba_shift) + lba, nlb, ptr);
		if (err) return err;
		len -= nlb << p->hd_sec_sz_s;
		ptr += nlb << p->hd_sec_sz_s;
		lba += nlb;
		if (unlikely( lba > lba_mask && len ))
		{
			lba = 0;
			iwlba = wbfs_ntohs( d->header->wlba_table[++wlba] );
			if (unlikely( iwlba == 0 )) return 1;
		}
	}
	if (unlikely( len ))
	{
		err = p->read_hdsector(p->callback_data, p->part_lba + (iwlba << iwlba_shift) + lba, 1, p->tmp_buffer);
		if (err) return err;
		wbfs_memcpy( ptr, p->tmp_buffer, len );
	}
	return 0;
}

// disc listing
u32 wbfs_count_discs(wbfs_t*p)
{
	u32 i, count = 0;
	for (i = 0; i < p->max_disc; i++)
		if (p->head->disc_table[i]) count++;
	return count;

}

u32 wbfs_sector_used(wbfs_t *p, wbfs_disc_info_t *di)
{
	u32 tot_blk = 0, j;
	for (j = 0; j < p->n_wbfs_sec_per_disc; j++)
		if (wbfs_ntohs( di->wlba_table[j] )) tot_blk++;
	return tot_blk;
}

u32 wbfs_get_disc_info(wbfs_t*p, u32 index, u8 *header, int header_size, u32 *size)//size in 32 bit
{
	u32 i, count = 0;
	if (!p) return 1;
	int disc_info_sz_lba = p->disc_info_sz >> p->hd_sec_sz_s;

	for (i = 0; i < p->max_disc; i++)
		if (p->head->disc_table[i])
		{
			if (count++ == index)
			{
				p->read_hdsector(p->callback_data, p->part_lba + 1 + i * disc_info_sz_lba, 1, p->tmp_buffer);
				if (header_size > (int) p->hd_sec_sz) header_size = p->hd_sec_sz;
				u32 magic = wbfs_ntohl( *( u32* )( p->tmp_buffer + 24 ) );
				if (magic != 0x5D1C9EA3)
				{
					p->head->disc_table[i] = 0;
					return 1;
				}
				memcpy(header, p->tmp_buffer, header_size);
				if (size)
				{
					u8 *header = wbfs_ioalloc( p->disc_info_sz );
					p->read_hdsector(p->callback_data, p->part_lba + 1 + i * disc_info_sz_lba, disc_info_sz_lba, header);
					u32 sec_used = wbfs_sector_used(p, (wbfs_disc_info_t *) header);
					wbfs_iofree( header );
					*size = sec_used << (p->wbfs_sec_sz_s - 2);
				}
				return 0;
			}
		}
	return 1;
}

static void load_freeblocks(wbfs_t*p)
{
	if (p->freeblks) return;
	// XXX should handle malloc error..
	p->freeblks = wbfs_ioalloc( ALIGN_LBA( p->n_wbfs_sec / 8 ) );
	p->read_hdsector(p->callback_data, p->part_lba + p->freeblks_lba, ALIGN_LBA( p->n_wbfs_sec / 8 ) >> p->hd_sec_sz_s,
			p->freeblks);

}
u32 wbfs_count_usedblocks(wbfs_t*p)
{
	u32 i, j, count = 0;
	load_freeblocks(p);
	for (i = 0; i < p->n_wbfs_sec / (8 * 4); i++)
	{
		u32 v = wbfs_ntohl( p->freeblks[i] );
		if (v == ~0U)
			count += 32;
		else if (v != 0) for (j = 0; j < 32; j++)
			if (v & (1 << j)) count++;
	}
	return count;
}

// write access


//static
int block_used(u8 *used, u32 i, u32 wblk_sz)
{
	u32 k;
	i *= wblk_sz;
	for (k = 0; k < wblk_sz; k++)
		if (i + k < 143432 * 2 && used[i + k]) return 1;
	return 0;
}

static u32 alloc_block(wbfs_t*p)
{
	u32 i, j;
	for (i = 0; i < p->n_wbfs_sec / (8 * 4); i++)
	{
		u32 v = wbfs_ntohl( p->freeblks[i] );
		if (v != 0)
		{
			for (j = 0; j < 32; j++)
				if (v & (1 << j))
				{
					p->freeblks[i] = wbfs_htonl( v & ~( 1 << j ) );
					return (i * 32) + j + 1;
				}
		}
	}
	return ~0;
}
static void free_block(wbfs_t *p, int bl)
{
	int i = (bl - 1) / (32);
	int j = (bl - 1) & 31;
	u32 v = wbfs_ntohl( p->freeblks[i] );
	p->freeblks[i] = wbfs_htonl( v | 1 << j );
}

int install_abort_signal = 0;

s32 wbfs_add_disc(wbfs_t*p, read_wiidisc_callback_t read_src_wii_disc, void *callback_data,
		progress_callback_t spinner, partition_selector_t sel, int copy_1_1)
{
	int i, discn, ret;
	u32 tot, cur;
	u32 wii_sec_per_wbfs_sect = 1 << (p->wbfs_sec_sz_s - p->wii_sec_sz_s);
	wiidisc_t *d = 0;
	u8 *used = 0;
	wbfs_disc_info_t *info = 0;
	u8* copy_buffer = 0;
	int retval = -1;
	int num_wbfs_sect_to_copy;
	u32 last_used;
	used = wbfs_malloc( p->n_wii_sec_per_disc );

	if (!used)
	ERROR( "unable to alloc memory" );
	// copy_1_1 needs disk usage for layers detection
	//if(!copy_1_1)
	{
		d = wd_open_disc(read_src_wii_disc, callback_data);
		if (!d)
		ERROR( "unable to open wii disc" );
		wd_build_disc_usage(d, sel, used);
		wd_close_disc(d);
		d = 0;
	}

	for (i = 0; i < p->max_disc; i++)// find a free slot.
		if (p->head->disc_table[i] == 0) break;
	if (i == p->max_disc)
	ERROR( "no space left on device (table full)" );
	p->head->disc_table[i] = 1;
	discn = i;
	load_freeblocks(p);

	// build disc info
	info = wbfs_ioalloc( p->disc_info_sz );
	read_src_wii_disc(callback_data, 0, 0x100, info->disc_header_copy);

	copy_buffer = wbfs_ioalloc( p->wii_sec_sz );
	if (!copy_buffer)
	ERROR( "alloc memory" );
	tot = 0;
	cur = 0;
	num_wbfs_sect_to_copy = p->n_wbfs_sec_per_disc;
	// count total number of sectors to write
	last_used = 0;
	for (i = 0; i < num_wbfs_sect_to_copy; i++)
	{
		if (block_used(used, i, wii_sec_per_wbfs_sect))
		{
			tot += wii_sec_per_wbfs_sect;
			last_used = i;
		}
	}
	if (copy_1_1)
	{
		// detect single or dual layer
		if ((last_used + 1) > (p->n_wbfs_sec_per_disc / 2))
		{
			// dual layer
			num_wbfs_sect_to_copy = p->n_wbfs_sec_per_disc;
		}
		else
		{
			// single layer
			num_wbfs_sect_to_copy = p->n_wbfs_sec_per_disc / 2;
		}
		tot = num_wbfs_sect_to_copy * wii_sec_per_wbfs_sect;
	}
	
	install_abort_signal = 0;
	/*
	 // num of hd sectors to copy could be specified directly
	 if (copy_1_1 > 1) {
	 u32 hd_sec_per_wii_sec = p->wii_sec_sz / p->hd_sec_sz;
	 num_wbfs_sect_to_copy = copy_1_1 / hd_sec_per_wii_sec / wii_sec_per_wbfs_sect;
	 tot = num_wbfs_sect_to_copy * wii_sec_per_wbfs_sect;
	 }*/
	if (spinner) spinner(0, tot);
	for (i = 0; i < num_wbfs_sect_to_copy; i++)
	{
		u16 bl = 0;
		if (copy_1_1 || block_used(used, i, wii_sec_per_wbfs_sect))
		{
			u16 j;

			bl = alloc_block(p);
			if (bl == 0xffff)
			ERROR( "no space left on device (disc full)" );
			for (j = 0; j < wii_sec_per_wbfs_sect; j++)
			{
				u32 offset = (i * (p->wbfs_sec_sz >> 2)) + (j * (p->wii_sec_sz >> 2));

				ret = read_src_wii_disc(callback_data, offset, p->wii_sec_sz, copy_buffer);
				if (ret)
				{
					if (copy_1_1 && i > p->n_wbfs_sec_per_disc / 2)
					{
						// end of dual layer data
						if (j > 0)
						{
							info->wlba_table[i] = wbfs_htons( bl );
						}
						spinner(tot, tot);
						break;
					}
					//ERROR("read error");
					printf("\rWARNING: read (%u) error (%d)\n", (unsigned int)offset, ret);
				}

				//fix the partition table
				if (offset == (0x40000 >> 2)) wd_fix_partition_table(d, sel, copy_buffer);
				p->write_hdsector(p->callback_data, p->part_lba + bl * (p->wbfs_sec_sz / p->hd_sec_sz) + j
						* (p->wii_sec_sz / p->hd_sec_sz), p->wii_sec_sz / p->hd_sec_sz, copy_buffer);
				cur++;
				if (spinner) spinner(cur, tot);
			}
		}
		if (install_abort_signal)
			break;
		info->wlba_table[i] = wbfs_htons( bl );
	}

	if(install_abort_signal)
	{
		int n;
		for(n = 0; n < i; n++)
		{
			u32 iwlba = wbfs_ntohs(info->wlba_table[n]);
			if (iwlba)
				free_block(p,iwlba);
		}
		wbfs_memset(info,0,p->disc_info_sz);
		p->head->disc_table[discn] = 0;
	}
	// write disc info
	int disc_info_sz_lba = p->disc_info_sz >> p->hd_sec_sz_s;
	p->write_hdsector(p->callback_data, p->part_lba + 1 + discn * disc_info_sz_lba, disc_info_sz_lba, info);
	wbfs_sync(p);
	retval = 0;
	error: if (d) wd_close_disc(d);
	if (used) wbfs_free( used );
	if (info) wbfs_iofree( info );
	if (copy_buffer) wbfs_iofree( copy_buffer );
	// init with all free blocks

	return retval;
}

u32 wbfs_rm_disc(wbfs_t*p, u8* discid)
{
	wbfs_disc_t *d = wbfs_open_disc(p, discid);
	int i;
	int discn = 0;
	int disc_info_sz_lba = p->disc_info_sz >> p->hd_sec_sz_s;
	if (!d) return 1;
	load_freeblocks(p);
	discn = d->i;
	for (i = 0; i < p->n_wbfs_sec_per_disc; i++)
	{
		u32 iwlba = wbfs_ntohs( d->header->wlba_table[i] );
		if (iwlba) free_block(p, iwlba);
	}
	memset(d->header, 0, p->disc_info_sz);
	p->write_hdsector(p->callback_data, p->part_lba + 1 + discn * disc_info_sz_lba, disc_info_sz_lba, d->header);
	p->head->disc_table[discn] = 0;
	wbfs_close_disc(d);
	wbfs_sync(p);
	return 0;
}

u32 wbfs_ren_disc(wbfs_t*p, u8* discid, u8* newname)
{
	wbfs_disc_t *d = wbfs_open_disc(p, discid);
	int disc_info_sz_lba = p->disc_info_sz >> p->hd_sec_sz_s;

	if (!d) return 1;

	memset(d->header->disc_header_copy + 0x20, 0, 0x40);
	strncpy((char *) d->header->disc_header_copy + 0x20, (char *) newname, 0x39);

	p->write_hdsector(p->callback_data, p->part_lba + 1 + d->i * disc_info_sz_lba, disc_info_sz_lba, d->header);
	wbfs_close_disc(d);
	return 0;
}

u32 wbfs_rID_disc(wbfs_t*p, u8* discid, u8* newID)
{
	wbfs_disc_t *d = wbfs_open_disc(p, discid);
	int disc_info_sz_lba = p->disc_info_sz >> p->hd_sec_sz_s;

	if (!d) return 1;

	memset(d->header->disc_header_copy, 0, 0x10);
	strncpy((char *) d->header->disc_header_copy, (char *) newID, 0x9);

	p->write_hdsector(p->callback_data, p->part_lba + 1 + d->i * disc_info_sz_lba, disc_info_sz_lba, d->header);
	wbfs_close_disc(d);
	return 0;
}

// trim the file-system to its minimum size
u32 wbfs_trim(wbfs_t*p)
{
	u32 maxbl;
	load_freeblocks(p);
	maxbl = alloc_block(p);
	p->n_hd_sec = maxbl << (p->wbfs_sec_sz_s - p->hd_sec_sz_s);
	p->head->n_hd_sec = wbfs_htonl( p->n_hd_sec );
	// make all block full
	memset(p->freeblks, 0, p->n_wbfs_sec / 8);
	wbfs_sync(p);
	// os layer will truncate the file.
	return maxbl;
}

// data extraction
u32 wbfs_extract_disc(wbfs_disc_t*d, rw_sector_callback_t write_dst_wii_sector, void *callback_data,
		progress_callback_t spinner)
{
	wbfs_t *p = d->p;
	u8* copy_buffer = 0;
	int i;
	int src_wbs_nlb = p->wbfs_sec_sz / p->hd_sec_sz;
	int dst_wbs_nlb = p->wbfs_sec_sz / p->wii_sec_sz;
	copy_buffer = wbfs_ioalloc( p->wbfs_sec_sz );
	if (!copy_buffer)
	{
		wbfs_error( "alloc memory" );
		return 1;
	}

	for (i = 0; i < p->n_wbfs_sec_per_disc; i++)
	{
		u32 iwlba = wbfs_ntohs( d->header->wlba_table[i] );
		if (iwlba)
		{

			if (spinner) spinner(i, p->n_wbfs_sec_per_disc);
			p->read_hdsector(p->callback_data, p->part_lba + iwlba * src_wbs_nlb, src_wbs_nlb, copy_buffer);
			write_dst_wii_sector(callback_data, i * dst_wbs_nlb, dst_wbs_nlb, copy_buffer);
		}
	}
	wbfs_iofree( copy_buffer );
	return 0;
}

u64 wbfs_estimate_disc(wbfs_t *p, read_wiidisc_callback_t read_src_wii_disc, void *callback_data, partition_selector_t sel)
{
	int i;
	u32 tot;
	u32 wii_sec_per_wbfs_sect = 1 << (p->wbfs_sec_sz_s - p->wii_sec_sz_s);
	wiidisc_t *d = 0;
	u8 *used = 0;
	wbfs_disc_info_t *info = 0;

	tot = 0;

	used = wbfs_malloc( p->n_wii_sec_per_disc );
	if (!used)
	{
		ERROR( "unable to alloc memory" );
	}

	d = wd_open_disc(read_src_wii_disc, callback_data);
	if (!d)
	{
		ERROR( "unable to open wii disc" );
	}

	wd_build_disc_usage(d, sel, used);
	wd_close_disc(d);
	d = 0;

	info = wbfs_ioalloc( p->disc_info_sz );
	read_src_wii_disc(callback_data, 0, 0x100, info->disc_header_copy);

	for (i = 0; i < p->n_wbfs_sec_per_disc; i++)
	{
		if (block_used(used, i, wii_sec_per_wbfs_sect))
		{
			tot++;
		}
	}

	error:
	if (d) wd_close_disc(d);
	if (used) wbfs_free( used );
	if (info) wbfs_iofree( info );

	return (u64) tot * (u64) p->wbfs_sec_sz;
}

u32 wbfs_size_disc(wbfs_t*p, read_wiidisc_callback_t read_src_wii_disc, void *callback_data, partition_selector_t sel,
		u32 *comp_size, u32 *real_size)
{
	int i;
	u32 tot = 0, last = 0;
	u32 wii_sec_per_wbfs_sect = 1 << (p->wbfs_sec_sz_s - p->wii_sec_sz_s);
	wiidisc_t *d = 0;
	u8 *used = 0;
	used = wbfs_malloc( p->n_wii_sec_per_disc );
	if (!used)
	ERROR( "unable to alloc memory" );
	d = wd_open_disc(read_src_wii_disc, callback_data);
	if (!d)
	ERROR( "unable to open wii disc" );
	wd_build_disc_usage(d, sel, used);
	wd_close_disc(d);
	d = 0;

	// count total number to write for spinner
	for (i = 0; i < p->n_wbfs_sec_per_disc; i++)
	{
		if (block_used(used, i, wii_sec_per_wbfs_sect))
		{
			tot += wii_sec_per_wbfs_sect;
			last = i * wii_sec_per_wbfs_sect;
		}
	}

	error: if (d) wd_close_disc(d);
	if (used) wbfs_free( used );

	*comp_size = tot;
	*real_size = last;

	return 0;
}

// offset is pointing 32bit words to address the whole dvd, although len is in bytes
//int wbfs_disc_read(wbfs_disc_t*d,u32 offset, u8 *data, u32 len)

// offset points 32bit words, count counts bytes
//int (*read_wiidisc_callback_t)(void*fp,u32 offset,u32 count,void*iobuf);

// connect wiidisc to wbfs_disc
s32 read_wiidisc_wbfsdisc(void*fp, u32 offset, u32 count, void*iobuf)
{
	return wbfs_disc_read((wbfs_disc_t*) fp, offset, count, iobuf);
}

int wbfs_extract_file(wbfs_disc_t*d, char *path, void **data)
{
	wiidisc_t *wd = 0;
	int ret = 0;

	wd = wd_open_disc(read_wiidisc_wbfsdisc, d);
	if (!wd)
	{
		wbfs_error( "opening wbfs disc" );
		return -1;
	}
	wd->extracted_size = 0;
	*data = wd_extract_file(wd, ONLY_GAME_PARTITION, path);
	ret = wd->extracted_size;
	if (!*data)
	{
		//ERROR("file not found");
		ret = -1;
	}
	wd_close_disc(wd);

	return ret;
}

int wbfs_get_fragments(wbfs_disc_t *d, _frag_append_t append_fragment, void *callback_data, u32 hdd_sector_size)
{
	if (!d) return -1;

	wbfs_t *p = d->p;
	int src_wbs_nlb = p->wbfs_sec_sz / hdd_sector_size;
	int i, ret, last = 0;
	for (i = 0; i < p->n_wbfs_sec_per_disc; i++)
	{
		u32 iwlba = wbfs_ntohs( d->header->wlba_table[i] );
		if (iwlba)
		{
			ret = append_fragment(callback_data, i * src_wbs_nlb, // offset
					p->part_lba + iwlba * src_wbs_nlb, // sector
					src_wbs_nlb); // count
			if (ret) return ret; // error
			last = i;
		}
	}
	if (last < p->n_wbfs_sec_per_disc / 2)
	{
		last = p->n_wbfs_sec_per_disc / 2;
	}
	u32 size = last * src_wbs_nlb;
	append_fragment(callback_data, size, 0, 0); // set size
	return 0;
}

// wrapper for reading .iso files using wbfs apis

#include <unistd.h>
#include <sys/stat.h>

// offset is pointing 32bit words to address the whole dvd, although len is in bytes
int wbfs_iso_file_read(wbfs_disc_t*d, u32 offset, u8 *data, u32 len)
{
	if (!d || d->p != &wbfs_iso_file) return -1;
	int fd = d->i;
	off_t off = ((u64) offset) << 2;
	off_t ret_off;
	int ret;
	ret_off = lseek(fd, off, SEEK_SET);
	if (ret_off != off) return -1;
	ret = read(fd, data, len);
	if (ret != (int) len) return -2;
	return 0;
}

u32 wbfs_disc_sector_used(wbfs_disc_t *d)
{
	if(!d) return 0;

	if (d->p == &wbfs_iso_file)
	{
		int fd = d->i;
		struct stat st;
		if (fstat(fd, &st) == -1) return 0;
		return (st.st_size >> 9);
	}

	return wbfs_sector_used(d->p, d->header);
}

