// by oggzee

#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "splits.h"

#define off64_t off_t
#define FMT_llu "%llu"
#define FMT_lld "%lld"

#define split_error(x)	  do { printf("\nsplit error: %s\n\n",x); } while(0)

// 1 cluster less than 4gb
u64 OPT_split_size = (u64) 4LL * 1024 * 1024 * 1024 - 32 * 1024;
// 1 cluster less than 2gb
//u64 OPT_split_size = (u64)2LL * 1024 * 1024 * 1024 - 32 * 1024;

//split_info_t split;

void split_get_fname(split_info_t *s, int idx, char *fname)
{
	strcpy(fname, s->fname);
	if (idx == 0 && s->create_mode)
	{
		strcat(fname, ".tmp");
	}
	else if (idx > 0)
	{
		char *c = fname + strlen(fname) - 1;
		*c = '0' + idx;
	}
}

int split_open_file(split_info_t *s, int idx)
{
	int fd = s->fd[idx];
	if (fd >= 0) return fd;
	char fname[1024];
	split_get_fname(s, idx, fname);
	//char *mode = s->create_mode ? "wb+" : "rb+";
	int mode = s->create_mode ? (O_CREAT | O_RDWR) : O_RDWR;
	//printf("SPLIT OPEN %s %s %d\n", fname, mode, idx); //Wpad_WaitButtons();
	//f = fopen(fname, mode);
	fd = open(fname, mode);
	if (fd < 0) return -1;
	if (idx > 0 && s->create_mode)
	{
		printf("%s Split: %d %s		  \n", s->create_mode ? "Create" : "Read", idx, fname);
	}
	s->fd[idx] = fd;
	return fd;
}

// faster as it uses larger chunks than ftruncate internally
int write_zero(int fd, off_t size)
{
	int buf[0x4000]; //64kb
	int chunk;
	int ret;
	memset(buf, 0, sizeof(buf));
	while (size)
	{
		chunk = size;
		if (chunk > (int) sizeof(buf)) chunk = sizeof(buf);
		ret = write(fd, buf, chunk);
		//printf("WZ %d %d / %lld \n", ret, chunk, size);
		size -= chunk;
		if (ret < 0) return ret;
	}
	return 0;
}

int split_fill(split_info_t *s, int idx, u64 size)
{
	int fd = split_open_file(s, idx);
	off64_t fsize = lseek(fd, 0, SEEK_END);
	if (fsize < (s64) size)
	{
		//printf("TRUNC %d "FMT_lld"\n", idx, size); Wpad_WaitButtons();
		//ftruncate(fd, size);
		write_zero(fd, size - fsize);
		return 1;
	}
	return 0;
}

int split_get_file(split_info_t *s, u32 lba, u32 *sec_count, int fill)
{
	int fd;
	if (lba >= s->total_sec)
	{
		fprintf(stderr, "SPLIT: invalid sector %u / %u\n", lba, (u32) s->total_sec);
		return -1;
	}
	int idx;
	idx = lba / s->split_sec;
	if (idx >= s->max_split)
	{
		fprintf(stderr, "SPLIT: invalid split %d / %d\n", idx, s->max_split - 1);
		return -1;
	}
	fd = s->fd[idx];
	if (fd < 0)
	{
		// opening new, make sure all previous are full
		int i;
		for (i = 0; i < idx; i++)
		{
			if (split_fill(s, i, s->split_size))
			{
				printf("FILL %d\n", i);
			}
		}
		fd = split_open_file(s, idx);
	}
	if (fd < 0)
	{
		fprintf(stderr, "SPLIT %d: no file\n", idx);
		return -1;
	}
	u32 sec = lba % s->split_sec; // inside file
	off64_t off = (off64_t ) sec * 512;
	// num sectors till end of file
	u32 to_end = s->split_sec - sec;
	if (*sec_count > to_end) *sec_count = to_end;
	if (s->create_mode)
	{
		if (fill)
		{
			// extend, so that read will be succesfull
			split_fill(s, idx, off + 512 * (*sec_count));
		}
		else
		{
			// fill up so that write continues from end of file
			// shouldn't be necessary, but libfat looks buggy
			// and this is faster
			split_fill(s, idx, off);
		}
	}
	lseek(fd, off, SEEK_SET);
	return fd;
}

int split_read_sector(void *_fp, u32 lba, u32 count, void*buf)
{
	split_info_t *s = _fp;
	int fd;
	u64 off = lba;
	off *= 512ULL;
	int i;
	u32 chunk;
	size_t ret;
	//fprintf(stderr,"READ %d %d\n", lba, count);
	for (i = 0; i < (int) count; i += chunk)
	{
		chunk = count - i;
		fd = split_get_file(s, lba + i, &chunk, 1);
		if (fd < 0)
		{
			fprintf(stderr, "\n\n"FMT_lld" %d %p\n", off, count, _fp);
			split_error( "error seeking in disc partition" );
			return 1;
		}
		void *ptr = ((u8 *) buf) + (i * 512);
		ret = read(fd, ptr, chunk * 512);
		if (ret != chunk * 512)
		{
			fprintf(stderr, "error reading %u %u [%u] %u = %u\n", lba, count, i, chunk, ret);
			split_error( "error reading disc" );
			return 1;
		}
	}
	return 0;
}

