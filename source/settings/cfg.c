#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <ogcsys.h>
#include <mxml.h>

#include "language/gettext.h"
#include "listfiles.h"
#include "xml/xml.h" /* XML - Lustar*/
#include "cfg.h"

struct SSettings Settings;

char bootDevice[10] = "SD:";

struct CFG CFG;
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
char alternatedname[40];

#define TITLE_MAX 200

struct ID_Title {
    u8 id[5];
    char title[TITLE_MAX];
};

struct ID_Control {
    u8 id[5];
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

struct TextMap {
    char *name;
    int id;
};

struct TextMap map_video[] = {
    { "system", CFG_VIDEO_SYS },
    { "game",   CFG_VIDEO_GAME },
    { "patch",  CFG_VIDEO_PATCH },
    { "pal50", CFG_VIDEO_PAL50 },
    { "pal60", CFG_VIDEO_PAL60 },
    { "ntsc", CFG_VIDEO_NTSC },
    { NULL, -1 }
};

struct TextMap map_language[] = {
    { "console",   CFG_LANG_CONSOLE },
    { "japanese",  CFG_LANG_JAPANESE },
    { "english",   CFG_LANG_ENGLISH },
    { "german",    CFG_LANG_GERMAN },
    { "french",    CFG_LANG_FRENCH },
    { "spanish",   CFG_LANG_SPANISH },
    { "italian",   CFG_LANG_ITALIAN },
    { "dutch",     CFG_LANG_DUTCH },
    { "schinese", CFG_LANG_S_CHINESE }, // without a dot between s and chinese to match the language filename "schinese.lang"
    { "tchinese", CFG_LANG_T_CHINESE },
    { "korean",    CFG_LANG_KOREAN },
    { NULL, -1 }
};


struct TextMap map_alignment[] = {
    { "left",   CFG_ALIGN_LEFT },
    { "right",  CFG_ALIGN_RIGHT },
    { "center",   CFG_ALIGN_CENTRE },
    { "top",    CFG_ALIGN_TOP },
    { "bottom",    CFG_ALIGN_BOTTOM },
    { "middle",   CFG_ALIGN_MIDDLE },
    { NULL, -1 }
};
char* strcopy(char *dest, char *src, int size) {
    strncpy(dest,src,size);
    dest[size-1] = 0;
    return dest;
}

int map_get_id(struct TextMap *map, char *name) {
    int i;
    for (i=0; map[i].name != NULL; i++)	{
        if (strcmp(name, map[i].name) == 0) return map[i].id;
    }
    return -1;
}

char* map_get_name(struct TextMap *map, short id) {
    int i;
    for (i=0; map[i].name != NULL; i++)	{
        if (id == map[i].id) return map[i].name;
    }
    return NULL;
}

bool map_auto(char *name, char *name2, char *val, struct TextMap *map, short *var) {
    if (strcmp(name, name2) != 0) return false;
    int id = map_get_id(map, val);
    if (id == -1) {
        //printf("MAP FAIL: %s=%s : %d\n", name, val, id); sleep(1);
        return false;
    }
    *var = id;
    //printf("MAP AUTO: %s=%s : %d\n", name, val, id); sleep(1);
    return true;
}

bool cfg_map_auto(char *name, struct TextMap *map, short *var) {
    return map_auto(name, cfg_name, cfg_val, map, var);
}

bool cfg_map(char *name, char *val, short *var, short id) {
    if (strcmp(name, cfg_name)==0 && strcmpi(val, cfg_val)==0) {
        *var = id;
        return true;
    }
    return false;
}

bool cfg_bool(char *name, short *var) {
    return (cfg_map(name, "0", var, 0) || cfg_map(name, "1", var, 1));
}

void cfg_int(char *name, short *var, int count) {
    char tmp[5];
    short i;

    if (count > 10) //avoid overflow
        return;

    for (i = 0; i < count; i++) {
        sprintf(tmp, "%d", i);
        cfg_map(name, tmp, var, i);
    }
}

/* Mapping */

//static char bg_path[100];

void CFG_Default(int widescreen) { // -1 = non forced Mode
    if (widescreen == -1)
        CFG.widescreen = CONF_GetAspectRatio();
    else
        CFG.widescreen = widescreen;

    if (CFG.widescreen) {
        snprintf(CFG.theme_path, sizeof(CFG.theme_path), "%s/wtheme/", bootDevice);
    } else {
        snprintf(CFG.theme_path, sizeof(CFG.theme_path), "%s/theme/", bootDevice);
    }

    if (widescreen == -1) {
        snprintf(Settings.covers_path, sizeof(Settings.covers_path), "%s/images/", bootDevice); //default image path
        snprintf(Settings.covers2d_path, sizeof(Settings.covers2d_path), "%s/images/2D/", bootDevice); //default image path
        snprintf(Settings.disc_path, sizeof(Settings.disc_path), "%s/images/disc/", bootDevice);
        snprintf(Settings.titlestxt_path, sizeof(Settings.titlestxt_path), "%s/config/", bootDevice);//default path for disc images
        char * empty = "";
        snprintf(Settings.unlockCode, sizeof(Settings.unlockCode), empty);		// default password
        snprintf(Settings.language_path, sizeof(Settings.language_path), "notset");
        snprintf(Settings.languagefiles_path, sizeof(Settings.languagefiles_path), "%s/config/language/", bootDevice);
        snprintf(Settings.oggload_path, sizeof(Settings.oggload_path), "%s/config/backgroundmusic/", bootDevice);
        snprintf(Settings.update_path, sizeof(Settings.update_path), "%s/apps/usbloader_gx/", bootDevice);
        snprintf(Settings.homebrewapps_path, sizeof(Settings.homebrewapps_path), "%s/apps/", bootDevice);
        snprintf(Settings.Cheatcodespath, sizeof(Settings.Cheatcodespath), "%s/codes/", bootDevice);
        snprintf(Settings.TxtCheatcodespath, sizeof(Settings.TxtCheatcodespath), "%s/txtcodes/", bootDevice);
        snprintf(Settings.dolpath, sizeof(Settings.dolpath), "%s/", bootDevice);
        sprintf(Settings.ogg_path, "notset");
	}
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
	
	THEME.clock = (GXColor) {138, 138, 138, 240};
	THEME.clock_align = CFG_ALIGN_CENTRE;
	THEME.clock_x = 0;
	THEME.clock_y = 335;//330;


	THEME.info = (GXColor) {55, 190, 237, 255};
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

	THEME.prompttext = (GXColor) {0, 0, 0, 255};
	THEME.settingstext = (GXColor) {0, 0, 0, 255};
	THEME.gametext = (GXColor) {0, 0, 0, 255};

	THEME.pagesize = 9;

	THEME.gamelist_favorite_x = CFG.widescreen ? 288 : 260;
	THEME.gamelist_favorite_y = 13;
	THEME.gamelist_search_x = CFG.widescreen ? 320 : 300;
	THEME.gamelist_search_y = 13;
	THEME.gamelist_abc_x = CFG.widescreen ? 352 : 340;
	THEME.gamelist_abc_y = 13;
	THEME.gamelist_count_x = CFG.widescreen ? 384 : 380;
	THEME.gamelist_count_y = 13;
	THEME.gamelist_list_x = CFG.widescreen ? 416 : 420;
	THEME.gamelist_list_y = 13;
	THEME.gamelist_grid_x = CFG.widescreen ? 448 : 460;
	THEME.gamelist_grid_y = 13;
	THEME.gamelist_carousel_x = CFG.widescreen ? 480 : 500;
	THEME.gamelist_carousel_y = 13;

	THEME.gamegrid_favorite_x = CFG.widescreen ? 208 : 180;
	THEME.gamegrid_favorite_y = 13;
	THEME.gamegrid_search_x = CFG.widescreen ? 240 : 220;
	THEME.gamegrid_search_y = 13;
	THEME.gamegrid_abc_x = CFG.widescreen ? 272 : 260;
	THEME.gamegrid_abc_y = 13;
	THEME.gamegrid_count_x = CFG.widescreen ? 304 : 300;
	THEME.gamegrid_count_y = 13;
	THEME.gamegrid_list_x = CFG.widescreen ? 336 : 340;
	THEME.gamegrid_list_y = 13;
	THEME.gamegrid_grid_x = CFG.widescreen ? 368 : 380;
	THEME.gamegrid_grid_y = 13;
	THEME.gamegrid_carousel_x = CFG.widescreen ? 400 : 420;
	THEME.gamegrid_carousel_y = 13;

	THEME.gamecarousel_favorite_x = CFG.widescreen ? 208 : 180;
	THEME.gamecarousel_favorite_y = 13;
	THEME.gamecarousel_search_x = CFG.widescreen ? 240 : 220;
	THEME.gamecarousel_search_y = 13;
	THEME.gamecarousel_abc_x = CFG.widescreen ? 272 : 260;
	THEME.gamecarousel_abc_y = 13;
	THEME.gamecarousel_count_x = CFG.widescreen ? 304 : 300;
	THEME.gamecarousel_count_y = 13;
	THEME.gamecarousel_list_x = CFG.widescreen ? 336 : 340;
	THEME.gamecarousel_list_y = 13;
	THEME.gamecarousel_grid_x = CFG.widescreen ? 368 : 380;
	THEME.gamecarousel_grid_y = 13;
	THEME.gamecarousel_carousel_x = CFG.widescreen ? 400 : 420;
	THEME.gamecarousel_carousel_y = 13;
}

void Global_Default(void) {
    Settings.video = discdefault;
    Settings.vpatch = off;
    Settings.language = ConsoleLangDefault;
    Settings.ocarina = off;
    Settings.hddinfo = hr12;
    Settings.sinfo = ((THEME.show_id) ? GameID : Neither);
    Settings.rumble = RumbleOn;
    if (THEME.show_region) {
        Settings.sinfo = ((Settings.sinfo == GameID) ? Both : GameRegion);
    }
    Settings.volume = 80;
    Settings.sfxvolume = 80;
    Settings.tooltips = TooltipsOn;
    char * empty = "";
    snprintf(Settings.unlockCode, sizeof(Settings.unlockCode), empty);
    Settings.godmode = 1;
    Settings.parentalcontrol = 0;
    Settings.cios = ios249;
    Settings.xflip = no;
    Settings.qboot = no;
    Settings.wiilight = 1;
    Settings.autonetwork = 0;
    Settings.discart = 0;
    Settings.patchcountrystrings = 0;
    Settings.gridRows = 3;
    Settings.error002 = 0;
    Settings.titlesOverride = 0;
    snprintf(Settings.db_url, sizeof(Settings.db_url), empty);
    snprintf(Settings.db_language, sizeof(Settings.db_language), empty);
    Settings.db_JPtoEN = 0;
    Settings.screensaver = 3;
}


char *cfg_get_title(u8 *id) {
    int i;
    for (i=0; i<num_title; i++) {
        if (memcmp(id, cfg_title[i].id, 4) == 0) {
            return cfg_title[i].title;
        }
    }
    return NULL;
}

char *get_title(struct discHdr *header) {
    char *title = cfg_get_title(header->id);
    if (title) return title;
    return header->title;
}

void title_set(char *id, char *title) {
    char *idt = cfg_get_title((u8*)id);
    if (idt) {
        // replace
        strcopy(idt, title, TITLE_MAX);
    } else {
        cfg_title = realloc(cfg_title, (num_title+1) * sizeof(struct ID_Title));
        if (!cfg_title) {
            // error
            num_title = 0;
            return;
        }
        // add
        memcpy(cfg_title[num_title].id, id, 4);
        cfg_title[num_title].id[4] = 0;
        strcopy(cfg_title[num_title].title, title, TITLE_MAX);
        num_title++;
    }
}

void titles_default() {
	int i;	
	for (i=0; i<num_title; i++) {
        memcpy(cfg_title[i].id, "", 4);
        cfg_title[i].id[4] = 0;
        strcopy(cfg_title[i].title, "", TITLE_MAX);
    }
}

u8 cfg_get_block(u8 *id) {
    int i;
    for (i=0; i<num_control; i++) {
        if (memcmp(id, cfg_control[i].id, 4) == 0) {
            return cfg_control[i].block;
        }
    }
    return 0;
}

u8 get_block(struct discHdr *header) {
    return cfg_get_block(header->id);
}

// trim leading and trailing whitespace
// copy at max n or at max size-1
char* trim_n_copy(char *dest, char *src, int n, int size) {
    int len;
    // trim leading white space
    while (isspace(*src)) {
        src++;
        n--;
    }
    len = strlen(src);
    if (len > n) len = n;
    // trim trailing white space
    while (len > 0 && isspace(src[len-1])) len--;
    if (len >= size) len = size-1;
    strncpy(dest, src, len);
    dest[len] = 0;
    //printf("trim_copy: '%s' %d\n", dest, len); //sleep(1);
    return dest;
}

char* trimcopy(char *dest, char *src, int size) {
    int len;
    while (*src == ' ') src++;
    len = strlen(src);
    // trim trailing " \r\n"
    while (len > 0 && strchr(" \r\n", src[len-1])) len--;
    if (len >= size) len = size-1;
    strncpy(dest, src, len);
    dest[len] = 0;
    return dest;
}

void widescreen_set(char *name, char *val) {
    cfg_name = name;
    cfg_val = val;

    short widescreen;
    if (cfg_bool("widescreen", &widescreen) && CFG.widescreen != widescreen)
        CFG_Default(widescreen); //reset default when forced an other Screenmode
}



void path_set(char *name, char *val) {
    cfg_name = name;
    cfg_val = val;

    // if these are defined in txt file, use them.  otherwise use defaults

    if (!CFG.widescreen &&(strcmp(name, "theme_path") == 0)) {// if in 4:3
        strcopy(CFG.theme_path, val, sizeof(CFG.theme_path));
        return;
    }

    if (CFG.widescreen && strcmp(name, "wtheme_path") == 0) { // if in 16:9
        strcopy(CFG.theme_path, val, sizeof(CFG.theme_path));
        return;
    }

    if (strcmp(name, "cover_path") == 0) {
        strcopy(Settings.covers_path, val, sizeof(Settings.covers_path));
        return;
    }

   if (strcmp(name, "cover2d_path") == 0) {
        strcopy(Settings.covers2d_path, val, sizeof(Settings.covers2d_path));
        return;
    }

    if (strcmp(name, "disc_path") == 0) {
        strcopy(Settings.disc_path, val, sizeof(Settings.disc_path));
        return;
    }
    if (strcmp(name, "titlestxt_path") == 0) {
        strcopy(Settings.titlestxt_path, val, sizeof(Settings.titlestxt_path));
        return;
    }
    if (strcmp(name, "language_path") == 0) {
        strcopy(Settings.language_path, val, sizeof(Settings.language_path));
        return;
    }
    if (strcmp(name, "languagefiles_path") == 0) {
        strcopy(Settings.languagefiles_path, val, sizeof(Settings.languagefiles_path));
        return;
    }
    if (strcmp(name, "update_path") == 0) {
        strcopy(Settings.update_path, val, sizeof(Settings.update_path));
        return;
    }
    if (strcmp(name, "homebrewapps_path") == 0) {
        strcopy(Settings.homebrewapps_path, val, sizeof(Settings.homebrewapps_path));
        return;
    }
    if (strcmp(name, "Cheatcodespath") == 0) {
        strcopy(Settings.Cheatcodespath, val, sizeof(Settings.Cheatcodespath));
        return;
    }
    if (strcmp(name, "TxtCheatcodespath") == 0) {
        strcopy(Settings.TxtCheatcodespath, val, sizeof(Settings.TxtCheatcodespath));
        return;
    }
    if (strcmp(name, "oggload_path") == 0) {
        strcopy(Settings.oggload_path, val, sizeof(Settings.oggload_path));
        return;
    }
    if (strcmp(name, "dolpath") == 0) {
        strcopy(Settings.dolpath, val, sizeof(Settings.dolpath));
        return;
    }
    if (strcmp(name, "ogg_path") == 0) {
        strcopy(Settings.ogg_path, val, sizeof(Settings.ogg_path));
        return;
    }

    return;

}

#define CFG_COORDS2(name)										\
	if (strcmp(cfg_name, #name "_coords") == 0) {			\
		short x,y;											\
		if (sscanf(val, "%hd,%hd", &x, &y) == 2) {			\
			THEME.name##_x = x;								\
			THEME.name##_y = y;								\
		}													\
	}
#define CFG_COORDS4(name)											\
	if (strcmp(cfg_name, #name "_coords") == 0) {				\
	short x,y,w,h;												\
	if (sscanf(val, "%hd,%hd,%hd,%hd", &x, &y, &w, &h) == 4) {	\
		THEME.name##_x = x;										\
		THEME.name##_y = y;										\
		THEME.name##_w = w;										\
		THEME.name##_h = h;										\
		}														\
	}
#define CFG_COLOR(name)												\
	if (strcmp(cfg_name, #name "_color") == 0) {					\
		short r,g,b,a;											\
		int c = sscanf(val, "%hd,%hd,%hd,%hd", &r, &g, &b, &a);	\
		if(c >= 3) {											\
			THEME.name.r = r;									\
			THEME.name.g = g;									\
			THEME.name.b = b;									\
			if(c >= 4)											\
				THEME.name.a = a;								\
		}														\
	}
#define CFG_VAL(name)											\
	if (strcmp(cfg_name, #name) == 0) {					\
		short v;												\
		if (sscanf(val, "%hd", &v) == 1) {						\
			THEME.name = v;									\
		}														\
	}



#define CFG_BOOL(name)	if(cfg_bool(#name, &THEME.name));

#define CFG_ALIGN(name) if(cfg_map_auto(#name "_align", map_alignment, &THEME.name##_align));

#define OLD_FAV_ICON		 1
#define OLD_ABC_ICON		 2
#define OLD_COUNT_ICON		 4
#define OLD_LIST_ICON		 8
#define OLD_GRID_ICON		16
#define OLD_CAROUSEL_ICON	32
short WorkAroundIconSet=0;
short WorkAroundBarOffset=100;

void theme_set(char *name, char *val) {
	cfg_name = name;
	cfg_val = val;

	CFG_COORDS4(gamelist)
	else CFG_COORDS4(gamegrid)
	else CFG_COORDS4(gamecarousel)
	
	else CFG_COORDS2(covers)

	else CFG_BOOL(show_id)
	else CFG_COORDS2(id)

	else CFG_BOOL(show_region)
	else CFG_COORDS2(region)

	else CFG_COORDS2(sdcard)
	else CFG_COORDS2(homebrew)
	else CFG_COORDS2(power)
	else CFG_COORDS2(home)
	else CFG_COORDS2(setting)
	else CFG_COORDS2(install)
	
	else CFG_COORDS2(clock)
	else CFG_ALIGN(clock)
	else CFG_COLOR(clock)

	else CFG_COLOR(info)
	else CFG_BOOL(show_hddinfo)
	else CFG_ALIGN(hddinfo)
	else CFG_COORDS2(hddinfo)

	else CFG_BOOL(show_gamecount)
	else CFG_ALIGN(gamecount)
	else CFG_COORDS2(gamecount)

	else CFG_BOOL(show_tooltip)
	else CFG_VAL(tooltipAlpha)

	else CFG_COLOR(prompttext)
	else CFG_COLOR(settingstext)
	else CFG_COLOR(gametext)

	else CFG_VAL(pagesize)

	else CFG_COORDS2(gamelist_favorite)
	else CFG_COORDS2(gamegrid_favorite)
	else CFG_COORDS2(gamecarousel_favorite)

	else CFG_COORDS2(gamelist_search)
	else CFG_COORDS2(gamegrid_search)
	else CFG_COORDS2(gamecarousel_search)
	
	else CFG_COORDS2(gamelist_abc)
	else CFG_COORDS2(gamegrid_abc)
	else CFG_COORDS2(gamecarousel_abc)

	else CFG_COORDS2(gamelist_count)
	else CFG_COORDS2(gamegrid_count)
	else CFG_COORDS2(gamecarousel_count)

	else CFG_COORDS2(gamelist_list)
	else CFG_COORDS2(gamegrid_list)
	else CFG_COORDS2(gamecarousel_list)

	else CFG_COORDS2(gamelist_grid)
	else CFG_COORDS2(gamegrid_grid)
	else CFG_COORDS2(gamecarousel_grid)

	else CFG_COORDS2(gamelist_carousel)
	else CFG_COORDS2(gamegrid_carousel)
	else CFG_COORDS2(gamecarousel_carousel)

	//**********************************
	// Workaround for old Themes
	//**********************************
	else if (strcmp(cfg_name, "favorite_coords") == 0) {
		short x,y;
		if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
			if(!CFG.widescreen) x-=20;
			// old themes have no search_coords
			// set the searchIcon to the Position of the favIcon
			THEME.gamelist_search_x = x;
			THEME.gamegrid_search_x = THEME.gamecarousel_search_x = x-WorkAroundBarOffset;
			THEME.gamelist_search_y = THEME.gamegrid_search_y = THEME.gamecarousel_search_y = y;
			// place the favIcon to the left side of the searchIcon
			if(!CFG.widescreen) x-= CFG.widescreen ? 32 : 40;
			THEME.gamelist_favorite_x = x;
			THEME.gamegrid_favorite_x = THEME.gamecarousel_favorite_x = x-WorkAroundBarOffset;
			THEME.gamelist_favorite_y = THEME.gamegrid_favorite_y = THEME.gamecarousel_favorite_y = y;
			WorkAroundIconSet |= OLD_FAV_ICON;
		}
	}
	else if (strcmp(cfg_name, "abc_coords") == 0) {
		short x,y;
		if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
			if(!CFG.widescreen) x-=12;
			THEME.gamelist_abc_x = x;
			THEME.gamegrid_abc_x = THEME.gamecarousel_abc_x = x-WorkAroundBarOffset;
			THEME.gamelist_abc_y = THEME.gamegrid_abc_y = THEME.gamecarousel_abc_y = y;
			WorkAroundIconSet |= OLD_ABC_ICON;
		}
	}
	else if (strcmp(cfg_name, "count_coords") == 0) {
		short x,y;
		if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
			if(!CFG.widescreen) x-=4;
			THEME.gamelist_count_x = x;
			THEME.gamegrid_count_x = THEME.gamecarousel_count_x = x-WorkAroundBarOffset;
			THEME.gamelist_count_y = THEME.gamegrid_count_y = THEME.gamecarousel_count_y = y;
			WorkAroundIconSet |= OLD_COUNT_ICON;
		}
	}
	else if (strcmp(cfg_name, "list_coords") == 0) {
		short x,y;
		if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
			if(!CFG.widescreen) x+=4;
			THEME.gamelist_list_x = x;
			THEME.gamegrid_list_x = THEME.gamecarousel_list_x = x-WorkAroundBarOffset;
			THEME.gamelist_list_y = THEME.gamegrid_list_y = THEME.gamecarousel_list_y = y;
			WorkAroundIconSet |= OLD_LIST_ICON;
		}
	}
	else if (strcmp(cfg_name, "grid_coords") == 0) {
		short x,y;
		if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
			if(!CFG.widescreen) x+=12;
			THEME.gamelist_grid_x = x;
			THEME.gamegrid_grid_x = THEME.gamecarousel_grid_x = x-WorkAroundBarOffset;
			THEME.gamelist_grid_y = THEME.gamegrid_grid_y = THEME.gamecarousel_grid_y = y;
			WorkAroundIconSet |= OLD_GRID_ICON;
		}
	}
	else if (strcmp(cfg_name, "carousel_coords") == 0) {
		short x,y;
		if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
			if(!CFG.widescreen) x+=20;
			THEME.gamelist_carousel_x = x;
			THEME.gamegrid_carousel_x = THEME.gamecarousel_carousel_x = x-WorkAroundBarOffset;
			THEME.gamelist_carousel_y = THEME.gamegrid_carousel_y = THEME.gamecarousel_carousel_y = y;
			WorkAroundIconSet |= OLD_CAROUSEL_ICON;
		}
	}

	else if (strcmp(cfg_name, "sortBarOffset") == 0) {
		short o;
		if (sscanf(val, "%hd", &o) == 1) {
			if(WorkAroundIconSet & OLD_FAV_ICON)
			{
				THEME.gamegrid_favorite_x += WorkAroundBarOffset - o;
				THEME.gamecarousel_favorite_x += WorkAroundBarOffset - o;
				THEME.gamegrid_search_x += WorkAroundBarOffset - o;
				THEME.gamecarousel_search_x += WorkAroundBarOffset - o;
			}
			if(WorkAroundIconSet & OLD_ABC_ICON)
			{
				THEME.gamegrid_abc_x += WorkAroundBarOffset - o;
				THEME.gamecarousel_abc_x += WorkAroundBarOffset - o;
			}
			if(WorkAroundIconSet & OLD_COUNT_ICON)
			{
				THEME.gamegrid_count_x += WorkAroundBarOffset - o;
				THEME.gamecarousel_count_x += WorkAroundBarOffset - o;
			}
			if(WorkAroundIconSet & OLD_LIST_ICON)
			{
				THEME.gamegrid_list_x += WorkAroundBarOffset - o;
				THEME.gamecarousel_list_x += WorkAroundBarOffset - o;
			}
			if(WorkAroundIconSet & OLD_GRID_ICON)
			{
				THEME.gamegrid_grid_x += WorkAroundBarOffset - o;
				THEME.gamecarousel_grid_x += WorkAroundBarOffset - o;
			}
			if(WorkAroundIconSet & OLD_CAROUSEL_ICON)
			{
				THEME.gamegrid_carousel_x += WorkAroundBarOffset - o;
				THEME.gamecarousel_carousel_x += WorkAroundBarOffset - o;
			}
			WorkAroundBarOffset = o;
		}
	}


}

