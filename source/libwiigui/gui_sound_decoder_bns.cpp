/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_sound_plugin_bns.cpp
 *
 * by ardi 2009
 *
 * Decoder for Wii bns-sound
 *
 * GUI class definitions
 ***************************************************************************/

#include <asndlib.h>
#include <unistd.h>

#include "gui_sound_decoder.h"
#define BIG_ENDIAN_HOST 1




class chanel_t
{
public:
	void Reset()
	{
		currentPos = startPos;
		hist1 = hist2 = 0;
	}
	int DecodeNextBlock()
	{
		int Offset = 0;
		if(currentPos == loopStart)
		{
			loop_hist1 = hist1;
			loop_hist2 = hist2;
		}
		if(loop && currentPos >= endPos)
		{
			currentPos = loopStart;
			hist1	= loop_hist1;
			hist2	= loop_hist2;
			Offset	= loopOffset;

		}
		
		if(currentPos+8 <= endPos)
		{
			u16 index = (*currentPos >> 4) & 0x07;
			s32 scale = 1 << (*currentPos++ & 0x0F);
			for(int i = 0; i < 14; i+=2) 
			{
				nibbles[i] = ((s8)*currentPos) >> 4;
				nibbles[i+1] = ((s8)((*currentPos++) << 4)) >> 4;
			}
			for(int i = 0; i < 14; ++i) 
			{
				s32 sample = (scale * nibbles[i])<<11;
				sample += coEfficients[index * 2] * hist1;
				sample += coEfficients[index * 2 + 1] * hist2;
				sample += 1024;
				sample = sample >> 11;
				if(sample > 32767)
					sample = 32767;
				else if(sample < -32768)
					sample = -32768;
				pcm[i] = sample;

				hist2 = hist1;
				hist1 = sample;
			}
			return Offset;
		}
		return -1;
	}
	
	const u8*	startPos;
	const u8*	endPos;
	const u8*	currentPos;
	s16 		coEfficients[16];
	s16			nibbles[14];
	s16			pcm[14];
	s16			hist1;
	s16			hist2;
	bool		loop;
	const u8*	loopStart;
	u16			loopOffset;
	s16			loop_hist1;
	s16			loop_hist2;
};

class GuiSoundDecoderBNS : public GuiSoundDecoder
{
protected:
	GuiSoundDecoderBNS(const u8 * snd, u32 len, bool snd_is_allocated)
	{
		sound			= snd;
		is_running		= false;
		is_allocated	= snd_is_allocated;

		const u8 *in_ptr = sound;
		
		/////////////////
		// READ HEADER //
		/////////////////
		if(be32inc(in_ptr) != 0x424E5320 /*'BNS '*/) throw("Not a BNS");

		in_ptr += 4;	// skip 4 byte

		u32 bnssize = be32inc(in_ptr);
		if(bnssize != len) throw("Wrong size");

		in_ptr += 4;		// skip unknown1

		const u8* infoStart	= sound + be32inc(in_ptr);
		in_ptr+=4;						// skip const u8* infoEnd	= infoStart + be32inc(in_ptr);

		channel[0].startPos	= sound + be32inc(in_ptr) + 8;
		channel[0].endPos	= channel[0].startPos + be32inc(in_ptr) - 8;

		///////////////
		// READ INFO //
		///////////////
		in_ptr = infoStart + 8;			// skip 'INFO' and Infosize
		
		in_ptr++;						// skip u8 codeType		= *in_ptr++;

		channel[0].loop = channel[1].loop = be16inc(in_ptr); // u8 loopFlag;
		
		channelCount 	= *in_ptr++;

		in_ptr++; 						// skip unknown byte

		sampleRate = be16inc(in_ptr);

		in_ptr+=2;						// skip unknown word
		
		u32 loopStart = be32inc(in_ptr);
		channel[0].loopStart	= channel[0].startPos + ((loopStart/14)*8);//LoopPos to BlockStart
		channel[1].loopStart	= channel[1].startPos + ((loopStart/14)*8); 
		channel[0].loopOffset	= channel[1].loopOffset = loopStart%14;

		in_ptr+=4;						// skip u32 SampleCount = be32inc(in_ptr);

		in_ptr+=24;						// skip unknown Bytes

		if(channelCount == 2)
		{
			in_ptr+=4;					// skip unknown long
			u32 ChannelSplit = be32inc(in_ptr);

			in_ptr+=8;					// skip 2x unknown long
		
			channel[1].endPos	= channel[0].endPos;
			channel[0].endPos	= channel[1].startPos = channel[0].startPos + ChannelSplit;

			channel[1].loopStart = channel[1].startPos + (channel[0].loopStart - channel[0].startPos);
		}
		for (int a = 0; a < 16; a++)
		{
			channel[0].coEfficients[a] = (s16)be16inc(in_ptr);
		}
		if(channelCount == 2)
		{
			in_ptr+=16;					// skip 16 byte
			for (int a = 0; a < 16; a++)
			{
				channel[1].coEfficients[a] = (s16)be16inc(in_ptr);
			}
		}
		channel[0].Reset();
		channel[1].Reset();
		currentBlockPos = 14;
    }
public:
	~GuiSoundDecoderBNS()
	{
		while(is_running) usleep(50);
		if(is_allocated) delete [] sound;
	}
	static GuiSoundDecoder *Create(const u8 * snd, u32 len, bool snd_is_allocated)
	{
		if(snd && len>4 && snd[0]=='B' && snd[1]=='N' && snd[2]=='S' && snd[3]==' ')
			return new GuiSoundDecoderBNS(snd, len, snd_is_allocated);
		return NULL;
	}
	s32 GetFormat()
	{
		return channelCount==2 ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
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
		is_running = true;
		u8 *write_pos = buffer;
		u8 *write_end = buffer+buffer_size;
		
		for(;;)
		{
			if(currentBlockPos >= 14)
			{
				int Offset = channel[0].DecodeNextBlock();
				if(Offset<0 || (channelCount == 2 && channel[1].DecodeNextBlock()<0) )
				{
					is_running = false;
					return write_pos-buffer;
				}
				currentBlockPos = Offset;
			}
			for(;currentBlockPos < 14; ++currentBlockPos)
			{
				if(write_pos >= write_end)
				{
					is_running = false;
					return write_pos-buffer;
				}
				*((s16*)write_pos) = channel[0].pcm[currentBlockPos]; 
				write_pos+=2;
				if(channelCount==2) // stereo
				{
					*((s16*)write_pos) = channel[1].pcm[currentBlockPos]; 
					write_pos+=2;
				}
			}
		}
		is_running = false;
		return 0;
	}
	int Rewind()
	{
		channel[0].Reset();
		channel[1].Reset();
		currentBlockPos = 14;
		return 0;
	}
private:
	const u8		*sound;
	bool			is_allocated;
	bool			is_running;
	chanel_t 		channel[2];
	u16				currentBlockPos;
	u16				channelCount;
	u32				sampleRate;
//	u16				loopOffset;
//	u16				bytePerSample;
//	const u8		*soundDataStart;
//	const u8		*soundDataEnd;
//	u32				soundDataLen;
};
REGISTER_GUI_SOUND_DECODER(GuiSoundDecoderBNS);
