/*
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
#ifndef OPENING_BNR_HPP_
#define OPENING_BNR_HPP_

#include <gctypes.h>
#include "usbloader/disc.h"
#include "CustomBanner.h"

typedef struct _GC_OpeningBnr
{
	u32 magic;					  // BNR1 or BNR2
	u8 pad[0x1C];
	u8 tpl_data[0x1800];			// 96x32 pixel format GX_TF_RGB5A3
	struct
	{
		u8 disc_title[0x20];		// Gamename
		u8 developer_short[0x20];   // Company/Developer
		u8 full_title[0x40];		// Full Game Title
		u8 developer[0x40];		 // Company/Developer Full name, or description
		u8 long_description[0x80];  // Game Description
	} description[6];			   // 6 only on BNR2 => English, German, French, Spanish, Italian, Dutch ??
} GC_OpeningBnr;

typedef struct _IMETHeader
{
	u8 zeroes[64];
	u32 fcc;
	u8 unk[8];
	u32 iconSize;
	u32 bannerSize;
	u32 soundSize;
	u32 flag1;
	u16 names[10][42]; // 10 languages (thanks dkosmari for the info)
	u16 zeroes_2[7*42]; // padding for 7 more languages (thanks dkosmari for the info)
	u8 crypto[16];
} __attribute__((packed)) IMETHeader;

typedef struct _IMD5Header
{
	u32 fcc;
	u32 filesize;
	u8 zeroes[8];
	u8 crypto[16];
} __attribute__((packed)) IMD5Header;

typedef struct _U8Header
{
	u32 fcc;
	u32 rootNodeOffset;
	u32 headerSize;
	u32 dataOffset;
	u8 zeroes[16];
} __attribute__((packed)) U8Header;

typedef struct _U8Entry
{
	struct
	{
		u32 fileType :8;
		u32 nameOffset :24;
	};
	u32 fileOffset;
	union
	{
		u32 fileLength;
		u32 numEntries;
	};
} __attribute__( ( packed ) ) U8Entry;


static inline const char * u8Filename(const U8Entry *fst, int i)
{
	return (char *) (fst + fst[0].numEntries) + fst[i].nameOffset;
}

class OpeningBNR
{
	public:
		OpeningBNR();
		~OpeningBNR();
		bool Load(const discHdr * header);
		bool LoadWiiBanner(const discHdr * header);
		bool LoadChannelBanner(const discHdr *header);
		CustomBanner *CreateGCBanner(const discHdr * header);
		CustomBanner *CreateGCIcon(const discHdr * header);

		const u8 * Get() const { return (const u8*) imetHdr; }
		u32 GetSize() const { return filesize; }

		bool LoadCachedBNR(const char *id);
		void WriteCachedBNR(const char *id, const u8 *buffer, u32 size);

		const u16 * GetIMETTitle(int lang);
		const u16 * GetIMETTitle(const discHdr * header, int lang) { Load(header); return GetIMETTitle(lang); }
	private:
		u8 *LoadGCBNR(const discHdr * header, u32 *len = 0);
		IMETHeader *imetHdr;
		u32 filesize;
		char gameID[7];
};

class BNRInstance : public OpeningBNR
{
	public:
		static BNRInstance * Instance() { if(!instance) instance = new BNRInstance; return instance; }
		static void DestroyInstance() { delete instance; instance = NULL; }
	private:
		BNRInstance() { }
		~BNRInstance() { }
		static BNRInstance * instance;
};

#endif
