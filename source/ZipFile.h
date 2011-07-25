/***************************************************************************
 * Copyright (C) 2009
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
 * ZipFile.cpp
 *
 * for Wii-FileXplorer 2009
 ***************************************************************************/
#ifndef _ZIPFILE_H_
#define _ZIPFILE_H_

#include <zip/unzip.h>
#include <string>

typedef struct
{
		u64 offset; // ZipFile offset
		u64 length; // uncompressed file length in 64 bits for sizes higher than 4GB
		bool isdir; // 0 - file, 1 - directory
		char filename[256]; // full filename
} FileStructure;

class ZipFile
{
	public:
		//!Constructor
		ZipFile(const char *filepath);
		//!Destructor
		~ZipFile();
		//!Extract all files from a zip file to a directory
		//!\param dest Destination path to where to extract
		bool ExtractAll(const char *dest);
		//!Find a file inside the zip and return if it is existent or not
		bool FindFile(const char *filename);
		//!Only needed a part of a filename to find the real one
		bool FindFilePart(const char *partfilename, std::string &realname);
	protected:
		bool LoadList();
		unzFile File;
		unz_file_info cur_file_info;
		FileStructure *FileList;
};

#endif
