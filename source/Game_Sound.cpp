/***************************************************************************
 * Copyright (C) 2009
 * by Hibernatus
 *
 * Game_Sound Class by Dimok
 * Many other modifications and adjustments by Dimok for USB Loader GX
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
 ***************************************************************************/
#include <stdio.h>
#include <ogcsys.h>
#include <unistd.h>

#include "usbloader/disc.h"
#include "usbloader/wbfs.h"
#include "prompts/PromptWindows.h"
#include "libwbfs/libwbfs.h"
#include "language/gettext.h"
#include "Game_Sound.h"


#define compare(src, str) ((src && str) ? strncasecmp((const char*) src, (const char *) str, strlen((const char *) str)) : -1)


GameSound::GameSound(const u8 * discid)
:GuiSound(NULL, 0, SOUND_PCM)
{
    sound = NULL;
    length = 0;
	type = SOUND_PCM;
	voice = 2;
	volume = 100;
	loop = false;
	freq = 0;
	format = 0;
	this->LoadSound(discid);
}

GameSound::~GameSound()
{
    Stop();

    if(sound)
        free(sound);
    sound = NULL;
}

bool GameSound::Play()
{
    Stop();

	if (!sound || length == 0)
		return false;
	if (volume == 0)
		return true;

	return ASND_SetVoice(voice, format, freq, 0, sound, length, volume, volume, 0) == SND_OK;
}


struct IMD5Header
{
	u32 fcc;
	u32 filesize;
	u8 zeroes[8];
	u8 crypto[16];
} __attribute__((packed));

struct IMETHeader
{
	u8 zeroes[64];
	u32 fcc;
	u8 unk[8];
	u32 iconSize;
	u32 bannerSize;
	u32 soundSize;
	u32 flag1;
	u8 names[7][84];
	u8 zeroes_2[0x348];
	u8 crypto[16];
} __attribute__((packed));

struct U8Header
{
	u32 fcc;
	u32 rootNodeOffset;
	u32 headerSize;
	u32 dataOffset;
	u8 zeroes[16];
} __attribute__((packed));

struct U8Entry
{
	struct
	{
		u32 fileType : 8;
		u32 nameOffset : 24;
	};
	u32 fileOffset;
	union
	{
		u32 fileLength;
		u32 numEntries;
	};
} __attribute__((packed));

struct LZ77Info
{
	u16 length : 4;
	u16 offset : 12;
} __attribute__((packed));

static char *u8Filename(const U8Entry *fst, int i)
{
	return (char *)(fst + fst[0].numEntries) + fst[i].nameOffset;
}

struct SWaveHdr
{
	u32 fccRIFF;
	u32 size;
	u32 fccWAVE;
} __attribute__((packed));

struct SWaveFmtChunk
{
	u32 fccFMT;
	u32 size;
	u16 format;
	u16 channels;
	u32 freq;
	u32 avgBps;
	u16 alignment;
	u16 bps;
} __attribute__((packed));

struct SWaveChunk
{
	u32 fcc;
	u32 size;
	u8 data;
} __attribute__((packed));

struct SAIFFCommChunk
{
	u32 fccCOMM;
	u32 size;
	u16 channels;
	u32 samples;
	u16 bps;
	u8 freq[10];
} __attribute__((packed));

struct SAIFFSSndChunk
{
	u32 fccSSND;
	u32 size;
	u32 offset;
	u32 blockSize;
	u8 data;
} __attribute__((packed));

inline u32 le32(u32 i)
{
	return ((i & 0xFF) << 24) | ((i & 0xFF00) << 8) | ((i & 0xFF0000) >> 8) | ((i & 0xFF000000) >> 24);
}

inline u16 le16(u16 i)
{
	return ((i & 0xFF) << 8) | ((i & 0xFF00) >> 8);
}

