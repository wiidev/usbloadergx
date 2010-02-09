#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <sys/dir.h>
#include <dirent.h>
#include <unistd.h>

#include "listfiles.h"
#include "libfat/fat.h"


static char alldirfiles[300][70];
char filenames[80];

bool findfile(const char * filename, const char * path) {
    DIR *dir;
    struct dirent *file;

    dir = opendir(path);

    char temp[11];
    while ((file = readdir(dir))) {
        snprintf(temp,sizeof(temp),"%s",file->d_name);
        if (!strncmpi(temp,filename,11)) {
            closedir(dir);
            return true;
        }
    }
    closedir(dir);
    return false;
}

bool subfoldercreate(const char * fullpath) {
    //check forsubfolders
    char dir[300];
    char * pch = NULL;
    u32 len;
    struct stat st;

    strlcpy(dir, fullpath, sizeof(dir));
    len = strlen(dir);
    if (len && len< sizeof(dir)-2 && dir[len-1] != '/') {
        dir[len++] = '/';
        dir[len] = '\0';
    }
    if (stat(dir, &st) != 0) { // fullpath not exist?
        while (len && dir[len-1] == '/')
            dir[--len] = '\0';				// remove all trailing /
        pch = strrchr(dir, '/');
        if (pch == NULL) return false;
        *pch = '\0';
        if (subfoldercreate(dir)) {
            *pch = '/';
            if (mkdir(dir, 0777) == -1)
                return false;
        } else
            return false;
    }
    return true;
}
bool subfolderremove(const char * fullpath, const char*fp) {
    struct stat st;
    if (stat(fullpath, &st) != 0) // fullpath not exist?
        return false;
    if (S_ISDIR(st.st_mode)) {
        DIR_ITER *dir = NULL;
        char filename[256];
        bool cont = true;
        while (cont) {
            cont = false;
            dir = diropen(fullpath);
            if (dir) {
                char* bind = fullpath[strlen(fullpath)-1] == '/' ? "":"/";
                while (dirnext(dir,filename,&st) == 0) {
                    if (strcmp(filename,".") != 0 && strcmp(filename,"..") != 0) {
                        char currentname[256];
                        if (S_ISDIR(st.st_mode))
                            snprintf(currentname, sizeof(currentname), "%s%s%s/", fullpath, bind, filename);
                        else
                            snprintf(currentname, sizeof(currentname), "%s%s%s", fullpath, bind, filename);
                        subfolderremove(currentname, fp);
                        cont = true;
                        break;
                    }
                }
                dirclose(dir);
            }
        }
    }
    return unlink(fullpath) == 0;
}
char * GetFileName(int i) {
    return alldirfiles[i];
}

s32 filenamescmp(const void *a, const void *b) {
    /* Compare strings */
    return stricmp((char *)a, (char *)b);
}

int GetAllDirFiles(char * filespath) {
    int countfiles = 0;

    struct stat st;
    DIR_ITER* dir;
    dir = diropen (filespath);

    if (dir == NULL) //If empty
        return 0;
    while (dirnext(dir,filenames,&st) == 0) {
        if ((st.st_mode & S_IFDIR) == 0) {
            // st.st_mode & S_IFDIR indicates a directory
            snprintf(alldirfiles[countfiles], 70, "%s", filenames);
            countfiles++;
        }
    }
    dirclose(dir);
    qsort(alldirfiles, countfiles, sizeof(char[70]), filenamescmp);
    return countfiles;
}

bool checkfile(char * path) {
    FILE * f;
    f = fopen(path,"r");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

bool SearchFile(const char * searchpath, const char * searched_filename, char * outfilepath) {
    struct stat st;
    DIR_ITER *dir = NULL;
    bool result = false;

    char filename[1024];
    char pathptr[strlen(searchpath)+1];
    snprintf(pathptr, sizeof(pathptr), "%s", searchpath);

    if (pathptr[strlen(pathptr)-1] == '/') {
        pathptr[strlen(pathptr)-1] = '\0';
    }

    dir = diropen(pathptr);
    if (!dir)
        return false;

    while (dirnext(dir,filename,&st) == 0 && result == false) {
        if (strcasecmp(filename, searched_filename) == 0) {
            if (outfilepath) {
                sprintf(outfilepath, "%s/%s", pathptr, filename);
            }
            result = true;
        } else if ((st.st_mode & S_IFDIR) != 0) {
            if (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0) {
                char newpath[1024];
                snprintf(newpath, sizeof(newpath), "%s/%s", pathptr, filename);
                result = SearchFile(newpath, searched_filename, outfilepath);
            }
        }
    }
    dirclose(dir);

    return result;
}
