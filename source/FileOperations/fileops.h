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
 * fileops.h
 * File operations for the WiiXplorer
 * Handling all the needed file operations
 ***************************************************************************/
#ifndef _FILEOPS_H_
#define _FILEOPS_H_

#include <gctypes.h>

#ifdef __cplusplus
extern "C" {
#endif

bool CreateSubfolder(const char * fullpath);
bool FindFile(const char * filename, const char * path);
bool SearchFile(const char * searchpath, const char * searched_filename, char * outfilepath);
bool CheckFile(const char * filepath);
u64 FileSize(const char * filepath);
int LoadFileToMem(const char * filepath, u8 **buffer, u64 *size);
int LoadFileToMemWithProgress(const char *progressText, const char *filePath, u8 **buffer, u64 *size);
int CopyFile(const char * src, const char * dest);
int CopyDirectory(const char * src, const char * dest);
int MoveDirectory(char * src, const char * dest);
int MoveFile(const char *srcpath, char *destdir);
int RemoveDirectory(char * dirpath);
bool RenameFile(const char * srcpath, const char * destpath);
bool RemoveFile(const char * filepath);
void GetFolderSize(const char * folderpath, u64 * foldersize, u32 * filenumber);

#ifdef __cplusplus
}
#endif

#endif