static u8 * uncompressLZ77(const u8 *inputBuf, u32 inputLength, u32 &size)
{
	u8 * buffer = NULL;
	if (inputLength <= 0x8 || compare(inputBuf, "LZ77") != 0 || inputBuf[4] != 0x10)
	{
		return buffer;
	}
	u32 uncSize = le32(((const u32 *)inputBuf)[1] << 8);
	const u8 *inBuf = inputBuf + 8;
	const u8 *inBufEnd = inputBuf + inputLength;
	buffer = (u8 *) malloc(uncSize);
	if (!buffer)
		return buffer;
	u8 *bufCur = buffer;
	u8 *bufEnd = buffer + uncSize;
	while (bufCur < bufEnd && inBuf < inBufEnd)
	{
		u8 flags = *inBuf;
		++inBuf;
		for (int i = 0; i < 8 && bufCur < bufEnd && inBuf < inBufEnd; ++i)
		{
			if ((flags & 0x80) != 0)
			{
				const LZ77Info &info = *(const LZ77Info *)inBuf;
				inBuf += sizeof (LZ77Info);
				int length = info.length + 3;
				if (bufCur - info.offset - 1 < buffer || bufCur + length > bufEnd)
					return buffer;
				memcpy(bufCur, bufCur - info.offset - 1, length);
				bufCur += length;
			}
			else
			{
				*bufCur = *inBuf;
				++inBuf;
				++bufCur;
			}
			flags <<= 1;
		}
	}
	size = uncSize;
	return buffer;
}

void GameSound::LoadSound(const u8 *discid)
{
    if(!discid)
        return;

    if(sound)
        free(sound);
    sound = NULL;

 	Disc_SetUSB(NULL);
	wbfs_disc_t *disc = wbfs_open_disc(GetHddInfo(), (u8 *) discid);
	if(!disc)
	{
        WindowPrompt(tr("Can't find disc"), 0, tr("OK"));
        return;
	}
	wiidisc_t *wdisc = wd_open_disc((int (*)(void *, u32, u32, void *))wbfs_disc_read, disc);
	if(!wdisc)
	{
        WindowPrompt(tr("Can't open disc"), 0, tr("OK"));
        return;
	}
	u32 size = 0;
	u8 * snddata = wd_extract_file(wdisc, &size, ALL_PARTITIONS, (char *) "opening.bnr");
	if(!snddata)
	{
        WindowPrompt(tr("ERROR"), tr("Failed to extract opening.bnr"), tr("OK"));
	    return;
	}

	wd_close_disc(wdisc);
	wbfs_close_disc(disc);

	const u8 *soundBin;
	const u8 *bnrArc;
	const U8Entry *fst;
	u32 i;
	const u8 *soundChunk;
	u32 soundChunkSize;

	const IMETHeader &imetHdr = *(IMETHeader *) snddata;
	if (compare(&imetHdr.fcc, "IMET") != 0)
    {
        WindowPrompt(tr("IMET Header wrong."), 0, tr("OK"));
		return;
    }
	bnrArc = (const u8 *)(&imetHdr + 1);

	const U8Header &bnrArcHdr = *(U8Header *)bnrArc;

	fst = (const U8Entry *)(bnrArc + bnrArcHdr.rootNodeOffset);
	for (i = 1; i < fst[0].numEntries; ++i)
		if (fst[i].fileType == 0 && strcasecmp(u8Filename(fst, i), "sound.bin") == 0)
			break;
	if (i >= fst[0].numEntries)
	{
		return;
	}
	soundBin = bnrArc + fst[i].fileOffset;
	if (compare((&((IMD5Header *)soundBin)->fcc), "IMD5") != 0)
	{
        WindowPrompt(tr("IMD5 Header not right."), 0, tr("OK"));
		return;
	}
	soundChunk = soundBin + sizeof (IMD5Header);
	soundChunkSize = fst[i].fileLength - sizeof (IMD5Header);
	if (compare(soundChunk, "LZ77") == 0)
	{
		u32 uncSize = NULL;
		u8 * uncompressed_data = uncompressLZ77(soundChunk, soundChunkSize, uncSize);
		if (!uncompressed_data)
		{
            WindowPrompt(tr("Can't decompress LZ77"), 0, tr("OK"));
			return;
		}
		soundChunk = uncompressed_data;
		soundChunkSize = uncSize;
	}

	if(compare(soundChunk, "RIFF") == 0)
	{
            fromWAV(soundChunk, soundChunkSize);
	}
	else if(compare(soundChunk, "BNS ") == 0)
    {
            fromBNS(soundChunk, soundChunkSize);
    }
    else if(compare(soundChunk, "FORM") == 0)
    {
            fromAIFF(soundChunk, soundChunkSize);
    }

    free(snddata);
    snddata = NULL;
}

