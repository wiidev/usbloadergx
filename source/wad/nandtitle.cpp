#include <ogc/isfs.h>
#include <stdlib.h>
#include "nandtitle.h"
#include "FileOperations/fileops.h"
#include "prompts/ProgressWindow.h"
#include "language/gettext.h"
#include "usbloader/playlog.h"
#include "utils/tools.h"
#include "gecko.h"

NandTitle NandTitles;

static u8 tmd_buf[MAX_SIGNED_TMD_SIZE] ATTRIBUTE_ALIGN( 32 );

NandTitle::NandTitle()
{
	currentIndex = 0;
	currentType = 0;
}

NandTitle::~NandTitle()
{
	titleIds.clear();
	NameList.clear();
}

s32 NandTitle::Get()
{
	s32 ret;
	u64 *list = NULL;
	u32 numTitles = 0;

	titleIds.clear();
	NameList.clear();

	ret = ES_GetNumTitles(&numTitles);
	if (ret < 0) return WII_EINTERNAL;

	list = (u64*) memalign(32, numTitles * sizeof(u64));
	if (!list)
	{
		return -1;
	}

	ret = ES_GetTitles(list, numTitles);
	if (ret < 0)
	{
		free(list);
		return WII_EINTERNAL;
	}

	for (u32 i = 0; i < numTitles; i++)
	{
		titleIds.push_back(list[i]);
	}

	free(list);

	int language = CONF_GetLanguage();
	ISFS_Initialize();

	wchar_t name[IMET_MAX_NAME_LEN];

	for (u32 i = 0; i < titleIds.size(); i++)
	{
		bool r = GetName(titleIds.at(i), language, name);
		if (r)
		{
			wString wsname(name);
			NameList[titleIds.at(i)] = wsname.toUTF8();
		}
	}

	ISFS_Deinitialize();
	return 1;
}

tmd* NandTitle::GetTMD(u64 tid)
{
	signed_blob *s_tmd = (signed_blob *) tmd_buf;
	u32 tmd_size;

	if (ES_GetStoredTMDSize(tid, &tmd_size) < 0)
	{
		return NULL;
	}

	s32 ret = ES_GetStoredTMD(tid, s_tmd, tmd_size);
	if (ret < 0)
	{
		return NULL;
	}

	tmd *t = (tmd*) SIGNATURE_PAYLOAD(s_tmd);

	return t;
}

