#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <fat.h>
#include <sys/dir.h>
#include <dirent.h>
#include <unistd.h>

#include "listfiles.h"


static char alldirfiles[300][70];
char filenames[80];

bool findfile(const char * filename, const char * path)
{
DIR *dir;
struct dirent *file;

dir = opendir(path);

char temp[11];
while ((file = readdir(dir)))
{
	snprintf(temp,sizeof(temp),"%s",file->d_name);
    if (!strncmpi(temp,filename,11))
		{
		closedir(dir);
		return true;
		}
	}
  closedir(dir);
  return false;
}

bool subfoldercreate(char * fullpath) {
        //check forsubfolders
        char dircheck[300];
        char dirnoslash[300];
		char * pch = NULL;
		u32 cnt = 0;
        struct stat st;

        snprintf(dirnoslash, strlen(fullpath), "%s", fullpath);

        if(stat(fullpath, &st) != 0) {
        pch = strrchr(dirnoslash, '/');
        cnt = pch-dirnoslash;
        snprintf(dircheck, cnt+2, "%s", dirnoslash);
        subfoldercreate(dircheck);
        };

        if (mkdir(dirnoslash, 0777) == -1) {
        return false;
        }

return true;
}

char * GetFileName(int i)
{
    return alldirfiles[i];
}

s32 filenamescmp(const void *a, const void *b)
{
	/* Compare strings */
	return stricmp((char *)a, (char *)b);
}

int GetAllDirFiles(char * filespath)
{
	int countfiles = 0;

	struct stat st;
	DIR_ITER* dir;
	dir = diropen (filespath);

	if (dir == NULL) //If empty
       return 0;
	while (dirnext(dir,filenames,&st) == 0)
	{
		if ((st.st_mode & S_IFDIR) == 0)
		{
			// st.st_mode & S_IFDIR indicates a directory
			snprintf(alldirfiles[countfiles], 70, "%s", filenames);
			countfiles++;
		}
	}
	dirclose(dir);
	qsort(alldirfiles, countfiles, sizeof(char[70]), filenamescmp);
	return countfiles;
}

bool checkfile(char * path)
{
    FILE * f;
    f = fopen(path,"r");
    if(f) {
    fclose(f);
    return true;
    }
return false;
}






