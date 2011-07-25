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
 * TplImage.h
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef TPL_IMAGE_H_
#define TPL_IMAGE_H_

#include <gccore.h>
#include <gd.h>
#include <vector>

typedef struct
{
	u32 magic;
	u32 num_textures;
	u32 head_size;
} TPL_Header;

typedef struct
{
	u32 text_header_offset;
	u32 text_palette_offset;
} TPL_Texture;

typedef struct
{
	u16		height;
	u16		width;
	u32		format;
	u32		offset;
	u32		wrap_s;
	u32		wrap_t;
	u32		min;
	u32		mag;
	f32		lod_bias;
	u8		edge_lod;
	u8		min_lod;
	u8		max_lod;
	u8		unpacked;
} TPL_Texture_Header;

typedef struct
{
	u16		num_items;
	u8		unpacked;
	u8		pad;
	u32		format;
	u32		offset;
} TPL_Palette_Header;

class TplImage
{
	public:
		TplImage(const char * filepath);
		TplImage(const u8 * imgBuffer, u32 imgSize);
		~TplImage();
		bool LoadImage(const u8 * imgBuffer, u32 imgSize);
		int GetWidth(int Texture);
		int GetHeight(int Texture);
		u32 GetFormat(int Texture);
		const u8 * GetTextureBuffer(int Texture);
		int GetTextureSize(int Texture);
		gdImagePtr ConvertToGD(int Texture);
	private:
		bool ParseTplFile();

		u8 * TPLBuffer;
		u32 TPLSize;
		const TPL_Header * TPLHeader;
		std::vector<const TPL_Texture *> Texture;
		std::vector<const TPL_Texture_Header *> TextureHeader;
		std::vector<const u8 *> TplTextureBuffer;
};

#endif
