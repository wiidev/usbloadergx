#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <ogcsys.h>

#include "language/gettext.h"
#include "listfiles.h"
#include "cfg.h"
#define isspace2(c) (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')

static bool WideScreen = false;

struct THEME THEME;
u8 ocarinaChoice = 0;
u8 videoChoice = 0;
u8 faveChoice = no;
u8 languageChoice = 0;
u8 viChoice = 0;
u8 iosChoice = 0;
u8 parentalcontrolChoice = 0;
u8 fix002 = 0;
u8 reloadblock = 0;
u8 countrystrings = 0;
u8 alternatedol = 0;
u32 alternatedoloffset = 0;
u8 xflip = 0;
u8 sort = 0;
u8 fave = 0;
u8 qboot = 0;
u8 wsprompt = 0;
u8 keyset = 0;
u8 favoritevar = 0;
u16 playcount = 0;
u8 listDisplay = 0;
u8 partition = -1;
char alternatedname[40];
u8 returnToLoaderGV = 1; //global variable used for returnToLoaderShit.  defaults to "yes, patch return to loader"

struct ID_Title
{
    char id[6];
    char * title;
};

struct ID_Control
{
    char id[6];
    u8 block;
};
// renamed titles
int num_title = 0; //number of titles
struct ID_Title *cfg_title = NULL;

int num_control = 0;
struct ID_Control *cfg_control = NULL;

#define MAX_SAVED_GAMES 1000
#define MAX_SAVED_GAME_NUM 1000
int num_saved_games = 0;
int num_saved_game_num = 0;
struct Game_CFG cfg_game[MAX_SAVED_GAMES];
struct Game_NUM cfg_game_num[MAX_SAVED_GAME_NUM];

/* For Mapping */

static char *cfg_name, *cfg_val;

struct TextMap
{
    char *name;
    int id;
};

struct TextMap map_video[] = { { "system", CFG_VIDEO_SYS }, { "game", CFG_VIDEO_GAME }, { "patch", CFG_VIDEO_PATCH }, {
        "pal50", CFG_VIDEO_PAL50 }, { "pal60", CFG_VIDEO_PAL60 }, { "ntsc", CFG_VIDEO_NTSC }, { NULL, -1 } };

struct TextMap map_language[] = { { "console", CFG_LANG_CONSOLE }, { "japanese", CFG_LANG_JAPANESE }, { "english",
        CFG_LANG_ENGLISH }, { "german", CFG_LANG_GERMAN }, { "french", CFG_LANG_FRENCH },
        { "spanish", CFG_LANG_SPANISH }, { "italian", CFG_LANG_ITALIAN }, { "dutch", CFG_LANG_DUTCH }, { "schinese",
                CFG_LANG_S_CHINESE }, // without a dot between s and chinese to match the language filename "schinese.lang"
        { "tchinese", CFG_LANG_T_CHINESE }, { "korean", CFG_LANG_KOREAN }, { NULL, -1 } };

struct TextMap map_alignment[] = { { "left", CFG_ALIGN_LEFT }, { "right", CFG_ALIGN_RIGHT }, { "center",
        CFG_ALIGN_CENTRE }, { "top", CFG_ALIGN_TOP }, { "bottom", CFG_ALIGN_BOTTOM }, { "middle", CFG_ALIGN_MIDDLE }, {
        NULL, -1 } };

int map_get_id(struct TextMap *map, char *name)
{
    int i;
    for (i = 0; map[i].name != NULL; i++)
    {
        if (strcmp(name, map[i].name) == 0) return map[i].id;
    }
    return -1;
}

char* map_get_name(struct TextMap *map, short id)
{
    int i;
    for (i = 0; map[i].name != NULL; i++)
    {
        if (id == map[i].id) return map[i].name;
    }
    return NULL;
}

bool map_auto(char *name, char *name2, char *val, struct TextMap *map, short *var)
{
    if (strcmp(name, name2) != 0) return false;
    int id = map_get_id(map, val);
    if (id == -1)
    {
        //printf("MAP FAIL: %s=%s : %d\n", name, val, id); sleep(1);
        return false;
    }
    *var = id;
    //printf("MAP AUTO: %s=%s : %d\n", name, val, id); sleep(1);
    return true;
}

bool cfg_map_auto(char *name, struct TextMap *map, short *var)
{
    return map_auto(name, cfg_name, cfg_val, map, var);
}

bool cfg_map(char *name, char *val, short *var, short id)
{
    if (strcmp(name, cfg_name) == 0 && strcmpi(val, cfg_val) == 0)
    {
        *var = id;
        return true;
    }
    return false;
}

bool cfg_bool(char *name, short *var)
{
    return (cfg_map(name, "0", var, 0) || cfg_map(name, "1", var, 1));
}

void cfg_int(char *name, short *var, int count)
{
    char tmp[6];
    short i;

    if (count > 10) //avoid overflow
    return;

    for (i = 0; i < count; i++)
    {
        sprintf(tmp, "%d", i);
        cfg_map(name, tmp, var, i);
    }
}

/* Mapping */

//static char bg_path[100];

