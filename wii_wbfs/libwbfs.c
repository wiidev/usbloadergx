// Copyright 2009 Kwiirk
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#include "libwbfs.h"


#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define ERROR(x) do {wbfs_error(x);goto error;}while(0)
#define ALIGN_LBA(x) (((x)+p->hd_sec_sz-1)&(~(p->hd_sec_sz-1)))
static int force_mode=0;
void wbfs_set_force_mode(int force)
{
        force_mode = force;
}
static u8 size_to_shift(u32 size)
{
        u8 ret = 0;
        while(size)
        {
                ret++;
                size>>=1;
        }
        return ret-1;
}
#define read_le32_unaligned(x) ((x)[0]|((x)[1]<<8)|((x)[2]<<16)|((x)[3]<<24))

static u8 *last_sect_buff=0;

static u32 last_sect=0xffffffff;
static int my_hd_sector_size=0;

wbfs_t*wbfs_open_hd(rw_sector_callback_t read_hdsector,
                 rw_sector_callback_t write_hdsector,
                 void *callback_data,
                    int hd_sector_size, int num_hd_sector __attribute((unused)), int partition, int reset)
{
        int i=num_hd_sector,ret;
        u8 *ptr,*tmp_buffer = wbfs_ioalloc(hd_sector_size);
        u8 part_table[16*4];
		u32 last_lba=0xFFFFFFFF;
		int l=0;
		last_sect=0xffffffff;
		if(!last_sect_buff) last_sect_buff = wbfs_ioalloc(hd_sector_size);
		my_hd_sector_size=hd_sector_size;
        ret = read_hdsector(callback_data,0,1,tmp_buffer);
        if(ret)
			{
		    wbfs_iofree(tmp_buffer);
            return 0;
			}
        //find wbfs partition
		if(tmp_buffer[0x1fe]!=0x55 || tmp_buffer[0x1ff]!=0xaa 
			|| !strncmp((void *) &tmp_buffer[3],"NTFS",4) 
			|| !strncmp((void *) &tmp_buffer[0x36],"FAT",3)
			|| !strncmp((void *) &tmp_buffer[0x52],"FAT",3)
		) wbfs_memset(part_table,0,16*4);
		else wbfs_memcpy(part_table,tmp_buffer+0x1be,16*4);

        ptr = part_table;

		

        for(i=0;i<4;i++,ptr+=16)
        { 
		u32 part_lba = read_le32_unaligned(ptr+0x8);
		wbfs_head_t *head = (wbfs_head_t *)tmp_buffer;

		#if 1
		if(head->magic != wbfs_htonl(WBFS_MAGIC)) if(ptr[4]==0) continue;
		
		if(ptr[4]==0xf)
			{
			u32 part_lba2=part_lba;
			u32 next_lba2=0;
			int n;
			
			for(n=0;n<8;n++) // max 8 logic partitions (i think it is sufficient!)
				{
					ret = read_hdsector(callback_data,part_lba+next_lba2 ,1,tmp_buffer);
					  if(ret)
						{
						wbfs_iofree(tmp_buffer);
						return 0;
						}

					part_lba2=part_lba+next_lba2+read_le32_unaligned(tmp_buffer+0x1C6);
					next_lba2=read_le32_unaligned(tmp_buffer+0x1D6);

					ret = read_hdsector(callback_data,part_lba2,1,tmp_buffer);
					  if(ret)
						{
						wbfs_iofree(tmp_buffer);
						return 0;
						}
					 // verify there is the magic.
					if (head->magic == wbfs_htonl(WBFS_MAGIC))
						{

						if(l==partition)
							{
							wbfs_t*p = wbfs_open_partition(read_hdsector,write_hdsector,
													callback_data,hd_sector_size,0,part_lba2,reset);
							wbfs_iofree(tmp_buffer);
							return p;
							}
						else
							{
							if(part_lba2!=last_lba) {l++;last_lba=part_lba2;}
							}
						}

					if(next_lba2==0) break;
				}
			}  
          else
			  #endif
				{
					ret = read_hdsector(callback_data,part_lba,1,tmp_buffer);

					if(ret)
						{
						wbfs_iofree(tmp_buffer);
						return 0;
						}
					// verify there is the magic.
					if (head->magic == wbfs_htonl(WBFS_MAGIC))
					{

					if(l==partition)
							{
							wbfs_t*p = wbfs_open_partition(read_hdsector,write_hdsector,
													callback_data,hd_sector_size,0,part_lba,reset);
							wbfs_iofree(tmp_buffer);
							return p;
							}
					else
							{
							if(part_lba!=last_lba) {l++;last_lba=part_lba;}
							}

					}
				}
        }
        if(reset)// XXX make a empty hd partition..
        {
        }
	wbfs_iofree(tmp_buffer);
        return 0;
}


