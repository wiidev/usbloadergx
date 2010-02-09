/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_sound_plugin_ogg.cpp
 *
 * by ardi 2009
 *
 * Decoder for ogg-vorbis with libtremor
 *
 * GUI class definitions
 ***************************************************************************/

#include <unistd.h>
#include <asndlib.h>
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>

#include "gui_sound_decoder.h"

class GuiSoundDecoderOGG : public GuiSoundDecoder
{
protected:
	GuiSoundDecoderOGG(const u8 * snd, u32 len, bool snd_is_allocated)
	{
		sound			= snd;
		is_allocated	= snd_is_allocated;
		ogg_fd			= mem_open((char *)snd, len);
		if(ogg_fd < 0) throw("mem open failed");

		if (ov_open((FILE*)&ogg_fd, &ogg_file, NULL, 0) < 0)
		{
			mem_close(ogg_fd); 
			throw("ogg open failed");
		}
		ogg_info		= ov_info(&ogg_file, -1);
		bitstream		= 0;
		is_running	= false;
    }
public:
	~GuiSoundDecoderOGG()
	{
		while(is_running) usleep(50);
		ov_clear(&ogg_file);
		if(is_allocated) delete [] sound;
	}
	static GuiSoundDecoder *Create(const u8 * snd, u32 len, bool snd_is_allocated)
	{
		if(snd && len>4 && snd[0]=='O' && snd[1]=='g' && snd[2]=='g' && snd[3]=='S')
			return new GuiSoundDecoderOGG(snd, len, snd_is_allocated);
		return NULL;
	}
	s32 GetFormat()
	{
		return ogg_info->channels==2 ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
	}
	s32 GetSampleRate()
	{
		return ogg_info->rate;
	}
	/*  Read reads data from stream to buffer
		return:	>0 = readed bytes;
	            0 = EOF;
				<0 = Error;
	*/
	int Read(u8 * buffer, int buffer_size)
	{
		is_running = true;
		int ret = ov_read(&ogg_file, (char *)buffer, buffer_size, &bitstream);
		if (ret < 0) 
		{
			/* error in the stream.  Not a problem, just reporting it in
			 case we (the app) cares.  In this case, we don't. */
			if (ret != OV_HOLE)
				ret = 0; // we says EOF
		}
		is_running = false;
		return ret;
	}
	int Rewind()
	{
		return ov_time_seek(&ogg_file, 0);	
	}
private:
	const u8		*sound;
	bool			is_allocated;
	int				ogg_fd;
	OggVorbis_File	ogg_file;
	vorbis_info		*ogg_info;
	int				bitstream;
	bool			is_running;
};
REGISTER_GUI_SOUND_DECODER(GuiSoundDecoderOGG);
