#ifndef _LISTFILES_H_
#define _LISTFILES_H_

#ifdef __cplusplus
extern "C"
{
#endif

bool findfile(const char * filename, const char * path);
char * GetFileName(int i);
int GetAllDirFiles(char * filespath);
bool subfoldercreate(char * fullpath);
bool checkfile(char * path);
u64 FileSize(const char * filepath);

#ifdef __cplusplus
}
#endif

#endif
