/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * 3Band resampling thanks to libmad
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
#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "SoundDecoder.hpp"
#include "main.h"

SoundDecoder::SoundDecoder()
{
	file_fd = NULL;
	Init();
}

SoundDecoder::SoundDecoder(const char * filepath)
{
	file_fd = new CFile(filepath, "rb");
	Init();
}

SoundDecoder::SoundDecoder(const u8 * buffer, int size)
{
	file_fd = new CFile(buffer, size);
	Init();
}

SoundDecoder::~SoundDecoder()
{
	ExitRequested = true;
	while(Decoding)
		usleep(100);

	if(file_fd)
		delete file_fd;
	file_fd = NULL;
}

void SoundDecoder::Init()
{
	SoundType = SOUND_RAW;
	SoundBlocks = 8;
	SoundBlockSize = 8192;
	CurPos = 0;
	Loop = false;
	EndOfFile = false;
	Decoding = false;
	ExitRequested = false;
	SoundBuffer.SetBufferBlockSize(SoundBlockSize);
	SoundBuffer.Resize(SoundBlocks);
}

int SoundDecoder::Rewind()
{
	CurPos = 0;
	EndOfFile = false;
	file_fd->rewind();

	return 0;
}

int SoundDecoder::Read(u8 * buffer, int buffer_size, int pos)
{
	int ret = file_fd->read(buffer, buffer_size);
	CurPos += ret;

	return ret;
}

void SoundDecoder::Decode()
{
	if(!file_fd || ExitRequested || EndOfFile)
		return;

	u16 newWhich = SoundBuffer.Which();
	u16 i = 0;
	for (i = 0; i < SoundBuffer.Size()-2; i++)
	{
		if(!SoundBuffer.IsBufferReady(newWhich))
			break;

		newWhich = (newWhich+1) % SoundBuffer.Size();
	}

	if(i == SoundBuffer.Size()-2)
		return;

	Decoding = true;

	int done  = 0;
	u8 * write_buf = SoundBuffer.GetBuffer(newWhich);
	if(!write_buf)
	{
		ExitRequested = true;
		Decoding = false;
		return;
	}

	while(done < SoundBlockSize)
	{
		int ret = Read(&write_buf[done], SoundBlockSize-done, Tell());

		if(ret <= 0)
		{
			if(Loop)
			{
				Rewind();
				continue;
			}
			else
			{
				EndOfFile = true;
				break;
			}
		}

		done += ret;
	}

	if(done > 0)
	{
		SoundBuffer.SetBufferSize(newWhich, done);
		SoundBuffer.SetBufferReady(newWhich, true);
	}

	if(!SoundBuffer.IsBufferReady((newWhich+1) % SoundBuffer.Size()))
		Decode();

	Decoding = false;
}

