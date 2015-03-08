/*
Copyright (c) 2010 - Wii Banner Player Project
Copyright (c) 2012 - Dimok

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#ifndef BANNER_TOOLS_H_
#define BANNER_TOOLS_H_

#include <gctypes.h>
#include "utils/gx_addons.h"
#include "gecko.h"

#define MAKE_FOURCC(a, b, c, d) ((a) * (1 << 24) + (b) * (1 << 16) + (c) * (1 << 8) + (d) * (1 << 0))

typedef struct
{
  u32 magic;
  u32 size;
} section_t;

typedef struct
{
	float x, y;
} Vec2f;

typedef struct
{
	float x, y, z;
} Vec3f;

#define ALIGN32(x) (((x) + 31) & ~31)
#define LIMIT(x, min, max)																	\
	({																						\
		typeof( x ) _x = x;																	\
		typeof( min ) _min = min;															\
		typeof( max ) _max = max;															\
		( ( ( _x ) < ( _min ) ) ? ( _min ) : ( ( _x ) > ( _max ) ) ? ( _max) : ( _x ) );	\
	})

#define MultiplyAlpha(a1, a2) (u8)((u16) (a1) * (u16) (a2) / 0xFF)
#define FLOAT_2_U8(x) ((u8)((x) > 255.0f ? 255.0f : ((x) < 0.0f ? 0.0f : (x) + 0.5f)))
#define FLOAT_2_S16(x) ((s16)((x) > 32767.0f ? 32767.0f : ((x) < -32768.0f ? 32768.0f : (x) + 0.5f)))

#endif
