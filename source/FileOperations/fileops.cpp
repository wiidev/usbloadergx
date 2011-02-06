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
 * fileops.cpp
 * File operations for the WiiXplorer
 * Handling all the needed file operations
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <sys/dir.h>
#include <dirent.h>
#include <unistd.h>
#include <vector>

#include "fileops.h"

#define BLOCKSIZE               70*1024      //70KB
#define VectorResize(List) if(List.capacity()-List.size() == 0) List.reserve(List.size()+100)


static bool actioncanceled  = false;

/****************************************************************************
 * CheckFile
 *
 * Check if file is existing
 ***************************************************************************/
extern "C" bool CheckFile(const char * filepath)
{
    if(!filepath)
        return false;

    struct stat filestat;

    char dirnoslash[strlen(filepath)+2];
    snprintf(dirnoslash, sizeof(dirnoslash), "%s", filepath);

    while(dirnoslash[strlen(dirnoslash)-1] == '/')
        dirnoslash[strlen(dirnoslash)-1] = '\0';

	char * notRoot = strrchr(dirnoslash, '/');
	if(!notRoot)
	{
	    strcat(dirnoslash, "/");
	}

    if (stat(dirnoslash, &filestat) == 0)
        return true;

    return false;
}

/****************************************************************************
 * FileSize
 *
 * Get filesize in bytes. u64 for files bigger than 4GB
 ***************************************************************************/
extern "C" u64 FileSize(const char * filepath)
{
  struct stat filestat;

  if (stat(filepath, &filestat) != 0)
    return 0;

  return filestat.st_size;
}

/****************************************************************************
 * LoadFileToMem
 *
 * Load up the file into a block of memory
 ***************************************************************************/
extern "C" int LoadFileToMem(const char *filepath, u8 **inbuffer, u64 *size)
{
    int ret = -1;
    u64 filesize = FileSize(filepath);
	char * filename = strrchr(filepath, '/');
	if(filename)
        filename++;

    *inbuffer = NULL;
    *size = 0;

    FILE *file = fopen(filepath, "rb");

    if (file == NULL)
        return -1;

    u8 *buffer = (u8 *) malloc(filesize);
    if (buffer == NULL)
    {
        fclose(file);
        return -2;
    }

    u64 done = 0;
    u32 blocksize = BLOCKSIZE;

    do
    {
        if(actioncanceled)
        {
            free(buffer);
            fclose(file);
            return -10;
        }

        if(blocksize > filesize-done)
            blocksize = filesize-done;

        ret = fread(buffer+done, 1, blocksize, file);
        if(ret < 0)
        {
            free(buffer);
            fclose(file);
            return -3;
        }
        else if(ret == 0)
        {
            //we are done
            break;
        }

        done += ret;

    }
    while(done < filesize);

    fclose(file);

    if (done != filesize)
    {
        free(buffer);
        return -3;
    }

    *inbuffer = buffer;
    *size = filesize;

    return 1;
}

/****************************************************************************
 * LoadFileToMemWithProgress
 *
 * Load up the file into a block of memory, while showing a progress dialog
 ***************************************************************************/
extern "C" int LoadFileToMemWithProgress(const char *progressText, const char *filepath, u8 **inbuffer, u64 *size)
{

    int ret = LoadFileToMem(filepath, inbuffer, size);

	return ret;
}

/****************************************************************************
 * CreateSubfolder
 *
 * Create recursive all subfolders to the given path
 ***************************************************************************/
extern "C" bool CreateSubfolder(const char * fullpath)
{
    if(!fullpath)
        return false;

    bool result  = false;

    char dirnoslash[strlen(fullpath)+1];
    strcpy(dirnoslash, fullpath);

    int pos = strlen(dirnoslash)-1;
    while(dirnoslash[pos] == '/')
    {
        dirnoslash[pos] = '\0';
        pos--;
    }

    if(CheckFile(dirnoslash))
    {
        return true;
    }
    else
    {
        char parentpath[strlen(dirnoslash)+2];
        strcpy(parentpath, dirnoslash);
        char * ptr = strrchr(parentpath, '/');

        if(!ptr)
        {
            //!Device root directory (must be with '/')
            strcat(parentpath, "/");
            struct stat filestat;
            if (stat(parentpath, &filestat) == 0)
                return true;

            return false;
        }

        ptr++;
        ptr[0] = '\0';

        result = CreateSubfolder(parentpath);
    }

    if(!result)
        return false;

    if (mkdir(dirnoslash, 0777) == -1)
    {
        return false;
    }

    return true;
}

