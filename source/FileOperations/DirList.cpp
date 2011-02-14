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
    if(!folder)
        return false;

    struct stat st;
    struct dirent *dirent = NULL;
    DIR *dir = NULL;
	std::string folderpath = folder;

	if(folderpath[folderpath.size()-1] == '/')
        folderpath.erase(folderpath.size()-1, 1);

	const char * notRoot = strchr(folderpath.c_str(), '/');
	if(!notRoot)
	    folderpath += '/';

    dir = opendir(folderpath.c_str());
    if (dir == NULL)
        return false;

    char * filename = new (std::nothrow) char[MAXPATHLEN];
    if(!filename)
    {
        closedir(dir);
        return false;
    }

    while ((dirent = readdir(dir)) != 0)
    {
        snprintf(filename, MAXPATHLEN, "%s/%s", folderpath.c_str(), dirent->d_name);

        if(stat(filename, &st) != 0)
            continue;

        snprintf(filename, MAXPATHLEN, dirent->d_name);

        if(st.st_mode & S_IFDIR)
        {
            if(!(flags & Dirs))
                continue;

            if(strcmp(filename,".") == 0 || strcmp(filename,"..") == 0)
                continue;

            if(flags & CheckSubfolders)
            {
                std::string newFolder = folderpath;
                if(notRoot) newFolder += '/';
                newFolder += filename;
                LoadPath(newFolder.c_str(), filter, flags);
            }
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
                AddEntrie(folderpath.c_str(), filename, st.st_size, (st.st_mode & S_IFDIR) ? true : false);
        }
        else
        {
            AddEntrie(folderpath.c_str(), filename, st.st_size, (st.st_mode & S_IFDIR) ? true : false);
        }
    }
    closedir(dir);
    delete [] filename;

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

    const char * Filename1 = FullpathToFilename(f1.FilePath);
    const char * Filename2 = FullpathToFilename(f2.FilePath);

	if(Filename1 && !Filename2) return true;
	if(!Filename1 && Filename2) return false;

    if(strcasecmp(Filename1, Filename2) > 0)
        return false;

    return true;
}

void DirList::SortList()
{
    std::sort(FileInfo.begin(), FileInfo.end(), SortCallback);
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
