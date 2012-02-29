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
#include <string.h>
#include <math.h>
#include "AifDecoder.hpp"

typedef struct
{
	u32 fccCOMM;
	u32 size;
	u16 channels;
	u8 frames[4];
	u16 bps;
	u8 freq[10];
} SAIFFCommChunk;

typedef struct
{
	u32 fccSSND;
	u32 size;
	u32 offset;
	u32 blockSize;
} SAIFFSSndChunk;

// ------
// Copyright (C) 1988-1991 Apple Computer, Inc.
#ifndef HUGE_VAL
# define HUGE_VAL HUGE
#endif

# define UnsignedToFloat(u)		 (((double)((long)(u - 2147483647L - 1))) + 2147483648.0)

static double ConvertFromIeeeExtended(const unsigned char* bytes)
{
	double	f;
	int	expon;
	unsigned long hiMant, loMant;

	expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
	hiMant	=	((unsigned long)(bytes[2] & 0xFF) << 24)
			|	((unsigned long)(bytes[3] & 0xFF) << 16)
			|	((unsigned long)(bytes[4] & 0xFF) << 8)
			|	((unsigned long)(bytes[5] & 0xFF));
	loMant	=	((unsigned long)(bytes[6] & 0xFF) << 24)
			|	((unsigned long)(bytes[7] & 0xFF) << 16)
			|	((unsigned long)(bytes[8] & 0xFF) << 8)
			|	((unsigned long)(bytes[9] & 0xFF));

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

AifDecoder::AifDecoder(const char * filepath)
	: SoundDecoder(filepath)
{
	SoundType = SOUND_AIF;

	if(!file_fd)
		return;

	OpenFile();
}

AifDecoder::AifDecoder(const u8 * snd, int len)
	: SoundDecoder(snd, len)
{
	SoundType = SOUND_AIF;

	if(!file_fd)
		return;

	OpenFile();
}

AifDecoder::~AifDecoder()
{
}

void AifDecoder::OpenFile()
{
	SWaveHdr Header;
	file_fd->read((u8 *) &Header, sizeof(SWaveHdr));

	if (Header.magicRIFF != 'FORM')
	{
		CloseFile();
		return;
	}
	else if(Header.magicWAVE != 'AIFF')
	{
		CloseFile();
		return;
	}

	u32 magic = 0;

	while(1)
	{
		int ret = file_fd->read((u8 *) &magic, sizeof(magic));
		if(ret <= 0)
		{
			CloseFile();
			return;
		}

		if(magic == 'COMM')
			break;
		else
			file_fd->seek(-3, SEEK_CUR);
	}

	// seek back to COMM chunk start
	file_fd->seek(-sizeof(magic), SEEK_CUR);

	SAIFFCommChunk CommHdr;
	file_fd->read((u8 *) &CommHdr, sizeof(SAIFFCommChunk));

	if(CommHdr.fccCOMM != 'COMM')
	{
		CloseFile();
		return;
	}

	// Seek to next chunk start
	file_fd->seek(-sizeof(SAIFFCommChunk) + sizeof(SWaveChunk) + CommHdr.size, SEEK_CUR);

	int ret = -1;
	SWaveChunk chunkHdr;
	memset(&chunkHdr, 0, sizeof(SWaveChunk));

	do
	{
		// Seek to next chunk start
		file_fd->seek(chunkHdr.size, SEEK_CUR);
		ret = file_fd->read((u8 *) &chunkHdr, sizeof(SWaveChunk));
	}
	while(ret > 0 && chunkHdr.magicDATA != 'SSND');

	// Seek back to start of SSND chunk
	file_fd->seek(-sizeof(SWaveChunk), SEEK_CUR);

	SAIFFSSndChunk SSndChunk;
	file_fd->read((u8 *) &SSndChunk, sizeof(SAIFFSSndChunk));

	if(SSndChunk.fccSSND != 'SSND')
	{
		CloseFile();
		return;
	}

	DataOffset = file_fd->tell();
	DataSize = SSndChunk.size-8;
	SampleRate = (u32) ConvertFromIeeeExtended(CommHdr.freq);
	Format = VOICE_STEREO_16BIT;

	if(CommHdr.channels == 1 && CommHdr.bps == 8)
		Format = VOICE_MONO_8BIT;
	else if (CommHdr.channels == 1 && CommHdr.bps == 16)
		Format = VOICE_MONO_16BIT;
	else if (CommHdr.channels == 2 && CommHdr.bps == 8)
		Format = VOICE_STEREO_8BIT;
	else if (CommHdr.channels == 2 && CommHdr.bps == 16)
		Format = VOICE_STEREO_16BIT;

	Decode();
}

void AifDecoder::CloseFile()
{
	if(file_fd)
		delete file_fd;

	file_fd = NULL;
}

int AifDecoder::Read(u8 * buffer, int buffer_size, int pos)
{
	if(!file_fd)
		return -1;

	if(CurPos >= (int) DataSize)
		return 0;

	file_fd->seek(DataOffset+CurPos, SEEK_SET);

	if(buffer_size > (int) DataSize-CurPos)
		buffer_size = DataSize-CurPos;

	int read = file_fd->read(buffer, buffer_size);
	if(read > 0)
	{
		CurPos += read;
	}

	return read;
}