void CFG_DefaultTheme() // -1 = non forced Mode
{
    //always set Theme defaults
    //all alignments are left top here
    THEME.gamelist_x = 200;
    THEME.gamelist_y = 49;//40;
    THEME.gamelist_w = 396;
    THEME.gamelist_h = 280;
    THEME.gamegrid_w = 640;
    THEME.gamegrid_h = 400;
    THEME.gamegrid_x = 0;
    THEME.gamegrid_y = 20;
    THEME.gamecarousel_w = 640;
    THEME.gamecarousel_h = 400;
    THEME.gamecarousel_x = 0;
    THEME.gamecarousel_y = -20;

    THEME.covers_x = 26;
    THEME.covers_y = 58;

    THEME.show_id = 1;
    THEME.id_x = 68;
    THEME.id_y = 305;
    THEME.show_region = 1;
    THEME.region_x = 68;
    THEME.region_y = 30;

    THEME.sdcard_x = 160;
    THEME.sdcard_y = 395;
    THEME.homebrew_x = 410;
    THEME.homebrew_y = 405;
    THEME.power_x = 576;
    THEME.power_y = 355;
    THEME.home_x = 489;//215;
    THEME.home_y = 371;
    THEME.setting_x = 64;//-210
    THEME.setting_y = 371;
    THEME.install_x = 16;//-280
    THEME.install_y = 355;

    THEME.clock = ( GXColor)
            {   138, 138, 138, 240};
            THEME.clock_align = CFG_ALIGN_CENTRE;
            THEME.clock_x = 0;
            THEME.clock_y = 335;//330;

            THEME.info = ( GXColor )
            {   55, 190, 237, 255};
            THEME.show_hddinfo = 1; //default
            THEME.hddinfo_align = CFG_ALIGN_CENTRE;
            THEME.hddinfo_x = 0;
            THEME.hddinfo_y = 400;
            THEME.show_gamecount = 1; //default
            THEME.gamecount_align = CFG_ALIGN_CENTRE;
            THEME.gamecount_x = 0;
            THEME.gamecount_y = 420;

            THEME.show_tooltip = 1; //1 means use settings, 0 means force turn off
            THEME.tooltipAlpha = 255;

            THEME.prompttext = ( GXColor )
            {   0, 0, 0, 255};
            THEME.settingstext = ( GXColor )
            {   0, 0, 0, 255};
            THEME.gametext = ( GXColor )
            {   0, 0, 0, 255};

            THEME.pagesize = 9;

            THEME.gamelist_favorite_x = WideScreen ? 256 : 220;
            THEME.gamelist_favorite_y = 13;
            THEME.gamelist_search_x = WideScreen ? 288 : 260;
            THEME.gamelist_search_y = 13;
            THEME.gamelist_abc_x = WideScreen ? 320 : 300;
            THEME.gamelist_abc_y = 13;
            THEME.gamelist_count_x = WideScreen ? 352 : 340;
            THEME.gamelist_count_y = 13;
            THEME.gamelist_list_x = WideScreen ? 384 : 380;
            THEME.gamelist_list_y = 13;
            THEME.gamelist_grid_x = WideScreen ? 416 : 420;
            THEME.gamelist_grid_y = 13;
            THEME.gamelist_carousel_x = WideScreen ? 448 : 460;
            THEME.gamelist_carousel_y = 13;
            THEME.gamelist_lock_x = WideScreen ? 480 : 500;
            THEME.gamelist_lock_y = 13;
            THEME.gamelist_dvd_x = WideScreen ? 512 : 540;
            THEME.gamelist_dvd_y = 13;

            THEME.gamegrid_favorite_x = WideScreen ? 192 : 160;
            THEME.gamegrid_favorite_y = 13;
            THEME.gamegrid_search_x = WideScreen ? 224 : 200;
            THEME.gamegrid_search_y = 13;
            THEME.gamegrid_abc_x = WideScreen ? 256 : 240;
            THEME.gamegrid_abc_y = 13;
            THEME.gamegrid_count_x = WideScreen ? 288 : 280;
            THEME.gamegrid_count_y = 13;
            THEME.gamegrid_list_x = WideScreen ? 320 : 320;
            THEME.gamegrid_list_y = 13;
            THEME.gamegrid_grid_x = WideScreen ? 352 : 360;
            THEME.gamegrid_grid_y = 13;
            THEME.gamegrid_carousel_x = WideScreen ? 384 : 400;
            THEME.gamegrid_carousel_y = 13;
            THEME.gamegrid_lock_x = WideScreen ? 416 : 440;
            THEME.gamegrid_lock_y = 13;
            THEME.gamegrid_dvd_x = WideScreen ? 448 : 480;
            THEME.gamegrid_dvd_y = 13;

            THEME.gamecarousel_favorite_x = WideScreen ? 192 : 160;
            THEME.gamecarousel_favorite_y = 13;
            THEME.gamecarousel_search_x = WideScreen ? 224 : 200;
            THEME.gamecarousel_search_y = 13;
            THEME.gamecarousel_abc_x = WideScreen ? 256 : 240;
            THEME.gamecarousel_abc_y = 13;
            THEME.gamecarousel_count_x = WideScreen ? 288 : 280;
            THEME.gamecarousel_count_y = 13;
            THEME.gamecarousel_list_x = WideScreen ? 320 : 320;
            THEME.gamecarousel_list_y = 13;
            THEME.gamecarousel_grid_x = WideScreen ? 352 : 360;
            THEME.gamecarousel_grid_y = 13;
            THEME.gamecarousel_carousel_x = WideScreen ? 384 : 400;
            THEME.gamecarousel_carousel_y = 13;
            THEME.gamecarousel_lock_x = WideScreen ? 416 : 440;
            THEME.gamecarousel_lock_y = 13;
            THEME.gamecarousel_dvd_x = WideScreen ? 448 : 480;
            THEME.gamecarousel_dvd_y = 13;
        }

char *cfg_get_title(u8 *id)
{
    if (!id) return NULL;

    int i;
    for (i = 0; i < num_title; i++)
    {
        if (strncmp((char*) id, cfg_title[i].id, 6) == 0)
        {
            return cfg_title[i].title;
        }
    }
    return NULL;
}

char *get_title(struct discHdr *header)
{
    if (!header) return NULL;

    char *title = cfg_get_title(header->id);
    if (title) return title;
    return header->title;
}

void title_set(char *id, char *title)
{
    if (!id || !title) return;

    if (!cfg_title) cfg_title = (struct ID_Title *) malloc(sizeof(struct ID_Title));

    char *idt = cfg_get_title((u8*) id);
    if (idt)
    {
        // replace
        free(idt);
        idt = strdup(title);
    }
    else
    {
        struct ID_Title * tmpStruct = (struct ID_Title *) realloc(cfg_title, (num_title + 1) * sizeof(struct ID_Title));
        if (!tmpStruct)
        {
            // error
            CFG_Cleanup();
            num_title = 0;
            return;
        }

        cfg_title = tmpStruct;

        // add
        strncpy(cfg_title[num_title].id, id, 6);
        cfg_title[num_title].title = strdup(title);
        num_title++;
    }
}

