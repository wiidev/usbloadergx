/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * modified by dimok and ardi
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
#define ROOTDIRLEN 10

typedef struct
{
		u64 offset; // DVD offset
		u64 length; // file length in 64 bytes for sizes higher than 4GB
		char isdir; // 0 - file, 1 - directory
		char filename[MAXJOLIET + 1]; // full filename
		char displayname[MAXDISPLAY + 1]; // name for browser display
} BROWSERENTRY;

typedef struct
{
		char dir[MAXPATHLEN]; // directory path of browserList
		char rootdir[ROOTDIRLEN];// directory path of browserList
		int pageIndex; // starting index of browserList page display
		std::vector<BROWSERENTRY> browserList;
} BROWSERINFO;
extern BROWSERINFO *browser;

#define FB_NOFOLDER_BTN	 0x0001
#define FB_NODEVICE_BTN	 0x0002
#define FB_TRYROOTDIR	   0x0004
#define FB_TRYSTDDEV		0x0008
#define FB_DEFAULT		  (FB_TRYROOTDIR | FB_TRYSTDDEV)

typedef int (*FILEFILTERCALLBACK)(BROWSERENTRY *Entry, void* Args);
int noDIRS(BROWSERENTRY *Entry, void* Args);
int noFILES(BROWSERENTRY *Entry, void* Args);
int noEXT(BROWSERENTRY *Entry, void* Args);

typedef struct _FILTERCASCADE
{
		FILEFILTERCALLBACK filter;
		void *filter_args;
		_FILTERCASCADE *next;
} FILTERCASCADE;

/****************************************************************************
 * BrowseDevice
 * Displays a list of files on the selected path
 * Path returns the selectet Path/File
 * Path_size is the space of the Path-array
 * Ret: 0 ok / -1 Error
 ***************************************************************************/
/***************************************************************************
 *	  Example:
 * FILTERKASKADE filter2 = {noEXT, NULL, NULL};
 * FILTERKASKADE filter1 = {noDirs, NULL, &filter2};
 * char Path[MAXPATHLEN] = "SD:/";
 * BrowseDevice(Path, MAXPATHLEN, FB_DEFAULT, &filter1);
 *
 *
 ***************************************************************************/
int BrowseDevice(char * Path, int Path_size, int Flags/*=FB_DEFAULT*/, FILTERCASCADE *Filter = NULL);
int BrowseDevice(char * Path, int Path_size, int Flags, FILEFILTERCALLBACK Filter, void *FilterArgs = NULL);

#endif

