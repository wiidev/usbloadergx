/***************************************************************************
 * Copyright (C) 2010 by dude
 * Copyright (C) 2011 by Miigotu
 * Copyright (C) 2011 by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/dirent.h>
#include "FileOperations/fileops.h"
#include "Controls/DeviceHandler.hpp"
#include "settings/CSettings.h"
#include "settings/GameTitles.h"
#include "patches/gamepatches.h"
#include "wad/nandtitle.h"
#include "memory/memory.h"
#include "utils/lz77.h"
#include "gecko.h"

#include "channels.h"

typedef struct _dolheader{
	u32 section_pos[18];
	u32 section_start[18];
	u32 section_size[18];
	u32 bss_start;
	u32 bss_size;
	u32 entry_point;
	u32 padding[7];
} __attribute__((packed)) dolheader;

Channels *Channels::instance = NULL;

void Channels::GetEmuChannelList()
{
	EmuChannels.clear();

	char filepath[1024];
	int language = CONF_GetLanguage();

	snprintf(filepath, sizeof(filepath), "%s/title/00010001", Settings.NandEmuChanPath);
	ParseTitleDir(filepath, language);

	snprintf(filepath, sizeof(filepath), "%s/title/00010004", Settings.NandEmuChanPath);
	ParseTitleDir(filepath, language);

	snprintf(filepath, sizeof(filepath), "%s/title/00010002", Settings.NandEmuChanPath);
	ParseTitleDir(filepath, language);
}

void Channels::GetChannelList()
{
	NandChannels.clear();

	// Get count of titles of the good titles
	InternalGetNandChannelList(0x00010001);
	InternalGetNandChannelList(0x00010004);
	InternalGetNandChannelList(0x00010002);
}

void Channels::InternalGetNandChannelList(u32 type)
{
	// Get count of system titles
	u32 num_titles = NandTitles.SetType(type);

	for (u32 i = 0; i < num_titles; i++)
	{
		u64 tid = NandTitles.Next();
		if (!tid)
			break;

		//these can't be booted anyways
		if (TITLE_LOWER( tid ) == 0x48414741 || TITLE_LOWER( tid ) == 0x48414141 || TITLE_LOWER( tid ) == 0x48414641)
			continue;

		//these aren't installed on the nand
		if (!NandTitles.Exists(tid))
			continue;

		char id[5];
		NandTitles.AsciiTID(tid, id);

		// Force old and new format to be "JODI" which is known by GameTDB
		if(tid == 0x000100014c554c5aLL || tid == 0x00010001AF1BF516LL || tid == 0x0001000148415858LL)
			strcpy(id, "JODI");

		const char *name = GameTitles.GetTitle(id);
		std::string TitleName;

		if(!name || *name == '\0')
		{
			name = NandTitles.NameOf(tid);
			// Set title for caching
			if(name)
				GameTitles.SetGameTitle(id, name);
		}

		int s = NandChannels.size();
		NandChannels.resize(s + 1);
		memset(&NandChannels[s], 0, sizeof(struct discHdr));
		memcpy(NandChannels[s].id, id, 4);
		NandChannels[s].tid = tid;
		NandChannels[s].type = TYPE_GAME_NANDCHAN;
		strncpy(NandChannels[s].title, name ? name : "", sizeof(NandChannels[s].title)-1);
	}
}

vector<struct discHdr> & Channels::GetNandHeaders(void)
{
	if(NandChannels.empty())
		this->GetChannelList();

	return NandChannels;
}

vector<struct discHdr> & Channels::GetEmuHeaders(void)
{
	if(EmuChannels.empty())
		this->GetEmuChannelList();

	return EmuChannels;
}

u8 * Channels::GetDol(const u64 &title, u8 *tmdBuffer)
{
	static const u8 dolsign[6] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
	static u8 dolhead[32] ATTRIBUTE_ALIGN(32);
	u8 *buffer = NULL;
	u32 filesize = 0;
	u32 bootcontent = 0xDEADBEAF;
	u32 high = TITLE_UPPER(title);
	u32 low  = TITLE_LOWER(title);

	char *filepath = (char *) memalign(32, ISFS_MAXPATH);
	if(!filepath)
		return NULL;

	_tmd * tmd_file = (_tmd *) SIGNATURE_PAYLOAD((u32 *)tmdBuffer);

	if(!Settings.UseChanLauncher)
	{
		for(u32 i = 0; i < tmd_file->num_contents; ++i)
		{
			if(tmd_file->contents[i].index == tmd_file->boot_index)
				continue; // Skip loader

			snprintf(filepath, ISFS_MAXPATH, "/title/%08x/%08x/content/%08x.app", (unsigned int)high, (unsigned int)low, (unsigned int)tmd_file->contents[i].cid);

			s32 fd = ISFS_Open(filepath, ISFS_OPEN_READ);
			if(fd < 0)
				continue;

			s32 ret = ISFS_Read(fd, dolhead, 32);
			ISFS_Close(fd);

			if(ret != 32)
				continue;

			if(memcmp(dolhead, dolsign, sizeof(dolsign)) == 0)
			{
				bootcontent = tmd_file->contents[i].cid;
				break;
			}
		}
	}

	//! Fall back to boot content if dol is not found
	if(bootcontent == 0xDEADBEAF)
	{
		bootcontent = tmd_file->contents[tmd_file->boot_index].cid;
		if(!Settings.UseChanLauncher) gprintf("Main dol not found -> ");
		gprintf("Loading boot content index\n");
	}

	snprintf(filepath, ISFS_MAXPATH, "/title/%08x/%08x/content/%08x.app", (unsigned int)high, (unsigned int)low, (unsigned int)bootcontent);
	gprintf("Loading Channel DOL: %s\n", filepath);

	if (NandTitle::LoadFileFromNand(filepath, &buffer, &filesize) < 0)
	{
		gprintf("Failed loading DOL file\n");
		free(filepath);
		return NULL;
	}

	free(filepath);

	if (isLZ77compressed(buffer))
	{
		u8 *decompressed = NULL;
		u32 size = 0;
		if (decompressLZ77content(buffer, filesize, &decompressed, &size) < 0)
		{
			gprintf("Decompression failed\n");
			free(buffer);
			return NULL;
		}
		free(buffer);
		buffer = decompressed;
		filesize = size;
	}

	// move dol to mem2
	u8 *outBuf = (u8 *) MEM2_alloc(filesize);
	if(!outBuf)
		return buffer;

	memcpy(outBuf, buffer, filesize);
	free(buffer);

	return outBuf;
}

u8 Channels::GetRequestedIOS(const u64 &title)
{
	u8 IOS = 0;

	u32 tmdSize = 0;
	u8 *titleTMD = GetTMD(title, &tmdSize, "");
	if (!titleTMD)
		return 0;

	if(tmdSize > 0x18B)
		IOS = titleTMD[0x18B];

	free(titleTMD);

	return IOS;
}

u8 *Channels::GetTMD(const u64 &tid, u32 *size, const char *prefix)
{
	char *filepath = (char *) memalign(32, ISFS_MAXPATH);
	if(!filepath)
		return NULL;

	if(!prefix)
		prefix = "";

	snprintf(filepath, ISFS_MAXPATH, "%s/title/%08x/%08x/content/title.tmd", prefix, (unsigned int)TITLE_UPPER(tid), (unsigned int)TITLE_LOWER(tid));

	u8 *tmdBuffer = NULL;
	u32 tmdSize = 0;

	int ret;

	if(*prefix != '\0')
		ret = LoadFileToMem(filepath, &tmdBuffer, &tmdSize);
	else
		ret = NandTitle::LoadFileFromNand(filepath, &tmdBuffer, &tmdSize);

	free(filepath);

	if (ret < 0)
	{
		gprintf("Reading TMD...Failed!\n");
		if(tmdBuffer)
			free(tmdBuffer);
		return NULL;
	}

	if(size)
		*size = tmdSize;

	return tmdBuffer;
}

u32 Channels::LoadChannel(const u64 &chantitle)
{
	ISFS_Initialize();

	u32 ios = 0;
	u32 tmdSize = 0;
	u8 *tmdBuffer = GetTMD(chantitle, &tmdSize, "");
	if(!tmdBuffer)
	{
		ISFS_Deinitialize();
		return 0;
	}

	u8 *chanDOL = GetDol(chantitle, tmdBuffer);
	if(!chanDOL)
	{
		ISFS_Deinitialize();
		free(tmdBuffer);
		return 0;
	}

	if(tmdSize > 0x18B)
		ios = tmdBuffer[0x18B];

	Identify(chantitle, tmdBuffer, tmdSize);

	free(tmdBuffer);

	dolheader *dolfile = (dolheader *)chanDOL;

	if(dolfile->bss_start)
	{
		dolfile->bss_start |= 0x80000000;

		if(dolfile->bss_start < 0x81800000)
		{
			// For homebrews...not all have it clean.
			if(dolfile->bss_start + dolfile->bss_size >= 0x81800000)
				dolfile->bss_size = 0x81800000 - dolfile->bss_start;

			memset((void *)dolfile->bss_start, 0, dolfile->bss_size);
			DCFlushRange((void *)dolfile->bss_start, dolfile->bss_size);
			ICInvalidateRange((void *)dolfile->bss_start, dolfile->bss_size);
		}
	}

	int i;
	for(i = 0; i < 18; i++)
	{
		if (!dolfile->section_size[i]) continue;
		if (dolfile->section_pos[i] < sizeof(dolheader)) continue;
		if(!(dolfile->section_start[i] & 0x80000000))
			dolfile->section_start[i] |= 0x80000000;

		u8 *dolChunkOffset = (u8 *)dolfile->section_start[i];
		u32 dolChunkSize = dolfile->section_size[i];

		memmove (dolChunkOffset, chanDOL + dolfile->section_pos[i], dolChunkSize);
		DCFlushRange(dolChunkOffset, dolChunkSize);
		ICInvalidateRange(dolChunkOffset, dolChunkSize);

		RegisterDOL(dolChunkOffset, dolChunkSize);
	}

	u32 chanEntryPoint = dolfile->entry_point;

	free(dolfile);

	// Preparations
	memset((void *)Disc_ID, 0, 6);
	*Disc_ID = TITLE_LOWER(chantitle);		// Game ID
	*Arena_H = 0;							// Arena High, the apploader does this too
	*BI2 = 0x817FE000;						// BI2, the apploader does this too
	*Bus_Speed = 0x0E7BE2C0;				// bus speed
	*CPU_Speed = 0x2B73A840;				// cpu speed
	*GameID_Address = 0x81000000;			// Game id address, while there's all 0s at 0x81000000 when using the apploader...
	memcpy((void *)Online_Check, (void *)Disc_ID, 4);// online check

	memset((void *)0x817FE000, 0, 0x2000); 	// Clearing BI2
	DCFlushRange((void*)0x817FE000, 0x2000);

	// IOS Version Check
	*(vu32*)0x80003140	= ((ios << 16)) | 0xFFFF;
	*(vu32*)0x80003188	= ((ios << 16)) | 0xFFFF;

	ISFS_Deinitialize();

	return chanEntryPoint;
}

static bool Identify_GenerateTik(signed_blob **outbuf, u32 *outlen)
{
	signed_blob *buffer = (signed_blob *)memalign(32, STD_SIGNED_TIK_SIZE);
	if (!buffer) return false;
	memset(buffer, 0, STD_SIGNED_TIK_SIZE);

	sig_rsa2048 *signature = (sig_rsa2048 *)buffer;
	signature->type = ES_SIG_RSA2048;

	tik *tik_data  = (tik *)SIGNATURE_PAYLOAD(buffer);
	strcpy(tik_data->issuer, "Root-CA00000001-XS00000003");
	memset(tik_data->cidx_mask, 0xFF, 32);

	*outbuf = buffer;
	*outlen = STD_SIGNED_TIK_SIZE;

	return true;
}

bool Channels::Identify(const u64 &titleid, u8 *tmdBuffer, u32 tmdSize)
{
	char *filepath = (char *) memalign(32, ISFS_MAXPATH);
	if(!filepath)
		return false;

	u32 tikSize;
	signed_blob *tikBuffer = NULL;

	if(!Identify_GenerateTik(&tikBuffer,&tikSize))
	{
		free(filepath);
		gprintf("Generating fake ticket...Failed!");
		return false;
	}

	strlcpy(filepath, "/sys/cert.sys", ISFS_MAXPATH);
	u8 *certBuffer = NULL;
	u32 certSize = 0;
	if (NandTitle::LoadFileFromNand(filepath, &certBuffer, &certSize) < 0)
	{
		gprintf("Reading certs...Failed!\n");
		free(tikBuffer);
		free(filepath);
		return false;
	}
	s32 ret = ES_Identify((signed_blob*)certBuffer, certSize, (signed_blob*)tmdBuffer, tmdSize, tikBuffer, tikSize, NULL);
	if (ret < 0)
	{
		switch(ret)
		{
			case ES_EINVAL:
				gprintf("Error! ES_Identify (ret = %d;) Data invalid!\n", ret);
				break;
			case ES_EALIGN:
				gprintf("Error! ES_Identify (ret = %d;) Data not aligned!\n", ret);
				break;
			case ES_ENOTINIT:
				gprintf("Error! ES_Identify (ret = %d;) ES not initialized!\n", ret);
				break;
			case ES_ENOMEM:
				gprintf("Error! ES_Identify (ret = %d;) No memory!\n", ret);
				break;
			default:
				gprintf("Error! ES_Identify (ret = %d)\n", ret);
				break;
		}
	}

	free(tikBuffer);
	free(filepath);
	free(certBuffer);

	return ret < 0 ? false : true;
}

bool Channels::emuExists(char *tmdpath)
{
	u8 *buffer = NULL;
	u32 size = 0;

	if(LoadFileToMem(tmdpath, &buffer, &size) < 0)
		return false;

	signed_blob *s_tmd = (signed_blob *) buffer;

	u32 i;
	tmd *titleTmd = (tmd *) SIGNATURE_PAYLOAD(s_tmd);

	for (i = 0; i < titleTmd->num_contents; i++)
		if (!titleTmd->contents[i].index)
			break;

	if(i == titleTmd->num_contents)
	{
		free(buffer);
		return false;
	}

	u32 cid  = titleTmd->contents[i].cid;
	
	free(buffer);

	char *ptr = strrchr(tmdpath, '/');
	if(!ptr)
		return false;

	//! tmdpath has length of 1024
	snprintf(ptr+1, 1024-(ptr+1-tmdpath), "%08x.app", (unsigned int)cid);

	FILE *f = fopen(tmdpath, "rb");
	if(!f)
		return false;

	fclose(f);
	
	return true;
}

bool Channels::ParseTitleDir(char *path, int language)
{
	if(!path)
		return false;

	const char *tidPtr = strrchr(path, '/');
	if(!tidPtr)
		return false;
	else
		tidPtr++;

	struct dirent *dirent = NULL;
	DIR *dir = opendir(path);
	if(!dir)
		return false;

	char *pathEndPtr = path + strlen(path);

	u32 tidHigh = strtoul(tidPtr, NULL, 16);
	struct stat st;

	while ((dirent = readdir(dir)) != 0)
	{
		if(!dirent->d_name)
			continue;

		//these can't be booted anyways
		if(*dirent->d_name == '.' || strcmp(dirent->d_name, "48414141") == 0 || strcmp(dirent->d_name, "48414641") == 0)
		{
			continue;
		}

		snprintf(pathEndPtr, 1024-(pathEndPtr-path), "/%s/content/title.tmd", dirent->d_name);

		if(stat(path, &st) != 0)
			continue;

		// check if content in tmd exists
		if(!emuExists(path))
			continue;
			
		u32 tidLow = strtoul(dirent->d_name, NULL, 16);
		char id[5];
		memset(id, 0, sizeof(id));
		memcpy(id, &tidLow, 4);

		u64 tid = ((u64)tidHigh << 32) | ((u64) tidLow);

		// Force old and new format to be "JODI" which is known by GameTDB
		if(tid == 0x000100014c554c5aLL || tid == 0x00010001AF1BF516LL || tid == 0x0001000148415858LL)
			strcpy(id, "JODI");

		std::string TitleName;

		const char *title = GameTitles.GetTitle(id);
		if(title && *title != '\0')
		{
			TitleName = title;
		}
		else if(GetEmuChanTitle(path, language, TitleName))
		{
			GameTitles.SetGameTitle(id, TitleName.c_str());
		}
		else
		{
			TitleName = id;
		}

		int s = EmuChannels.size();
		EmuChannels.resize(s + 1);
		memset(&EmuChannels[s], 0, sizeof(struct discHdr));
		memcpy(EmuChannels[s].id, id, 4);
		EmuChannels[s].tid = tid;
		EmuChannels[s].type = TYPE_GAME_EMUNANDCHAN;
		strncpy(EmuChannels[s].title, TitleName.c_str(), sizeof(EmuChannels[s].title)-1);
	}

	closedir(dir);

	return true;
}

bool Channels::GetEmuChanTitle(char *tmdpath, int language, std::string &Title)
{
	u8 *buffer = NULL;
	u32 size = 0;

	if(LoadFileToMem(tmdpath, &buffer, &size) < 0)
		return false;

	signed_blob *s_tmd = (signed_blob *) buffer;

	u32 i;
	tmd *titleTmd = (tmd *) SIGNATURE_PAYLOAD(s_tmd);

	for (i = 0; i < titleTmd->num_contents; i++)
		if (!titleTmd->contents[i].index)
			break;

	if(i == titleTmd->num_contents)
	{
		free(buffer);
		return false;
	}

	u32 cid  = titleTmd->contents[i].cid;

	free(buffer);

	char *ptr = strrchr(tmdpath, '/');
	if(!ptr)
		return false;

	//! tmdpath has length of 1024
	snprintf(ptr+1, 1024-(ptr+1-tmdpath), "%08x.app", (unsigned int)cid);

	FILE *f = fopen(tmdpath, "rb");
	if(!f)
		return false;

	if(fseek(f, IMET_OFFSET, SEEK_SET) != 0)
	{
		fclose(f);
		return false;
	}

	IMET *imet = (IMET*) malloc(sizeof(IMET));
	if(!imet)
	{
		fclose(f);
		return false;
	}

	if(fread(imet, 1, sizeof(IMET), f) != sizeof(IMET))
	{
		free(imet);
		fclose(f);
		return false;
	}

	fclose(f);

	if (imet->sig != IMET_SIGNATURE)
	{
		free(imet);
		return false;
	}

	// names not available
	if (imet->name_japanese[language * IMET_MAX_NAME_LEN] == 0)
	{
		if(imet->name_english[0] != 0)
			language = CONF_LANG_ENGLISH;
		else
		{
			free(imet);
			return false;
		}
	}

	wchar_t wName[IMET_MAX_NAME_LEN];

	// retrieve channel name in system language or on english
	for (int i = 0; i < IMET_MAX_NAME_LEN; i++)
		wName[i] = imet->name_japanese[i + (language * IMET_MAX_NAME_LEN)];

	wString wsname(wName);
	Title = wsname.toUTF8();

	free(imet);

	return true;
}

u8 *Channels::GetOpeningBnr(const u64 &title, u32 * outsize, const char *pathPrefix)
{
	u8 *banner = NULL;
	u32 high = TITLE_UPPER(title);
	u32 low  = TITLE_LOWER(title);

	char *filepath = (char *) memalign(32, ISFS_MAXPATH + strlen(pathPrefix));
	if(!filepath)
		return NULL;

	do
	{
		snprintf(filepath, ISFS_MAXPATH, "%s/title/%08x/%08x/content/title.tmd", pathPrefix, (unsigned int)high, (unsigned int)low);

		u8 *buffer = NULL;
		u32 filesize = 0;

		int ret = 0;

		if(pathPrefix && *pathPrefix != 0)
			ret = LoadFileToMem(filepath, &buffer, &filesize);
		else
			ret = NandTitle::LoadFileFromNand(filepath, &buffer, &filesize);

		if (ret < 0)
			break;

		_tmd * tmd_file = (_tmd *) SIGNATURE_PAYLOAD((u32 *)buffer);
		bool found = false;
		u32 bootcontent = 0;
		for(u32 i = 0; i < tmd_file->num_contents; ++i)
		{
			if(tmd_file->contents[i].index == 0)
			{
				bootcontent = tmd_file->contents[i].cid;
				found = true;
				break;
			}
		}

		free(buffer);
		buffer = NULL;
		filesize = 0;

		if(!found)
			break;

		snprintf(filepath, ISFS_MAXPATH, "%s/title/%08x/%08x/content/%08x.app", pathPrefix, (unsigned int)high, (unsigned int)low, (unsigned int)bootcontent);

		if(pathPrefix && *pathPrefix != 0)
			ret = LoadFileToMem(filepath, &buffer, &filesize);
		else
			ret = NandTitle::LoadFileFromNand(filepath, &buffer, &filesize);

		if (ret < 0)
			break;

		IMET *imet = (IMET *) (buffer + IMET_OFFSET);
		if(imet->sig != 'IMET')
		{
			free(buffer);
			break;
		}

		// move IMET_OFFSET bytes back
		filesize -= IMET_OFFSET;

		banner = (u8 *) memalign(32, filesize);
		if(!banner)
		{
			free(buffer);
			break;
		}

		memcpy(banner, buffer + IMET_OFFSET, filesize);

		free(buffer);

		if(outsize)
			*outsize = filesize;
	}
	while(0);

	free(filepath);

	return banner;
}
