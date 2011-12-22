#include <malloc.h>
#include <stdio.h>
#include "Channels/channels.h"
#include "usbloader/disc.h"
#include "usbloader/wbfs.h"
#include "utils/uncompress.h"
#include "OpeningBNR.hpp"

BNRInstance * BNRInstance::instance = NULL;

OpeningBNR::OpeningBNR()
	: imetHdr(0)
{
	memset(gameID, 0, sizeof(gameID));
}

OpeningBNR::~OpeningBNR()
{
	if(imetHdr)
		free(imetHdr);
}

bool OpeningBNR::Load(const u64 &tid, const char *pathPrefix)
{
	if(tid == 0)
		return false;

	u32 tidLow = (u32) (tid & 0xFFFFFFFF);
	char id[6];
	memset(id, 0, sizeof(id));
	memcpy(id, &tidLow, 4);

	if(memcmp(gameID, id, 6) == 0)
		return true;

	memcpy(gameID, id, 6);

	if(imetHdr)
		free(imetHdr);
	imetHdr = NULL;

	imetHdr = (IMETHeader*) Channels::GetOpeningBnr(tid, pathPrefix);
	if(!imetHdr)
		return false;

	if (imetHdr->fcc != 'IMET')
	{
		free(imetHdr);
		imetHdr = NULL;
		return false;
	}

	return true;
}

bool OpeningBNR::Load(const u8 * discid)
{
	if(!discid)
		return false;

	if(memcmp(gameID, discid, 6) == 0)
		return true;

	memcpy(gameID, discid, 6);

	if(imetHdr)
		free(imetHdr);
	imetHdr = NULL;

	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) gameID);
	if (!disc)
		return false;

	wiidisc_t *wdisc = wd_open_disc((int(*)(void *, u32, u32, void *)) wbfs_disc_read, disc);
	if (!wdisc)
	{
		WBFS_CloseDisc(disc);
		return false;
	}

	imetHdr = (IMETHeader*) wd_extract_file(wdisc, ALL_PARTITIONS, (char *) "opening.bnr");

	wd_close_disc(wdisc);
	WBFS_CloseDisc(disc);

	if(!imetHdr)
		return false;

	if (imetHdr->fcc != 'IMET')
	{
		free(imetHdr);
		imetHdr = NULL;
		return false;
	}

	return true;
}

const u16 * OpeningBNR::GetIMETTitle(int lang)
{
	if(!imetHdr || lang < 0 || lang >= 10)
		return NULL;

	if(imetHdr->names[lang][0] == 0)
		lang = CONF_LANG_ENGLISH;

	return imetHdr->names[lang];
}

const u8 * OpeningBNR::GetBannerSound(u32 * size)
{
	if(!imetHdr)
		return NULL;

	const U8Header *bnrArcHdr = (U8Header *) (imetHdr + 1);
	const U8Entry *fst = (const U8Entry *) (((const u8 *) bnrArcHdr) + bnrArcHdr->rootNodeOffset);

	u32 i;
	for (i = 1; i < fst[0].numEntries; ++i)
		if (fst[i].fileType == 0 && strcasecmp(u8Filename(fst, i), "sound.bin") == 0) break;

	if (i >= fst[0].numEntries)
	{
		return NULL;
	}

	const u8 *sound_bin = ((const u8 *) bnrArcHdr) + fst[i].fileOffset;
	if (((IMD5Header *) sound_bin)->fcc != 'IMD5')
	{
		return NULL;
	}
	const u8 *soundChunk = sound_bin + sizeof(IMD5Header);
	u32 soundChunkSize = fst[i].fileLength - sizeof(IMD5Header);

	if (*((u32*) soundChunk) == 'LZ77')
	{
		u32 uncSize = 0;
		u8 * uncompressed_data = uncompressLZ77(soundChunk, soundChunkSize, &uncSize);
		if (!uncompressed_data)
		{
			return NULL;
		}
		if (size) *size = uncSize;

		return uncompressed_data;
	}

	u8 *out = (u8 *) malloc(soundChunkSize);
	if (out)
	{
		memcpy(out, soundChunk, soundChunkSize);
		if (size) *size = soundChunkSize;
	}

	return out;
}