void titles_default()
{
    int i;
    for (i = 0; i < num_title; i++)
    {
        memset(cfg_title[i].id, 0, 6);
        free(cfg_title[i].title);
        cfg_title[i].title = NULL;
    }
}

u8 cfg_get_block(u8 *id)
{
    int i;
    for (i = 0; i < num_control; i++)
    {
        if (memcmp(id, cfg_control[i].id, 6) == 0)
        {
            return cfg_control[i].block;
        }
    }
    return 0;
}

u8 get_block(struct discHdr *header)
{
    return cfg_get_block(header->id);
}

s8 get_pegi_block(struct discHdr *header)
{
    switch (get_block(header))
    {
        case 1:
            return 7;
        case 2:
            return 12;
        case 3:
            return 16;
        case 4:
            return 18;
        default:
            return -1;
    }
}

// trim leading and trailing whitespace
// copy at max n or at max size-1
char* trim_n_copy(char *dest, char *src, int n, int size)
{
    int len;
    // trim leading white space
    while (isspace2( *src ))
    {
        src++;
        n--;
    }
    len = strlen(src);
    if (len > n) len = n;
    // trim trailing white space
    while (len > 0 && isspace2( src[len-1] ))
        len--;
    if (len >= size) len = size - 1;
    strlcpy(dest, src, len + 1);
    //printf("trim_copy: '%s' %d\n", dest, len); //sleep(1);
    return dest;
}

char* trimcopy(char *dest, char *src, int size)
{
    int len;
    while (*src == ' ')
        src++;
    len = strlen(src);
    // trim trailing " \r\n"
    while (len > 0 && strchr(" \r\n", src[len - 1]))
        len--;
    if (len >= size) len = size - 1;
    strlcpy(dest, src, len + 1);
    return dest;
}

static u32 wCOORDS_FLAGS[2] = { 0, 0 }; // space for 64 coords - this is enough, also for the future
#define GET_wCOORD_FLAG(i)  (wCOORDS_FLAGS[i/32] & (1UL << (i%32)))
#define SET_wCOORD_FLAG(i)  (wCOORDS_FLAGS[i/32] |= (1UL << (i%32)))
#define CLEAR_wCOORD_FLAGS  (wCOORDS_FLAGS[0] = wCOORDS_FLAGS[1] = 0)

