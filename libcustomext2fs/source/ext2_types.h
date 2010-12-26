/*
 * If linux/types.h is already been included, assume it has defined
 * everything we need.  (cross fingers)  Other header files may have
 * also defined the types that we need.
 */
#ifndef _EXT2_TYPES_H
#define _EXT2_TYPES_H

typedef unsigned char __u8;
typedef signed char __s8;
typedef	unsigned short	__u16;
typedef	short		__s16;
typedef	unsigned int	__u32;
typedef	int		__s32;
typedef unsigned long long	__u64;
typedef signed long long 	__s64;

#endif /* _EXT2_TYPES_H */
