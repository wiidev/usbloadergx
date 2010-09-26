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

CTheme Theme;

typedef struct _TextMap
{
    const char * name;
    int id;
} TextMap;

const TextMap map_alignment[] =
{
    { "left", ALIGN_LEFT },
    { "right", ALIGN_RIGHT },
    { "center", ALIGN_CENTRE },
    { "top", ALIGN_TOP },
    { "bottom", ALIGN_BOTTOM },
    { "middle", ALIGN_MIDDLE },
    { NULL, -1 }
};

static u32 wCOORDS_FLAGS[2] = { 0, 0 }; // space for 64 coords - this is enough, also for the future
#define GET_wCOORD_FLAG(i)  (wCOORDS_FLAGS[i/32] & (1UL << (i%32)))
#define SET_wCOORD_FLAG(i)  (wCOORDS_FLAGS[i/32] |= (1UL << (i%32)))

#define CFG_COORDS2(name)                                           \
    if ((wcoords_idx++, 1) && !GET_wCOORD_FLAG(wcoords_idx) &&      \
                        strcmp(cfg_name, #name "_coords") == 0) {   \
        short x,y;                                                  \
        if (sscanf(value, "%hd,%hd", &x, &y) == 2) {                  \
            name##_x = x;                                     \
            name##_y = y;                                     \
        }                                                           \
        return true;                                                     \
    }                                                               \
    else if (WideScreen &&                                      \
                strcmp(cfg_name, "w" #name "_coords") == 0) {       \
        short x,y;                                                  \
        if (sscanf(value, "%hd,%hd", &x, &y) == 2) {                  \
            name##_x = x;                                     \
            name##_y = y;                                     \
            SET_wCOORD_FLAG(wcoords_idx);                           \
        }                                                           \
        return true;                                                     \
    }

#define CFG_COORDS4(name)                                           \
    if ((wcoords_idx++, 1) && !GET_wCOORD_FLAG(wcoords_idx) &&      \
                        strcmp(cfg_name, #name "_coords") == 0) {   \
        short x,y,w,h;                                              \
        if (sscanf(value, "%hd,%hd,%hd,%hd", &x, &y, &w, &h) == 4) {  \
            name##_x = x;                                     \
            name##_y = y;                                     \
            name##_w = w;                                     \
            name##_h = h;                                     \
        }                                                           \
        return true;                                                     \
    }                                                                   \
    else if (WideScreen && strcmp(cfg_name, "w" #name "_coords") == 0)  \
    {                                                                   \
        short x,y,w,h;                                              \
        if (sscanf(value, "%hd,%hd,%hd,%hd", &x, &y, &w, &h) == 4)  \
        {                                                           \
            name##_x = x;                                           \
            name##_y = y;                                           \
            name##_w = w;                                           \
            name##_h = h;                                           \
            SET_wCOORD_FLAG(wcoords_idx);                           \
        }                                                           \
        return true;                                                     \
    }

#define CFG_COLOR(name)                                             \
    if (strcmp(cfg_name, #name "_color") == 0) {                    \
        short r,g,b,a;                                          \
        int c = sscanf(value, "%hd,%hd,%hd,%hd", &r, &g, &b, &a); \
        if(c >= 3) {                                            \
            name.r = r;                                   \
            name.g = g;                                   \
            name.b = b;                                   \
            if(c >= 4)                                          \
                name.a = a;                               \
        }                                                       \
        return true;                                            \
    }

#define CFG_VAL(name)                                           \
    if (strcmp(cfg_name, #name) == 0) {                 \
        short v;                                                \
        if (sscanf(value, "%hd", &v) == 1) {                      \
            name = v;                                 \
        }                                                       \
        return true;                                            \
    }

#define CFG_ALIGN(varname)                                         \
    for(int i = 0; map_alignment[i].name != NULL; ++i)          \
    {                                                           \
        if(strcasestr(map_alignment[i].name, value) != NULL)   \
        {                                                       \
            varname##_align = map_alignment[i].id;                  \
            break;                                              \
        }                                                       \
        return true;                                            \
    }

CTheme::CTheme()
{
    WideScreen = (CONF_GetAspectRatio() == CONF_ASPECT_16_9);
    SetDefault();
}

CTheme::~CTheme()
{
    Resources::Clear();
}

void CTheme::SetDefault()
{
    Resources::Clear();

    gamelist_x = 200;
    gamelist_y = 49;//40;
    gamelist_w = 396;
    gamelist_h = 280;
    gamegrid_w = 640;
    gamegrid_h = 400;
    gamegrid_x = 0;
    gamegrid_y = 20;
    gamecarousel_w = 640;
    gamecarousel_h = 400;
    gamecarousel_x = 0;
    gamecarousel_y = -20;
    covers_x = 26;
    covers_y = 58;
    show_id = 1;
    id_x = 68;
    id_y = 305;
    show_region = 1;
    region_x = 68;
    region_y = 30;
    sdcard_x = 160;
    sdcard_y = 395;
    homebrew_x = 410;
    homebrew_y = 405;
    power_x = 576;
    power_y = 355;
    home_x = 489;//215;
    home_y = 371;
    setting_x = 64;//-210
    setting_y = 371;
    install_x = 16;//-280
    install_y = 355;
    clock = (GXColor) {138, 138, 138, 240};
    clock_align = ALIGN_CENTRE;
    clock_x = 0;
    clock_y = 335;//330;
    info = ( GXColor ) {55, 190, 237, 255};
    show_hddinfo = 1;
    hddinfo_align = ALIGN_CENTRE;
    hddinfo_x = 0;
    hddinfo_y = 400;
    show_gamecount = 1; //default
    gamecount_align = ALIGN_CENTRE;
    gamecount_x = 0;
    gamecount_y = 420;
    show_tooltip = 1; //1 means use settings, 0 means force turn off
    tooltipAlpha = 255;
    prompttext = (GXColor) {0, 0, 0, 255};
    settingstext = (GXColor) {0, 0, 0, 255};
    gametext = (GXColor) {0, 0, 0, 255};
    pagesize = 9;
    gamelist_favorite_x = WideScreen ? 256 : 220;
    gamelist_favorite_y = 13;
    gamelist_search_x = WideScreen ? 288 : 260;
    gamelist_search_y = 13;
    gamelist_abc_x = WideScreen ? 320 : 300;
    gamelist_abc_y = 13;
    gamelist_count_x = WideScreen ? 352 : 340;
    gamelist_count_y = 13;
    gamelist_list_x = WideScreen ? 384 : 380;
    gamelist_list_y = 13;
    gamelist_grid_x = WideScreen ? 416 : 420;
    gamelist_grid_y = 13;
    gamelist_carousel_x = WideScreen ? 448 : 460;
    gamelist_carousel_y = 13;
    gamelist_lock_x = WideScreen ? 480 : 500;
    gamelist_lock_y = 13;
    gamelist_dvd_x = WideScreen ? 512 : 540;
    gamelist_dvd_y = 13;
    gamegrid_favorite_x = WideScreen ? 192 : 160;
    gamegrid_favorite_y = 13;
    gamegrid_search_x = WideScreen ? 224 : 200;
    gamegrid_search_y = 13;
    gamegrid_abc_x = WideScreen ? 256 : 240;
    gamegrid_abc_y = 13;
    gamegrid_count_x = WideScreen ? 288 : 280;
    gamegrid_count_y = 13;
    gamegrid_list_x = WideScreen ? 320 : 320;
    gamegrid_list_y = 13;
    gamegrid_grid_x = WideScreen ? 352 : 360;
    gamegrid_grid_y = 13;
    gamegrid_carousel_x = WideScreen ? 384 : 400;
    gamegrid_carousel_y = 13;
    gamegrid_lock_x = WideScreen ? 416 : 440;
    gamegrid_lock_y = 13;
    gamegrid_dvd_x = WideScreen ? 448 : 480;
    gamegrid_dvd_y = 13;
    gamecarousel_favorite_x = WideScreen ? 192 : 160;
    gamecarousel_favorite_y = 13;
    gamecarousel_search_x = WideScreen ? 224 : 200;
    gamecarousel_search_y = 13;
    gamecarousel_abc_x = WideScreen ? 256 : 240;
    gamecarousel_abc_y = 13;
    gamecarousel_count_x = WideScreen ? 288 : 280;
    gamecarousel_count_y = 13;
    gamecarousel_list_x = WideScreen ? 320 : 320;
    gamecarousel_list_y = 13;
    gamecarousel_grid_x = WideScreen ? 352 : 360;
    gamecarousel_grid_y = 13;
    gamecarousel_carousel_x = WideScreen ? 384 : 400;
    gamecarousel_carousel_y = 13;
    gamecarousel_lock_x = WideScreen ? 416 : 440;
    gamecarousel_lock_y = 13;
    gamecarousel_dvd_x = WideScreen ? 448 : 480;
    gamecarousel_dvd_y = 13;
}

bool CTheme::SetSetting(char *name, char *value)
{
    char * cfg_name = name;
    int wcoords_idx = -1;

    CFG_COORDS4( gamelist )
    CFG_COORDS4( gamegrid )
    CFG_COORDS4( gamecarousel )

    CFG_COORDS2( covers )

    CFG_VAL( show_id )
    CFG_COORDS2( id )

    CFG_VAL( show_region )
    CFG_COORDS2( region )

    CFG_COORDS2( sdcard )
    CFG_COORDS2( homebrew )
    CFG_COORDS2( power )
    CFG_COORDS2( home )
    CFG_COORDS2( setting )
    CFG_COORDS2( install )

    CFG_COORDS2( clock )
    CFG_ALIGN( clock )
    CFG_COLOR( clock )

    CFG_COLOR( info )
    CFG_VAL( show_hddinfo )
    CFG_ALIGN( hddinfo )
    CFG_COORDS2( hddinfo )

    CFG_VAL( show_gamecount )
    CFG_ALIGN( gamecount )
    CFG_COORDS2( gamecount )

    CFG_VAL( show_tooltip )
    CFG_VAL( tooltipAlpha )

    CFG_COLOR( prompttext )
    CFG_COLOR( settingstext )
    CFG_COLOR( gametext )

    CFG_VAL( pagesize )

    CFG_COORDS2( gamelist_favorite )
    CFG_COORDS2( gamegrid_favorite )
    CFG_COORDS2( gamecarousel_favorite )

    CFG_COORDS2( gamelist_search )
    CFG_COORDS2( gamegrid_search )
    CFG_COORDS2( gamecarousel_search )

    CFG_COORDS2( gamelist_abc )
    CFG_COORDS2( gamegrid_abc )
    CFG_COORDS2( gamecarousel_abc )

    CFG_COORDS2( gamelist_count )
    CFG_COORDS2( gamegrid_count )
    CFG_COORDS2( gamecarousel_count )

    CFG_COORDS2( gamelist_list )
    CFG_COORDS2( gamegrid_list )
    CFG_COORDS2( gamecarousel_list )

    CFG_COORDS2( gamelist_grid )
    CFG_COORDS2( gamegrid_grid )
    CFG_COORDS2( gamecarousel_grid )

    CFG_COORDS2( gamelist_carousel )
    CFG_COORDS2( gamegrid_carousel )
    CFG_COORDS2( gamecarousel_carousel )

    CFG_COORDS2( gamelist_lock )
    CFG_COORDS2( gamegrid_lock )
    CFG_COORDS2( gamecarousel_lock )

    CFG_COORDS2( gamelist_dvd )
    CFG_COORDS2( gamegrid_dvd )
    CFG_COORDS2( gamecarousel_dvd )

    return false;
}

bool CTheme::Load(const char * theme_path)
{
    char filepath[300];
    snprintf(filepath, sizeof(filepath), "%sGXtheme.cfg", theme_path);

    char line[1024];
    wCOORDS_FLAGS[0] = 0;
    wCOORDS_FLAGS[1] = 0;

    FILE * file = fopen(filepath, "r");
    if (!file) return false;

    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == '#') continue;

        this->ParseLine(line);
    }
    fclose(file);

    Resources::LoadFiles(theme_path);

    return true;
}

void CTheme::ParseLine(char *line)
{
    char temp[1024], name[1024], value[1024];

    strncpy(temp, line, sizeof(temp));

    char * eq = strchr(temp, '=');

    if (!eq) return;

    *eq = 0;

    this->TrimLine(name, temp, sizeof(name));
    this->TrimLine(value, eq + 1, sizeof(value));

    this->SetSetting(name, value);
}

void CTheme::TrimLine(char *dest, char *src, int size)
{
    int len;
    while (*src == ' ')
        src++;
    len = strlen(src);
    while (len > 0 && strchr(" \r\n", src[len - 1]))
        len--;
    if (len >= size) len = size - 1;
    strncpy(dest, src, len);
    dest[len] = 0;
}
