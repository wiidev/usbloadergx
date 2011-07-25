/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef __UNCOMPRESS_H
#define __UNCOMPRESS_H

#include <gctypes.h>

#define le16(i) ((((u16) ((i) & 0xFF)) << 8) | ((u16) (((i) & 0xFF00) >> 8)))
#define le32(i) ((((u32)le16((i) & 0xFFFF)) << 16) | ((u32)le16(((i) & 0xFFFF0000) >> 16)))
#define le64(i) ((((u64)le32((i) & 0xFFFFFFFFLL)) << 32) | ((u64)le32(((i) & 0xFFFFFFFF00000000LL) >> 32)))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	u32 magic;   //Yaz0
	u32 decompressed_size;
	u8 zeros[8];
} Yaz0_Header;

u8 * uncompressLZ77(const u8 *inBuf, u32 inLength, u32 * uncSize);
void uncompressYaz0(const u8* srcBuf, u8* dst, int uncompressedSize);
u32 CheckIMD5Type(const u8 * buffer, int length);


#ifdef __cplusplus
}
#endif

#endif
