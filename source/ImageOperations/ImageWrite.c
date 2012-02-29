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
#include <stdio.h>
#include <gctypes.h>
#include <gd.h>
#include "ImageWrite.h"

#define LIMIT(x, min, max) ( (x < min) ? min : (x > max) ? max : x )

bool WriteGDImage(const char * filepath, gdImagePtr gdImg, u8 format, u8 compression)
{
	if(gdImg == 0)
		return false;

	FILE * file = fopen(filepath, "wb");
	if(!file)
		return false;

	switch(format)
	{
		default:
		case IMAGE_PNG:
			gdImagePng(gdImg, file);
			break;
		case IMAGE_JPEG:
			gdImageJpeg(gdImg, file, LIMIT(100-compression, 0, 100));
			break;
	}

	fclose(file);

	return true;
}
