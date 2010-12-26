/***************************************************************************
 * Copyright (C) 2010
 * by dude, Dimok
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
#include <string.h>

//! No need for high security crap. It's a simple encrypter/decrypter
//! with a constant sid.
const char * sid = "USBLoaderGX";

void EncryptString(const char *src, char *dst)
{
	unsigned int i;
	char tmp[3];
	dst[0] = 0;

	for (i = 0; i < strlen(src); i++)
	{
		sprintf(tmp, "%02x", src[i] ^ sid[i%10]);
		strcat(dst, tmp);
	}
}

void DecryptString(const char *src, char *dst)
{
	unsigned int i;
	for (i = 0; i < strlen(src); i += 2)
	{
		char c = (src[i] >= 'a' ? (src[i] - 'a') + 10 : (src[i] - '0')) << 4;
		c += (src[i+1] >= 'a' ? (src[i+1] - 'a') + 10 : (src[i+1] - '0'));
		dst[i>>1] = c ^ sid[(i>>1)%10];
	}
	dst[strlen(src)>>1] = 0;
}

