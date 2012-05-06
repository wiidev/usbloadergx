/****************************************************************************
 * Copyright (C) 2012 Dimok, giantpune
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "gecko.h"
#include "SystemMenu/SystemMenuResources.h"
#include "memory/mem2.h"
#include "utils/U8Archive.h"
#include "wad/nandtitle.h"

SystemMenuResources *SystemMenuResources::instance = NULL;

SystemMenuResources::SystemMenuResources():
	isInited( false ),
	wbf1( NULL ),
	wbf2( NULL ),
	wbf1Buffer( NULL ),
	wbf2Buffer( NULL ),
	chanTtlAsh( NULL ),
	chanSelAsh( NULL ),
	GCBannAsh( NULL ),
	systemFont( NULL )
{
}

SystemMenuResources::~SystemMenuResources()
{
	FreeEverything();
}

bool SystemMenuResources::Init()
{
	if( isInited )
		return true;

	// get tmd
	tmd *p_tmd = NandTitles.GetTMD( 0x100000002ull );
	if( !p_tmd )
	{
		gprintf( "can\'t get system menu TMD\n" );
		return false;
	}

	// determine resource cid
	u16 idx = 0xffff;
	tmd_content *contents = TMD_CONTENTS( p_tmd );
	for( u16 i = 0; i < p_tmd->num_contents; i++ )
	{
		if( contents[ i ].index == 1 )
		{
			idx = i;
			break;
		}
	}
	if( idx == 0xffff )
	{
		gprintf( "SM main resource not found\n" );
		return false;
	}
	// build file path
	char path[ ISFS_MAXPATH ]__attribute__((aligned( 32 )));
	sprintf( path, "/title/00000001/00000002/content/%08x.app", contents[ idx ].cid );

	// get resource archive
	u8* resourceArc = NULL;
	u32 resourceLen = 0;
	int ret;

	if( ( ret = NandTitle::LoadFileFromNand( path, &resourceArc, &resourceLen ) ) < 0 )
	{
		gprintf( "Error reading resource from nand: %i\n", ret );
		return false;
	}

	// create U8 archive for reading files
	U8Archive mainArc( resourceArc, resourceLen );

	// Load the font archive
	InitFontArchive();

	// read ash files
	chanTtlAsh = mainArc.GetFileAllocated( "/layout/common/chanTtl.ash", &chanTtlAshSize);
	if(!chanTtlAsh)
	{
		gprintf( "Error while loading chanTtl.ash\n" );
	}
	// move this to mem2
	else if(!isMEM2Buffer(chanTtlAsh))
	{
		u8 *tmp = (u8 *) MEM2_alloc(chanTtlAshSize);
		if(tmp)
		{
			memcpy(tmp, chanTtlAsh, chanTtlAshSize);
			free(chanTtlAsh);
			chanTtlAsh = tmp;
		}
	}

	// read ash files
	chanSelAsh = mainArc.GetFileAllocated( "/layout/common/chanSel.ash", &chanSelAshSize);
	if(!chanSelAsh)
	{
		gprintf( "Error while loading chanSel.ash\n" );
	}
	// move this to mem2
	else if(!isMEM2Buffer(chanSelAsh))
	{
		u8 *tmp = (u8 *) MEM2_alloc(chanSelAshSize);
		if(tmp)
		{
			memcpy(tmp, chanSelAsh, chanSelAshSize);
			free(chanSelAsh);
			chanSelAsh = tmp;
		}
	}
/*  Currently not used since we use a custom banner
	// load GC banner for GC games
	GCBannAsh = mainArc.GetFileAllocated( "/layout/common/GCBann.ash", &GCBannAshSize);
	if(!GCBannAsh)
	{
		gprintf( "Error while loading GCBannAsh.ash\n" );
	}
	// move this to mem2
	else if(!isMEM2Buffer(GCBannAsh))
	{
		u8 *tmp = (u8 *) MEM2_alloc(GCBannAshSize);
		if(tmp)
		{
			memcpy(tmp, GCBannAsh, GCBannAshSize);
			free(GCBannAsh);
			GCBannAsh = tmp;
		}
	}
*/
	// done with the huge U8 archie now that we already got everything out of it
	free( resourceArc );

	isInited = true;
	return true;
}

typedef struct map_entry
{
	char name[8];
	u8 hash[20];
} __attribute__((packed)) map_entry_t;

static const char contentMapPath[] ATTRIBUTE_ALIGN(32) = "/shared1/content.map";
static const u8 WFB_HASH[] = { 0x4f, 0xad, 0x97, 0xfd, 0x4a, 0x28, 0x8c, 0x47, 0xe0, 0x58, 0x7f, 0x3b, 0xbd, 0x29, 0x23, 0x79, 0xf8, 0x70, 0x9e, 0xb9 };
static const u8 WIIFONT_HASH[] = {0x32, 0xb3, 0x39, 0xcb, 0xbb, 0x50, 0x7d, 0x50, 0x27, 0x79, 0x25, 0x9a, 0x78, 0x66, 0x99, 0x5d, 0x03, 0x0b, 0x1d, 0x88};
static const u8 WIIFONT_HASH_KOR[] = {0xb7, 0x15, 0x6d, 0xf0, 0xf4, 0xae, 0x07, 0x8f, 0xd1, 0x53, 0x58, 0x3e, 0x93, 0x6e, 0x07, 0xc0, 0x98, 0x77, 0x49, 0x0e};