bool GameSound::fromWAV(const u8 *buffer, u32 size)
{
	const u8 *bufEnd = buffer + size;
	const SWaveHdr &hdr = *(SWaveHdr *)buffer;
	if (size < sizeof hdr)
		return false;
	if (compare(&hdr.fccRIFF, "RIFF") != 0)
		return false;
	if (size < le32(hdr.size) + sizeof hdr.fccRIFF + sizeof hdr.size)
		return false;
	if (compare(&hdr.fccWAVE, "WAVE") != 0)
		return false;
	// Find fmt
	const SWaveChunk *chunk = (const SWaveChunk *)(buffer + sizeof hdr);
	while (&chunk->data < bufEnd && compare(&chunk->fcc, "fmt ") != 0)
		chunk = (const SWaveChunk *)(&chunk->data + le32(chunk->size));
	if (&chunk->data >= bufEnd)
		return false;
	const SWaveFmtChunk &fmtChunk = *(const SWaveFmtChunk *)chunk;
	// Check format
	if (le16(fmtChunk.format) != 1)
		return false;
	format = (u8)-1;
	if (le16(fmtChunk.channels) == 1 && le16(fmtChunk.bps) == 8 && le16(fmtChunk.alignment) <= 1)
		format = VOICE_MONO_8BIT;
	else if (le16(fmtChunk.channels) == 1 && le16(fmtChunk.bps) == 16 && le16(fmtChunk.alignment) <= 2)
		format = VOICE_MONO_16BIT;
	else if (le16(fmtChunk.channels) == 2 && le16(fmtChunk.bps) == 8 && le16(fmtChunk.alignment) <= 2)
		format = VOICE_STEREO_8BIT;
	else if (le16(fmtChunk.channels) == 2 && le16(fmtChunk.bps) == 16 && le16(fmtChunk.alignment) <= 4)
		format = VOICE_STEREO_16BIT;
	if (format == (u8)-1)
		return false;
	freq = le32(fmtChunk.freq);
	// Find data
	chunk = (const SWaveChunk *)(&chunk->data + le32(chunk->size));
	while (&chunk->data < bufEnd && compare(&chunk->fcc, "data") != 0)
		chunk = (const SWaveChunk *)(&chunk->data + le32(chunk->size));
	if (compare(&chunk->fcc, "data") != 0 || &chunk->data + le32(chunk->size) > bufEnd)
		return false;
	// Data found
	sound = (u8 *) malloc(le32(chunk->size));
	if (!sound)
		return false;
	memcpy(sound, &chunk->data, le32(chunk->size));
	length = le32(chunk->size);
	// Endianness
	if (le16(fmtChunk.bps) == 16)
		for (u32 i = 0; i < length / sizeof (u16); ++i)
			((u16 *) sound)[i] = le16(((u16 *) sound)[i]);
	return true;
}

// ------
// Copyright (C) 1988-1991 Apple Computer, Inc.
#ifndef HUGE_VAL
# define HUGE_VAL HUGE
#endif

# define UnsignedToFloat(u)         (((double)((long)(u - 2147483647L - 1))) + 2147483648.0)

