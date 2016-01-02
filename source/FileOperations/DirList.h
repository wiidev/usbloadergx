/****************************************************************************
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
 * DirList Class
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef ___DIRLIST_H_
#define ___DIRLIST_H_

#include <vector>
#include <string>
#include <gctypes.h>

typedef struct
{
	char * FilePath;
	u64 FileSize;
	bool isDir;
} FileInfos;

class DirList
{
	public:
		//!Constructor
		DirList() {}
		//!Overload
		//!\param path Path from where to load the filelist of all files
		//!\param filter A fileext that needs to be filtered
		//!\param flags search/filter flags from the enum
		DirList(const char * path, const char *filter = NULL, u32 flags = Files | Dirs);
		//!Destructor
		~DirList();
		//! Load all the files from a directory
		bool LoadPath(const char * path, const char *filter = NULL, u32 flags = Files | Dirs);
		bool LoadPath(std::string &path, const char *filter = NULL, u32 flags = Files | Dirs);
		//! Get a filename of the list
		//!\param list index
		const char * GetFilename(int index);
		//! Get the a filepath of the list
		//!\param list index
		const char * GetFilepath(int index) { if(!valid(index)) return NULL; return FileInfo[index].FilePath; }
		//! Get the a filesize of the list
		//!\param list index
		u64 GetFilesize(int index) { if(!valid(index)) return 0; return FileInfo[index].FileSize; }
		//! Is index a dir or a file
		//!\param list index
		bool IsDir(int index) { if(!valid(index)) return 0; return FileInfo[index].isDir; }
		//! Erase an entry of the list
		//!\param list index
		void RemoveEntrie(int index) { if(!valid(index)) return; FileInfo.erase(FileInfo.begin()+index); }
		//! Get the filecount of the whole list
		int GetFilecount() { return FileInfo.size(); }
		//! Sort list by filepath
		void SortList();
		//! Custom sort command for custom sort functions definitions
		void SortList(bool (*SortFunc)(const FileInfos &a, const FileInfos &b));
		//! Get the index of the specified filename
		int GetFileIndex(const char *filename);
		//! Enum for search/filter flags
		enum
		{
			Files = 0x01,
			Dirs = 0x02,
			CheckSubfolders = 0x08
		};
	protected:
		//!Add a list entrie
		void AddEntrie(const char * folderpath, const char * filename, u64 filesize, bool isDir);
		//! Clear the list
		void ClearList();
		//! Check if valid pos is requested
		inline bool valid(int pos) { return (pos >= 0 && pos < (int) FileInfo.size()); }

		std::vector<FileInfos> FileInfo;
};

#endif
