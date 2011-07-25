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
#include <unistd.h>
#include <malloc.h>
#include "OggDecoder.hpp"

extern "C"  int ogg_read(void * punt, int bytes, int blocks, int *f)
{
	return ((CFile *) f)->read((u8 *) punt, bytes*blocks);
}

extern "C" int ogg_seek(int *f, ogg_int64_t offset, int mode)
{
	return ((CFile *) f)->seek((u64) offset, mode);
}

extern "C" int ogg_close(int *f)
{
	((CFile *) f)->close();
	return 0;
}

extern "C" long ogg_tell(int *f)
{
	return (long) ((CFile *) f)->tell();
}

static ov_callbacks callbacks = {
	(size_t (*)(void *, size_t, size_t, void *))  ogg_read,
	(int (*)(void *, ogg_int64_t, int))		   ogg_seek,
	(int (*)(void *))							 ogg_close,
	(long (*)(void *))							ogg_tell
};

OggDecoder::OggDecoder(const char * filepath)
	: SoundDecoder(filepath)
{
	SoundType = SOUND_OGG;

	if(!file_fd)
		return;

	OpenFile();
}

OggDecoder::OggDecoder(const u8 * snd, int len)
	: SoundDecoder(snd, len)
{
	SoundType = SOUND_OGG;

	if(!file_fd)
		return;

	OpenFile();
}

OggDecoder::~OggDecoder()
{
	ExitRequested = true;
	while(Decoding)
		usleep(100);

	if(file_fd)
		ov_clear(&ogg_file);
}

void OggDecoder::OpenFile()
{
	if (ov_open_callbacks(file_fd, &ogg_file, NULL, 0, callbacks) < 0)
	{
		delete file_fd;
		file_fd = NULL;
		return;
	}

	ogg_info = ov_info(&ogg_file, -1);
	Decode();
}

int OggDecoder::GetFormat()
{
	if(!file_fd)
		return VOICE_STEREO_16BIT;

	return ((ogg_info->channels == 2) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT);
}

int OggDecoder::GetSampleRate()
{
	if(!file_fd)
		return 0;

	return (int) ogg_info->rate;
}

int OggDecoder::Rewind()
{
	if(!file_fd)
		return -1;

	int ret = ov_time_seek(&ogg_file, 0);
	CurPos = 0;
	EndOfFile = false;

	return ret;
}

int OggDecoder::Read(u8 * buffer, int buffer_size, int pos)
{
	if(!file_fd)
		return -1;

	int bitstream = 0;

	int read = ov_read(&ogg_file, (char *) buffer, buffer_size, &bitstream);

	if(read > 0)
		CurPos += read;

	return read;
}