bool NandTitle::GetName(u64 tid, int language, wchar_t* name)
{
	if (TITLE_UPPER( tid ) != 0x10001 && TITLE_UPPER( tid ) != 0x10002 && TITLE_UPPER( tid ) != 0x10004) return false;
	//gprintf("GetName( %016llx ): ", tid );
	char app[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	IMET *imet = (IMET*) memalign(32, sizeof(IMET));

	tmd* titleTmd = GetTMD(tid);
	if (!titleTmd)
	{
		//gprintf("no TMD\n");
		free(imet);
		return false;
	}

	u16 i;
	bool ok = false;
	for (i = 0; i < titleTmd->num_contents; i++)
	{
		if (!titleTmd->contents[i].index)
		{
			ok = true;
			break;
		}
	}
	if (!ok)
	{
		free(imet);
		return false;
	}

	snprintf(app, sizeof(app), "/title/%08x/%08x/content/%08x.app", TITLE_UPPER( tid ), TITLE_LOWER( tid ),
			titleTmd->contents[i].cid);
	//gprintf("%s\n", app );

	if (language > CONF_LANG_KOREAN) language = CONF_LANG_ENGLISH;

	s32 fd = ISFS_Open(app, ISFS_OPEN_READ);
	if (fd < 0)
	{
		//gprintf("fd: %d\n", fd );
		free(imet);
		return false;
	}

	if (ISFS_Seek(fd, IMET_OFFSET, SEEK_SET) != IMET_OFFSET)
	{
		ISFS_Close(fd);
		free(imet);
		return false;
	}

	if (ISFS_Read(fd, imet, sizeof(IMET)) != sizeof(IMET))
	{
		ISFS_Close(fd);
		free(imet);
		return false;
	}

	ISFS_Close(fd);

	if (imet->sig != IMET_SIGNATURE)
	{
		free(imet);
		return false;
	}

	if (imet->name_japanese[language * IMET_MAX_NAME_LEN] == 0)
	{
		// channel name is not available in system language
		if (imet->name_english[0] != 0)
		{
			language = CONF_LANG_ENGLISH;
		}
		else
		{
			// channel name is also not available on english, get ascii name
			for (int i = 0; i < 4; i++)
			{
				name[i] = (TITLE_LOWER( tid ) >> (24 - i * 8)) & 0xFF;
			}
			name[4] = 0;
			free(imet);
			return true;
		}
	}

	// retrieve channel name in system language or on english
	for (int i = 0; i < IMET_MAX_NAME_LEN; i++)
	{
		name[i] = imet->name_japanese[i + (language * IMET_MAX_NAME_LEN)];
	}

	free(imet);

	return true;
}

bool NandTitle::Exists(u64 tid)
{
	char app[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	tmd* titleTmd = GetTMD(tid);
	if (!titleTmd) return false;

	u16 i;
	bool ok = false;
	for (i = 0; i < titleTmd->num_contents; i++)
	{
		if (!titleTmd->contents[i].index)
		{
			ok = true;
			break;
		}
	}
	if (!ok) return false;

	snprintf(app, sizeof(app), "/title/%08x/%08x/content/%08x.app", TITLE_UPPER( tid ), TITLE_LOWER( tid ),
			titleTmd->contents[i].cid);
	s32 fd = ISFS_Open(app, ISFS_OPEN_READ);
	if (fd >= 0) ISFS_Close(fd);

	//gprintf(" fd: %d\n", fd );
	return fd >= 0 || fd == -102; //102 means it exists, but we dont have permission to open it

}

bool NandTitle::ExistsFromIndex(u32 i)
{
	if (i >= titleIds.size()) return false;

	return Exists(titleIds.at(i));
}

u64 NandTitle::At(u32 i)
{
	if (i >= titleIds.size()) return 0;

	return titleIds.at(i);
}

int NandTitle::IndexOf(u64 tid)
{
	for (u32 i = 0; i < titleIds.size(); i++)
	{
		if (titleIds.at(i) == tid) return i;
	}

	return WII_EINSTALL;
}

const char* NandTitle::NameOf(u64 tid)
{
	map<u64, string>::iterator itr = NameList.find(tid);
	if (itr != NameList.end()) return itr->second.c_str();

	return NULL;
}

const char* NandTitle::NameFromIndex(u32 i)
{
	if (i >= titleIds.size()) return NULL;

	map<u64, string>::iterator itr = NameList.find(titleIds.at(i));
	if (itr != NameList.end()) return itr->second.c_str();

	return NULL;
}

u16 NandTitle::VersionOf(u64 tid)
{
	for (u32 i = 0; i < titleIds.size(); i++)
	{
		if (titleIds.at(i) == tid)
		{
			tmd* Tmd = GetTMD(tid);
			if (!Tmd) break;

			return Tmd->title_version;
		}
	}
	return 0;

}

u16 NandTitle::VersionFromIndex(u32 i)
{
	if (i >= titleIds.size()) return 0;

	tmd* Tmd = GetTMD(titleIds.at(i));
	if (!Tmd) return 0;

	return Tmd->title_version;
}

u32 NandTitle::CountType(u32 type)
{
	u32 ret = 0;
	for (u32 i = 0; i < titleIds.size(); i++)
	{
		if (TITLE_UPPER( titleIds.at( i ) ) == type)
		{
			ret++;
		}
	}
	return ret;
}

u32 NandTitle::SetType(u32 upper)
{
	currentType = upper;
	currentIndex = 0;

	return CountType(upper);
}

u64 NandTitle::Next()
{
	u64 ret = 0;
	//gprintf("Next( %08x, %u )\n", currentType, currentIndex );
	u32 i;
	for (i = currentIndex; i < titleIds.size(); i++)
	{
		if (currentType)
		{
			if (currentType == TITLE_UPPER( titleIds.at( i ) ))
			{
				ret = titleIds.at(i);
				break;
			}
		}
		else
		{
			ret = titleIds.at(i);
			break;
		}
	}
	currentIndex = i + 1;

	return ret;
}

void NandTitle::ResetCounter()
{
	currentIndex = 0;
}

void NandTitle::AsciiTID(u64 tid, char* out)
{
	//gprintf("AsciiTID( %016llx ): ");
	out[0] = ascii(TITLE_3( tid ));
	out[1] = ascii(TITLE_2( tid ));
	out[2] = ascii(TITLE_1( tid ));
	out[3] = ascii((u8) (tid));
	out[4] = 0;
	//gprintf("%s\n", out );
}

void NandTitle::AsciiFromIndex(u32 i, char* out)
{
	if (i >= titleIds.size())
	{
		out[0] = 0;
		return;
	}

	AsciiTID(titleIds.at(i), out);
}

s32 NandTitle::GetTicketViews(u64 tid, tikview **outbuf, u32 *outlen)
{
	tikview *views = NULL;

	u32 nb_views;
	s32 ret;

	/* Get number of ticket views */
	ret = ES_GetNumTicketViews(tid, &nb_views);
	if (ret < 0) return ret;

	/* Allocate memory */
	views = (tikview *) memalign(32, sizeof(tikview) * nb_views);
	if (!views) return -1;

	/* Get ticket views */
	ret = ES_GetTicketViews(tid, views, nb_views);
	if (ret < 0) goto err;

	/* Set values */
	*outbuf = views;
	*outlen = nb_views;

	return 0;

	err:
	/* Free memory */
	if (views) free(views);

	return ret;
}

u64 NandTitle::FindU64(const char *s)
{
	u64 tid = strtoull(s, NULL, 16);

	for (u32 i = 0; i < titleIds.size(); i++)
	{
		if(titleIds[i] == tid)
			return titleIds[i];
	}

	return 0;
}

u64 NandTitle::FindU32(const char *s)
{
	u32 tid = (u32) strtoull(s, NULL, 16);
	if(!tid)
		return 0;

	for (u32 i = 0; i < titleIds.size(); i++)
	{
		if (TITLE_LOWER(titleIds[i]) == tid)
			return titleIds[i];
	}
	return 0;
}

int NandTitle::LoadFileFromNand(const char *filepath, u8 **outbuffer, u32 *outfilesize)
{
	if(!filepath)
		return -1;

	fstats *stats = (fstats *) memalign(32, ALIGN32(sizeof(fstats)));
	if(!stats)
		return IPC_ENOMEM;

	int fd = ISFS_Open(filepath, ISFS_OPEN_READ);
	if(fd < 0)
	{
		free(stats);
		return fd;
	}

	int ret = ISFS_GetFileStats(fd, stats);
	if (ret < 0)
	{
		free(stats);
		ISFS_Close(fd);
		return ret;
	}

	u32 filesize = stats->file_length;

	free(stats);

	u8 *buffer = (u8 *) memalign(32, ALIGN32(filesize));
	if(!buffer)
	{
		ISFS_Close(fd);
		return IPC_ENOMEM;
	}

	ret = ISFS_Read(fd, buffer, filesize);

	ISFS_Close(fd);

	if (ret < 0)
	{
		free(buffer);
		return ret;
	}

	*outbuffer = buffer;
	*outfilesize = filesize;

	return 0;
}

typedef struct _ReplaceStruct
{
	const char * replace;
	char orig;
} ReplaceStruct;

//! More replacements can be added if needed
static const ReplaceStruct Replacements[] =
{
	{ "&gt;", '>' },
	{ "&lt;", '<' },
	{ "&st;", '*' },
	{ "&cl;", ':' },
	{ "&qt;", '\"' },
	{ "&qm;", '?' },
	{ "&vb;", '|' },
	{ NULL, '\0' }
};

static void ConvertInvalidCharacters(std::string &filepath)
{
	size_t startPos;
	size_t pos;

	for(int i = 0; Replacements[i].replace != 0; ++i)
	{
		//! Skip the first ':' because it is the device delimiter
		if(Replacements[i].orig == ':')
			startPos = filepath.find(Replacements[i].orig)+1;
		else
			startPos = 0;

		while((pos = filepath.find(Replacements[i].orig, startPos)) != std::string::npos)
		{
			filepath.erase(pos, 1);
			filepath.insert(pos, Replacements[i].replace);
		}
	}
}

int NandTitle::ExtractFile(const char *nandPath, const char *filepath)
{
	if(!nandPath || !filepath)
		return -1;

	char *strDup = strdup(filepath);
	if(!strDup)
		return -666;

	char *ptr = strrchr(strDup, '/');
	if(!ptr)
	{
		free(strDup);
		return -333;
	}
	else
	{
		*ptr = 0;
		CreateSubfolder(strDup);
		free(strDup);
	}

	const char *filename = strrchr(filepath, '/')+1;
	int done = 0;
	int fd = -1;
	int blocksize = 32*1024;
	u8 *buffer = (u8 *) memalign(32, ALIGN32(blocksize));
	if(!buffer)
		return -666;

	fstats *stats = (fstats *) memalign(32, ALIGN32(sizeof(fstats)));
	if(!stats)
	{
		free(buffer);
		return -666;
	}

	do
	{
		fd = ISFS_Open(nandPath, ISFS_OPEN_READ);
		if(fd < 0)
			break;

		int ret = ISFS_GetFileStats(fd, stats);
		if (ret < 0)
			break;

		int filesize = stats->file_length;

		FILE *pFile = fopen(filepath, "wb");
		if(!pFile)
			break;

		while(done < filesize)
		{
			if(ProgressCanceled())
				break;

			if(filesize-done < blocksize)
				blocksize = filesize-done;

			ShowProgress(filename, done, filesize);

			ret = ISFS_Read(fd, buffer, blocksize);
			if(ret < 0)
			{
				done = ret;
				break;
			}

			fwrite(buffer, 1, ret, pFile);

			done += ret;
		}

		// Show last size information
		ShowProgress(filename, done, filesize);

		fclose(pFile);

	} while(0);

	free(buffer);
	free(stats);

	if(fd >= 0)
		ISFS_Close(fd);

	if(ProgressCanceled())
		return PROGRESS_CANCELED;

	return done;

}

int NandTitle::InternalExtractDir(char *nandPath, std::string &filepath)
{
	int ret = -1;

	u32 list_len = 0;
	ret = ISFS_ReadDir(nandPath, NULL, &list_len);
	if(ret < 0)
		return ret;

	char * name_list = (char *) memalign(32, ALIGN32(list_len * ISFS_MAXPATH));
	if(!name_list)
		return -666;

	ret = ISFS_ReadDir(nandPath, name_list, &list_len);
	if(ret < 0)
	{
		free(name_list);
		return ret;
	}

	char *entry = name_list;

	for(u32 i = 0; i < list_len; ++i)
	{
		if(ProgressCanceled())
			break;

		u32 dummy;
		int posNandPath = strlen(nandPath);
		int posFilePath = filepath.size();

		if(posFilePath > 0 && filepath[posFilePath-1] != '/')
			filepath += '/';
		filepath += entry;

		if(posNandPath > 0 && nandPath[posNandPath-1] != '/')
			strcat(nandPath, "/");
		strcat(nandPath, entry);

		if(ISFS_ReadDir(nandPath, NULL, &dummy) < 0)
		{
			std::string filepathCpy = filepath;
			ConvertInvalidCharacters(filepathCpy);

			int res = ExtractFile(nandPath, filepathCpy.c_str());
			if(res < 0) {
				gprintf("ExtractFile: Error %i occured on file extract: %s\n", res, nandPath);
				ret = -2;
			}
		}
		else
		{
			int res = InternalExtractDir(nandPath, filepath);
			if(res < 0) {
				gprintf("InternalExtractDir: Error %i occured in: %s\n", res, nandPath);
				ret = -3;
			}
		}

		nandPath[posNandPath] = 0;
		filepath.erase(posFilePath);
		entry += strlen(entry) + 1;
	}

	free(name_list);

	if(ProgressCanceled())
		return PROGRESS_CANCELED;

	return ret;
}

int NandTitle::ExtractDir(const char *nandPath, const char *filepath)
{
	if(!filepath || !nandPath)
		return -1;

	std::string internFilePath = filepath;
	char *internNandPath = (char *) memalign(32, ISFS_MAXPATH);
	if(!internNandPath)
		return -666;

	snprintf(internNandPath, ISFS_MAXPATH, nandPath);

	int ret = InternalExtractDir(internNandPath, internFilePath);

	free(internNandPath);

	return ret;
}
