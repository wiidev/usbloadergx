#ifndef _LISTFILES_H_
#define _LISTFILES_H_

#ifdef __cplusplus
extern "C" {
#endif

    bool findfile(const char * filename, const char * path);
    char * GetFileName(int i);
    int GetAllDirFiles(char * filespath);
    bool subfoldercreate(const char * fullpath);
    bool checkfile(char * path);

#ifdef __cplusplus
}
#endif

#endif
