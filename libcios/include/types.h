#ifndef __GCTYPES_H__
#define __GCTYPES_H__

/*! \file gctypes.h 
\brief Data type definitions

*/ 
typedef signed char int8_t ;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

/*+----------------------------------------------------------------------------------------------+*/
typedef unsigned char u8;									///< 8bit unsigned integer
typedef unsigned short u16;								///< 16bit unsigned integer
typedef unsigned int u32;									///< 32bit unsigned integer
typedef unsigned long long u64;						///< 64bit unsigned integer
/*+----------------------------------------------------------------------------------------------+*/
typedef signed char s8;										///< 8bit signed integer
typedef signed short s16;									///< 16bit signed integer
typedef signed int s32;										///< 32bit signed integer
typedef signed long long s64;							///< 64bit signed integer
/*+----------------------------------------------------------------------------------------------+*/
typedef volatile unsigned char vu8;				///< 8bit unsigned volatile integer
typedef volatile unsigned short vu16;			///< 16bit unsigned volatile integer
typedef volatile unsigned int vu32;				///< 32bit unsigned volatile integer
typedef volatile unsigned long long vu64;	///< 64bit unsigned volatile integer
/*+----------------------------------------------------------------------------------------------+*/
typedef volatile signed char vs8;					///< 8bit signed volatile integer
typedef volatile signed short vs16;				///< 16bit signed volatile integer
typedef volatile signed int vs32;					///< 32bit signed volatile integer
typedef volatile signed long long vs64;		///< 64bit signed volatile integer
/*+----------------------------------------------------------------------------------------------+*/
// fixed point math typedefs
typedef s16 sfp16;                              ///< 1:7:8 fixed point
typedef s32 sfp32;                              ///< 1:19:8 fixed point
typedef u16 ufp16;                              ///< 8:8 fixed point
typedef u32 ufp32;                              ///< 24:8 fixed point
/*+----------------------------------------------------------------------------------------------+*/
typedef float f32;
typedef double f64;
/*+----------------------------------------------------------------------------------------------+*/
typedef volatile float vf32;
typedef volatile double vf64;
/*+----------------------------------------------------------------------------------------------+*/

// bool is a standard type in cplusplus, but not in c.
#ifndef __cplusplus
/** C++ compatible bool for C

*/
typedef u8 bool;
enum { false, true };
#endif

typedef unsigned int BOOL;
/*+----------------------------------------------------------------------------------------------+*/
// alias type typedefs
#define FIXED s32                               ///< Alias type for sfp32
/*+----------------------------------------------------------------------------------------------+*/
// boolean defines
#ifndef boolean
#define boolean  u8
#endif
/*+----------------------------------------------------------------------------------------------+*/
#ifndef TRUE
#define TRUE 1                                  ///< True
#endif
/*+----------------------------------------------------------------------------------------------+*/
#ifndef FALSE
#define FALSE 0                                 ///< False
#endif
/*+----------------------------------------------------------------------------------------------+*/
#ifndef NULL
#define NULL			0                        ///< Pointer to 0
#endif
/*+----------------------------------------------------------------------------------------------+*/
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN  3412
#endif /* LITTLE_ENDIAN */
/*+----------------------------------------------------------------------------------------------+*/
#ifndef BIG_ENDIAN
#define BIG_ENDIAN     1234
#endif /* BIGE_ENDIAN */
/*+----------------------------------------------------------------------------------------------+*/
#ifndef BYTE_ORDER
#define BYTE_ORDER     BIG_ENDIAN
#endif /* BYTE_ORDER */
/*+----------------------------------------------------------------------------------------------+*/


//!	argv structure
/*!	\struct __argv

	structure used to set up argc/argv

*/
struct __argv {
	int argvMagic;		//!< argv magic number, set to 0x5f617267 ('_arg') if valid 
	char *commandLine;	//!< base address of command line, set of null terminated strings
	int length;//!< total length of command line
	int argc;
	char **argv;
	char **endARGV;
};

//!	Default location for the system argv structure.
extern struct __argv *__system_argv;

// argv struct magic number
#define ARGV_MAGIC 0x5f617267


typedef uint32_t		sec_t;

/* Attributes */
#ifndef ATTRIBUTE_ALIGN
# define ATTRIBUTE_ALIGN(v)	__attribute__((aligned(v)))
#endif
#ifndef ATTRIBUTE_PACKED
# define ATTRIBUTE_PACKED	__attribute__((packed))
#endif

/* Stack align */
#define STACK_ALIGN(type, name, cnt, alignment)	\
	u8 _al__##name[((sizeof(type)*(cnt)) + (alignment) + (((sizeof(type)*(cnt))%(alignment)) > 0 ? ((alignment) - ((sizeof(type)*(cnt))%(alignment))) : 0))]; \
	type *name = (type*)(((u32)(_al__##name)) + ((alignment) - (((u32)(_al__##name))&((alignment)-1))))



#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif /* TYPES_H */


/* END OF FILE */
