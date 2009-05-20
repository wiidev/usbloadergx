#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <sys/dir.h>
#include <unistd.h>
#include <mp3player.h>

#include "mp3s.h"

char filename[80];

//Files
FILE* pFile;


s32  my_reader(void *fp,void *dat, s32 size)
{
	return fread(dat, 1, size, fp);
}


void StopMp3()
{
    if(MP3Player_IsPlaying()) {
    MP3Player_Stop();
    }
}

void CloseMp3() {

    if(MP3Player_IsPlaying()) {
    MP3Player_Stop();
    }
    fclose(pFile);
}

void SetMp3Volume(u32 vol)
{
    MP3Player_Volume(vol);
}

int PlayMp3(char * path) {

    if(!MP3Player_IsPlaying()) {
        MP3Player_Init();
    } else {
        StopMp3();
    }

    pFile = fopen (path, "r");
    MP3Player_PlayFile(pFile, my_reader, NULL);
    return 1;
}

/*
int PlayMp3(char * path)
{


    if(!MP3Player_IsPlaying()) {
        MP3Player_Init();
    } else {
        MP3Player_Stop();
    }
    long lSize = 0;
    char * buffermp3 = NULL;
    size_t resultmp3;

    pFile = fopen (path, "r");

    //Check that pFile exist
    if (pFile!=NULL)
    {
				// obtain file size:
    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);

    // allocate memory to contain the whole file:
    buffermp3 = (char*) malloc (sizeof(char)*lSize);
    if (buffermp3 == NULL) {fputs ("   Memory error",stderr); exit (2);}

    // copy the file into the buffer:
    resultmp3 = fread (buffermp3,1,lSize,pFile);
    if (resultmp3 != lSize) {fputs ("   Reading error",stderr); exit (3);}


	fclose (pFile);
    }

    if (buffermp3 != NULL)
    {
        MP3Player_PlayBuffer(buffermp3,lSize,NULL);
        return 1;
    }

return 0;
}
*/

s32 filenamescmp(const void *a, const void *b)
{
	/* Compare strings */
	return stricmp((char *)a, (char *)b);
}

int GetFiles(char * mp3path)
{
int countmp3 = 0;

struct stat st;
DIR_ITER* dir;
dir = diropen (mp3path);

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
			sprintf(mp3files[countmp3], "%s", filename);
			countmp3++;
			}
		}
	}

	qsort(mp3files, countmp3, sizeof(char[70]), filenamescmp);

return countmp3;
}
