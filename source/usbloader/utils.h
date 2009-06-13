#ifndef _UTILS_H_
#define _UTILS_H_

/* Constants */
#define KB_SIZE		1024.0
#define MB_SIZE		1048576.0
#define GB_SIZE		1073741824.0

/* Macros */
#define round_up(x,n)	(-(-(x) & -(n)))

/* Prototypes */
u32 swap32(u32);

#endif
