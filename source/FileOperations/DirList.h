/****************************************************************************
 * DirList Class
 * for WiiXplorer
 ***************************************************************************/
#ifndef ___DIRLIST_H_
#define ___DIRLIST_H_

#include <gctypes.h>

typedef struct
{
    char        *FileName;
    char        *FilePath;
    u64         FileSize;
    bool        isDir;
} FileInfos;

class DirList
{
    public:
        //!Constructor
        //!\param path Path from where to load the filelist of all files
        //!\param filter A fileext that needs to be filtered
        DirList(const char * path, const char *filter = NULL);
        //!Destructor
        ~DirList();
        //! Load all the files from a directory
        //!\param path Path where to check for homebrew files
        bool LoadPath(const char * path, const char *filter = NULL);
        //! Get a filename of the list
        //!\param list index
        char * GetFilename(int index);
        //! Get the a filepath of the list
        //!\param list index
        char * GetFilepath(int index);
        //! Get the a filesize of the list
        //!\param list index
        unsigned int GetFilesize(int index);
        //! Is index a dir or a file
        //!\param list index
        bool IsDir(int index);
        //! Get the filecount of the whole list
        int GetFilecount() { return filecount; };
        //! Sort list by filepath
        void SortList();
		//! Get the index of the specified filename
		int GetFileIndex(const char *filename);
    protected:
        //!Add a list entrie
        bool AddEntrie(const char * folderpath, const char * filename, u64 filesize, bool isDir);
        //! Clear the list
        void ClearList();

        int filecount;
        FileInfos *FileInfo;
};

#endif
