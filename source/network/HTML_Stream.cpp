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
 * HTML_Stream Class
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HTML_Stream.h"
#include "networkops.h"
#include "http.h"

#define htmlstringcompare(text, cmp, pos) strncasecmp((const char*) &text[pos], (const char*) cmp, strlen((const char*) cmp))

HTML_Stream::HTML_Stream()
{
	HTML_File = NULL;
	position = 0;
	filesize = 0;
}

HTML_Stream::HTML_Stream(const char * url)
{
	HTML_File = NULL;
	position = 0;
	filesize = 0;

	LoadLink(url);
}

HTML_Stream::~HTML_Stream()
{
	if (HTML_File) free(HTML_File);
}

bool HTML_Stream::LoadLink(const char * url)
{
	if (!IsNetworkInit()) return false;

	struct block file = downloadfile(url);

	if (!file.data || !file.size) return false;

	if (HTML_File) free(HTML_File);

	HTML_File = (char *) file.data;
	filesize = file.size;
	position = 0;

	return true;
}

const char * HTML_Stream::FindStringStart(const char * string)
{
	if (!HTML_File) return NULL;

	while ((u32) position < filesize)
	{
		if (htmlstringcompare( HTML_File, string, position ) == 0) break;

		position++;
	}

	return &HTML_File[position];
}

const char * HTML_Stream::FindStringEnd(const char * string)
{
	if (!HTML_File) return NULL;

	while ((u32) position < filesize)
	{
		if (htmlstringcompare( HTML_File, string, position ) == 0) break;

		position++;
	}

	if ((u32) position >= filesize)
	{
		return NULL;
	}

	position += strlen(string);

	return &HTML_File[position];
}

char * HTML_Stream::CopyString(const char * stopat)
{
	if (!stopat || !HTML_File) return NULL;

	u32 blocksize = 1024;
	u32 counter = 0;

	char * outtext = (char*) malloc(blocksize);
	if (!outtext) return NULL;

	memset(outtext, 0, blocksize);

	while ((htmlstringcompare( HTML_File, stopat, position ) != 0) && (position + strlen(stopat) < filesize))
	{
		if (counter > blocksize)
		{
			blocksize += 1024;
			char * tmpblock = (char*) realloc(outtext, blocksize);
			if (!tmpblock)
			{
				free(outtext);
				outtext = NULL;
				free(tmpblock);
				return NULL;
			}

			outtext = tmpblock;
		}

		outtext[counter] = HTML_File[position];
		position++;
		counter++;
	}

	outtext[counter] = '\0';
	outtext = (char*) realloc(outtext, counter + 1);

	return outtext;
}

int HTML_Stream::Seek(u32 pos, int origin)
{
	if (!HTML_File) return -1;

	switch (origin)
	{
		case SEEK_SET:
			position = pos;
			break;
		case SEEK_CUR:
			position += pos;
			break;
		case SEEK_END:
			position = filesize + pos;
			break;
	}

	return 0;
}

void HTML_Stream::Rewind()
{
	if (!HTML_File) return;

	position = 0;
}

int HTML_Stream::GetPosition()
{
	return position;
}
