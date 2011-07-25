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
#include <mad.h>

#include "SoundDecoder.hpp"

class Mp3Decoder : public SoundDecoder
{
	public:
		Mp3Decoder(const char * filepath);
		Mp3Decoder(const u8 * sound, int len);
		virtual ~Mp3Decoder();
		int GetFormat() { return Format; };
		int GetSampleRate() { return SampleRate; };
		int Rewind();
		int Read(u8 * buffer, int buffer_size, int pos);
	protected:
		void OpenFile();
		struct mad_stream Stream;
		struct mad_frame Frame;
		struct mad_synth Synth;
		mad_timer_t Timer;
		u8 * GuardPtr;
		u8 * ReadBuffer;
		u8 Format;
		u32 SampleRate;
		u32 SynthPos;
};
