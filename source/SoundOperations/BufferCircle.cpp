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
#include "BufferCircle.hpp"

#define ALIGN32(x) (((x) + 31) & ~31)

BufferCircle::BufferCircle()
{
	which = 0;
	BufferBlockSize = 0;
}

BufferCircle::~BufferCircle()
{
	FreeBuffer();
	SoundBuffer.clear();
	BufferSize.clear();
	BufferReady.clear();
}

void BufferCircle::SetBufferBlockSize(int size)
{
	if(size < 0)
		return;

	BufferBlockSize = size;

	for(int i = 0; i < Size(); i++)
	{
		if(SoundBuffer[i] != NULL)
			free(SoundBuffer[i]);

		SoundBuffer[i] = (u8 *) memalign(32, ALIGN32(BufferBlockSize));
		BufferSize[i] = 0;
		BufferReady[i] = false;
	}
}

void BufferCircle::Resize(int size)
{
	while(size < Size())
		RemoveBuffer(Size()-1);

	int oldSize = Size();

	SoundBuffer.resize(size);
	BufferSize.resize(size);
	BufferReady.resize(size);

	for(int i = oldSize; i < Size(); i++)
	{
		if(BufferBlockSize > 0)
			SoundBuffer[i] = (u8 *) memalign(32, ALIGN32(BufferBlockSize));
		else
			SoundBuffer[i] = NULL;
		BufferSize[i] = 0;
		BufferReady[i] = false;
	}
}

void BufferCircle::RemoveBuffer(int pos)
{
	if(!Valid(pos))
		return;

	if(SoundBuffer[pos] != NULL)
		free(SoundBuffer[pos]);

	SoundBuffer.erase(SoundBuffer.begin()+pos);
	BufferSize.erase(BufferSize.begin()+pos);
	BufferReady.erase(BufferReady.begin()+pos);
}

void BufferCircle::ClearBuffer()
{
	for(int i = 0; i < Size(); i++)
	{
		BufferSize[i] = 0;
		BufferReady[i] = false;
	}
	which = 0;
}

void BufferCircle::FreeBuffer()
{
	for(int i = 0; i < Size(); i++)
	{
		if(SoundBuffer[i] != NULL)
			free(SoundBuffer[i]);

		SoundBuffer[i] = NULL;
		BufferSize[i] = 0;
		BufferReady[i] = false;
	}
}

void BufferCircle::LoadNext()
{
	int pos = (which+Size()-1) % Size();
	BufferReady[pos] = false;
	BufferSize[pos] = 0;

	which = (which+1) % Size();
}

void BufferCircle::SetBufferReady(int pos, bool state)
{
	if(!Valid(pos))
		return;

	BufferReady[pos] = state;
}

void BufferCircle::SetBufferSize(int pos, int size)
{
	if(!Valid(pos))
		return;

	BufferSize[pos] = size;
}
