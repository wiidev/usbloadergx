/****************************************************************************
 * HomebrewFiles Class
 * for USB Loader GX
 ***************************************************************************/
#ifndef ___HOMEBREWFILES_H_
#define ___HOMEBREWFILES_H_

#define MAXHOMEBREWS     500

typedef struct {
    char            FileName[100];
    char            FilePath[150];
    unsigned int    FileSize;
} FileInfos;

class HomebrewFiles {
public:
    //!Constructor
    //!\param path Path where to check for homebrew files
    HomebrewFiles(const char * path);
    //!Destructor
    ~HomebrewFiles();
    //! Load the dol/elf list of a path
    //!\param path Path where to check for homebrew files
    bool LoadPath(const char * path);
    //! Get the a filename of the list
    //!\param list index
    char * GetFilename(int index);
    //! Get the a filepath of the list
    //!\param list index
    char * GetFilepath(int index);
    //! Get the a filesize of the list
    //!\param list index
    unsigned int GetFilesize(int index);
    //! Get the filecount of the whole list
    int GetFilecount();
    //! Sort list by filepath
    void SortList();
protected:
    int filecount;
    FileInfos *FileInfo;
};

#endif