void global_cfg_set(char *name, char *val) {
    cfg_name = name;
    cfg_val = val;

    if (strcmp(name, "video") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.video = i;
        }
        return;
    } else if (strcmp(name, "vpatch") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.vpatch =i;
        }
        return;
    }

    else if (strcmp(name, "language") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.language = i;
        }
        return;
    } else if (strcmp(name, "ocarina") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.ocarina = i;
        }
        return;
    } else if (strcmp(name, "sort") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.sort = i;
        }
        return;
    } else if (strcmp(name, "fave") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.fave = i;
        }
        return;
    } else if (strcmp(name, "keyset") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.keyset = i;
        }
        return;
    } else if (strcmp(name, "hddinfo") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.hddinfo = i;
        }
        return;
    } else if (strcmp(name, "sinfo") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.sinfo = i;
        }
        return;
    } else if (strcmp(name, "rumble") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.rumble = i;
        }
        return;
    } else if (strcmp(name, "volume") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.volume = i;
        }
        return;
    } else if (strcmp(name, "sfxvolume") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.sfxvolume = i;
        }
        return;
    } else if (strcmp(name, "tooltips") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.tooltips = i;
        }
        return;
    } else if (strcmp(name, "password") == 0) {
        strcopy(Settings.unlockCode, val, sizeof(Settings.unlockCode));
        return;
    } else if (strcmp(name, "parentalcontrol") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.parentalcontrol = i;
        }
        return;
    } else if (strcmp(name, "cios") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.cios = i;
        }
        return;
    } else if (strcmp(name, "gridRows") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.gridRows = i;
        }
        return;
    } else if (strcmp(name, "xflip") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.xflip = i;
        }
        return;
    } else if (strcmp(name, "qboot") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.qboot = i;
        }
        return;
    } else if (strcmp(name, "wsprompt") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.wsprompt = i;
        }
        return;
    } else if (strcmp(name, "wiilight") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.wiilight = i;
        }
        return;
    } else if (strcmp(name, "autonetwork") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.autonetwork = i;
        }
        return;
    } else if (strcmp(name, "discart") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.discart = i;
        }
        return;
    } else if (strcmp(name, "patchcountrystrings") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.patchcountrystrings = i;
        }
        return;
    } else if (strcmp(name, "error002") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.error002 = i;
        }
        return;
    } else if (strcmp(name, "titlesOverride") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.titlesOverride = i;
        }
        return;
    } else if (strcmp(name, "db_JPtoEN") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.db_JPtoEN = i;
        }
        return;
    } else if (strcmp(name, "gameDisplay") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.gameDisplay = i;
        }
        return;
    } else if (strcmp(name, "screensaver") == 0) {
        int i;
        if (sscanf(val, "%d", &i) == 1) {
            Settings.screensaver = i;
        }
        return;
    }

    cfg_bool("godmode", &Settings.godmode);

    return;
}

