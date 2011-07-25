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
#ifndef BNSDECODER_HPP_
#define BNSDECODER_HPP_

#include "SoundDecoder.hpp"

typedef struct _SoundBlock
{
	u8 * buffer;
	u32 size;
	u8 format;
	u32 frequency;
	u32 loopStart;
	u32 loopEnd;
	u8 loopFlag;
} SoundBlock;

class BNSDecoder : public SoundDecoder
{
	public:
		BNSDecoder(const char * filepath);
		BNSDecoder(const u8 * snd, int len);
		virtual ~BNSDecoder();
		int GetFormat() { return SoundData.format; };
		int GetSampleRate() { return SoundData.frequency; };
		int Read(u8 * buffer, int buffer_size, int pos);
	protected:
		void OpenFile();
		void CloseFile();
		SoundBlock SoundData;
};

#endif
