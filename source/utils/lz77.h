/*******************************************************************************
 * lz77.h
 *
 * Copyright (c) 2009 The Lemon Man
 * Copyright (c) 2009 Nicksasa
 * Copyright (c) 2009 WiiPower
 *
 * Distributed under the terms of the GNU General Public License (v2)
 * See http://www.gnu.org/licenses/gpl-2.0.txt for more info.
 *
 * Description:
 * -----------
 *
 ******************************************************************************/
#ifndef _LZ77_MODULE
#define _LZ77_MODULE

#define LZ77_0x10_FLAG 0x10
#define LZ77_0x11_FLAG 0x11

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int isLZ77compressed( const u8 *buffer);
int decompressLZ77content( const u8 *buffer, u32 length, u8 **output, u32 *outputLen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