// split line to part1 delimiter part2
bool trimsplit(char *line, char *part1, char *part2, char delim, int size) {
    char *eq = strchr(line, delim);
    if (!eq) return false;
    trim_n_copy(part1, line, eq-line, size);
    trimcopy(part2, eq+1, size);
    return true;
}
void cfg_parseline(char *line, void (*set_func)(char*, char*)) {
    // split name = value
    char tmp[300], name[200], val[200];
    strcopy(tmp, line, sizeof(tmp));
    char *eq = strchr(tmp, '=');
    if (!eq) return;
    *eq = 0;
    trimcopy(name, tmp, sizeof(name));
    trimcopy(val, eq+1, sizeof(val));
    //printf("CFG: %s = %s\n", name, val);
    set_func(name, val);
}

void cfg_parsetitleline(char *line, void (*set_func)(char*, char*, u8)) {
    // split name = value
    char tmp[200], name[200], val[200];
    int block = 0;
    strcopy(tmp, line, sizeof(tmp));
    char *eq = strchr(tmp, '=');
    if (!eq) return;
    *eq = 0;
    trimcopy(name, tmp, sizeof(name));

    char *blockpos = strrchr(eq+1, '=');

    if (!blockpos)
        trimcopy(val, eq+1, sizeof(val));

    else {
        *blockpos = 0;
        trimcopy(val, eq+1, sizeof(val));
        if (sscanf(blockpos+1, "%d", &block) != 1) {
            block = 0;
        }
    }
    set_func(name, val, block);
}

