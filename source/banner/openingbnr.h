/****************************************************************************
 * USB Loader GX Team
 * openingbnr
 *
 * Extract opening.bnr/banner.bin/sound.bin/icon.bin
 ***************************************************************************/

#ifndef _OPENINGBNR_H_
#define _OPENINGBNR_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************
*    Error description:
*     0      Successfully extracted
*    -1      No U8 tag
*    -2      Unknown type
*    -3      Archive inconsistency, too much padding
*    -4      No IMD5 tag
*    -5      MD5 mismatch
*    -6      Size mismatch
*    -7      Inconsistency in LZ77 encoding
************************************************************/

//! Extract opening.bnr from filepath to destpath
//! Files extracted: banner.bin icon.bin and sound.bin
int extractbnrfile(const char * filepath, const char * destpath);
int unpackBin(const char * filename,const char * outdir);
//int unpackBanner(const char * filename,const char * outdir);
int unpackBanner(const char * gameid, const char * outdir);
//! Extract the lz77 compressed banner, icon and sound .bin
u8* decompress_lz77(u8 *data, size_t data_size, size_t* decompressed_size);

u16 be16(const u8 *p);
u32 be32(const u8 *p);

#ifdef __cplusplus
}
#endif

#endif
