
#ifndef _INFLATE_H
#define _INFLATE_H

#include <stdio.h>
#include <zlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int inflateFile( FILE *source, FILE *dest );

#ifdef __cplusplus
}
#endif

#endif //_INFLATE_H