/****************************************************************************
 * CompareDevices
 *
 * Compare if its the devices are equal
 ***************************************************************************/
static bool CompareDevices(const char *src, const char *dest)
{
    if(!src || !dest)
        return false;

    char *device1 = strchr(src, ':');
    char *device2 = strchr(dest, ':');

	if(!device1 || !device2)
        return false;

	int position1 = device1-src+1;
	int position2 = device2-dest+1;

	char temp1[50];
	char temp2[50];

	snprintf(temp1, position1, "%s", src);
	snprintf(temp2, position2, "%s", dest);

    if(strcasecmp(temp1, temp2) == 0)
        return true;

    return false;
}

/****************************************************************************
 * CopyFile
 *
 * Copy the file from source filepath to destination filepath
 ***************************************************************************/
extern "C" int CopyFile(const char * src, const char * dest)
{
	int read = 1;
	int wrote = 1;

	char * filename = strrchr(src, '/');
	if(filename)
        filename++;
    else
        return -1;

    u64 sizesrc = FileSize(src);

	FILE * source = fopen(src, "rb");

	if(!source)
		return -2;

    u32 blksize = BLOCKSIZE;

	u8 * buffer = (u8 *) malloc(blksize);

	if(buffer == NULL){
	    //no memory
        fclose(source);
		return -1;
	}

	FILE * destination = fopen(dest, "wb");

    if(destination == NULL)
    {
        free(buffer);
        fclose(source);
        return -3;
    }

    u64 done = 0;

    do
    {
        if(actioncanceled)
        {
            fclose(source);
            fclose(destination);
            free(buffer);
            RemoveFile((char *) dest);
            return -10;
        }

        if(blksize > sizesrc - done)
            blksize = sizesrc - done;

        //Display progress
        read = fread(buffer, 1, blksize, source);
        if(read < 0)
        {
            fclose(source);
            fclose(destination);
            free(buffer);
            RemoveFile((char *) dest);
            return -3;
        }

        wrote = fwrite(buffer, 1, read, destination);
        if(wrote < 0)
        {
            fclose(source);
            fclose(destination);
            free(buffer);
            RemoveFile((char *) dest);
            return -3;
        }

        done += wrote;
    }
    while (read > 0);

    free(buffer);
    fclose(source);
    fclose(destination);

    if(sizesrc != done)
        return -4;

	return 1;
}

/****************************************************************************
* ClearList
*
* Clearing a vector list
****************************************************************************/
static inline void ClearList(std::vector<char *> &List)
{
    for(u32 i = 0; i < List.size(); ++i)
    {
        if(List[i])
            free(List[i]);
        List[i] = NULL;
    }
    List.clear();
    std::vector<char *>().swap(List);
}

/****************************************************************************
 * MoveFile
 *
 * Move a file from srcpath to destdir
 ***************************************************************************/
extern "C" int MoveFile(const char *srcpath, char *destdir)
{
    if(CompareDevices(srcpath, destdir))
    {
        if(RenameFile(srcpath, destdir))
            return 1;
        else
            return -1;
    }

    int res = CopyFile(srcpath, destdir);
    if(res < 0)
        return -1;

    if(RemoveFile(srcpath))
        return 1;

    return -1;
}

/****************************************************************************
 * RemoveFile
 *
 * Delete the file from a given filepath
 ***************************************************************************/
extern "C" bool RemoveFile(const char * filepath)
{
    return (remove(filepath) == 0);
}

/****************************************************************************
 * RenameFile
 *
 * Rename the file from a given srcpath to a given destpath
 ***************************************************************************/
extern "C" bool RenameFile(const char * srcpath, const char * destpath)
{
    return (rename(srcpath, destpath) == 0);
}