#define CFG_COORDS2(name)                                           \
    if ((wcoords_idx++, 1) && !GET_wCOORD_FLAG(wcoords_idx) &&      \
                        strcmp(cfg_name, #name "_coords") == 0) {   \
        short x,y;                                                  \
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {                  \
            THEME.name##_x = x;                                     \
            THEME.name##_y = y;                                     \
        }                                                           \
    }                                                               \
    else if (WideScreen &&                                      \
                strcmp(cfg_name, "w" #name "_coords") == 0) {       \
        short x,y;                                                  \
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {                  \
            THEME.name##_x = x;                                     \
            THEME.name##_y = y;                                     \
            SET_wCOORD_FLAG(wcoords_idx);                           \
        }                                                           \
    }
#define CFG_COORDS4(name)                                           \
    if ((wcoords_idx++, 1) && !GET_wCOORD_FLAG(wcoords_idx) &&      \
                        strcmp(cfg_name, #name "_coords") == 0) {   \
        short x,y,w,h;                                              \
        if (sscanf(val, "%hd,%hd,%hd,%hd", &x, &y, &w, &h) == 4) {  \
            THEME.name##_x = x;                                     \
            THEME.name##_y = y;                                     \
            THEME.name##_w = w;                                     \
            THEME.name##_h = h;                                     \
        }                                                           \
    }                                                               \
    else if (WideScreen &&                                      \
                strcmp(cfg_name, "w" #name "_coords") == 0) {       \
    short x,y,w,h;                                                  \
    if (sscanf(val, "%hd,%hd,%hd,%hd", &x, &y, &w, &h) == 4) {      \
            THEME.name##_x = x;                                     \
            THEME.name##_y = y;                                     \
            THEME.name##_w = w;                                     \
            THEME.name##_h = h;                                     \
            SET_wCOORD_FLAG(wcoords_idx);                           \
        }                                                           \
    }
#define CFG_COLOR(name)                                             \
    if (strcmp(cfg_name, #name "_color") == 0) {                    \
        short r,g,b,a;                                          \
        int c = sscanf(val, "%hd,%hd,%hd,%hd", &r, &g, &b, &a); \
        if(c >= 3) {                                            \
            THEME.name.r = r;                                   \
            THEME.name.g = g;                                   \
            THEME.name.b = b;                                   \
            if(c >= 4)                                          \
                THEME.name.a = a;                               \
        }                                                       \
    }
#define CFG_VAL(name)                                           \
    if (strcmp(cfg_name, #name) == 0) {                 \
        short v;                                                \
        if (sscanf(val, "%hd", &v) == 1) {                      \
            THEME.name = v;                                 \
        }                                                       \
    }

#define CFG_BOOL(name)  if(cfg_bool(#name, &THEME.name));

#define CFG_ALIGN(name) if(cfg_map_auto(#name "_align", map_alignment, &THEME.name##_align));

#define OLD_FAV_ICON         1
#define OLD_ABC_ICON         2
#define OLD_COUNT_ICON       4
#define OLD_LIST_ICON        8
#define OLD_GRID_ICON       16
#define OLD_CAROUSEL_ICON   32
static short WorkAroundIconSet = 0;
static short WorkAroundBarOffset = 100;

void theme_set(char *name, char *val)
{
    cfg_name = name;
    cfg_val = val;
    int wcoords_idx = -1;

    CFG_COORDS4( gamelist )
else    CFG_COORDS4( gamegrid )
    else CFG_COORDS4( gamecarousel )

    else CFG_COORDS2( covers )

    else CFG_BOOL( show_id )
    else CFG_COORDS2( id )

    else CFG_BOOL( show_region )
    else CFG_COORDS2( region )

    else CFG_COORDS2( sdcard )
    else CFG_COORDS2( homebrew )
    else CFG_COORDS2( power )
    else CFG_COORDS2( home )
    else CFG_COORDS2( setting )
    else CFG_COORDS2( install )

    else CFG_COORDS2( clock )
    else CFG_ALIGN( clock )
    else CFG_COLOR( clock )

    else CFG_COLOR( info )
    else CFG_BOOL( show_hddinfo )
    else CFG_ALIGN( hddinfo )
    else CFG_COORDS2( hddinfo )

    else CFG_BOOL( show_gamecount )
    else CFG_ALIGN( gamecount )
    else CFG_COORDS2( gamecount )

    else CFG_BOOL( show_tooltip )
    else CFG_VAL( tooltipAlpha )

    else CFG_COLOR( prompttext )
    else CFG_COLOR( settingstext )
    else CFG_COLOR( gametext )

    else CFG_VAL( pagesize )

    else CFG_COORDS2( gamelist_favorite )
    else CFG_COORDS2( gamegrid_favorite )
    else CFG_COORDS2( gamecarousel_favorite )

    else CFG_COORDS2( gamelist_search )
    else CFG_COORDS2( gamegrid_search )
    else CFG_COORDS2( gamecarousel_search )

    else CFG_COORDS2( gamelist_abc )
    else CFG_COORDS2( gamegrid_abc )
    else CFG_COORDS2( gamecarousel_abc )

    else CFG_COORDS2( gamelist_count )
    else CFG_COORDS2( gamegrid_count )
    else CFG_COORDS2( gamecarousel_count )

    else CFG_COORDS2( gamelist_list )
    else CFG_COORDS2( gamegrid_list )
    else CFG_COORDS2( gamecarousel_list )

    else CFG_COORDS2( gamelist_grid )
    else CFG_COORDS2( gamegrid_grid )
    else CFG_COORDS2( gamecarousel_grid )

    else CFG_COORDS2( gamelist_carousel )
    else CFG_COORDS2( gamegrid_carousel )
    else CFG_COORDS2( gamecarousel_carousel )

    else CFG_COORDS2( gamelist_lock )
    else CFG_COORDS2( gamegrid_lock )
    else CFG_COORDS2( gamecarousel_lock )

    else CFG_COORDS2( gamelist_dvd )
    else CFG_COORDS2( gamegrid_dvd )
    else CFG_COORDS2( gamecarousel_dvd )

    //**********************************
    // Workaround for old Themes
    //**********************************

    else if ( strcmp( cfg_name, "favorite_coords" ) == 0 )
    {
        short x, y;
        if ( sscanf( val, "%hd,%hd", &x, &y ) == 2 )
        {
            // the old Icons are aligned at center and the new at the left top corner.
            // we must add 320 and sub the half image width to get the correct position.
            x += 300;
            // the old stuff is optimized for WideScreen.
            // if no WideScreen, we must reposition the pos
            if ( !WideScreen ) x -= 20;
            // old themes have no search_coords
            // set the searchIcon to the Position of the favIcon
            THEME.gamelist_search_x = x;
            THEME.gamegrid_search_x = THEME.gamecarousel_search_x = x - WorkAroundBarOffset;
            THEME.gamelist_search_y = THEME.gamegrid_search_y = THEME.gamecarousel_search_y = y;
            // place the favIcon to the left side of the searchIcon
            if ( !WideScreen ) x -= WideScreen ? 32 : 40;
            THEME.gamelist_favorite_x = x;
            THEME.gamegrid_favorite_x = THEME.gamecarousel_favorite_x = x - WorkAroundBarOffset;
            THEME.gamelist_favorite_y = THEME.gamegrid_favorite_y = THEME.gamecarousel_favorite_y = y;
            WorkAroundIconSet |= OLD_FAV_ICON;
        }
    }
    else if ( strcmp( cfg_name, "abc_coords" ) == 0 )
    {
        short x, y;
        if ( sscanf( val, "%hd,%hd", &x, &y ) == 2 )
        {
            // the old Icons are aligned at center and the new at the left top corner.
            // we must add 320 and sub the half image width to get the correct position.
            x += 300;
            // the old stuff is optimized for WideScreen.
            // if no WideScreen, we must reposition the pos
            if ( !WideScreen ) x -= 12;
            THEME.gamelist_abc_x = x;
            THEME.gamegrid_abc_x = THEME.gamecarousel_abc_x = x - WorkAroundBarOffset;
            THEME.gamelist_abc_y = THEME.gamegrid_abc_y = THEME.gamecarousel_abc_y = y;
            WorkAroundIconSet |= OLD_ABC_ICON;
        }
    }
    else if ( strcmp( cfg_name, "count_coords" ) == 0 )
    {
        short x, y;
        if ( sscanf( val, "%hd,%hd", &x, &y ) == 2 )
        {
            // the old Icons are aligned at center and the new at the left top corner.
            // we must add 320 and sub the half image width to get the correct position.
            x += 300;
            // the old stuff is optimized for WideScreen.
            // if no WideScreen, we must reposition the pos
            if ( !WideScreen ) x -= 4;
            THEME.gamelist_count_x = x;
            THEME.gamegrid_count_x = THEME.gamecarousel_count_x = x - WorkAroundBarOffset;
            THEME.gamelist_count_y = THEME.gamegrid_count_y = THEME.gamecarousel_count_y = y;
            WorkAroundIconSet |= OLD_COUNT_ICON;
        }
    }
    else if ( strcmp( cfg_name, "list_coords" ) == 0 )
    {
        short x, y;
        if ( sscanf( val, "%hd,%hd", &x, &y ) == 2 )
        {
            // the old Icons are aligned at center and the new at the left top corner.
            // we must add 320 and sub the half image width to get the correct position.
            x += 300;
            // the old stuff is optimized for WideScreen.
            // if no WideScreen, we must reposition the pos
            if ( !WideScreen ) x += 4;
            THEME.gamelist_list_x = x;
            THEME.gamegrid_list_x = THEME.gamecarousel_list_x = x - WorkAroundBarOffset;
            THEME.gamelist_list_y = THEME.gamegrid_list_y = THEME.gamecarousel_list_y = y;
            WorkAroundIconSet |= OLD_LIST_ICON;
        }
    }
    else if ( strcmp( cfg_name, "grid_coords" ) == 0 )
    {
        short x, y;
        if ( sscanf( val, "%hd,%hd", &x, &y ) == 2 )
        {
            // the old Icons are aligned at center and the new at the left top corner.
            // we must add 320 and sub the half image width to get the correct position.
            x += 300;
            // the old stuff is optimized for WideScreen.
            // if no WideScreen, we must reposition the pos
            if ( !WideScreen ) x += 12;
            THEME.gamelist_grid_x = x;
            THEME.gamegrid_grid_x = THEME.gamecarousel_grid_x = x - WorkAroundBarOffset;
            THEME.gamelist_grid_y = THEME.gamegrid_grid_y = THEME.gamecarousel_grid_y = y;
            WorkAroundIconSet |= OLD_GRID_ICON;
        }
    }
    else if ( strcmp( cfg_name, "carousel_coords" ) == 0 )
    {
        short x, y;
        if ( sscanf( val, "%hd,%hd", &x, &y ) == 2 )
        {
            // the old Icons are aligned at center and the new at the left top corner.
            // we must add 320 and sub the half image width to get the correct position.
            x += 300;
            // the old stuff is optimized for WideScreen.
            // if no WideScreen, we must reposition the pos
            if ( !WideScreen ) x += 20;
            THEME.gamelist_carousel_x = x;
            THEME.gamegrid_carousel_x = THEME.gamecarousel_carousel_x = x - WorkAroundBarOffset;
            THEME.gamelist_carousel_y = THEME.gamegrid_carousel_y = THEME.gamecarousel_carousel_y = y;
            WorkAroundIconSet |= OLD_CAROUSEL_ICON;

            // old themes have no dvd_coords
            // place the dvdIcon to the right side of the carouselIcon
            if ( !WideScreen ) x += WideScreen ? 32 : 40;
            THEME.gamelist_lock_x = x;
            THEME.gamegrid_lock_x = THEME.gamecarousel_lock_x = x - WorkAroundBarOffset;
            THEME.gamelist_lock_y = THEME.gamegrid_lock_y = THEME.gamecarousel_lock_y = y;

            x += WideScreen ? 32 : 40;
            THEME.gamelist_dvd_x = x;
            THEME.gamegrid_dvd_x = THEME.gamecarousel_dvd_x = x - WorkAroundBarOffset;
            THEME.gamelist_dvd_y = THEME.gamegrid_dvd_y = THEME.gamecarousel_dvd_y = y;
        }
    }

    else if ( strcmp( cfg_name, "sortBarOffset" ) == 0 )
    {
        short o;
        if ( sscanf( val, "%hd", &o ) == 1 )
        {
            if ( WorkAroundIconSet & OLD_FAV_ICON )
            {
                THEME.gamegrid_favorite_x += WorkAroundBarOffset - o;
                THEME.gamecarousel_favorite_x += WorkAroundBarOffset - o;
                THEME.gamegrid_search_x += WorkAroundBarOffset - o;
                THEME.gamecarousel_search_x += WorkAroundBarOffset - o;
            }
            if ( WorkAroundIconSet & OLD_ABC_ICON )
            {
                THEME.gamegrid_abc_x += WorkAroundBarOffset - o;
                THEME.gamecarousel_abc_x += WorkAroundBarOffset - o;
            }
            if ( WorkAroundIconSet & OLD_COUNT_ICON )
            {
                THEME.gamegrid_count_x += WorkAroundBarOffset - o;
                THEME.gamecarousel_count_x += WorkAroundBarOffset - o;
            }
            if ( WorkAroundIconSet & OLD_LIST_ICON )
            {
                THEME.gamegrid_list_x += WorkAroundBarOffset - o;
                THEME.gamecarousel_list_x += WorkAroundBarOffset - o;
            }
            if ( WorkAroundIconSet & OLD_GRID_ICON )
            {
                THEME.gamegrid_grid_x += WorkAroundBarOffset - o;
                THEME.gamecarousel_grid_x += WorkAroundBarOffset - o;
            }
            if ( WorkAroundIconSet & OLD_CAROUSEL_ICON )
            {
                THEME.gamegrid_carousel_x += WorkAroundBarOffset - o;
                THEME.gamecarousel_carousel_x += WorkAroundBarOffset - o;
                THEME.gamegrid_lock_x += WorkAroundBarOffset - o;
                THEME.gamecarousel_lock_x += WorkAroundBarOffset - o;
                THEME.gamegrid_dvd_x += WorkAroundBarOffset - o;
                THEME.gamecarousel_dvd_x += WorkAroundBarOffset - o;
            }
            WorkAroundBarOffset = o;
        }
    }

}

// split line to part1 delimiter part2
bool trimsplit(char *line, char *part1, char *part2, char delim, int size)
{
    char *eq = strchr(line, delim);
    if (!eq) return false;
    trim_n_copy(part1, line, eq - line, size);
    trimcopy(part2, eq + 1, size);
    return true;
}
void cfg_parseline(char *line, void(*set_func)(char*, char*))
{
    // split name = value
    char tmp[300], name[200], val[200];
    strlcpy(tmp, line, sizeof(tmp));
    char *eq = strchr(tmp, '=');
    if (!eq) return;
    *eq = 0;
    trimcopy(name, tmp, sizeof(name));
    trimcopy(val, eq + 1, sizeof(val));
    //printf("CFG: %s = %s\n", name, val);
    set_func(name, val);
}

void cfg_parsetitleline(char *line, void(*set_func)(char*, char*, u8))
{
    // split name = value
    char tmp[200], name[200], val[200];
    int block = 0;
    strlcpy(tmp, line, sizeof(tmp));
    char *eq = strchr(tmp, '=');
    if (!eq) return;
    *eq = 0;
    trimcopy(name, tmp, sizeof(name));

    char *blockpos = strrchr(eq + 1, '=');

    if (!blockpos)
        trimcopy(val, eq + 1, sizeof(val));

    else
    {
        *blockpos = 0;
        trimcopy(val, eq + 1, sizeof(val));
        if (sscanf(blockpos + 1, "%d", &block) != 1)
        {
            block = 0;
        }
    }
    set_func(name, val, block);
}

bool cfg_parsefile(char *fname, void(*set_func)(char*, char*))
{
    FILE *f;
    char line[300];

    //printf("opening(%s)\n", fname);
    f = fopen(fname, "r");
    if (!f)
    {
        //printf("error opening(%s)\n", fname);
        return false;
    }
    while (fgets(line, sizeof(line), f))
    {
        // lines starting with # are comments
        if (line[0] == '#') continue;
        cfg_parseline(line, set_func);
    }
    fclose(f);
    return true;
}

bool cfg_parsetitlefile(char *fname, void(*set_func)(char*, char*, u8))
{
    FILE *f;
    char line[200];

    //printf("opening(%s)\n", fname);
    f = fopen(fname, "r");
    if (!f)
    {
        //printf("error opening(%s)\n", fname);
        return false;
    }

    while (fgets(line, sizeof(line), f))
    {
        // lines starting with # are comments
        if (line[0] == '#') continue;
        cfg_parsetitleline(line, set_func);
    }
    fclose(f);
    return true;
}

/*
 void cfg_parsearg(int argc, char **argv)
 {
 int i;
 char *eq;
 char pathname[200];
 for (i=1; i<argc; i++) {
 //printf("arg[%d]: %s\n", i, argv[i]);
 eq = strchr(argv[i], '=');
 if (eq) {
 cfg_parseline(argv[i], &cfg_set);
 } else {
 snprintf(pathname, sizeof(pathname), "%s%s", cfg_path, argv[i]);
 cfg_parsefile(pathname, &cfg_set);
 }
 }
 }
 */

// PER-GAME SETTINGS


// return existing or new
struct Game_CFG* cfg_get_game(u8 *id)
{
    struct Game_CFG *game = CFG_get_game_opt(id);
    if (game) return game;
    if (num_saved_games >= MAX_SAVED_GAMES) return NULL;
    game = &cfg_game[num_saved_games];
    num_saved_games++;
    return game;
}

// current options to game
void cfg_set_game_opt(struct Game_CFG *game, u8 *id)
{
    strncpy((char*) game->id, (char*) id, 6);
    game->id[6] = 0;
    game->video = videoChoice;
    game->language = languageChoice;
    game->ocarina = ocarinaChoice;
    game->vipatch = viChoice;
    game->ios = iosChoice;
    game->parentalcontrol = parentalcontrolChoice;
    game->errorfix002 = fix002;
    game->iosreloadblock = reloadblock;
    game->patchcountrystrings = countrystrings;
    game->loadalternatedol = alternatedol;
    if (game->loadalternatedol == 0)
    {
        alternatedoloffset = 0;
        strcpy(alternatedname, "");
    }
    game->alternatedolstart = alternatedoloffset;
    strlcpy(game->alternatedolname, alternatedname, sizeof(game->alternatedolname));
    game->returnTo = returnToLoaderGV;
}

struct Game_NUM* cfg_get_game_num(u8 *id)
{
    struct Game_NUM *game = CFG_get_game_num(id);
    if (game) return game;
    if (num_saved_game_num >= MAX_SAVED_GAME_NUM) return NULL;
    game = &cfg_game_num[num_saved_game_num];
    num_saved_game_num++;
    return game;
}

// current options to game
void cfg_set_game_num(struct Game_NUM *game, u8 *id)
{
    strncpy((char*) game->id, (char*) id, 6);
    game->id[6] = 0;
    game->favorite = favoritevar;
    game->count = playcount;
}

void game_set(char *name, char *val)
{
    // sample line:
    // game:RTNP41 = video:game; language:english; ocarina:0;
    // game:RYWP01 = video:patch; language:console; ocarina:1;
    //printf("GAME: '%s=%s'\n", name, val);
    u8 id[8];
    struct Game_CFG *game;
    if (strncmp(name, "game:", 5) != 0) return;
    trimcopy((char*) id, name + 5, sizeof(id));
    game = cfg_get_game(id);
    // set id and current options as default
    cfg_set_game_opt(game, id);
    //printf("GAME(%s) '%s'\n", id, val); sleep(1);

    // parse val
    // first split options by ;
    char opt[300], *p, *np;
    p = val;

    while (p)
    {
        np = strchr(p, ';');
        if (np)
            trim_n_copy(opt, p, np - p, sizeof(opt));
        else trimcopy(opt, p, sizeof(opt));
        //printf("GAME(%s) (%s)\n", id, opt); sleep(1);
        // parse opt 'language:english'
        char opt_name[200], opt_val[200];
        if (trimsplit(opt, opt_name, opt_val, ':', sizeof(opt_name)))
        {
            //printf("GAME(%s) (%s=%s)\n", id, opt_name, opt_val); sleep(1);
            short opt_v, opt_l, opt_c;
            if (map_auto("video", opt_name, opt_val, map_video, &opt_v))
            {
                // valid option, assign
                game->video = opt_v;
            }
            if (map_auto("language", opt_name, opt_val, map_language, &opt_l))
            {
                // valid option, assign
                game->language = opt_l;
            }
            if (strcmp("ocarina", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->ocarina = opt_c;
                }
            }
            if (strcmp("vipatch", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->vipatch = opt_c;
                }
            }
            if (strcmp("ios", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->ios = opt_c;
                }
            }
            if (strcmp("pctrl", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->parentalcontrol = opt_c;
                }
            }
            if (strcmp("errorfix002", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->errorfix002 = opt_c;
                }
            }
            if (strcmp("iosreloadblock", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->iosreloadblock = opt_c;
                }
            }
            if (strcmp("patchcountrystrings", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->patchcountrystrings = opt_c;
                }
            }
            if (strcmp("loadalternatedol", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->loadalternatedol = opt_c;
                }
            }
            if (strcmp("alternatedolstart", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->alternatedolstart = opt_c;
                }
            }
            if (strcmp("alternatedolname", opt_name) == 0)
            {
                strlcpy(game->alternatedolname, opt_val, sizeof(game->alternatedolname));
            }
            if (strcmp("returnTo", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->returnTo = opt_c;
                }
            }
        }
        // next opt
        if (np)
            p = np + 1;
        else p = NULL;
    }
}

void parental_set(char *name, char *val)
{
    // sample line:
    // game:RTNP41 = video:game; language:english; ocarina:0;
    // game:RYWP01 = video:patch; language:console; ocarina:1;
    //printf("GAME: '%s=%s'\n", name, val);
    u8 id[8];

    if (strncmp(name, "game:", 5) != 0) return;
    trimcopy((char*) id, name + 5, sizeof(id));

    // parse val
    // first split options by ;
    char opt[200], *p, *np;
    p = val;

    while (p)
    {
        np = strchr(p, ';');
        if (np)
            trim_n_copy(opt, p, np - p, sizeof(opt));
        else trimcopy(opt, p, sizeof(opt));
        //printf("GAME(%s) (%s)\n", id, opt); sleep(1);
        // parse opt 'language:english'
        char opt_name[200], opt_val[200];
        if (trimsplit(opt, opt_name, opt_val, ':', sizeof(opt_name)))
        {
            //printf("GAME(%s) (%s=%s)\n", id, opt_name, opt_val); sleep(1);
            short opt_c;

            if (strcmp("pctrl", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    cfg_control = realloc(cfg_control, (num_control + 1) * sizeof(struct ID_Control));
                    if (!cfg_control)
                    {
                        // error
                        num_control = 0;
                        return;
                    }
                    // add
                    strcpy(cfg_control[num_control].id, (char*) id);
                    cfg_control[num_control].block = opt_c;
                    num_control++;
                }
            }

        }
        // next opt
        if (np)
            p = np + 1;
        else p = NULL;
    }
}

void game_set_num(char *name, char *val)
{
    u8 id[8];
    struct Game_NUM *game;
    if (strncmp(name, "game:", 5) != 0) return;
    trimcopy((char*) id, name + 5, sizeof(id));
    game = cfg_get_game_num(id);

    cfg_set_game_num(game, id);

    // parse val
    // first split options by ;
    char opt[200], *p, *np;
    p = val;

    while (p)
    {
        np = strchr(p, ';');
        if (np)
            trim_n_copy(opt, p, np - p, sizeof(opt));
        else trimcopy(opt, p, sizeof(opt));

        char opt_name[200], opt_val[200];
        if (trimsplit(opt, opt_name, opt_val, ':', sizeof(opt_name)))
        {

            short opt_c;
            if (strcmp("favorite", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->favorite = opt_c;
                }
            }
            if (strcmp("count", opt_name) == 0)
            {
                if (sscanf(opt_val, "%hd", &opt_c) == 1)
                {
                    game->count = opt_c;
                }
            }
        }

        if (np)
            p = np + 1;
        else p = NULL;
    }
}

bool cfg_load_games()
{
    char GXGameSettings_cfg[32];
    sprintf(GXGameSettings_cfg, "%s/config/GXGameSettings.cfg", bootDevice);
    return cfg_parsefile(GXGameSettings_cfg, &game_set);
}

bool cfg_load_game_num()
{
    char GXGameFavorites_cfg[32];
    sprintf(GXGameFavorites_cfg, "%s/config/GXGameFavorites.cfg", bootDevice);
    return cfg_parsefile(GXGameFavorites_cfg, &game_set_num);
}

bool cfg_save_games()
{
    FILE *f;
    int i;
    char GXGameSettings_cfg[50];
    sprintf(GXGameSettings_cfg, "%s/config", bootDevice);
    mkdir(GXGameSettings_cfg, 0777);

    sprintf(GXGameSettings_cfg, "%s/config/GXGameSettings.cfg", bootDevice);
    f = fopen(GXGameSettings_cfg, "w");
    if (!f)
    {
        printf("Error saving %s\n", "GXGameSettings.cfg");
        sleep(1);
        return false;
    }
    fprintf(f, "# USB Loader settings file\n");
    fprintf(f, "# note: this file is automatically generated\n");
    fclose(f);
    /* Closing and reopening because of a write issue we are having right now */
    f = fopen(GXGameSettings_cfg, "w");
    fprintf(f, "# USB Loader settings file\n");
    fprintf(f, "# note: this file is automatically generated\n");
    fprintf(f, "# Num Games: %d\n", num_saved_games);
    for (i = 0; i < num_saved_games; i++)
    {
        char *s;
        fprintf(f, "game:%s = ", cfg_game[i].id);
        s = map_get_name(map_video, cfg_game[i].video);
        if (s) fprintf(f, "video:%s; ", s);
        s = map_get_name(map_language, cfg_game[i].language);
        if (s) fprintf(f, "language:%s; ", s);
        fprintf(f, "ocarina:%d; ", cfg_game[i].ocarina);
        fprintf(f, "vipatch:%d; ", cfg_game[i].vipatch);
        fprintf(f, "ios:%d; ", cfg_game[i].ios);
        fprintf(f, "pctrl:%d; ", cfg_game[i].parentalcontrol);
        fprintf(f, "errorfix002:%d; ", cfg_game[i].errorfix002);
        fprintf(f, "iosreloadblock:%d; ", cfg_game[i].iosreloadblock);
        fprintf(f, "patchcountrystrings:%d; ", cfg_game[i].patchcountrystrings);
        fprintf(f, "loadalternatedol:%d;", cfg_game[i].loadalternatedol);
        fprintf(f, "alternatedolstart:%d;", cfg_game[i].alternatedolstart);
        fprintf(f, "alternatedolname:%s;\n", cfg_game[i].alternatedolname);
        fprintf(f, "returnTo:%d;\n", cfg_game[i].returnTo);
    }
    fprintf(f, "# END\n");
    fclose(f);
    return true;
}

bool cfg_save_game_num()
{
    FILE *f;
    int i;
    char GXGameFavorites_cfg[32];
    sprintf(GXGameFavorites_cfg, "%s/config", bootDevice);
    mkdir(GXGameFavorites_cfg, 0777);

    sprintf(GXGameFavorites_cfg, "%s/config/GXGameFavorites.cfg", bootDevice);
    f = fopen(GXGameFavorites_cfg, "w");
    if (!f)
    {
        printf("Error saving %s\n", "GXGameFavorites.cfg");
        sleep(1);
        return false;
    }
    fprintf(f, "# USB Loader settings file\n");
    fprintf(f, "# note: this file is automatically generated\n");
    fclose(f);
    /* Closing and reopening because of a write issue we are having right now */
    f = fopen(GXGameFavorites_cfg, "w");
    fprintf(f, "# USB Loader settings file\n");
    fprintf(f, "# note: this file is automatically generated\n");
    fprintf(f, "# Num Games: %d\n", num_saved_game_num);
    for (i = 0; i < num_saved_game_num; i++)
    {
        fprintf(f, "game:%s = ", cfg_game_num[i].id);
        fprintf(f, "favorite:%d; ", cfg_game_num[i].favorite);
        fprintf(f, "count:%d;\n", cfg_game_num[i].count);
    }
    fprintf(f, "# END\n");
    fclose(f);
    return true;
}

bool CFG_reset_all_playcounters()
{
    FILE *f;
    int i;
    char GXGameFavorites_cfg[32];
    sprintf(GXGameFavorites_cfg, "%s/config", bootDevice);
    mkdir(GXGameFavorites_cfg, 0777);

    sprintf(GXGameFavorites_cfg, "%s/config/GXGameFavorites.cfg", bootDevice);
    f = fopen(GXGameFavorites_cfg, "w");
    if (!f)
    {
        printf("Error saving %s\n", "GXGameFavorites.cfg");
        sleep(1);
        return false;
    }
    fprintf(f, "# USB Loader settings file\n");
    fprintf(f, "# note: this file is automatically generated\n");
    fclose(f);
    /* Closing and reopening because of a write issue we are having right now */
    f = fopen(GXGameFavorites_cfg, "w");
    fprintf(f, "# USB Loader settings file\n");
    fprintf(f, "# note: this file is automatically generated\n");
    fprintf(f, "# Num Games: %d\n", num_saved_game_num);
    for (i = 0; i < num_saved_game_num; i++)
    {
        fprintf(f, "game:%s = ", cfg_game_num[i].id);
        fprintf(f, "favorite:%d; ", cfg_game_num[i].favorite);
        fprintf(f, "count:0;\n");
    }
    fprintf(f, "# END\n");
    fclose(f);
    return true;
}

struct Game_CFG* CFG_get_game_opt(const u8 *id)
{
    int i;
    for (i = 0; i < num_saved_games; i++)
    {
        if (memcmp(id, cfg_game[i].id, 6) == 0)
        {
            return &cfg_game[i];
        }
    }
    return NULL;
}

struct Game_NUM* CFG_get_game_num(const u8 *id)
{
    int i;
    for (i = 0; i < num_saved_game_num; i++)
    {
        if (memcmp(id, cfg_game_num[i].id, 6) == 0)
        {
            return &cfg_game_num[i];
        }
    }
    return NULL;
}

bool CFG_save_game_opt(u8 *id)
{
    struct Game_CFG *game = cfg_get_game(id);
    if (!game) return false;
    cfg_set_game_opt(game, id);
    return cfg_save_games();
}

bool CFG_save_game_num(u8 *id)
{
    struct Game_NUM *game = cfg_get_game_num(id);
    if (!game) return false;
    cfg_set_game_num(game, id);
    return cfg_save_game_num();
}

bool CFG_forget_game_opt(u8 *id)
{
    struct Game_CFG *game = CFG_get_game_opt(id);
    int i;
    if (!game) return true;
    // move entries down
    num_saved_games--;
    for (i = game - cfg_game; i < num_saved_games; i++)
    {
        cfg_game[i] = cfg_game[i + 1];
    }
    memset(&cfg_game[num_saved_games], 0, sizeof(struct Game_CFG));
    return cfg_save_games();
}

bool CFG_forget_game_num(u8 *id)
{
    struct Game_NUM *game = CFG_get_game_num(id);
    int i;
    if (!game) return true;
    // move entries down
    num_saved_game_num--;
    for (i = game - cfg_game_num; i < num_saved_game_num; i++)
    {
        cfg_game[i] = cfg_game[i + 1];
    }
    memset(&cfg_game[num_saved_game_num], 0, sizeof(struct Game_NUM));
    return cfg_save_game_num();
}

void CFG_LoadTheme(bool wide, const char * theme_path)
{
    char pathname[200];
    WideScreen = wide;

    CFG_DefaultTheme(); // set defaults non forced

    WorkAroundIconSet = 0;
    WorkAroundBarOffset = 100; // set Workaroundstuff to defaults
    CLEAR_wCOORD_FLAGS;
    snprintf(pathname, sizeof(pathname), "%sGXtheme.cfg", theme_path);
    cfg_parsefile(pathname, &theme_set); //finally set theme information

    snprintf(pathname, sizeof(pathname), "%s/config/GXGameSettings.cfg", bootDevice);
    cfg_parsefile(pathname, &parental_set);

    // load per-game settings
    cfg_load_games();
    cfg_load_game_num();
}

void CFG_Cleanup(void)
{
    int i = 0;
    for (i = 0; i < num_title; i++)
    {
        if (cfg_title[i].title) free(cfg_title[i].title);
        cfg_title[i].title = NULL;
    }
    if (cfg_title)
    {
        free(cfg_title);
        cfg_title = NULL;
    }
    num_title = 0;
}