double ConvertFromIeeeExtended(const unsigned char* bytes)
{
    double    f;
    int    expon;
    unsigned long hiMant, loMant;

    expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
    hiMant    =    ((unsigned long)(bytes[2] & 0xFF) << 24)
            |    ((unsigned long)(bytes[3] & 0xFF) << 16)
            |    ((unsigned long)(bytes[4] & 0xFF) << 8)
            |    ((unsigned long)(bytes[5] & 0xFF));
    loMant    =    ((unsigned long)(bytes[6] & 0xFF) << 24)
            |    ((unsigned long)(bytes[7] & 0xFF) << 16)
            |    ((unsigned long)(bytes[8] & 0xFF) << 8)
            |    ((unsigned long)(bytes[9] & 0xFF));

    if (expon == 0 && hiMant == 0 && loMant == 0) {
        f = 0;
    }
    else {
        if (expon == 0x7FFF) {
            f = HUGE_VAL;
        }
        else {
            expon -= 16383;
            f  = ldexp(UnsignedToFloat(hiMant), expon-=31);
            f += ldexp(UnsignedToFloat(loMant), expon-=32);
        }
    }

    if (bytes[0] & 0x80)
        return -f;
    else
        return f;
}
// ------

bool GameSound::fromAIFF(const u8 *buffer, u32 size)
{
	const u8 *bufEnd = buffer + size;
	const SWaveHdr &hdr = *(SWaveHdr *)buffer;
	if (size < sizeof hdr)
		return false;
	if (compare(&hdr.fccRIFF, "FORM") != 0)
		return false;
	if (size < hdr.size + sizeof hdr.fccRIFF + sizeof hdr.size)
		return false;
	if (compare(&hdr.fccWAVE, "AIFF") != 0)
		return false;
	// Find fmt
	const SWaveChunk *chunk = (const SWaveChunk *)(buffer + sizeof hdr);
	while (&chunk->data < bufEnd && compare(&chunk->fcc, "COMM") != 0)
		chunk = (const SWaveChunk *)(&chunk->data + chunk->size);
	if (&chunk->data >= bufEnd)
		return false;
	const SAIFFCommChunk &fmtChunk = *(const SAIFFCommChunk *)chunk;
	// Check format
	format = (u8)-1;
	if (le16(fmtChunk.channels) == 1 && fmtChunk.bps == 8)
		format = VOICE_MONO_8BIT;
	else if (fmtChunk.channels == 1 && fmtChunk.bps == 16)
		format = VOICE_MONO_16BIT;
	else if (fmtChunk.channels == 2 && fmtChunk.bps == 8)
		format = VOICE_STEREO_8BIT;
	else if (fmtChunk.channels == 2 && fmtChunk.bps == 16)
		format = VOICE_STEREO_16BIT;
	if (format == (u8)-1)
		return false;
	freq = (u32)ConvertFromIeeeExtended(fmtChunk.freq);
	// Find data
	chunk = (const SWaveChunk *)(&chunk->data + chunk->size);
	while (&chunk->data < bufEnd && compare(&chunk->fcc, "SSND") != 0)
		chunk = (const SWaveChunk *)(&chunk->data + chunk->size);
	if (compare(&chunk->fcc, "SSND") != 0 || &chunk->data + chunk->size > bufEnd)
		return false;
	// Data found
	const SAIFFSSndChunk &dataChunk = *(const SAIFFSSndChunk *)chunk;
	sound = (u8 *) malloc(dataChunk.size - 8);
	if (!sound)
		return false;
	memcpy(sound, &dataChunk.data, dataChunk.size - 8);
	length = dataChunk.size - 8;
	return true;
}

struct BNSHeader
{
	u32 fccBNS;
	u32 magic;
	u32 size;
	u16 unk1;
	u16 unk2;
	u32 infoOffset;
	u32 infoSize;
	u32 dataOffset;
	u32 dataSize;
} __attribute__((packed));

