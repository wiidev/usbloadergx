#ifndef RECOURCES_H_
#define RECOURCES_H_

#include "GUI/gui_imagedata.h"

typedef struct _RecourceFile
{
	const char *filename;
	const u8   *DefaultFile;
	const u32   DefaultFileSize;
	u8		 *CustomFile;
	u32		 CustomFileSize;
} RecourceFile;

class Resources
{
	public:
		static void Clear();
		static bool LoadFiles(const char * path);
		static const u8 * GetFile(const char * filename);
		static u32 GetFileSize(const char * filename);
		static GuiImageData * GetImageData(const char * filename);

	private:
		static RecourceFile RecourceFiles[];
};

#endif
