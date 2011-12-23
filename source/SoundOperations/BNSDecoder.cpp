/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
 * for WiiXplorer 2010
 ***************************************************************************/
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "BNSDecoder.hpp"

SoundBlock DecodefromBNS(const u8 *buffer, u32 size);

BNSDecoder::BNSDecoder(const char * filepath)
	: SoundDecoder(filepath)
{
	SoundType = SOUND_BNS;
	memset(&SoundData, 0, sizeof(SoundBlock));

	if(!file_fd)
		return;

	OpenFile();
}

BNSDecoder::BNSDecoder(const u8 * snd, int len)
	: SoundDecoder(snd, len)
{
	SoundType = SOUND_BNS;
	memset(&SoundData, 0, sizeof(SoundBlock));

	if(!file_fd)
		return;

	OpenFile();
}

BNSDecoder::~BNSDecoder()
{
	ExitRequested = true;
	while(Decoding)
		usleep(100);

	if(SoundData.buffer != NULL)
		free(SoundData.buffer);

	SoundData.buffer = NULL;
}

void BNSDecoder::OpenFile()
{
	u8 * tempbuff = new (std::nothrow) u8[file_fd->size()];
	if(!tempbuff)
	{
		CloseFile();
		return;
	}

	int done = 0;

	while(done < file_fd->size())
	{
		int read = file_fd->read(tempbuff+done, file_fd->size()-done);
		if(read > 0)
			done += read;
		else
		{
			CloseFile();
			return;
		}
	}

	SoundData = DecodefromBNS(tempbuff, done);
	if(SoundData.buffer == NULL)
	{
		CloseFile();
		return;
	}

	delete [] tempbuff;
	tempbuff = NULL;

	Decode();
}

void BNSDecoder::CloseFile()
{
	if(file_fd)
		delete file_fd;

	file_fd = NULL;
}

int BNSDecoder::Read(u8 * buffer, int buffer_size, int pos)
{
	if(!SoundData.buffer)
		return -1;

	if(SoundData.loopFlag)
	{
		int factor = SoundData.format == VOICE_STEREO_16BIT ? 4 : 2;
		if(CurPos >= (int) SoundData.loopEnd*factor)
			CurPos = SoundData.loopStart*factor;

		if(buffer_size > (int) SoundData.loopEnd*factor-CurPos)
			buffer_size = SoundData.loopEnd*factor-CurPos;
	}
	else
	{
		if(CurPos >= (int) SoundData.size)
			return 0;

		if(buffer_size > (int) SoundData.size-CurPos)
			buffer_size = SoundData.size-CurPos;
	}

	memcpy(buffer, SoundData.buffer+CurPos, buffer_size);
	CurPos += buffer_size;

	return buffer_size;
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

SoundBlock DecodefromBNS(const u8 *buffer, u32 size)
{
	SoundBlock OutBlock;
	memset(&OutBlock, 0, sizeof(SoundBlock));

	const BNSHeader &hdr = *(BNSHeader *)buffer;
	if (size < sizeof hdr)
		return OutBlock;
	if (hdr.fccBNS != 'BNS ')
		return OutBlock;
	// Find info and data
	BNSInfo infoChunk;
	loadBNSInfo(infoChunk, buffer + hdr.infoOffset);
	const BNSData &dataChunk = *(const BNSData *)(buffer + hdr.dataOffset);
	// Check sizes
	if (size < hdr.size || size < hdr.infoOffset + hdr.infoSize || size < hdr.dataOffset + hdr.dataSize
		|| hdr.infoSize < 0x60 || hdr.dataSize < sizeof dataChunk
		|| infoChunk.size != hdr.infoSize || dataChunk.size > hdr.dataSize)
		return OutBlock;
	// Check format
	if (infoChunk.codecNum != 0)	// Only codec i've found : 0 = ADPCM. Maybe there's also 1 and 2 for PCM 8 or 16 bits ?
		return OutBlock;
	u8 format = (u8)-1;
	if (infoChunk.chanCount == 1 && infoChunk.codecNum == 0)
		format = VOICE_MONO_16BIT;
	else if (infoChunk.chanCount == 2 && infoChunk.codecNum == 0)
		format = VOICE_STEREO_16BIT;
	if (format == (u8)-1)
		return OutBlock;
	u32 freq = (u32) infoChunk.freq;
	u32 length = 0;
	// Copy data
	if (infoChunk.codecNum == 0)
	{
		OutBlock.buffer = decodeBNS(length, infoChunk, dataChunk);
		if (!OutBlock.buffer)
			return OutBlock;
	}
	else
	{
		OutBlock.buffer = (u8*) malloc(dataChunk.size);
		if (!OutBlock.buffer)
			return OutBlock;
		memcpy(OutBlock.buffer, &dataChunk.data, dataChunk.size);
		length = dataChunk.size;
	}

	OutBlock.frequency = freq;
	OutBlock.format = format;
	OutBlock.size = length;
	OutBlock.loopStart = infoChunk.loopStart;
	OutBlock.loopEnd = infoChunk.loopEnd;
	OutBlock.loopFlag = infoChunk.loopFlag;

	return OutBlock;
}
