#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __cplusplus
extern "C"
{
#endif
	/* Constants */
#define KB_SIZE	 1024.0
#define MB_SIZE	 1048576.0
#define GB_SIZE	 1073741824.0

	/* Macros */
#define round_up(x,n)   (-(-(x) & -(n)))

	/* Prototypes */
	unsigned int swap32(unsigned int);

#ifdef __cplusplus
}
#endif

#endif