bool SystemMenuResources::InitFontArchive(void)
{
	// get content.map
	u8 *contentMap = NULL;
	u32 mapsize = 0;

	NandTitle::LoadFileFromNand(contentMapPath, &contentMap, &mapsize);
	if(!contentMap)
	{
		gprintf( "!contentMap\n" );
		return false;
	}

	int fileCount = mapsize / sizeof(map_entry_t);
	map_entry_t *mapEntryList = (map_entry_t *) contentMap;

	// search content.map for brfna archive
	for( int i = 0; i < fileCount; i++ )
	{
		if( memcmp(mapEntryList[i].hash, WFB_HASH, 20 ) )
			continue;

		// Name found
		char font_filename[32] ATTRIBUTE_ALIGN(32);
		snprintf( font_filename, sizeof( font_filename ), "/shared1/%.8s.app", mapEntryList[ i ].name );

		u8 *fontArchiveBuffer = NULL;
		u32 fontArchiveSize;
		NandTitle::LoadFileFromNand( font_filename, &fontArchiveBuffer, &fontArchiveSize );
		if( !fontArchiveBuffer )
		{
			free(contentMap);
			return false;
		}

		U8Archive fontArc(fontArchiveBuffer, fontArchiveSize);
		wbf1Buffer = fontArc.GetFileAllocated("wbf1.brfna");
		wbf2Buffer = fontArc.GetFileAllocated("wbf2.brfna");

		if(wbf1Buffer)
		{
			wbf1 = new WiiFont;
			wbf1->SetName("wbf1.brfna");
			wbf1->Load(wbf1Buffer);
		}
		if(wbf2Buffer)
		{
			wbf2 = new WiiFont;
			wbf2->SetName("wbf2.brfna");
			wbf2->Load(wbf2Buffer);
		}

		free(fontArchiveBuffer);
		break;
	}

	if(!systemFont)
		InitSystemFontArchive(CONF_GetLanguage() == CONF_LANG_KOREAN, contentMap, mapsize);
	if(!systemFont)
		InitSystemFontArchive(CONF_GetLanguage() != CONF_LANG_KOREAN, contentMap, mapsize);

	free( contentMap );

	// not found
	if( !systemFont || !wbf1Buffer || !wbf2Buffer )
	{
		// shared fonts not found
		return false;
	}

	return true;
}

bool SystemMenuResources::InitSystemFontArchive(bool korean, u8 *contentMap, u32 mapsize)
{
	int fileCount = mapsize / sizeof(map_entry_t);

	map_entry_t *mapEntryList = (map_entry_t *) contentMap;

	for (int i = 0; i < fileCount; i++)
	{
		if (memcmp(mapEntryList[i].hash, korean ? WIIFONT_HASH_KOR : WIIFONT_HASH, 20) != 0)
			continue;

		// Name found, load it and unpack it
		char font_filename[32] ATTRIBUTE_ALIGN(32);
		snprintf(font_filename, sizeof(font_filename), "/shared1/%.8s.app", mapEntryList[i].name);

		u8 *fontArchive = NULL;
		u32 filesize = 0;

		NandTitle::LoadFileFromNand(font_filename, &fontArchive, &filesize);
		if(!fontArchive)
			continue;

		U8Archive systemFontArc(fontArchive, filesize);

		systemFont = systemFontArc.GetFileAllocated(1, &systemFontSize);
		if(!systemFont)
		{
			free(fontArchive);
			continue;
		}
		// move this to mem2
		else if(!isMEM2Buffer(systemFont))
		{
			u8 *tmp = (u8 *) MEM2_alloc(systemFontSize);
			if(tmp)
			{
				memcpy(tmp, systemFont, systemFontSize);
				free(systemFont);
				systemFont = tmp;
			}
		}

		free(fontArchive);
		gprintf("Loaded Wii System Font\n");
		return true;
	}

	return false;
}

void SystemMenuResources::FreeEverything()
{
	if(chanTtlAsh)
		free(chanTtlAsh);
	if(chanSelAsh)
		free(chanSelAsh);
	if(GCBannAsh)
		free(GCBannAsh);
	if(wbf1Buffer)
		free(wbf1Buffer);
	if(wbf2Buffer)
		free(wbf2Buffer);
	if(systemFont)
		free(systemFont);

	delete wbf1;
	delete wbf2;

	chanTtlAsh = NULL;
	chanSelAsh = NULL;
	GCBannAsh = NULL;
	systemFont = NULL;
	wbf1Buffer = NULL;
	wbf2Buffer = NULL;
	wbf1 = NULL;
	wbf2 = NULL;
}