wbfs_t*wbfs_open_partition(rw_sector_callback_t read_hdsector,
                           rw_sector_callback_t write_hdsector,
                           void *callback_data,
                           int hd_sector_size, int num_hd_sector, u32 part_lba, int reset)
{
        wbfs_t *p = wbfs_malloc(sizeof(wbfs_t));
        
        wbfs_head_t *head = wbfs_ioalloc(hd_sector_size?hd_sector_size:512);

        //constants, but put here for consistancy
        p->wii_sec_sz = 0x8000;
        p->wii_sec_sz_s = size_to_shift(0x8000);
       // p->n_wii_sec = (num_hd_sector/0x8000)*hd_sector_size;
        p->n_wii_sec_per_disc = 143432*2;//support for double layers discs..
        p->head = head;
        p->part_lba = part_lba;

		my_hd_sector_size=hd_sector_size;

		 p->n_wii_sec =(u32) ((u64) num_hd_sector)/ ((u64) 0x8000);
		 p->n_wii_sec=(u32) ( ((u64) p->n_wii_sec) * ((u64) hd_sector_size) );

        // init the partition
        if (reset)
        {
                u8 sz_s;
                wbfs_memset(head,0,hd_sector_size);
                head->magic = wbfs_htonl(WBFS_MAGIC);
                head->hd_sec_sz_s = size_to_shift(hd_sector_size);
                head->n_hd_sec = wbfs_htonl(num_hd_sector);
                // choose minimum wblk_sz that fits this partition size
                for(sz_s=6;sz_s<11;sz_s++)
                {
                        // ensure that wbfs_sec_sz is big enough to address every blocks using 16 bits
                        if(p->n_wii_sec <((1U<<16)*(1<<sz_s)))
                                break;
                }
                head->wbfs_sec_sz_s = sz_s+p->wii_sec_sz_s;
        }else
                read_hdsector(callback_data,p->part_lba,1,head);
        if (head->magic != wbfs_htonl(WBFS_MAGIC))
                ERROR("bad magic");
        if(!force_mode && hd_sector_size && head->hd_sec_sz_s !=  size_to_shift(hd_sector_size))
                ERROR("hd sector size doesn't match");
        if(!force_mode && num_hd_sector && head->n_hd_sec != wbfs_htonl(num_hd_sector))
                ERROR("hd num sector doesn't match");
        p->hd_sec_sz = 1<<head->hd_sec_sz_s;
        p->hd_sec_sz_s = head->hd_sec_sz_s;
        p->n_hd_sec = wbfs_ntohl(head->n_hd_sec);

        p->n_wii_sec = (p->n_hd_sec/p->wii_sec_sz)*(p->hd_sec_sz);
        
        p->wbfs_sec_sz_s = head->wbfs_sec_sz_s;
        p->wbfs_sec_sz = 1<<p->wbfs_sec_sz_s;
        p->n_wbfs_sec = p->n_wii_sec >> (p->wbfs_sec_sz_s - p->wii_sec_sz_s);
        p->n_wbfs_sec_per_disc = p->n_wii_sec_per_disc >> (p->wbfs_sec_sz_s - p->wii_sec_sz_s);
        p->disc_info_sz = ALIGN_LBA(sizeof(wbfs_disc_info_t) + p->n_wbfs_sec_per_disc*2);

        //printf("hd_sector_size %X wii_sector size %X wbfs sector_size %X\n",p->hd_sec_sz,p->wii_sec_sz,p->wbfs_sec_sz);
        p->read_hdsector = read_hdsector;
        p->write_hdsector = write_hdsector;
        p->callback_data = callback_data;

        p->freeblks_lba = (p->wbfs_sec_sz - p->n_wbfs_sec/8)>>p->hd_sec_sz_s;
        
        if(!reset)
                p->freeblks = 0; // will alloc and read only if needed
        else
        {
                // init with all free blocks
                p->freeblks = wbfs_ioalloc(ALIGN_LBA(p->n_wbfs_sec/8));
                wbfs_memset(p->freeblks,0xff,p->n_wbfs_sec/8);
        }
        p->max_disc = (p->freeblks_lba-1)/(p->disc_info_sz>>p->hd_sec_sz_s);
        if(p->max_disc > p->hd_sec_sz - sizeof(wbfs_head_t))
                p->max_disc = p->hd_sec_sz - sizeof(wbfs_head_t);

        p->tmp_buffer = wbfs_ioalloc(p->hd_sec_sz);
        p->n_disc_open = 0;
        return p;
error:
        wbfs_free(p);
        wbfs_iofree(head);
        return 0;
            
}



