#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <fat.h>
#include <sys/dir.h>
#include <dirent.h>
#include <unistd.h>

#include "listfiles.h"


char alldirfiles[300][70];
char filename[80];

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
		//WindowPrompt(path, filename,"go" ,0);
		closedir(dir);
		return true;
		}
	}
  closedir(dir);
  return false;
}


s32 filenamescmp(const void *a, const void *b)
{
	/* Compare strings */
	return stricmp((char *)a, (char *)b);
}

int GetFiles(char * filespath)
{

int countfiles = 0;

struct stat st;
DIR_ITER* dir;
dir = diropen (filespath);

if (dir == NULL) //If empty
	{
       return 0;
	}
	else
	{
	while (dirnext(dir,filename,&st) == 0)
		{
		if ((st.st_mode & S_IFDIR) == 0)
			{
			// st.st_mode & S_IFDIR indicates a directory
			snprintf(alldirfiles[countfiles], 70, "%s", filename);
			countfiles++;
			}
		}
	}

	qsort(alldirfiles, countfiles, sizeof(char[70]), filenamescmp);

return countfiles;
}
