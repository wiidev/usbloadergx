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
#ifndef ___HTML_STREAM_H_
#define ___HTML_STREAM_H_

#include <gctypes.h>

class HTML_Stream
{
	public:
		//!Constructor
		HTML_Stream();
		//!\param url from where to the HTML file
		HTML_Stream(const char * url);
		//!Destructor
		~HTML_Stream();
		//!Load url
		bool LoadLink(const char * url);
		//! Find start of a string from current position in the html
		//!\param string to find
		const char * FindStringStart(const char * string);
		//! Find end of a string from current position in the html
		//!\param string to find
		const char * FindStringEnd(const char * string);
		//!CopyString from current position in html till stopat string
		//!\param stopat string before which to stop copying (e.g. </html>)
		//!\param outtext variable is allocated with malloc and must be set 0 before
		char * CopyString(const char * stopat);
		//!Seek position in file
		//!\param position seeked
		//!\param seek origin (SEEK_SET, SEEK_CUR, SEEK_END)
		int Seek(u32 pos, int origin);
		//!Rewind to the start of the html
		void Rewind();
		//!Get current position
		int GetPosition();
	protected:
		int position;
		u32 filesize;
		char * HTML_File;
};

#endif