bool cfg_parsefile(char *fname, void (*set_func)(char*, char*)) {
    FILE *f;
    char line[300];

    //printf("opening(%s)\n", fname);
    f = fopen(fname, "r");
    if (!f) {
        //printf("error opening(%s)\n", fname);
        return false;
    }
    while (fgets(line, sizeof(line), f)) {
        // lines starting with # are comments
        if (line[0] == '#') continue;
        cfg_parseline(line, set_func);
    }
    fclose(f);
    return true;
}

bool cfg_parsetitlefile(char *fname, void (*set_func)(char*, char*, u8)) {
    FILE *f;
    char line[200];

    //printf("opening(%s)\n", fname);
    f = fopen(fname, "r");
    if (!f) {
        //printf("error opening(%s)\n", fname);
        return false;
    }

    while (fgets(line, sizeof(line), f)) {
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
struct Game_CFG* cfg_get_game(u8 *id) {
    struct Game_CFG *game = CFG_get_game_opt(id);
    if (game) return game;
    if (num_saved_games >= MAX_SAVED_GAMES) return NULL;
    game = &cfg_game[num_saved_games];
    num_saved_games++;
    return game;
}

// current options to game
void cfg_set_game_opt(struct Game_CFG *game, u8 *id) {
    strncpy((char*)game->id, (char*)id, 6);
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
	if (game->loadalternatedol == 0) {
		alternatedoloffset = 0;
		strcpy(alternatedname, "");
	}
    game->alternatedolstart = alternatedoloffset;
    strlcpy(game->alternatedolname, alternatedname,sizeof(game->alternatedolname));
}

struct Game_NUM* cfg_get_game_num(u8 *id) {
    struct Game_NUM *game = CFG_get_game_num(id);
    if (game) return game;
    if (num_saved_game_num >= MAX_SAVED_GAME_NUM) return NULL;
    game = &cfg_game_num[num_saved_game_num];
    num_saved_game_num++;
    return game;
}

// current options to game
void cfg_set_game_num(struct Game_NUM *game, u8 *id) {
    strncpy((char*)game->id, (char*)id, 6);
    game->id[6] = 0;
    game->favorite = favoritevar;
    game->count = playcount;
}


bool cfg_save_global() { // save global settings
    char GXGlobal_cfg[26];
    sprintf(GXGlobal_cfg, "%s/config", bootDevice);
    struct stat st;
    if (stat(GXGlobal_cfg, &st) != 0) {
        mkdir(GXGlobal_cfg, 0777);
    }
    FILE *f;
    sprintf(GXGlobal_cfg, "%s/config/GXGlobal.cfg", bootDevice);
    f = fopen(GXGlobal_cfg, "w");
    if (!f) {
        printf("Error saving %s\n", GXGlobal_cfg);
        sleep(1);
        return false;
    }
    fprintf(f, "# USB Loader global settings file\n");
    fprintf(f, "# Note: This file is automatically generated\n");
    fclose(f);
    /* Closing and reopening because of a write issue we are having right now */
    f = fopen(GXGlobal_cfg, "w");
    fprintf(f, "# USB Loader global settings file\n");
    fprintf(f, "# Note: This file is automatically generated\n");
    fprintf(f, "video = %d\n ", Settings.video);
    fprintf(f, "vpatch = %d\n ", Settings.vpatch);
    fprintf(f, "language = %d\n ", Settings.language);
    fprintf(f, "ocarina = %d\n ", Settings.ocarina);
    fprintf(f, "hddinfo = %d\n ", Settings.hddinfo);
    fprintf(f, "sinfo = %d\n ", Settings.sinfo);
    fprintf(f, "rumble = %d\n ", Settings.rumble);
    fprintf(f, "volume = %d\n ", Settings.volume);
    fprintf(f, "sfxvolume = %d\n ", Settings.sfxvolume);
    fprintf(f, "tooltips = %d\n ", Settings.tooltips);
    fprintf(f, "password = %s\n ", Settings.unlockCode);
    fprintf(f, "sort = %d\n ", Settings.sort);
    fprintf(f, "fave = %d\n ", Settings.fave);
    fprintf(f, "cios = %d\n ", Settings.cios);
    fprintf(f, "keyset = %d\n ", Settings.keyset);
    fprintf(f, "xflip = %d\n ", Settings.xflip);
    fprintf(f, "gridRows = %d\n ", Settings.gridRows);
    fprintf(f, "qboot = %d\n ", Settings.qboot);
    fprintf(f, "wsprompt = %d\n", Settings.wsprompt);
    fprintf(f, "parentalcontrol = %d\n ", Settings.parentalcontrol);
    fprintf(f, "cover_path = %s\n ", Settings.covers_path);
    fprintf(f, "cover2d_path = %s\n ", Settings.covers2d_path);
    if (CFG.widescreen) {
        fprintf(f, "wtheme_path = %s\n ", CFG.theme_path);
    } else {
        fprintf(f, "theme_path = %s\n ", CFG.theme_path);
    }
    fprintf(f, "disc_path = %s\n ", Settings.disc_path);
    fprintf(f, "language_path = %s\n ", Settings.language_path);
    fprintf(f, "languagefiles_path = %s\n ", Settings.languagefiles_path);
    fprintf(f, "oggload_path = %s\n ", Settings.oggload_path);
    fprintf(f, "TxtCheatcodespath = %s\n ", Settings.TxtCheatcodespath);
    fprintf(f, "titlestxt_path = %s\n ", Settings.titlestxt_path);
    if (!strcmp("", Settings.unlockCode)) {
        fprintf(f, "godmode = %d\n ", Settings.godmode);
    } else {
        fprintf(f, "godmode = %d\n ", 0);
    }
    fprintf(f, "dolpath = %s\n ", Settings.dolpath);
    fprintf(f, "ogg_path = %s\n ", Settings.ogg_path);
    fprintf(f, "wiilight = %d\n ", Settings.wiilight);
    fprintf(f, "gameDisplay = %d\n ", Settings.gameDisplay);
    fprintf(f, "update_path = %s\n ", Settings.update_path);
    fprintf(f, "homebrewapps_path = %s\n ", Settings.homebrewapps_path);
    fprintf(f, "Cheatcodespath = %s\n ", Settings.Cheatcodespath);
    fprintf(f, "titlesOverride = %d\n ", Settings.titlesOverride);
    //fprintf(f, "db_url = %s\n ", Settings.db_url);
    //fprintf(f, "db_JPtoEN = %d\n ", Settings.db_JPtoEN);
    //fprintf(f, "db_language = %d\n ", Settings.language);
    fprintf(f, "patchcountrystrings = %d\n ", Settings.patchcountrystrings);
    fprintf(f, "screensaver = %d\n ", Settings.screensaver);
    fprintf(f, "error002 = %d\n ", Settings.error002);
    fprintf(f, "autonetwork = %d\n ", Settings.autonetwork);
    fprintf(f, "discart = %d\n ", Settings.discart);
    fclose(f);
    return true;
}

void game_set(char *name, char *val) {
    // sample line:
    // game:RTNP41 = video:game; language:english; ocarina:0;
    // game:RYWP01 = video:patch; language:console; ocarina:1;
    //printf("GAME: '%s=%s'\n", name, val);
    u8 id[8];
    struct Game_CFG *game;
    if (strncmp(name, "game:", 5) != 0) return;
    trimcopy((char*)id, name+5, sizeof(id));
    game = cfg_get_game(id);
    // set id and current options as default
    cfg_set_game_opt(game, id);
    //printf("GAME(%s) '%s'\n", id, val); sleep(1);

    // parse val
    // first split options by ;
    char opt[300], *p, *np;
    p = val;

    while (p) {
        np = strchr(p, ';');
        if (np) trim_n_copy(opt, p, np-p, sizeof(opt));
        else trimcopy(opt, p, sizeof(opt));
        //printf("GAME(%s) (%s)\n", id, opt); sleep(1);
        // parse opt 'language:english'
        char opt_name[200], opt_val[200];
        if (trimsplit(opt, opt_name, opt_val, ':', sizeof(opt_name))) {
            //printf("GAME(%s) (%s=%s)\n", id, opt_name, opt_val); sleep(1);
            short opt_v, opt_l, opt_c;
            if (map_auto("video", opt_name, opt_val, map_video, &opt_v)) {
                // valid option, assign
                game->video = opt_v;
            }
            if (map_auto("language", opt_name, opt_val, map_language, &opt_l)) {
                // valid option, assign
                game->language = opt_l;
            }
            if (strcmp("ocarina", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    game->ocarina = opt_c;
                }
            }
            if (strcmp("vipatch", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    game->vipatch = opt_c;
                }
            }
            if (strcmp("ios", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    game->ios = opt_c;
                }
            }
            if (strcmp("pctrl", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    game->parentalcontrol = opt_c;
                }
            }
            if (strcmp("errorfix002", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    game->errorfix002 = opt_c;
                }
            }
            if (strcmp("iosreloadblock", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    game->iosreloadblock = opt_c;
                }
            }
            if (strcmp("patchcountrystrings", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    game->patchcountrystrings = opt_c;
                }
            }
            if (strcmp("loadalternatedol", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    game->loadalternatedol = opt_c;
                }
            }
            if (strcmp("alternatedolstart", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    game->alternatedolstart = opt_c;
                }
            }
            if (strcmp("alternatedolname", opt_name) == 0) {
                strlcpy(game->alternatedolname, opt_val, sizeof(game->alternatedolname));
            }
        }
        // next opt
        if (np) p = np + 1;
        else p = NULL;
    }
}

void parental_set(char *name, char *val) {
    // sample line:
    // game:RTNP41 = video:game; language:english; ocarina:0;
    // game:RYWP01 = video:patch; language:console; ocarina:1;
    //printf("GAME: '%s=%s'\n", name, val);
    u8 id[8];

    if (strncmp(name, "game:", 5) != 0) return;
    trimcopy((char*)id, name+5, sizeof(id));

    // parse val
    // first split options by ;
    char opt[200], *p, *np;
    p = val;

    while (p) {
        np = strchr(p, ';');
        if (np) trim_n_copy(opt, p, np-p, sizeof(opt));
        else trimcopy(opt, p, sizeof(opt));
        //printf("GAME(%s) (%s)\n", id, opt); sleep(1);
        // parse opt 'language:english'
        char opt_name[200], opt_val[200];
        if (trimsplit(opt, opt_name, opt_val, ':', sizeof(opt_name))) {
            //printf("GAME(%s) (%s=%s)\n", id, opt_name, opt_val); sleep(1);
            short opt_c;

            if (strcmp("pctrl", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    cfg_control = realloc(cfg_control, (num_control+1) * sizeof(struct ID_Control));
                    if (!cfg_control) {
                        // error
                        num_control = 0;
                        return;
                    }
                    // add
                    memcpy(cfg_control[num_control].id, id, 4);
                    cfg_control[num_control].id[4] = 0;
                    cfg_control[num_control].block = opt_c;
                    num_control++;
                }
            }

        }
        // next opt
        if (np) p = np + 1;
        else p = NULL;
    }
}

void game_set_num(char *name, char *val) {
    u8 id[8];
    struct Game_NUM *game;
    if (strncmp(name, "game:", 5) != 0) return;
    trimcopy((char*)id, name+5, sizeof(id));
    game = cfg_get_game_num(id);

    cfg_set_game_num(game, id);


    // parse val
    // first split options by ;
    char opt[200], *p, *np;
    p = val;

    while (p) {
        np = strchr(p, ';');
        if (np) trim_n_copy(opt, p, np-p, sizeof(opt));
        else trimcopy(opt, p, sizeof(opt));

        char opt_name[200], opt_val[200];
        if (trimsplit(opt, opt_name, opt_val, ':', sizeof(opt_name))) {

            short opt_c;
            if (strcmp("favorite", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    game->favorite = opt_c;
                }
            }
            if (strcmp("count", opt_name) == 0) {
                if (sscanf(opt_val, "%hd", &opt_c) == 1) {
                    game->count = opt_c;
                }
            }
        }

        if (np) p = np + 1;
        else p = NULL;
    }
}

bool cfg_load_games() {
    char GXGameSettings_cfg[32];
    sprintf(GXGameSettings_cfg, "%s/config/GXGameSettings.cfg", bootDevice);
    return cfg_parsefile(GXGameSettings_cfg, &game_set);
}

bool cfg_load_game_num() {
    char GXGameFavorites_cfg[32];
    sprintf(GXGameFavorites_cfg, "%s/config/GXGameFavorites.cfg", bootDevice);
    return cfg_parsefile(GXGameFavorites_cfg, &game_set_num);
}

bool cfg_save_games() {
    FILE *f;
    int i;
    char GXGameSettings_cfg[50];
    sprintf(GXGameSettings_cfg, "%s/config", bootDevice);
    mkdir(GXGameSettings_cfg, 0777);

    sprintf(GXGameSettings_cfg, "%s/config/GXGameSettings.cfg", bootDevice);
    f = fopen(GXGameSettings_cfg, "w");
    if (!f) {
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
    for (i=0; i<num_saved_games; i++) {
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
    }
    fprintf(f, "# END\n");
    fclose(f);
    return true;
}

bool cfg_save_game_num() {
    FILE *f;
    int i;
    char GXGameFavorites_cfg[32];
    sprintf(GXGameFavorites_cfg, "%s/config", bootDevice);
    mkdir(GXGameFavorites_cfg, 0777);

    sprintf(GXGameFavorites_cfg, "%s/config/GXGameFavorites.cfg", bootDevice);
    f = fopen(GXGameFavorites_cfg, "w");
    if (!f) {
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
    for (i=0; i<num_saved_game_num; i++) {
        fprintf(f, "game:%s = ", cfg_game_num[i].id);
        fprintf(f, "favorite:%d; ", cfg_game_num[i].favorite);
        fprintf(f, "count:%d;\n", cfg_game_num[i].count);
    }
    fprintf(f, "# END\n");
    fclose(f);
    return true;
}

bool CFG_reset_all_playcounters() {
    FILE *f;
    int i;
    char GXGameFavorites_cfg[32];
    sprintf(GXGameFavorites_cfg, "%s/config", bootDevice);
    mkdir(GXGameFavorites_cfg, 0777);

    sprintf(GXGameFavorites_cfg, "%s/config/GXGameFavorites.cfg", bootDevice);
    f = fopen(GXGameFavorites_cfg, "w");
    if (!f) {
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
    for (i=0; i<num_saved_game_num; i++) {
        fprintf(f, "game:%s = ", cfg_game_num[i].id);
        fprintf(f, "favorite:%d; ", cfg_game_num[i].favorite);
        fprintf(f, "count:0;\n");
    }
    fprintf(f, "# END\n");
    fclose(f);
    return true;
}

bool cfg_load_global() {
    char GXGlobal_cfg[26];
    sprintf(GXGlobal_cfg, "%s/config/GXGlobal.cfg", bootDevice);
    //Default values defined by dev team
    Settings.video = discdefault;
    Settings.vpatch = off;
    Settings.language = ConsoleLangDefault;
    Settings.ocarina = off;
    Settings.xflip = off;
    Settings.qboot = off;
    Settings.wsprompt = off;
    Settings.keyset = us;
    Settings.hddinfo = hr12;
    Settings.gameDisplay = list;
    Settings.sinfo = ((THEME.show_id) ? GameID : Neither);
    Settings.rumble = RumbleOn;
    if (THEME.show_region) {
        Settings.sinfo = ((Settings.sinfo == GameID) ? Both : GameRegion);
    }
    Settings.volume = 80;
    Settings.sfxvolume = 80;

    Settings.titlesOverride = 0;
    char * empty = "";
    snprintf(Settings.db_url, sizeof(Settings.db_url), empty);
    snprintf(Settings.db_language, sizeof(Settings.db_language), empty);
    Settings.db_JPtoEN = 0;
    return cfg_parsefile(GXGlobal_cfg, &global_cfg_set);
}



struct Game_CFG* CFG_get_game_opt(u8 *id) {
    int i;
    for (i=0; i<num_saved_games; i++) {
        if (memcmp(id, cfg_game[i].id, 6) == 0) {
            return &cfg_game[i];
        }
    }
    return NULL;
}


struct Game_NUM* CFG_get_game_num(u8 *id) {
    int i;
    for (i=0; i<num_saved_game_num; i++) {
        if (memcmp(id, cfg_game_num[i].id, 6) == 0) {
            return &cfg_game_num[i];
        }
    }
    return NULL;
}

bool CFG_save_game_opt(u8 *id) {
    struct Game_CFG *game = cfg_get_game(id);
    if (!game) return false;
    cfg_set_game_opt(game, id);
    return cfg_save_games();
}

bool CFG_save_game_num(u8 *id) {
    struct Game_NUM *game = cfg_get_game_num(id);
    if (!game) return false;
    cfg_set_game_num(game, id);
    return cfg_save_game_num();
}

bool CFG_forget_game_opt(u8 *id) {
    struct Game_CFG *game = CFG_get_game_opt(id);
    int i;
    if (!game) return true;
    // move entries down
    num_saved_games--;
    for (i=game-cfg_game; i<num_saved_games; i++) {
        cfg_game[i] = cfg_game[i+1];
    }
    memset(&cfg_game[num_saved_games], 0, sizeof(struct Game_CFG));
    return cfg_save_games();
}

bool CFG_forget_game_num(u8 *id) {
    struct Game_NUM *game = CFG_get_game_num(id);
    int i;
    if (!game) return true;
    // move entries down
    num_saved_game_num--;
    for (i=game-cfg_game_num; i<num_saved_game_num; i++) {
        cfg_game[i] = cfg_game[i+1];
    }
    memset(&cfg_game[num_saved_game_num], 0, sizeof(struct Game_NUM));
    return cfg_save_game_num();
}


void CFG_Load(void) {
// GUI should be stopped at the time of calling CFG_Load() to prevent default settings from having any effect

    char pathname[200];
//	bool ret = false;

    //set app path
//	chdir_app(argv[0]);

    CFG_Default(-1); // set defaults non forced

    snprintf(pathname, sizeof(pathname), "%s/config/GXGlobal.cfg", bootDevice);

    cfg_parsefile(pathname, &widescreen_set); //first set widescreen
    cfg_parsefile(pathname, &path_set); //then set config and layout options

	WorkAroundIconSet=0; WorkAroundBarOffset=100; // set Workaroundstuff to defaults
   snprintf(pathname, sizeof(pathname), "%sGXtheme.cfg", CFG.theme_path);
    cfg_parsefile(pathname, &theme_set); //finally set theme information

	// set GUI language, use Wii's language setting if language is set to default
	int wiilang;
	bool langisdefault = false;
	wiilang = CONF_GetLanguage();
	if (!strcmp("notset", Settings.language_path)) {
		snprintf(Settings.language_path, sizeof(Settings.language_path), "%s%s.lang", Settings.languagefiles_path, map_get_name(map_language, wiilang + 1)); // + 1 because because CONF_LANG starts at 0
		if (!checkfile(Settings.language_path)) {
			sprintf(Settings.language_path, "notset");
		}
	    gettextLoadLanguage(Settings.language_path);
		langisdefault = true;
	}
	snprintf(pathname, sizeof(pathname), Settings.language_path);
	gettextLoadLanguage(pathname);

//	cfg_parsefile(pathname, &language_set);

    snprintf(pathname, sizeof(pathname), "%s/config/GXGameSettings.cfg", bootDevice);
    cfg_parsefile(pathname, &parental_set);

    // load per-game settings
    cfg_load_games();
    cfg_load_game_num();

    Global_Default(); //global default depends on theme information
    CFG_LoadGlobal();

	// use GUI language for the database (Settings.db_language is used for game info/titles and covers)
	char * languagefile;
    languagefile = strrchr(Settings.language_path, '/')+1;
	int mainlangid = -1;
	int i;
	if(strcmp("notset", Settings.language_path)) {
		for (i=0; map_language[i].name != NULL; i++) {
			if (strstr(languagefile, map_language[i].name) != NULL) {
				mainlangid = i - 1; // - 1 because CONF_LANG starts at 0
				break;
			}
		}
	} else {
        mainlangid = wiilang;
	}
	GetLanguageToLangCode(&mainlangid, Settings.db_language);

	// set language code for countries that don't have a language setting on the Wii
	if (!strcmp(Settings.db_language,"")) {
		if (strstr(languagefile, "portuguese") != NULL)
			strcpy(Settings.db_language,"PT");
	}
	if (CONF_GetArea() == CONF_AREA_AUS)
		strcpy(Settings.db_language,"AU");
		
	// open database if needed, load titles if needed
	OpenXMLDatabase(Settings.titlestxt_path,Settings.db_language, Settings.db_JPtoEN, true, Settings.titlesOverride==1?true:false, true);

    // titles.txt loaded after database to override database titles with custom titles
    //took out this titles.txt shit because it is useless now.  teh xml has all the titles in it
    //snprintf(pathname, sizeof(pathname), "%stitles.txt", Settings.titlestxt_path);
    //cfg_parsefile(pathname, &title_set);

//	cfg_parsearg(argc, argv);
	// if GUI language is set to default Settings.language_path needs to remain "notset" (if the detected setting was kept detection wouldn't work next time)
	if (langisdefault)
		sprintf(Settings.language_path, "notset");
}

void CFG_LoadGlobal(void) {
    char GXGlobal_cfg[26];
    sprintf(GXGlobal_cfg, "%s/config/GXGlobal.cfg", bootDevice);
    cfg_parsefile(GXGlobal_cfg, &global_cfg_set);
}

void CFG_Cleanup(void) {
    if (cfg_title) {
        free(cfg_title);
        cfg_title = NULL;
    }
}


/* map language id (or Wii language settting if langid is set to -1) to language code. CONF_LANG_JAPANESE = 0, not 1 */
void GetLanguageToLangCode(int *langid, char *langcode) {

	if (langid < 0)
		*langid = CONF_GetLanguage();

    switch (*langid) {
    case CONF_LANG_JAPANESE:
        sprintf(langcode, "JA");
        break;
    case CONF_LANG_ENGLISH:
        sprintf(langcode, "EN");
        break;
    case CONF_LANG_GERMAN:
        sprintf(langcode, "DE");
        break;
    case CONF_LANG_FRENCH:
        sprintf(langcode, "FR");
        break;
    case CONF_LANG_SPANISH:
        sprintf(langcode, "ES");
        break;
    case CONF_LANG_ITALIAN:
        sprintf(langcode, "IT");
        break;
    case CONF_LANG_DUTCH:
        sprintf(langcode, "NL");
        break;
    case CONF_LANG_SIMP_CHINESE:
        sprintf(langcode, "ZHCN"); // People's Republic of China
        break;
    case CONF_LANG_TRAD_CHINESE:
        sprintf(langcode, "ZHTW"); // Taiwan
        break;
    case CONF_LANG_KOREAN:
        sprintf(langcode, "KO");
        break;
    }
}
