#ifndef GCDISC_H
#define GCDISC_H

#include <stdio.h>
#include "libwbfs_os.h" // this file is provided by the project wanting to compile libwbfs and wiidisc
#include "wiidisc.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	typedef struct gcdisc_s
	{
			read_wiidisc_callback_t read;
			void *fp;

			const char *extract_pathname;
			u8 *extracted_buffer;
			int extracted_size;
	} gcdisc_t;

	gcdisc_t *gc_open_disc(read_wiidisc_callback_t read, void*fp);
	void gc_close_disc(gcdisc_t *);
	// returns a buffer allocated with wbfs_ioalloc() or NULL if not found of alloc error
	u8 * gc_extract_file(gcdisc_t *d, const char *pathname);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
