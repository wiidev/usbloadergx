/****************************************************************************
 * HomebrewFiles Class
 * for USB Loader GX
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dirent.h>

#include "HomebrewFiles.h"

HomebrewFiles::HomebrewFiles(const char * path)
{
    filecount = 0;

    FileInfo = (FileInfos *) malloc(sizeof(FileInfos));
    if (!FileInfo)
    {
        return;
    }

    memset(&FileInfo[filecount], 0, sizeof(FileInfos));

    this->LoadPath(path);
    this->SortList();
}

HomebrewFiles::~HomebrewFiles()
{
    if (FileInfo)
    {
        free(FileInfo);
        FileInfo = NULL;
    }
}

bool HomebrewFiles::LoadPath(const char * folderpath)
{
    struct stat st;
    DIR *dir = NULL;
	struct dirent *dirent = NULL;
    char filename[1024];

    dir = opendir(folderpath);
    if (dir == NULL)
    {
        return false;
    }

    while ((dirent = readdir(dir)) != 0)
    {
        snprintf(filename, 1024, "%s/%s", folderpath, dirent->d_name);

        if(stat(filename, &st) != 0)
            continue;

        snprintf(filename, 1024, dirent->d_name);
		
        if ((st.st_mode & S_IFDIR) != 0)
        {
            if (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0)
            {
                char currentname[200];
                snprintf(currentname, sizeof(currentname), "%s%s/", folderpath, filename);
                this->LoadPath(currentname);
            }
        }
        else
        {
            char temp[5];
            for (int i = 0; i < 5; i++)
            {
                temp[i] = filename[strlen(filename) - 4 + i];
            }

            if ((strncasecmp(temp, ".dol", 4) == 0 || strncasecmp(temp, ".elf", 4) == 0) && filecount < MAXHOMEBREWS
                    && filename[0] != '.')
            {

                FileInfo = (FileInfos *) realloc(FileInfo, (filecount + 1) * sizeof(FileInfos));

                if (!FileInfo)
                {
                    free(FileInfo);
                    FileInfo = NULL;
                    filecount = 0;
                    closedir(dir);
                    return false;
                }

                memset(&(FileInfo[filecount]), 0, sizeof(FileInfo));

                strlcpy(FileInfo[filecount].FilePath, folderpath, sizeof(FileInfo[filecount].FilePath));
                strlcpy(FileInfo[filecount].FileName, filename, sizeof(FileInfo[filecount].FileName));
                FileInfo[filecount].FileSize = st.st_size;
                filecount++;
            }
        }
    }
    closedir(dir);

    return true;
}

char * HomebrewFiles::GetFilename(int ind)
{
    if (ind > filecount)
        return NULL;
    else return FileInfo[ind].FileName;
}

char * HomebrewFiles::GetFilepath(int ind)
{
    if (ind > filecount)
        return NULL;
    else return FileInfo[ind].FilePath;
}

unsigned int HomebrewFiles::GetFilesize(int ind)
{
    if (ind > filecount || !filecount || !FileInfo)
        return 0;
    else return FileInfo[ind].FileSize;
}

static int ListCompare(const void *a, const void *b)
{
    FileInfos *ab = (FileInfos*) a;
    FileInfos *bb = (FileInfos*) b;

    return stricmp((char *) ab->FilePath, (char *) bb->FilePath);
}
void HomebrewFiles::SortList()
{
    qsort(FileInfo, filecount, sizeof(FileInfos), ListCompare);
}
