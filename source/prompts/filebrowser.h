/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * modified by dimok
 *
 * filebrowser.h
 *
 * Generic file routines - reading, writing, browsing
 ****************************************************************************/

#ifndef _FILEBROWSER_H_
#define _FILEBROWSER_H_

#include <unistd.h>
#include <gccore.h>

#define MAXJOLIET 255
#define MAXDISPLAY MAXPATHLEN


enum {
    SD,
    USB
};

typedef struct {
    char dir[MAXPATHLEN]; // directory path of browserList
    char rootdir[10]; // directory path of browserList
    int numEntries; // # of entries in browserList
    int selIndex; // currently selected index of browserList
    int pageIndex; // starting index of browserList page display
} BROWSERINFO;

typedef struct {
    u64 offset; // DVD offset
    u64 length; // file length in 64 bytes for sizes higher than 4GB
    char isdir; // 0 - file, 1 - directory
    char filename[MAXJOLIET + 1]; // full filename
    char displayname[MAXDISPLAY + 1]; // name for browser display
} BROWSERENTRY;

extern BROWSERINFO browser;
extern BROWSERENTRY * browserList;

int UpdateDirName();
int FileSortCallback(const void *f1, const void *f2);
void ResetBrowser();
int ParseDirectory();
int BrowserChangeFolder();
int BrowseDevice(int device);
int BrowseDevice(char * var, int force =-1);

#endif
