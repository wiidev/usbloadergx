#include <malloc.h>
#include <string.h>
#include "FileOperations/fileops.h"
#include "Resources.h"
#include "filelist.h"

void Resources::Clear()
{
	for(int i = 0; RecourceFiles[i].filename != NULL; ++i)
	{
		if(RecourceFiles[i].CustomFile)
		{
			free(RecourceFiles[i].CustomFile);
			RecourceFiles[i].CustomFile = NULL;
		}

		if(RecourceFiles[i].CustomFileSize != 0)
			RecourceFiles[i].CustomFileSize = 0;
	}
}

bool Resources::LoadFiles(const char * path)
{
	if(!path)
		return false;

	bool result = false;
	Clear();

	char fullpath[1024];

	for(int i = 0; RecourceFiles[i].filename != NULL; ++i)
	{
		snprintf(fullpath, sizeof(fullpath), "%s/%s", path, RecourceFiles[i].filename);

		if(CheckFile(fullpath))
		{
			u8 * buffer = NULL;
			u32 filesize = 0;

			LoadFileToMem(fullpath, &buffer, &filesize);

			RecourceFiles[i].CustomFile = buffer;
			RecourceFiles[i].CustomFileSize = (u32) filesize;
			result = true;
		}
	}

	return result;
}

const u8 * Resources::GetFile(const char * filename)
{
	for(int i = 0; RecourceFiles[i].filename != NULL; ++i)
	{
		if(strcasecmp(filename, RecourceFiles[i].filename) == 0)
		{
			return (RecourceFiles[i].CustomFile ? RecourceFiles[i].CustomFile : RecourceFiles[i].DefaultFile);
		}
	}

	return NULL;
}

u32 Resources::GetFileSize(const char * filename)
{
	for(int i = 0; RecourceFiles[i].filename != NULL; ++i)
	{
		if(strcasecmp(filename, RecourceFiles[i].filename) == 0)
		{
			return (RecourceFiles[i].CustomFile ? RecourceFiles[i].CustomFileSize : RecourceFiles[i].DefaultFileSize);
		}
	}

	return 0;
}

GuiImageData * Resources::GetImageData(const char * filename)
{
	for(int i = 0; RecourceFiles[i].filename != NULL; ++i)
	{
		if(strcasecmp(filename, RecourceFiles[i].filename) == 0)
		{
			const u8 * buff = RecourceFiles[i].CustomFile ? RecourceFiles[i].CustomFile : RecourceFiles[i].DefaultFile;
			const u32 size = RecourceFiles[i].CustomFile ? RecourceFiles[i].CustomFileSize : RecourceFiles[i].DefaultFileSize;

			if(buff != NULL)
				return (new GuiImageData(buff, size));
			else
				return NULL;
		}
	}

	return NULL;
}
