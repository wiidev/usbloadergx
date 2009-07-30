#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <ogcsys.h>
#include <mxml.h>

#include "language/gettext.h"
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
    { "s.chinese", CFG_LANG_S_CHINESE },
    { "t.chinese", CFG_LANG_T_CHINESE },
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

        //all alignments are left top here
        THEME.selection_x = 200;
        THEME.selection_y = 49;//40;
        THEME.selection_w = 396;
        THEME.selection_h = 280;
        THEME.batteryUnused = 70;
        THEME.gamegrid_w = 640;
        THEME.gamegrid_h = 400;
        THEME.gamegrid_x = 0;
        THEME.gamegrid_y = 20;
        THEME.gamecarousel_w = 640;
        THEME.gamecarousel_h = 400;
        THEME.gamecarousel_x = 0;
        THEME.gamecarousel_y = -20;
        THEME.clock_r = 138;
        THEME.clock_g = 138;
        THEME.clock_b = 138;
        THEME.settingsTxt_r = 0;
        THEME.settingsTxt_g = 0;
        THEME.settingsTxt_b = 0;
        THEME.cover_x = 26;
        THEME.cover_y = 58;
        THEME.homebrew_x = 425;
        THEME.homebrew_y = 400;
        THEME.showID = 1;
        //	THEME.maxcharacters = 36;
        THEME.id_x = 68;
        THEME.id_y = 305;
        THEME.region_x = 68;
        THEME.region_y = 30;
        THEME.power_x = 576;
        THEME.tooltipAlpha = 255;
        THEME.power_y = 355;
        THEME.home_x = 485;//215;
        THEME.home_y = 367;
        THEME.setting_x = 60;//-210
        THEME.setting_y = 367;
        THEME.showHDD = 1; //default
        THEME.showGameCnt = 1; //default
        THEME.showToolTip = 1; //1 means use settings, 0 means force turn off
        THEME.install_x = 16;//-280
        THEME.install_y = 355;
        THEME.showBattery = 1;
        THEME.showRegion = 1;
        THEME.hddInfo_x = 0;
        THEME.hddInfo_y = 410;
        THEME.hddInfoAlign = CFG_ALIGN_CENTRE;
        THEME.gameCnt_x = 0;
        THEME.gameCnt_y = 430;
        THEME.gameCntAlign = CFG_ALIGN_CENTRE;
        THEME.battery1_x = 245;
        THEME.battery1_y = 400;
        THEME.battery2_x = 335;
        THEME.battery2_y = 400;
        THEME.battery3_x = 245;
        THEME.battery3_y = 425;
        THEME.battery4_x = 335;
        THEME.battery4_y = 425;
        THEME.info_r =  55;
        THEME.info_g =  190;
        THEME.info_b =  237;
        THEME.prompttxt_r = 0;
        THEME.prompttxt_g = 0;
        THEME.prompttxt_b = 0;
        THEME.clock_x = 0;
        THEME.clock_y = 335;//330;
        THEME.clockAlign = CFG_ALIGN_CENTRE;
        THEME.sdcard_x = 150;
        THEME.sdcard_y = 390;
        THEME.gameText_r = 0;
        THEME.gameText_g = 0;
        THEME.gameText_b = 0;
        THEME.pagesize = 9;
        THEME.favorite_x = 4;
        THEME.favorite_y = 13;
        THEME.abc_x = 36;
        THEME.abc_y = 13;
        THEME.list_x = 100;
        THEME.list_y = 13;
        THEME.grid_x = 132;
        THEME.grid_y = 13;
        THEME.carousel_x = 164;
        THEME.carousel_y = 13;
        THEME.count_x = 68;
        THEME.count_y = 13;
        THEME.sortBarOffset = 100;
    }
}

