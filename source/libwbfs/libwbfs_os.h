#ifndef LIBWBFS_GLUE_H
#define LIBWBFS_GLUE_H

#include <gctypes.h>

#define debug_printf(fmt, ...);

#include <stdio.h>
#define wbfs_fatal(x)		do { printf("\nwbfs panic: %s\n\n",x); while(1); } while(0)
#define wbfs_error(x)		do { printf("\nwbfs error: %s\n\n",x); } while(0)

#include <stdlib.h>
#include <malloc.h>

#define wbfs_malloc(x)		malloc(x)
#define wbfs_free(x)		free(x)
#define wbfs_ioalloc(x)		memalign(32, x)
#define wbfs_iofree(x)		free(x)
#define wbfs_be16(x)		(*((u16*)(x)))
#define wbfs_be32(x)		(*((u32*)(x)))
#define wbfs_ntohl(x)		(x)
#define wbfs_htonl(x)		(x)
#define wbfs_ntohs(x)		(x)
#define wbfs_htons(x)		(x)

#include <string.h>

#define wbfs_memcmp(x,y,z)	memcmp(x,y,z)
#define wbfs_memcpy(x,y,z)	memcpy(x,y,z)
#define wbfs_memset(x,y,z)	memset(x,y,z)


#endif
