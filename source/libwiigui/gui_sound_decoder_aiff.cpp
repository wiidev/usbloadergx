/****************************************************************************
 * libwiigui
 *
 * ardi 2009
 *
 * gui_sound_plugin_aif.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include <limits.h>
#include <asndlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "gui_sound_decoder.h"

// ------
// Copyright (C) 1988-1991 Apple Computer, Inc.
#ifndef HUGE_VAL
# define HUGE_VAL HUGE
#endif

# define UnsignedToFloat(u)         (((double)((long)(u - 2147483647L - 1))) + 2147483648.0)

static double ConvertFromIeeeExtended(const u8* bytes)
{
	double f;
	int expon;
	u32 hiMant, loMant;

	expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
	hiMant	=    ((unsigned long)(bytes[2] & 0xFF) << 24)
				|    ((unsigned long)(bytes[3] & 0xFF) << 16)
				|    ((unsigned long)(bytes[4] & 0xFF) << 8)
				|    ((unsigned long)(bytes[5] & 0xFF));
	loMant	=    ((unsigned long)(bytes[6] & 0xFF) << 24)
				|    ((unsigned long)(bytes[7] & 0xFF) << 16)
				|    ((unsigned long)(bytes[8] & 0xFF) << 8)
				|    ((unsigned long)(bytes[9] & 0xFF));

	if (expon == 0 && hiMant == 0 && loMant == 0)
		f = 0;
	else
	{
		if (expon == 0x7FFF) 
			f = HUGE_VAL;
		else
		{
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

class GuiSoundDecoderAIFF : public GuiSoundDecoder
{
protected:
	GuiSoundDecoderAIFF(const u8 * snd, u32 len, bool snd_is_allocated)
	{
		sound			= snd;
		length			=len;
		is_allocated	= snd_is_allocated;
		is_running = false;

		const u8 *in_ptr = sound;

		if(be32inc(in_ptr) != 0x464F524D /*'FORM'*/)							throw("No FORM chunk");
		if(be32inc(in_ptr)+8 != len)												throw("wrong Size");
		if(be32inc(in_ptr) != 0x41494646 /*'AIFF'*/)							throw("No AIFF chunk");

		while(in_ptr+8 < sound+len)
		{ 
			u32 chunk_id				= be32inc(in_ptr);
			u32 chunk_size				= be32inc(in_ptr);
			const u8 *chunk_start	= in_ptr;
			switch(chunk_id)
			{
				case 0x434F4D4D /*'COMM'*/:
					channelCount = be16inc(in_ptr);
					in_ptr += 4; 								// skip numSampleFrames
					bytePerSample = (be16inc(in_ptr)+7)/8;
					if(bytePerSample < 1 && bytePerSample > 2)				throw("wrong bits per Sample");
					sampleRate = ConvertFromIeeeExtended(in_ptr);
					break;
				case 0x53534E44 /*'SSND'*/:
					pcm_start	= in_ptr + 8;	
					pcm_end		= chunk_start+chunk_size;	
					break;
			}
			in_ptr = chunk_start+chunk_size; 
		}
		currentPos = pcm_start;
	}
public:
	~GuiSoundDecoderAIFF()
	{
		while(is_running) usleep(50);
		if(is_allocated) delete [] sound;
	}
	static GuiSoundDecoder *Create(const u8 * snd, u32 len, bool snd_is_allocated)
	{
		if(snd && len>12 && snd[0]=='F' && snd[1]=='O' && snd[2]=='R' && snd[3]=='M'
					&& snd[8]=='A' && snd[9]=='I' && snd[10]=='F' && snd[11]=='F')
			return new GuiSoundDecoderAIFF(snd, len, snd_is_allocated);
		return NULL;
	}
	s32 GetFormat()
	{
		if(bytePerSample == 2)
			return channelCount==2 ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
		else
			return channelCount==2 ? VOICE_STEREO_8BIT : VOICE_MONO_8BIT;
	}
	s32 GetSampleRate()
	{
		return sampleRate;
	}
	/*  Read reads data from stream to buffer
		return:	>0 = readed bytes;
	            0 = EOF;
				<0 = Error;
	*/
	int Read(u8 * buffer, int buffer_size)
	{
		if(currentPos >= pcm_end)
			return 0; // EOF

		is_running = true;
		if(currentPos + buffer_size > pcm_end)
			buffer_size = pcm_end-currentPos;
		memcpy(buffer, currentPos, buffer_size);
		currentPos += buffer_size;
		is_running = false;
		return buffer_size;
	}
	int Rewind()
	{
		while(is_running) usleep(50);
		currentPos = pcm_start;
		return 0;
	}
private:
	const u8	*sound;
	u32			length;
	bool		is_allocated;
	bool		is_running;

	u32			sampleRate;
	u16			channelCount;
	u16			bytePerSample;
	const u8	*pcm_start;
	const u8	*pcm_end;
	const u8	*currentPos;
};

REGISTER_GUI_SOUND_DECODER(GuiSoundDecoderAIFF);
