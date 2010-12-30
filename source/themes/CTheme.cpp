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
#include "libwiigui/gui.h"
#include "settings/CSettings.h"
#include "FileOperations/fileops.h"
#include "FreeTypeGX.h"

FreeTypeGX * fontSystem = NULL;
static FT_Byte * MainFont = (FT_Byte *) font_ttf;
static u32 MainFontSize = font_ttf_size;

bool Theme::ShowTooltips = true;

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
    Resources::LoadFiles(theme_path);

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

    ClearFontData();

    snprintf(FontPath, sizeof(FontPath), "%s/font.ttf", path);

    pfile = fopen(FontPath, "rb");

    if (pfile)
    {
        fseek(pfile, 0, SEEK_END);
        MainFontSize = ftell(pfile);
        rewind(pfile);

        MainFont = new (std::nothrow) FT_Byte[MainFontSize];
        if (!MainFont)
        {
            MainFont = (FT_Byte *) font_ttf;
            MainFontSize = font_ttf_size;
        }
        else
        {
            fread(MainFont, 1, MainFontSize, pfile);
            result = true;
        }
        fclose(pfile);
    }

    fontSystem = new FreeTypeGX(MainFont, MainFontSize);

    return result;
}

void Theme::ClearFontData()
{
    if (fontSystem) delete fontSystem;
    fontSystem = NULL;

    if (MainFont != (FT_Byte *) font_ttf)
    {
        if (MainFont != NULL) delete [] MainFont;
        MainFont = (FT_Byte *) font_ttf;
        MainFontSize = font_ttf_size;
    }
}