void Global_Default(void) {
    Settings.video = discdefault;
    Settings.vpatch = off;
    Settings.language = ConsoleLangDefault;
    Settings.ocarina = off;
    Settings.hddinfo = hr12;
    Settings.sinfo = ((THEME.showID) ? GameID : Neither);
    Settings.rumble = RumbleOn;
    if (THEME.showRegion) {
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



void theme_set(char *name, char *val) {
    cfg_name = name;
    cfg_val = val;

    if (strcmp(cfg_name, "gamelist_coords") == 0) {
        int x,y,w,h;
        if (sscanf(val, "%d,%d,%d,%d", &x, &y, &w, &h) == 4) {
            THEME.selection_x = x - (x % 4);
            THEME.selection_y = y;
            THEME.selection_w = w;
            THEME.selection_h = h;
        }
    }

    if (strcmp(cfg_name, "gamegrid_coords") == 0) {
        int x,y,w,h;
        if (sscanf(val, "%d,%d,%d,%d", &x, &y, &w, &h) == 4) {
            THEME.gamegrid_x = x - (x % 4);
            THEME.gamegrid_y = y;
            THEME.gamegrid_w = w;
            THEME.gamegrid_h = h;
        }
    }

    if (strcmp(cfg_name, "gamecarousel_coords") == 0) {
        int x,y,w,h;
        if (sscanf(val, "%d,%d,%d,%d", &x, &y, &w, &h) == 4) {
            THEME.gamecarousel_x = x - (x % 4);
            THEME.gamecarousel_y = y;
            THEME.gamecarousel_w = w;
            THEME.gamecarousel_h = h;
        }
    }

    else if (strcmp(cfg_name, "covers_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.cover_x = x - (x % 4);
            THEME.cover_y = y;
        }
    }

    else if (strcmp(cfg_name, "id_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.id_x = x - (x % 4);
            THEME.id_y = y;
        }
    }

    else if (strcmp(cfg_name, "hddinfo_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.hddInfo_x = x - (x % 4);
            THEME.hddInfo_y = y;
        }
    }

    else if (strcmp(cfg_name, "gamecount_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.gameCnt_x = x - (x % 4);
            THEME.gameCnt_y = y;
        }
    }

    else if (strcmp(cfg_name, "region_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.region_x = x - (x % 4);
            THEME.region_y = y;
        }
    }

    else if (strcmp(cfg_name, "homebrew_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.homebrew_x = x - (x % 4);
            THEME.homebrew_y = y;
        }
    }

    else if (strcmp(cfg_name, "power_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.power_x = x - (x % 4);
            THEME.power_y = y;
        }
    }

    else if (strcmp(cfg_name, "home_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.home_x = x - (x % 4);
            THEME.home_y = y;
        }
    }

    else if (strcmp(cfg_name, "setting_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.setting_x = x - (x % 4);
            THEME.setting_y = y;
        }
    }

    else if (strcmp(cfg_name, "install_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.install_x = x - (x % 4);
            THEME.install_y = y;
        }
    }

    else if (strcmp(cfg_name, "battery1_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.battery1_x = x - (x % 4);
            THEME.battery1_y = y;
        }
    }

    else if (strcmp(cfg_name, "battery2_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.battery2_x = x - (x % 4);
            THEME.battery2_y = y;
        }
    }

    else if (strcmp(cfg_name, "battery3_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.battery3_x = x - (x % 4);
            THEME.battery3_y = y;
        }
    }

    else if (strcmp(cfg_name, "battery4_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.battery4_x = x - (x % 4);
            THEME.battery4_y = y;
        }
    }

    else if (strcmp(cfg_name, "clock_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.clock_x = x - (x % 4);
            THEME.clock_y = y;
        }
    }

    else if (strcmp(cfg_name, "sdcard_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.sdcard_x = x - (x % 4);
            THEME.sdcard_y = y;
        }
    }

    else if (strcmp(cfg_name, "favorite_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.favorite_x = x - (x % 4);
            THEME.favorite_y = y;
        }
    }

    else if (strcmp(cfg_name, "abc_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.abc_x = x - (x % 4);
            THEME.abc_y = y;
        }
    }

    else if (strcmp(cfg_name, "count_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.count_x = x - (x % 4);
            THEME.count_y = y;
        }
    }

    else if (strcmp(cfg_name, "carousel_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.carousel_x = x - (x % 4);
            THEME.carousel_y = y;
        }
    }

    else if (strcmp(cfg_name, "grid_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.grid_x = x - (x % 4);
            THEME.grid_y = y;
        }
    }

    else if (strcmp(cfg_name, "list_coords") == 0) {
        short x,y;
        if (sscanf(val, "%hd,%hd", &x, &y) == 2) {
            THEME.list_x = x - (x % 4);
            THEME.list_y = y;
        }
    }

    else if (strcmp(cfg_name, "sortBarOffset") == 0) {
        short x;
        if (sscanf(val, "%hd", &x) == 1) {
            THEME.sortBarOffset = x;
        }
    }

    else if (strcmp(cfg_name, "info_color") == 0) {
        short x,y,z;
        if (sscanf(val, "%hd,%hd, %hd", &x, &y, &z) == 3) {
            THEME.info_r = x;
            THEME.info_g = y;
            THEME.info_b = z;
        }
    }

    else if (strcmp(cfg_name, "gametext_color") == 0) {
        short x,y,z;
        if (sscanf(val, "%hd,%hd, %hd", &x, &y, &z) == 3) {
            THEME.gameText_r = x;
            THEME.gameText_g = y;
            THEME.gameText_b = z;
        }
    }

    else if (strcmp(cfg_name, "prompttext_color") == 0) {
        short x,y,z;
        if (sscanf(val, "%hd,%hd, %hd", &x, &y, &z) == 3) {
            THEME.prompttxt_r = x;
            THEME.prompttxt_g = y;
            THEME.prompttxt_b = z;
        }
    }

    else if (strcmp(cfg_name, "clock_color") == 0) {
        short x,y,z;
        if (sscanf(val, "%hd,%hd, %hd", &x, &y, &z) == 3) {
            THEME.clock_r = x;
            THEME.clock_g = y;
            THEME.clock_b = z;
        }
    }

    else if (strcmp(cfg_name, "settingstext_color") == 0) {
        short x,y,z;
        if (sscanf(val, "%hd,%hd, %hd", &x, &y, &z) == 3) {
            THEME.settingsTxt_r = x;
            THEME.settingsTxt_g = y;
            THEME.settingsTxt_b = z;
        }
    }

    else if (strcmp(cfg_name, "pagesize") == 0) {
        short x;
        if (sscanf(val, "%hd", &x) == 1) {
            THEME.pagesize = x;
        }
    }

    else if (strcmp(cfg_name, "batteryUnused") == 0) {
        short x;
        if (sscanf(val, "%hd", &x) == 1) {
            THEME.batteryUnused = x;
        }
    } else if (strcmp(cfg_name, "tooltipAlpha") == 0) {
        short x;
        if (sscanf(val, "%hd", &x) == 1) {
            THEME.tooltipAlpha = x;
        }
    }

    /*
    	else if (strcmp(cfg_name, "maxcharacters") == 0) {
    		short x;
    		if (sscanf(val, "%hd", &x) == 1) {
    			THEME.maxcharacters = x;
    		}
    	}
    */



    cfg_bool("show_id", &THEME.showID);
    cfg_bool("show_tooltip", &THEME.showToolTip);
    cfg_bool("show_hddinfo", &THEME.showHDD);
    cfg_bool("show_gamecount", &THEME.showGameCnt);
    cfg_bool("show_region", &THEME.showRegion);
    cfg_bool("show_battery", &THEME.showBattery);
    cfg_map_auto("hddinfo_align", map_alignment, &THEME.hddInfoAlign);
    cfg_map_auto("gamecount_align", map_alignment, &THEME.gameCntAlign);
    cfg_map_auto("clock_align", map_alignment, &THEME.clockAlign);

    /*
    else if (strcmp(cfg_name, "entry_lines") == 0) {
    	int x;
    	if (sscanf(val, "%d", &x) == 1) {
    		ENTRIES_PER_PAGE = x;
    	}
    }

    else if (strcmp(cfg_name, "max_characters") == 0) {
    	int x;
    	if (sscanf(val, "%d", &x) == 1) {
    		MAX_CHARACTERS = x;
    	}
    }*/
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
    char tmp[200], name[200], val[200];
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
    game->alternatedolstart = alternatedoloffset;
    strcpy(game->alternatedolname, alternatedname);
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
                char temp3[40];
                int i = 0;
                while (i < 40) {

                    if (opt_val[i] == ';')
                        break;

                    temp3[i] = opt_val[i];
                    i++;
                }
                temp3[i] = '\0';
                strncpy(game->alternatedolname, temp3, 39);
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
    Settings.sinfo = ((THEME.showID) ? GameID : Neither);
    Settings.rumble = RumbleOn;
    if (THEME.showRegion) {
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
    char pathname[200];
//	bool ret = false;

    //set app path
//	chdir_app(argv[0]);

    CFG_Default(-1); // set defaults non forced

    snprintf(pathname, sizeof(pathname), "%s/config/GXGlobal.cfg", bootDevice);

    cfg_parsefile(pathname, &widescreen_set); //first set widescreen
    cfg_parsefile(pathname, &path_set); //then set config and layout options

    snprintf(pathname, sizeof(pathname), "%sGXtheme.cfg", CFG.theme_path);
    cfg_parsefile(pathname, &theme_set); //finally set theme information

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

    //moved this to the HDD wait screen to avoid the garbled green screen while it is loading *maybe*
    //OpenXMLDatabase(Settings.titlestxt_path, Settings.db_language, Settings.db_JPtoEN, true, Settings.titlesOverride==1?true:false, true);
    // loaded after database to override database titles with custom titles

    //took out this titles.txt shit because it is useless now.  teh xml has all the titles in it
    //snprintf(pathname, sizeof(pathname), "%stitles.txt", Settings.titlestxt_path);
    //cfg_parsefile(pathname, &title_set);

//	cfg_parsearg(argc, argv);
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
