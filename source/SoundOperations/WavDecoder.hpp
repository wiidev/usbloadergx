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
#ifndef WAVDECODER_HPP_
#define WAVDECODER_HPP_

#include "SoundDecoder.hpp"

typedef struct
{
	u32 magicRIFF;
	u32 size;
	u32 magicWAVE;
} SWaveHdr;

typedef struct
{
	u32 magicFMT;
	u32 size;
	u16 format;
	u16 channels;
	u32 freq;
	u32 avgBps;
	u16 alignment;
	u16 bps;
} SWaveFmtChunk;

typedef struct
{
	u32 magicDATA;
	u32 size;
} SWaveChunk;

class WavDecoder : public SoundDecoder
{
	public:
		WavDecoder(const char * filepath);
		WavDecoder(const u8 * snd, int len);
		virtual ~WavDecoder();
		int GetFormat() { return Format; };
		int GetSampleRate() { return SampleRate; };
		int Read(u8 * buffer, int buffer_size, int pos);
	protected:
		void OpenFile();
		void CloseFile();
		u32 DataOffset;
		u32 DataSize;
		u32 SampleRate;
		u8 Format;
		bool Is16Bit;
};

#endif