void wbfs_close(wbfs_t*p)
{

        if(p->n_disc_open)
                ERROR("trying to close wbfs while discs still open");

        wbfs_iofree(p->head);
        wbfs_iofree(p->tmp_buffer);
        if(p->freeblks)
                wbfs_iofree(p->freeblks);
        
        wbfs_free(p);
        
error:
        return;
}


wbfs_disc_t *wbfs_open_disc(wbfs_t* p, u8 *discid)
{
        u32 i;
        int disc_info_sz_lba = p->disc_info_sz>>p->hd_sec_sz_s;
        wbfs_disc_t *d = 0;
		last_sect=0xffffffff;
        for(i=0;i<p->max_disc;i++)
        {
                if (p->head->disc_table[i]){
                        p->read_hdsector(p->callback_data,
                                         p->part_lba+1+i*disc_info_sz_lba,1,p->tmp_buffer);
                        if(wbfs_memcmp(discid,p->tmp_buffer,6)==0){
                                d = wbfs_malloc(sizeof(*d));
                                if(!d)
                                        ERROR("allocating memory");
                                d->p = p;
                                d->i = i;
                                d->header = wbfs_ioalloc(p->disc_info_sz);
                                if(!d->header)
                                        ERROR("allocating memory");
                                p->read_hdsector(p->callback_data,
                                                  p->part_lba+1+i*disc_info_sz_lba,
                                                  disc_info_sz_lba,d->header);
                                p->n_disc_open ++;
//                                for(i=0;i<p->n_wbfs_sec_per_disc;i++)
//                                        printf("%d,",wbfs_ntohs(d->header->wlba_table[i]));
                                return d;
                        }
                }
        }
        return 0;
error:
        if(d)
                wbfs_iofree(d);
        return 0;
        
}
void wbfs_close_disc(wbfs_disc_t*d)
{
        d->p->n_disc_open --;
        wbfs_iofree(d->header);
        wbfs_free(d);
}
// offset is pointing 32bit words to address the whole dvd, although len is in bytes
int wbfs_disc_read(wbfs_disc_t*d,u32 offset, u8 *data, u32 len)
{
 
        wbfs_t *p = d->p;
        u16 wlba = offset>>(p->wbfs_sec_sz_s-2);
        u32 iwlba_shift = p->wbfs_sec_sz_s - p->hd_sec_sz_s;
        u32 lba_mask = (p->wbfs_sec_sz-1)>>(p->hd_sec_sz_s);
        u32 lba = (offset>>(p->hd_sec_sz_s-2))&lba_mask;
        u32 off = offset&((p->hd_sec_sz>>2)-1);
        u16 iwlba = wbfs_ntohs(d->header->wlba_table[wlba]);
        u32 len_copied;
        int err = 0;
        u8  *ptr = data;
        if(unlikely(iwlba==0))
                return 1;
        if(unlikely(off)){
                off*=4;

				if(last_sect==(p->part_lba + (iwlba<<iwlba_shift) + lba))
					{
					err=0;wbfs_memcpy(p->tmp_buffer, last_sect_buff, my_hd_sector_size);
					}
				else
					{
					err = p->read_hdsector(p->callback_data,
                                       p->part_lba + (iwlba<<iwlba_shift) + lba, 1, p->tmp_buffer);
					if(err)
                        return err;

					last_sect=(p->part_lba + (iwlba<<iwlba_shift) + lba);
					wbfs_memcpy(last_sect_buff, p->tmp_buffer, my_hd_sector_size);
					}

                len_copied = p->hd_sec_sz - off;
                if(likely(len < len_copied))
                        len_copied = len;
                wbfs_memcpy(ptr, p->tmp_buffer + off, len_copied);
                len -= len_copied;
                ptr += len_copied;
                lba++;
                if(unlikely(lba>lba_mask && len)){
                        lba=0;
                        iwlba =  wbfs_ntohs(d->header->wlba_table[++wlba]);
                        if(unlikely(iwlba==0))
                                return 1;
                }
        }
        while(likely(len>=p->hd_sec_sz))
        {
                u32 nlb = len>>(p->hd_sec_sz_s);
                
                if(unlikely(lba + nlb > p->wbfs_sec_sz)) // dont cross wbfs sectors..
                        nlb = p->wbfs_sec_sz-lba;
                err = p->read_hdsector(p->callback_data,
                                 p->part_lba + (iwlba<<iwlba_shift) + lba, nlb, ptr);
                if(err)
                        return err;
                len -= nlb<<p->hd_sec_sz_s;
                ptr += nlb<<p->hd_sec_sz_s;
                lba += nlb;
                if(unlikely(lba>lba_mask && len)){
                        lba = 0;
                        iwlba =wbfs_ntohs(d->header->wlba_table[++wlba]);
                        if(unlikely(iwlba==0))
                                return 1;
                }
        }
        if(unlikely(len)){
                err = p->read_hdsector(p->callback_data,
                                 p->part_lba + (iwlba<<iwlba_shift) + lba, 1, p->tmp_buffer);
                if(err)
                        return err;
				last_sect=(p->part_lba + (iwlba<<iwlba_shift) + lba);
				wbfs_memcpy(last_sect_buff, p->tmp_buffer, my_hd_sector_size);
                wbfs_memcpy(ptr, p->tmp_buffer, len);
        }     
        return 0;
}
// hermes
int wbfs_disc_read2(wbfs_disc_t*d,u32 offset, u8 *data, u32 len)
{
 
        wbfs_t *p = d->p;
        u16 wlba = offset>>(p->wbfs_sec_sz_s-2);
        u32 iwlba_shift = p->wbfs_sec_sz_s - p->hd_sec_sz_s;
        u32 lba_mask = (p->wbfs_sec_sz-1)>>(p->hd_sec_sz_s);
        u32 lba = (offset>>(p->hd_sec_sz_s-2))&lba_mask;
        u32 off = offset&((p->hd_sec_sz>>2)-1);
        u16 iwlba = wbfs_ntohs(d->header->wlba_table[wlba]);
        u32 len_copied;
        int err = 0;
        u8  *ptr = data;
        
               
        if(unlikely(off)){
                off*=4;

				if(unlikely(iwlba==0))
					{
					len_copied = p->hd_sec_sz - off;
					 
					 if(likely(len < len_copied))
								len_copied = len;
						wbfs_memset(ptr, 0, len_copied);
						len -= len_copied;
						ptr += len_copied;
						lba++;
					}
				else
					{
					err = p->read_hdsector(p->callback_data,
										   p->part_lba + (iwlba<<iwlba_shift) + lba, 1, p->tmp_buffer);
					if(err)
							return err;
					len_copied = p->hd_sec_sz - off;
					if(likely(len < len_copied))
							len_copied = len;
					wbfs_memcpy(ptr, p->tmp_buffer + off, len_copied);
					len -= len_copied;
					ptr += len_copied;
					lba++;
					if(unlikely(lba>lba_mask && len && unlikely(iwlba!=0))){
							lba=0;
							iwlba =  wbfs_ntohs(d->header->wlba_table[++wlba]);
                        //if(unlikely(iwlba==0))
                         //       return 1;
						}
					}
                }
       
        while(likely(len>=p->hd_sec_sz))
        {
                u32 nlb = len>>(p->hd_sec_sz_s);
                
				
					if(unlikely(lba + nlb > p->wbfs_sec_sz)) // dont cross wbfs sectors..
                        nlb = p->wbfs_sec_sz-lba;

					if(unlikely(iwlba==0))
						{
						wbfs_memset(ptr, 0, nlb<<p->hd_sec_sz_s);
						}
					else
						{
						err = p->read_hdsector(p->callback_data,
									 p->part_lba + (iwlba<<iwlba_shift) + lba, nlb, ptr);
						if(err)
							return err;
						}
					
                len -= nlb<<p->hd_sec_sz_s;
                ptr += nlb<<p->hd_sec_sz_s;
				lba += nlb;
				if(!unlikely(iwlba==0))
						{
						
						if(unlikely(lba>lba_mask && len && unlikely(iwlba!=0))){
								lba = 0;
								iwlba =wbfs_ntohs(d->header->wlba_table[++wlba]);
							/*	if(unlikely(iwlba==0))
										return 1;*/
							}
						}
        }
        if(unlikely(len)){
				if(!unlikely(iwlba==0))
					{
					err = p->read_hdsector(p->callback_data,
                                 p->part_lba + (iwlba<<iwlba_shift) + lba, 1, p->tmp_buffer);
					if(err)
                        return err;
					wbfs_memcpy(ptr, p->tmp_buffer, len);
					}
				else wbfs_memset(ptr, 0, len);
                
        }     
        return 0;
}



