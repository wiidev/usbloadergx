/*
Copyright (c) 2012 - giantpune
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

#ifndef __WII_FONT_H_
#define __WII_FONT_H_

#include <list>
#include <vector>

#include "Pane.h"

class WiiFont
{
public:
	static const u32 MAGIC_FONT = MAKE_FOURCC('R', 'F', 'N', 'T');
	static const u32 MAGIC_FONT_ARCHIVE = MAKE_FOURCC('R', 'F', 'N', 'A');
	static const u32 MAGIC_VERSION = 0xFEFF0104;
	static const u32 MAGIC_GLYPH_GROUP = MAKE_FOURCC('G', 'L', 'G', 'R');
	static const u32 MAGIC_FONT_INFORMATION = MAKE_FOURCC('F', 'I', 'N', 'F');
	static const u32 MAGIC_TEXTURE_GLYPH = MAKE_FOURCC('T', 'G', 'L', 'P');
	static const u32 MAGIC_CHARACTER_CODE_MAP = MAKE_FOURCC('C', 'M', 'A', 'P');
	static const u32 MAGIC_CHARACTER_WIDTH = MAKE_FOURCC('C', 'W', 'D', 'H');

	WiiFont();
	~WiiFont();

	// load the file
	bool Load(const u8 *file);

	// apply texture, non-const because we load texture on demand
	bool Apply(u16 tex_idx);

	// struct to hold info for a character to keep from searching and calculating it all every frame
	struct CharInfo
	{
		u32 sheetIdx;
		f32 s1;
		f32 t1;
		f32 s2;
		f32 t2;
		s8 advanceKerning;
		u8 unk;
		s8 advanceGlyphX;
	};

	// get the character information
	const CharInfo *GetCharInfo(u16 charCode);

	// check if the font was loaded correctly
	bool IsLoaded() const { return font_loaded; }

	// get some parameters from the FINF header
	u8 CharacterWidth() const { return finf ? finf->charWidth : 0; }
	u8 CharacterHeight() const { return finf ? finf->height : 0; }

	const std::string& getName() const { return name; }
	void SetName(const std::string& _name) { name = _name; }

private:
	struct Header
	{
	  u32 magic;
	  u32 version;
	  u32 filesize;
	  u16 header_len;
	  u16 section_count;
	} __attribute__((packed));

	struct GlgrHeader
	{
		u32 magic;
		u32 sectionSize;
		u32 sheet_size;
		u16 glyphs_per_sheet;
		u16 set_count;
		u16 sheet_count;
		u16 cwdh_count;
		u16 cmap_count;
	}__attribute__(( packed ));

	struct FinfHeader
	{
		u32 magic;
		u32 headerSize;		// finf size
		u8 unk8_1;			// font type?
		u8 leading;			//
		u16 defaultChar;
		u8 leftMargin;
		u8 charWidth;
		u8 fullWidth;
		u8 encoding;
		u32 tglpOffset;		// TLGP offset
		u32 cwdhOffset;		// CWDH offset
		u32 cmapOffset;		// CMAP offset
		u8 height;
		u8 width;
		u8 ascent;
		u8 unk8_10;
	} __attribute__(( packed ));

	struct TglpHeader
	{
		u32 magic;
		u32 tglpSize;			// TGLP size

		u8 cellWidth;			// font width - 1
		u8 cellHeight;			// font heigh - 1
		u8 baselinePos;
		u8 maxCharWidth;

		u32 texSize;			// length of 1 image

		u16 texCnt;				// number of images
		u16 texType;			//
		u16 charColumns;		// character per row
		u16 charRows;			// characters per column
		u16 width;				// width of image
		u16 height;				// height of image
		u32 dataOffset;			// data offset
	} __attribute__(( packed ));

	struct CwdhHeader
	{
		u32 magic;
		u32 length;		// section length?
		u16 startIdx;	//
		u16 endIdx;		//
		u32 next;		//

	} __attribute__(( packed ));

	struct CmapEntry
	{
		u16 start;
		u16 end;
		u16 type;
		u16 pad;
		u32 pos;
		u16 charCode;
	} __attribute__(( packed ));

	struct Cwdh
	{
		s8 advanceKerning;
		u8 unk;
		s8 advanceGlyphX;
	} __attribute__(( packed ));

	// font texture decompress functions
	static bool Decompress_0x28( unsigned char *outBuf, u32 outLen, const unsigned char *inBuf, u32 inLen );
	u8 *GetUnpackedTexture(u16 sheetNo);

	// load a GX texture from index
	GXTexObj *LoadTextureObj(u16 texture_idx);

	// cmap parser
	bool ParseCmap(CmapEntry *cmapEntry);

	// just a duplicate check
	bool CheckCmap(u16 charCode, u16 mapValue);

	// get index of a character
	u16 CharToIdx(u16 charCode)
	{
		std::map<u16, u16>::iterator itr = cmap.find(charCode);
		if(itr != cmap.end())
			return itr->second;

		return finf->defaultChar;
	}

	WiiFont::Header *header;

	// pointers to each sections
	FinfHeader *finf;
	TglpHeader *tglp;
	CwdhHeader *cwdh;
	GlgrHeader *glgr;

	// struct to contain a decompressed texture caches
	struct TextureCache
	{
		u8* texture_data;
		bool allocated;
		GXTexObj texObj;
	};
	std::map<u16, TextureCache> textureMap;

	// character info cache map
	std::map<u16, CharInfo> charInfoMap;

	// holds all the character codes and their index within the font
	std::map<u16, u16> cmap;

	std::string name;
	bool font_loaded;
};

class FontList : public std::vector<WiiFont *>
{
public:
	static const u32 MAGIC = MAKE_FOURCC('f', 'n', 'l', '1');
};

#endif