int split_write_sector(void *_fp, u32 lba, u32 count, void*buf)
{
	split_info_t *s = _fp;
	int fd;
	u64 off = lba;
	off *= 512ULL;
	int i;
	u32 chunk;
	size_t ret;
	//printf("WRITE %d %d %p \n", lba, count, buf);
	for (i = 0; i < (int) count; i += chunk)
	{
		chunk = count - i;
		fd = split_get_file(s, lba + i, &chunk, 0);
		//if (chunk != count)
		//  fprintf(stderr, "WRITE CHUNK %d %d/%d\n", lba+i, chunk, count);
		if (fd < 0 || !chunk)
		{
			fprintf(stderr, "\n\n"FMT_lld" %d %p\n", off, count, _fp);
			split_error( "error seeking in disc partition" );
			return 1;
		}
		//if (fwrite(buf+i*512, 512ULL, chunk, f) != chunk) {
		//printf("write %d %p %d \n", fd, buf+i*512, chunk * 512);
		void *ptr = ((u8 *) buf) + (i * 512);
		ret = write(fd, ptr, chunk * 512);
		//printf("write ret = %d \n", ret);
		if (ret != chunk * 512)
		{
			split_error( "error writing disc" );
			return 1;
		}
	}
	return 0;
}

void split_init(split_info_t *s, char *fname)
{
	int i;
	char *p;
	//fprintf(stderr, "SPLIT_INIT %s\n", fname);
	memset(s, 0, sizeof(*s));
	for (i = 0; i < MAX_SPLIT; i++)
	{
		s->fd[i] = -1;
	}
	strcpy(s->fname, fname);
	s->max_split = 1;
	p = strrchr(fname, '.');
	if (p && (strcasecmp(p, ".wbfs") == 0))
	{
		s->max_split = MAX_SPLIT;
	}
}

void split_set_size(split_info_t *s, u64 split_size, u64 total_size)
{
	s->total_size = total_size;
	s->split_size = split_size;
	s->total_sec = total_size / 512;
	s->split_sec = split_size / 512;
}

void split_close(split_info_t *s)
{
	int i;
	char fname[1024];
	char tmpname[1024];
	for (i = 0; i < s->max_split; i++)
	{
		if (s->fd[i] >= 0)
		{
			close(s->fd[i]);
		}
	}
	if (s->create_mode)
	{
		split_get_fname(s, -1, fname);
		split_get_fname(s, 0, tmpname);
		rename(tmpname, fname);
	}
	memset(s, 0, sizeof(*s));
}

int split_create(split_info_t *s, char *fname, u64 split_size, u64 total_size, bool overwrite)
{
	int i;
	int fd;
	char sname[1024];
	int error = 0;
	split_init(s, fname);
	s->create_mode = 1;
	// check if any file already exists
	for (i = -1; i < s->max_split; i++)
	{
		split_get_fname(s, i, sname);
		if (overwrite)
		{
			remove(sname);
		}
		else
		{
			fd = open(sname, O_RDONLY);
			if (fd >= 0)
			{
				fprintf(stderr, "Error: file already exists: %s\n", sname);
				close(fd);
				error = 1;
			}
		}
	}
	if (error)
	{
		split_init(s, "");
		return -1;
	}
	split_set_size(s, split_size, total_size);
	return 0;
}

int split_open(split_info_t *s, char *fname)
{
	int i;
	u64 size = 0;
	u64 total_size = 0;
	u64 split_size = 0;
	int fd;
	split_init(s, fname);
	for (i = 0; i < s->max_split; i++)
	{
		fd = split_open_file(s, i);
		if (fd < 0)
		{
			if (i == 0) goto err;
			break;
		}
		// check previous size - all splits except last must be same size
		if (i > 0 && size != split_size)
		{
			fprintf(stderr, "split %d: invalid size "FMT_lld"", i, size);
			goto err;
		}
		// get size
		//fseeko(f, 0, SEEK_END);
		//size = ftello(f);
		size = lseek(fd, 0, SEEK_END);
		// check sector alignment
		if (size % 512)
		{
			fprintf(stderr, "split %d: size ("FMT_lld") not sector (512) aligned!", i, size);
		}
		// first sets split size
		if (i == 0)
		{
			split_size = size;
		}
		total_size += size;
	}
	split_set_size(s, split_size, total_size);
	return 0;
	err: split_close(s);
	return -1;
}

