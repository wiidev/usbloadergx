/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CTheme.h"
#include "GUI/gui.h"
#include "settings/CSettings.h"
#include "banner/OpeningBNR.hpp"
#include "FileOperations/fileops.h"
#include "SystemMenu/SystemMenuResources.h"
#include "menu/menus.h"
#include "wad/nandtitle.h"
#include "FreeTypeGX.h"

FreeTypeGX * fontSystem = NULL;
static FT_Byte * customFont = NULL;
static u32 customFontSize = 0;

bool Theme::ShowTooltips = true;

void Theme::Reload()
{
	HaltGui();
	mainWindow->Remove(bgImg);
	for(int i = 0; i < 4; ++i)
	{
		char image[50];
		snprintf(image, sizeof(image), "player%i_point.png", i+1);
		pointer[i]->SetImage(image);
	}
	delete btnSoundClick;
	delete btnSoundClick2;
	delete btnSoundOver;
	btnSoundClick = new GuiSound(Resources::GetFile("button_click.wav"), Resources::GetFileSize("button_click.wav"), Settings.sfxvolume);
	btnSoundClick2 = new GuiSound(Resources::GetFile("button_click2.wav"), Resources::GetFileSize("button_click2.wav"), Settings.sfxvolume);
	btnSoundOver = new GuiSound(Resources::GetFile("button_over.wav"), Resources::GetFileSize("button_over.wav"), Settings.sfxvolume);
	delete background;
	background = Resources::GetImageData(Settings.widescreen ? "wbackground.png" : "background.png");
	delete bgImg;
	bgImg = new GuiImage(background);
	mainWindow->Append(bgImg);
	ResumeGui();
}

void Theme::CleanUp()
{
	ThemeCleanUp();
	Resources::Clear();
	ClearFontData();
}

void Theme::SetDefault()
{
	ShowTooltips = true;
	CleanUp();
	strcpy(Settings.theme, "");
	LoadFont("");
}

bool Theme::Load(const char * theme_file_path)
{
	bool result = LoadTheme(theme_file_path);
	if(!result)
		return result;

	Theme::ShowTooltips = (thInt("1 - Enable tooltips: 0 for off and 1 for on") != 0);

	FILE * file = fopen(theme_file_path, "rb");
	if(!file)
		return false;

	char line[300];
	char * Foldername = NULL;

	while (fgets(line, sizeof(line), file))
	{
		char * ptr = strcasestr(line, "Image-Folder:");
		if(!ptr)
			continue;

		ptr += strlen("Image-Folder:");

		while(*ptr != '\0' && *ptr == ' ') ptr++;

		Foldername = ptr;

		while(*ptr != '\\' && *ptr != '"' && *ptr != '\0') ptr++;

		*ptr = '\0';
		break;
	}

	fclose(file);

	if(!Foldername)
		return result;

	char theme_path[300];
	snprintf(theme_path, sizeof(theme_path), theme_file_path);

	char * ptr = strrchr(theme_path, '/');
	if(ptr) *ptr = '\0';

	snprintf(theme_path, sizeof(theme_path), "%s/%s", theme_path, Foldername);
	if(!Resources::LoadFiles(theme_path))
	{
		const char * ThemeFilename = strrchr(theme_file_path, '/')+1;
		char Filename[255];
		snprintf(Filename, sizeof(Filename), ThemeFilename);

		char * fileext = strrchr(Filename, '.');
		if(fileext) *fileext = 0;

		char * ptr = strrchr(theme_path, '/');
		*ptr = 0;
		snprintf(theme_path, sizeof(theme_path), "%s/%s", theme_path, Filename);
		Resources::LoadFiles(theme_path);
	}

	//! Override font.ttf with the theme font.ttf if it exists in the image folder
	char FontPath[300];
	snprintf(FontPath, sizeof(FontPath), "%s/font.ttf", theme_path);

	if(CheckFile(FontPath))
		Theme::LoadFont(theme_path);

	return result;
}

bool Theme::LoadFont(const char *path)
{
	char FontPath[300];
	bool result = false;
	FILE *pfile = NULL;

	delete [] customFont;
	customFont = NULL;

	snprintf(FontPath, sizeof(FontPath), "%s/font.ttf", path);

	pfile = fopen(FontPath, "rb");

	if (pfile)
	{
		fseek(pfile, 0, SEEK_END);
		customFontSize = ftell(pfile);
		rewind(pfile);

		customFont = new (std::nothrow) FT_Byte[customFontSize];
		if (customFont)
		{
			fread(customFont, 1, customFontSize, pfile);
			result = true;
		}
		fclose(pfile);
	}

	bool isSystemFont = false;
	FT_Byte *loadedFont = customFont;
	u32 loadedFontSize = customFontSize;

	if(!loadedFont && Settings.UseSystemFont)
	{
		//! Default to system font if no custom is loaded
		loadedFont = (u8 *) SystemMenuResources::Instance()->GetSystemFont();
		loadedFontSize = SystemMenuResources::Instance()->GetSystemFontSize();
		if(loadedFont)
			isSystemFont = true;
	}
	if(!loadedFont)
	{
		loadedFont = (FT_Byte *) Resources::GetFile("font.ttf");
		loadedFontSize = Resources::GetFileSize("font.ttf");
	}

	delete fontSystem;

	fontSystem = new FreeTypeGX(loadedFont, loadedFontSize, isSystemFont);

	return result;
}

void Theme::ClearFontData()
{
	if (fontSystem)
		delete fontSystem;
	fontSystem = NULL;

	if(customFont)
		delete [] customFont;
	customFont = NULL;
}
