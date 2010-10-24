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
#include <sys/dir.h>

#include "utils/StringTools.h"
#include "DirList.h"

DirList::DirList(const char * path, const char *filter)
{
    filecount = 0;
    FileInfo = NULL;

    this->LoadPath(path, filter);
    this->SortList();
}

DirList::~DirList()
{
    ClearList();
}

bool DirList::LoadPath(const char * folder, const char *filter)
{
    struct stat st;
    DIR_ITER *dir = NULL;
    char filename[1024];

	char folderpath[strlen(folder)+2];
	sprintf(folderpath, "%s", folder);

	if(folderpath[strlen(folderpath)-1] == '/')
        folderpath[strlen(folderpath)-1] = '\0';

	char * notRoot = strrchr(folderpath, '/');
	if(!notRoot)
	{
	    strcat(folderpath, "/");
	}

    dir = diropen(folderpath);
    if (dir == NULL)
        return false;

    while (dirnext(dir,filename,&st) == 0)
    {
        if (strcmp(filename,".") != 0 && strcmp(filename,"..") != 0)
        {
            if(filter)
            {
				char * fileext = strrchr(filename, '.');
                if(fileext)
                {
                    if(strtokcmp(fileext, filter, ",") == 0)
                    {
                        bool result = AddEntrie(folderpath, filename, st.st_size, (st.st_mode & S_IFDIR) ? true : false);
                        if(!result)
                        {
                            dirclose(dir);
                            return false;
                        }
                    }
                }
            }
            else
            {
                bool result = AddEntrie(folderpath, filename, st.st_size, (st.st_mode & S_IFDIR) ? true : false);
                if(!result)
                {
                    dirclose(dir);
                    return false;
                }
            }
        }
    }
    dirclose(dir);

    return true;
}

bool DirList::AddEntrie(const char * folderpath, const char * filename, u64 filesize, bool isDir)
{
    if(!FileInfo)
    {
        FileInfo = (FileInfos *) malloc(sizeof(FileInfos));
        if (!FileInfo)
            return false;
    }

    FileInfos *TempFileInfo = (FileInfos *) realloc(FileInfo, (filecount+1)*sizeof(FileInfos));

    if (!TempFileInfo)
    {
        ClearList();
        filecount = 0;
        return false;
    }

    FileInfo = TempFileInfo;

    memset(&(FileInfo[filecount]), 0, sizeof(FileInfo));

    FileInfo[filecount].FilePath = strdup(folderpath);
    FileInfo[filecount].FileName = strdup(filename);
    FileInfo[filecount].FileSize = filesize;
    FileInfo[filecount].isDir = isDir;

    if (!FileInfo[filecount].FilePath || !FileInfo[filecount].FileName)
    {
        ClearList();
        filecount = 0;
        return false;
    }

    filecount++;

	return true;
}

void DirList::ClearList()
{
    for(int i = 0; i < filecount; i++)
    {
        if(FileInfo[i].FilePath)
        {
            free(FileInfo[i].FilePath);
            FileInfo[i].FilePath = NULL;
        }
        if(FileInfo[i].FileName)
        {
            free(FileInfo[i].FileName);
            FileInfo[i].FileName = NULL;
        }
    }

    if (FileInfo)
    {
        free(FileInfo);
        FileInfo = NULL;
    }
}

char *DirList::GetFilename(int ind)
{
    if (ind >= filecount || ind < 0)
        return NULL;
    else
        return FileInfo[ind].FileName;
}

char *DirList::GetFilepath(int ind)
{
    if (ind >= filecount || ind < 0)
        return NULL;
    else
        return FileInfo[ind].FilePath;
}

unsigned int DirList::GetFilesize(int ind)
{
    if (ind >= filecount || !filecount || !FileInfo)
        return 0;
    else
        return FileInfo[ind].FileSize;
}

bool DirList::IsDir(int ind)
{
    if (ind >= filecount || !filecount || !FileInfo)
        return false;
    else
        return FileInfo[ind].isDir;
}

static int ListCompare(const void *a, const void *b)
{
    FileInfos *ab = (FileInfos*) a;
    FileInfos *bb = (FileInfos*) b;

    return stricmp((char *) ab->FileName, (char *) bb->FileName);
}

void DirList::SortList()
{
    if(!FileInfo)
        return;

    qsort(FileInfo, filecount, sizeof(FileInfos), ListCompare);
}

int DirList::GetFileIndex(const char *filename)
{
    if(!filename || !FileInfo)
        return -1;

	for (int i = 0; i < filecount; i++)
	{
		if (strcasecmp(FileInfo[i].FileName, filename) == 0)
		{
			return i;
		}
	}
	return -1;
}
