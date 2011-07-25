#ifndef _SPLITS_H
#define _SPLITS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_SPLIT 10

	typedef struct split_info
	{
			char fname[1024];
			//FILE *f[MAX_SPLIT];
			int fd[MAX_SPLIT];
			//u64 fsize[MAX_SPLIT];
			u32 split_sec;
			u32 total_sec;
			u64 split_size;
			u64 total_size;
			int create_mode;
			int max_split;
	} split_info_t;

	void split_get_fname(split_info_t *s, int idx, char *fname);
	//FILE *split_open_file(split_info_t *s, int idx);
	//FILE *split_get_file(split_info_t *s, u32 lba, u32 *sec_count, int fill);
	int split_open_file(split_info_t *s, int idx);
	int split_get_file(split_info_t *s, u32 lba, u32 *sec_count, int fill);
	int split_fill(split_info_t *s, int idx, u64 size);
	int split_read_sector(void *_fp, u32 lba, u32 count, void*buf);
	int split_write_sector(void *_fp, u32 lba, u32 count, void*buf);
	void split_init(split_info_t *s, char *fname);
	void split_set_size(split_info_t *s, u64 split_size, u64 total_size);
	void split_close(split_info_t *s);
	int split_open(split_info_t *s, char *fname);
	int split_create(split_info_t *s, char *fname, u64 split_size, u64 total_size, bool overwrite);

#ifdef __cplusplus
}
#endif

#endif //_SPLITS_H
