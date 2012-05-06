/*
Copyright (c) 2010 - Wii Banner Player Project
Copyright (c) 2012 - Dimok
Copyright (c) 2012 - giantpune

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
#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <vector>
#include <string>
#include <gccore.h>
#include "BannerTools.h"

class Texture
{
public:
	static const u32 MAGIC = MAKE_FOURCC(0x00, ' ', 0xAF, 0x30);

	Texture() : header(NULL), palette(NULL), texture_data(NULL) {}
	virtual ~Texture();

	void Load(const u8 *texture);
	const std::string &getName() const { return name; }
	void setName(const std::string& _name) { name = _name; }

	// load custom data into the texture object
	void LoadTextureData(const u8 *data, u16 width, u16 height, u8 fmt);

	void Apply(u8 &tlutName, u8 map_id, u8 wrap_s, u8 wrap_t) const;
private:
	struct Header
	{
		u32 magic;
		u32 num_textures;
		u32 header_size;
	} __attribute__((packed)) ;

	struct TPL_Texture
	{
		u32 texture_offset;
		u32 palette_offset;
	} __attribute__((packed)) ;

	struct TPL_Texture_Header
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
	} __attribute__((packed));

	struct TPL_Palette_Header
	{
		u16		num_items;
		u8		unpacked;
		u8		pad;
		u32		format;
		u32		offset;
	} __attribute__((packed));

	Texture::Header *header;
	TPL_Palette_Header *palette;
	u8 *texture_data;
	GXTexObj texobj;
	std::string name;
};

class TextureList : public std::vector<Texture *>
{
	public:
		static const u32 MAGIC = MAKE_FOURCC('t', 'x', 'l', '1');
};

#endif