struct BNSInfo
{
	u32 fccINFO;
	u32 size;
	u8 codecNum;
	u8 loopFlag;
	u8 chanCount;
	u8 zero;
	u16 freq;
	u8 pad1[2];
	u32 loopStart;
	u32 loopEnd;
	u32 offsetToChanStarts;
	u8 pad2[4];
	u32 chan1StartOffset;
	u32 chan2StartOffset;
	u32 chan1Start;
	u32 coeff1Offset;
	u8 pad3[4];
	u32 chan2Start;
	u32 coeff2Offset;
	u8 pad4[4];
	s16 coefficients1[8][2];
	u16 chan1Gain;
	u16 chan1PredictiveScale;
	s16 chan1PrevSamples[2];
	u16 chan1LoopPredictiveScale;
	s16 chan1LoopPrevSamples[2];
	u16 chan1LoopPadding;
	s16 coefficients2[8][2];
	u16 chan2Gain;
	u16 chan2PredictiveScale;
	s16 chan2PrevSamples[2];
	u16 chan2LoopPredictiveScale;
	s16 chan2LoopPrevSamples[2];
	u16 chan2LoopPadding;
} __attribute__((packed));

struct BNSData
{
	u32 fccDATA;
	u32 size;
	u8 data;
} __attribute__((packed));

struct ADPCMByte
{
	s8 sample1 : 4;
	s8 sample2 : 4;
} __attribute__((packed));

struct BNSADPCMBlock
{
	u8 pad : 1;
	u8 coeffIndex : 3;
	u8 lshift : 4;
	ADPCMByte samples[7];
} __attribute__((packed));

struct BNSDecObj
{
	s16 prevSamples[2];
	s16 coeff[8][2];
};

static void loadBNSInfo(BNSInfo &bnsInfo, const u8 *buffer)
{
	const u8 *ptr = buffer + 8;
	bnsInfo = *(const BNSInfo *)buffer;
	if (bnsInfo.offsetToChanStarts == 0x18 && bnsInfo.chan1StartOffset == 0x20 && bnsInfo.chan2StartOffset == 0x2C
		&& bnsInfo.coeff1Offset == 0x38 && bnsInfo.coeff2Offset == 0x68)
		return;
	bnsInfo.chan1StartOffset = *(const u32 *)(ptr + bnsInfo.offsetToChanStarts);
	bnsInfo.chan1Start = *(const u32 *)(ptr + bnsInfo.chan1StartOffset);
	bnsInfo.coeff1Offset = *(const u32 *)(ptr + bnsInfo.chan1StartOffset + 4);
	if ((u8 *)bnsInfo.coefficients1 != ptr + bnsInfo.coeff1Offset)
		memcpy(bnsInfo.coefficients1, ptr + bnsInfo.coeff1Offset, (u8 *)bnsInfo.coefficients2 - (u8 *)&bnsInfo.coefficients1);
	if (bnsInfo.chanCount == 2)
	{
		bnsInfo.chan2StartOffset = *(const u32 *)(ptr + bnsInfo.offsetToChanStarts + 4);
		bnsInfo.chan2Start = *(const u32 *)(ptr + bnsInfo.chan2StartOffset);
		bnsInfo.coeff2Offset = *(const u32 *)(ptr + bnsInfo.chan2StartOffset + 4);
		if ((u8 *)bnsInfo.coefficients2 != ptr + bnsInfo.coeff2Offset)
			memcpy(bnsInfo.coefficients2, ptr + bnsInfo.coeff2Offset, (u8 *)bnsInfo.coefficients2 - (u8 *)&bnsInfo.coefficients1);
	}
}

static void decodeADPCMBlock(s16 *buffer, const BNSADPCMBlock &block, BNSDecObj &bnsDec)
{
	int h1 = bnsDec.prevSamples[0];
	int h2 = bnsDec.prevSamples[1];
	int c1 = bnsDec.coeff[block.coeffIndex][0];
	int c2 = bnsDec.coeff[block.coeffIndex][1];
	for (int i = 0; i < 14; ++i)
	{
		int nibSample = ((i & 1) == 0) ? block.samples[i / 2].sample1 : block.samples[i / 2].sample2;
		int sampleDeltaHP = (nibSample << block.lshift) << 11;
		int predictedSampleHP = c1 * h1 + c2 * h2;
		int sampleHP = predictedSampleHP + sampleDeltaHP;
		buffer[i] = std::min(std::max(-32768, (sampleHP + 1024) >> 11), 32767);
		h2 = h1;
		h1 = buffer[i];
	}
	bnsDec.prevSamples[0] = h1;
	bnsDec.prevSamples[1] = h2;
}

