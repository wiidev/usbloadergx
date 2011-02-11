#ifndef WIIDISC_H
#define WIIDISC_H
#include <stdio.h>
#include "libwbfs_os.h" // this file is provided by the project wanting to compile libwbfs and wiidisc

#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */
#if 0 //removes extra automatic indentation by editors
   }
#endif
// callback definition. Return 1 on fatal error (callback is supposed to make retries until no hopes..)
// offset points 32bit words, count counts bytes
typedef int (*read_wiidisc_callback_t)(void*fp,u32 offset,u32 count,void*iobuf);

typedef enum{
        UPDATE_PARTITION_TYPE=0,
        GAME_PARTITION_TYPE,
        OTHER_PARTITION_TYPE,
        // value in between selects partition types of that value
        ALL_PARTITIONS=0xffffffff-3,
        REMOVE_UPDATE_PARTITION, // keeps game + channel installers
        ONLY_GAME_PARTITION,
}partition_selector_t;

typedef struct wiidisc_s
{
        read_wiidisc_callback_t read;
        void *fp;
        u8 *sector_usage_table;

        // everything points 32bit words.
        u32 disc_raw_offset;
        u32 partition_raw_offset;
        u32 partition_data_offset;
        u32 partition_data_size;
        u32 partition_block;
        
        u8 *tmp_buffer;
        u8 *tmp_buffer2;
        u8 disc_key[16];
        int dont_decrypt;

        partition_selector_t part_sel;

        char *extract_pathname;
        u8  *extracted_buffer;
}wiidisc_t;

wiidisc_t *wd_open_disc(read_wiidisc_callback_t read,void*fp);
void wd_close_disc(wiidisc_t *);
// returns a buffer allocated with wbfs_ioalloc() or NULL if not found of alloc error
u8 * wd_extract_file(wiidisc_t *d, partition_selector_t partition_type, char *pathname);

void wd_build_disc_usage(wiidisc_t *d, partition_selector_t selector, u8* usage_table);

// effectively remove not copied partition from the partition table.
void wd_fix_partition_table(wiidisc_t *d, partition_selector_t selector, u8* partition_table);

#if 0
{
#endif
#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif
