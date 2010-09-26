#ifndef RECOURCES_H_
#define RECOURCES_H_

#include "libwiigui/gui_imagedata.h"
#include "filelist.h"

typedef struct _RecourceFile
{
    const char *filename;
    const u8   *DefaultFile;
    const u32   DefaultFileSize;
    u8         *CustomFile;
    u32         CustomFileSize;
} RecourceFile;

class Resources
{
    public:
        static void Clear();
        static void LoadFiles(const char * path);
        static const u8 * GetFile(const char * filename);
        static const u32 GetFileSize(const char * filename);
        static GuiImageData * GetImageData(const char * filename);

    private:
        static RecourceFile RecourceFiles[];
};

#endif
