#ifndef _MP3S_H_
#define _MP3S_H_

#ifdef __cplusplus
extern "C"
{
#endif

char mp3files[500][80];

void StopMp3();
void SetMp3Volume(u32 vol);
int PlayMp3(char * path);
int OpenMp3(char * path);
void CloseMp3();
int GetFiles(char * mp3path);

#ifdef __cplusplus
}
#endif

#endif
