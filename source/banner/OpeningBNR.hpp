#ifndef OPENING_BNR_HPP_
#define OPENING_BNR_HPP_

#include <gctypes.h>

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
		bool Load(const u8 * gameID);
		bool Load(const u64 &tid);
		const u16 * GetIMETTitle(int lang);
		const u16 * GetIMETTitle(const u8 * gameID, int lang) { Load(gameID); return GetIMETTitle(lang); };
		const u8 * GetBannerSound(u32 * size);
		const u8 * GetBannerSound(const u8 * gameID, u32 * size) { Load(gameID); return GetBannerSound(size); };
		const u8 * GetBannerSound(const u64 &tid, u32 * size) { Load(tid); return GetBannerSound(size); };
	private:
		IMETHeader *imetHdr;
		char gameID[7];
};

class BNRInstance : public OpeningBNR
{
	public:
		static BNRInstance * Instance() { if(!instance) instance = new BNRInstance; return instance; };
		static void DestroyInstance() { delete instance; instance = NULL; };
	private:
		BNRInstance() { };
		~BNRInstance() { };
		static BNRInstance * instance;
};

#endif
