#ifndef LIBWBFS_H
#define LIBWBFS_H

#include "libwbfs_os.h" // this file is provided by the project wanting to compile libwbfs
#include "wiidisc.h"

#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

typedef u32 be32_t;
typedef u16 be16_t;



typedef struct wbfs_head
{
        be32_t magic;
        // parameters copied in the partition for easy dumping, and bug reports
        be32_t n_hd_sec;	       // total number of hd_sec in this partition
        u8  hd_sec_sz_s;       // sector size in this partition
        u8  wbfs_sec_sz_s;     // size of a wbfs sec
        u8  padding3[2];
        u8  disc_table[0];	// size depends on hd sector size
}__attribute((packed)) wbfs_head_t ;

typedef struct wbfs_disc_info
{
        u8 disc_header_copy[0x100];
        be16_t wlba_table[0];
}wbfs_disc_info_t;

//  WBFS first wbfs_sector structure:
//
//  -----------
// | wbfs_head |  (hd_sec_sz)
//  -----------
// |	       |
// | disc_info |
// |	       |
//  -----------
// |	       |
// | disc_info |
// |	       |
//  -----------
// |	       |
// | ...       |
// |	       |
//  -----------
// |	       |
// | disc_info |
// |	       |
//  -----------
// |	       |
// |freeblk_tbl|
// |	       |
//  -----------
//

// callback definition. Return 1 on fatal error (callback is supposed to make retries until no hopes..)
typedef int (*rw_sector_callback_t)(void*fp,u32 lba,u32 count,void*iobuf);
typedef void (*progress_callback_t)(int status,int total);


typedef struct wbfs_s
{
        wbfs_head_t *head;

        /* hdsectors, the size of the sector provided by the hosting hard drive */
        u32 hd_sec_sz;
        u8  hd_sec_sz_s; // the power of two of the last number
        u32 n_hd_sec;	 // the number of hd sector in the wbfs partition

        /* standard wii sector (0x8000 bytes) */
        u32 wii_sec_sz;
        u8  wii_sec_sz_s;
        u32 n_wii_sec;
        u32 n_wii_sec_per_disc;

        /* The size of a wbfs sector */
        u32 wbfs_sec_sz;
        u32 wbfs_sec_sz_s;
        u16 n_wbfs_sec;   // this must fit in 16 bit!
        u16 n_wbfs_sec_per_disc;   // size of the lookup table

        u32 part_lba;
        /* virtual methods to read write the partition */
        rw_sector_callback_t read_hdsector;
        rw_sector_callback_t write_hdsector;
        void *callback_data;

        u16 max_disc;
        u32 freeblks_lba;
        u32 *freeblks;
        u16 disc_info_sz;

        u8  *tmp_buffer;  // pre-allocated buffer for unaligned read

        u32 n_disc_open;

}wbfs_t;

typedef struct wbfs_disc_s
{
        wbfs_t *p;
        wbfs_disc_info_t  *header;	  // pointer to wii header
        int i;		  		  // disc index in the wbfs header (disc_table)
}wbfs_disc_t;


#define WBFS_MAGIC (('W'<<24)|('B'<<16)|('F'<<8)|('S'))

/*! @brief open a MSDOS partitionned harddrive. This tries to find a wbfs partition into the harddrive
   @param read_hdsector,write_hdsector: accessors to a harddrive
   @hd_sector_size: size of the hd sector. Can be set to zero if the partition in already initialized
   @num_hd_sector:  number of sectors in this disc. Can be set to zero if the partition in already initialized
   @reset: not implemented, This will format the whole harddrive with one wbfs partition that fits the whole disk.
   calls wbfs_error() to have textual meaning of errors
   @return NULL in case of error
*/
wbfs_t*wbfs_open_hd(rw_sector_callback_t read_hdsector,
                 rw_sector_callback_t write_hdsector,
                 void *callback_data,
                 int hd_sector_size, int num_hd_sector, int reset);

