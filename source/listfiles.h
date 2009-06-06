#ifndef _LISTFILES_H_
#define _LISTFILES_H_

#ifdef __cplusplus
extern "C"
{
#endif

bool findfile(const char * filename, const char * path);
char * GetFileName(int i);
int GetAllDirFiles(char * filespath);


#ifdef __cplusplus
}
#endif

#endif