static u8 * decodeBNS(u32 &size, const BNSInfo &bnsInfo, const BNSData &bnsData)
{
	static s16 smplBlock[14];
	BNSDecObj decObj;
	int numBlocks = (bnsData.size - 8) / 8;
	int numSamples = numBlocks * 14;
	const BNSADPCMBlock *inputBuf = (const BNSADPCMBlock *)&bnsData.data;
	u8 * buffer = (u8 *) malloc(numSamples * sizeof (s16));
	s16 *outputBuf;

	if (!buffer)
		return buffer;
	memcpy(decObj.coeff, bnsInfo.coefficients1, sizeof decObj.coeff);
	memcpy(decObj.prevSamples, bnsInfo.chan1PrevSamples, sizeof decObj.prevSamples);
	outputBuf = (s16 *)buffer;
	if (bnsInfo.chanCount == 1)
		for (int i = 0; i < numBlocks; ++i)
		{
			decodeADPCMBlock(smplBlock, inputBuf[i], decObj);
			memcpy(outputBuf, smplBlock, sizeof smplBlock);
			outputBuf += 14;
		}
	else
	{
		numBlocks /= 2;
		for (int i = 0; i < numBlocks; ++i)
		{
			decodeADPCMBlock(smplBlock, inputBuf[i], decObj);
			for (int j = 0; j < 14; ++j)
				outputBuf[j * 2] = smplBlock[j];
			outputBuf += 2 * 14;
		}
		outputBuf = (s16 *)buffer + 1;
		memcpy(decObj.coeff, bnsInfo.coefficients2, sizeof decObj.coeff);
		memcpy(decObj.prevSamples, bnsInfo.chan2PrevSamples, sizeof decObj.prevSamples);
		for (int i = 0; i < numBlocks; ++i)
		{
			decodeADPCMBlock(smplBlock, inputBuf[numBlocks + i], decObj);
			for (int j = 0; j < 14; ++j)
				outputBuf[j * 2] = smplBlock[j];
			outputBuf += 2 * 14;
		}
	}
	size = numSamples * sizeof (s16);
	return buffer;
}

bool GameSound::fromBNS(const u8 *buffer, u32 size)
{
	const BNSHeader &hdr = *(BNSHeader *)buffer;
	if (size < sizeof hdr)
		return false;
	if (compare(&hdr.fccBNS, "BNS ") != 0)
		return false;
	// Find info and data
	BNSInfo infoChunk;
	loadBNSInfo(infoChunk, buffer + hdr.infoOffset);
	const BNSData &dataChunk = *(const BNSData *)(buffer + hdr.dataOffset);
	// Check sizes
	if (size < hdr.size || size < hdr.infoOffset + hdr.infoSize || size < hdr.dataOffset + hdr.dataSize
		|| hdr.infoSize < 0x60 || hdr.dataSize < sizeof dataChunk
		|| infoChunk.size != hdr.infoSize || dataChunk.size != hdr.dataSize)
		return false;
	// Check format
	if (infoChunk.codecNum != 0)	// Only codec i've found : 0 = ADPCM. Maybe there's also 1 and 2 for PCM 8 or 16 bits ?
		return false;
	format = (u8)-1;
	if (infoChunk.chanCount == 1 && infoChunk.codecNum == 0)
		format = VOICE_MONO_16BIT;
	else if (infoChunk.chanCount == 2 && infoChunk.codecNum == 0)
		format = VOICE_STEREO_16BIT;
	if (format == (u8)-1)
		return false;
	freq = infoChunk.freq;
	// Copy data
	if (infoChunk.codecNum == 0)
	{
		sound = decodeBNS(length, infoChunk, dataChunk);
		if (!sound)
			return false;
	}
	else
	{
		sound = (u8*) malloc(dataChunk.size);
		if (!sound)
			return false;
		memcpy(sound, &dataChunk.data, dataChunk.size);
		length = dataChunk.size;
	}
	return true;
}