/*! @brief open a wbfs partition
   @param read_hdsector,write_hdsector: accessors to the partition
   @hd_sector_size: size of the hd sector. Can be set to zero if the partition in already initialized
   @num_hd_sector:  number of sectors in this partition. Can be set to zero if the partition in already initialized
   @partition_lba:  The partitio offset if you provided accessors to the whole disc.
   @reset: initialize the partition with an empty wbfs.
   calls wbfs_error() to have textual meaning of errors
   @return NULL in case of error
*/
wbfs_t*wbfs_open_partition(rw_sector_callback_t read_hdsector,
                           rw_sector_callback_t write_hdsector,
                           void *callback_data,
                           int hd_sector_size, int num_hd_sector, u32 partition_lba, int reset);


/*! @brief close a wbfs partition, and sync the metadatas to the disc */
void wbfs_close(wbfs_t*);

/*! @brief open a disc inside a wbfs partition use a 6 char discid+vendorid
  @return NULL if discid is not present
*/
wbfs_disc_t *wbfs_open_disc(wbfs_t* p, u8 *diskid);

/*! @brief close a already open disc inside a wbfs partition */
void wbfs_close_disc(wbfs_disc_t*d);

u32 wbfs_sector_used(wbfs_t *p,wbfs_disc_info_t *di);

/*! @brief accessor to the wii disc
  @param d: a pointer to already open disc
  @param offset: an offset inside the disc, *points 32bit words*, allowing to access 16GB data
  @param len: The length of the data to fetch, in *bytes*
 */
// offset is pointing 32bit words to address the whole dvd, although len is in bytes
int wbfs_disc_read(wbfs_disc_t*d,u32 offset, u8 *data, u32 len);

/*! @return the number of discs inside the paritition */
u32 wbfs_count_discs(wbfs_t*p);
/*! get the disc info of ith disc inside the partition. It correspond to the first 0x100 bytes of the wiidvd
  http://www.wiibrew.org/wiki/Wiidisc#Header
  @param i: index of the disc inside the partition
  @param header: pointer to 0x100 bytes to write the header
  @size: optional pointer to a 32bit word that will get the size in 32bit words of the DVD taken on the partition.
*/
u32 wbfs_get_disc_info(wbfs_t*p, u32 i,u8 *header,int header_size,u32 *size);

/*! get the number of used block of the partition.
  to be multiplied by p->wbfs_sec_sz (use 64bit multiplication) to have the number in bytes
*/
u32 wbfs_count_usedblocks(wbfs_t*p);

/******************* write access  ******************/

/*! add a wii dvd inside the partition
  @param read_src_wii_disc: a callback to access the wii dvd. offsets are in 32bit, len in bytes!
  @callback_data: private data passed to the callback
  @spinner: a pointer to a function that is regulary called to update a progress bar.
  @sel: selects which partitions to copy.
  @copy_1_1: makes a 1:1 copy, whenever a game would not use the wii disc format, and some data is hidden outside the filesystem.
 */
u32 wbfs_add_disc(wbfs_t*p,read_wiidisc_callback_t read_src_wii_disc, void *callback_data,
                  progress_callback_t spinner,partition_selector_t sel,int copy_1_1);


/*! remove a wiidvd inside a partition */
u32 wbfs_rm_disc(wbfs_t*p, u8* discid);

/*! rename a game */
u32 wbfs_ren_disc(wbfs_t*p, u8* discid, u8* newname);

/*! trim the file-system to its minimum size
  This allows to use wbfs as a wiidisc container
 */
u32 wbfs_trim(wbfs_t*p);

/*! extract a disc from the wbfs, unused sectors are just untouched, allowing descent filesystem to only really usefull space to store the disc.
Even if the filesize is 4.7GB, the disc usage will be less.
 */
u32 wbfs_extract_disc(wbfs_disc_t*d, rw_sector_callback_t write_dst_wii_sector,void *callback_data,progress_callback_t spinner);

/*! extract a file from the wii disc filesystem.
  E.G. Allows to extract the opening.bnr to install a game as a system menu channel
 */
u32 wbfs_extract_file(wbfs_disc_t*d, char *path);

// remove some sanity checks
void wbfs_set_force_mode(int force);

u32 wbfs_estimate_disc(
		wbfs_t *p, read_wiidisc_callback_t read_src_wii_disc,
		void *callback_data,
		partition_selector_t sel);

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif
