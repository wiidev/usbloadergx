/***************************************************************************
 * Copyright (C) 2010
 * Dimok
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
 * TextureConverter.h
 *
 * A texture to GD image converter.
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef __TEXTURE_CONVERTER_H_
#define __TEXTURE_CONVERTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <gccore.h>
#include <gd.h>
#include "utils/tools.h"

#define coordsRGBA8(x, y, w) (((((y >> 2) * (w >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1)
#define datasizeRGBA8(w, h) ALIGN32(((w+3)>>2)*((h+3)>>2)*32*2)

bool I4ToGD(const u8 * buffer, u32 width, u32 height, gdImagePtr * im);
bool IA4ToGD(const u8 * buffer, u32 width, u32 height, gdImagePtr * im);
bool I8ToGD(const u8 * buffer, u32 width, u32 height, gdImagePtr * im);
bool IA8ToGD(const u8 * buffer, u32 width, u32 height, gdImagePtr * im);
bool CMPToGD(const u8* buffer, u32 width, u32 height, gdImagePtr * im);
bool RGB565ToGD(const u8* buffer, u32 width, u32 height, gdImagePtr * im);
bool RGB565A3ToGD(const u8* buffer, u32 width, u32 height, gdImagePtr * im);
bool RGBA8ToGD(const u8* buffer, u32 width, u32 height, gdImagePtr * im);
bool YCbYCrToGD(const u8* buffer, u32 width, u32 height, gdImagePtr * im);
u8 * GDImageToRGBA8(gdImagePtr * gdImg, int * w, int * h);
u8 * FlipRGBAImage(const u8 *src, u32 width, u32 height);
u8 * RGB8ToRGBA8(const u8 *src, u32 width, u32 height);

#ifdef __cplusplus
}
#endif

#endif //__TEXTURE_CONVERTER_H_
