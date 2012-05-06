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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <sys/dirent.h>

#define MAXPATHLEN  1024

#include "utils/StringTools.h"
#include "DirList.h"

DirList::DirList(const char * path, const char *filter, u32 flags)
{
	this->LoadPath(path, filter, flags);
	this->SortList();
}

DirList::~DirList()
{
	ClearList();
}

bool DirList::LoadPath(const char * folder, const char *filter, u32 flags)
{
	if(!folder) return false;

	std::string folderpath(folder);

	return LoadPath(folderpath, filter, flags);
}

bool DirList::LoadPath(std::string &folderpath, const char *filter, u32 flags)
{
	if(folderpath.size() < 3)
		return false;

	struct dirent *dirent = NULL;
	DIR *dir = NULL;

	if(folderpath[folderpath.size()-1] == '/')
		folderpath.erase(folderpath.size()-1);

	bool isRoot = (folderpath.find('/') == std::string::npos);
	if(isRoot)
		folderpath += '/';

	dir = opendir(folderpath.c_str());
	if (dir == NULL)
		return false;

	char * filename = new (std::nothrow) char[MAXPATHLEN];
	struct stat *st = new (std::nothrow) struct stat;
	if(!filename || !st)
	{
		delete [] filename;
		delete st;
		closedir(dir);
		return false;
	}

	while ((dirent = readdir(dir)) != 0)
	{
		if(!dirent->d_name)
			continue;

		snprintf(filename, MAXPATHLEN, "%s/%s", folderpath.c_str(), dirent->d_name);

		if(stat(filename, st) != 0)
			continue;

		snprintf(filename, MAXPATHLEN, dirent->d_name);

		if(st->st_mode & S_IFDIR)
		{
			if(strcmp(filename,".") == 0 || strcmp(filename,"..") == 0)
				continue;

			if(flags & CheckSubfolders)
			{
				int length = folderpath.size();
				if(!isRoot) folderpath += '/';
				folderpath += filename;
				LoadPath(folderpath, filter, flags);
				folderpath.erase(length);
			}

			if(!(flags & Dirs))
				continue;
		}
		else
		{
			if(!(flags & Files))
				continue;
		}

		if(filter)
		{
			char * fileext = strrchr(filename, '.');
			if(!fileext)
				continue;

			if(strtokcmp(fileext, filter, ",") == 0)
				AddEntrie(folderpath.c_str(), filename, st->st_size, (st->st_mode & S_IFDIR) ? true : false);
		}
		else
		{
			AddEntrie(folderpath.c_str(), filename, st->st_size, (st->st_mode & S_IFDIR) ? true : false);
		}
	}
	closedir(dir);
	delete [] filename;
	delete st;

	return true;
}

void DirList::AddEntrie(const char * folderpath, const char * filename, u64 filesize, bool isDir)
{
	if(!folderpath || !filename)
		return;

	int pos = FileInfo.size();

	FileInfo.resize(pos+1);

	FileInfo[pos].FilePath = new (std::nothrow) char[strlen(folderpath)+strlen(filename)+2];
	if(FileInfo[pos].FilePath)
		sprintf(FileInfo[pos].FilePath, "%s/%s", folderpath, filename);
	FileInfo[pos].FileSize = filesize;
	FileInfo[pos].isDir = isDir;
}

void DirList::ClearList()
{
	for(u32 i = 0; i < FileInfo.size(); ++i)
	{
		if(FileInfo[i].FilePath)
			delete [] FileInfo[i].FilePath;
	}

	FileInfo.clear();
	std::vector<FileInfos>().swap(FileInfo);
}

const char * DirList::GetFilename(int ind)
{
	if (!valid(ind))
		return NULL;

	return FullpathToFilename(FileInfo[ind].FilePath);
}

static bool SortCallback(const FileInfos & f1, const FileInfos & f2)
{
	if(f1.isDir && !(f2.isDir)) return true;
	if(!(f1.isDir) && f2.isDir) return false;

	if(f1.FilePath && !f2.FilePath) return true;
	if(!f1.FilePath) return false;

	if(strcasecmp(f1.FilePath, f2.FilePath) > 0)
		return false;

	return true;
}

void DirList::SortList()
{
	if(FileInfo.size() > 1)
		std::sort(FileInfo.begin(), FileInfo.end(), SortCallback);
}

void DirList::SortList(bool (*SortFunc)(const FileInfos &a, const FileInfos &b))
{
	if(FileInfo.size() > 1)
		std::sort(FileInfo.begin(), FileInfo.end(), SortFunc);
}

int DirList::GetFileIndex(const char *filename)
{
	if(!filename)
		return -1;

	for (u32 i = 0; i < FileInfo.size(); ++i)
	{
		if (strcasecmp(GetFilename(i), filename) == 0)
			return i;
	}

	return -1;
}
