#ifndef _CFG_H_
#define _CFG_H_

#include <gctypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "usbloader/disc.h"

#define CFG_HOME_REBOOT 0
#define CFG_HOME_EXIT   1

#define CFG_VIDEO_SYS   0  // system default
#define CFG_VIDEO_DEFAULT  1
#define CFG_VIDEO_GAME  1  // game default
#define CFG_VIDEO_PATCH 2  // patch mode
#define CFG_VIDEO_PAL50	3  // force PAL
#define CFG_VIDEO_PAL60	4  // force PAL60
#define CFG_VIDEO_NTSC	5  // force NTSC
#define CFG_VIDEO_COUNT 6

#define CFG_LANG_CONSOLE  0
#define CFG_LANG_JAPANESE 1
#define CFG_LANG_ENGLISH  2
#define CFG_LANG_GERMAN   3
#define CFG_LANG_FRENCH   4
#define CFG_LANG_SPANISH  5
#define CFG_LANG_ITALIAN  6
#define CFG_LANG_DUTCH    7
#define CFG_LANG_S_CHINESE 8
#define CFG_LANG_T_CHINESE 9
#define CFG_LANG_KOREAN   10
#define CFG_LANG_COUNT    11

#define CFG_ALIGN_LEFT 0
#define CFG_ALIGN_RIGHT 1
#define CFG_ALIGN_CENTRE 2
#define CFG_ALIGN_TOP 3
#define CFG_ALIGN_BOTTOM 4
#define CFG_ALIGN_MIDDLE 5

extern char bootDevice[10];
//extern char *cfg_path;

struct CFG
{
	short widescreen;
	char theme_path[100];
};


struct THEME
{
	int selection_x;
	int selection_y;
	int selection_w;
	int selection_h;
	int gamegrid_x;
	int gamegrid_y;
	int gamegrid_w;
	int gamegrid_h;
	int tooltipAlpha;
	int gamecarousel_x;
	int gamecarousel_y;
	int gamecarousel_w;
	int gamecarousel_h;
	short cover_x;
	short cover_y;
	short showID;
//	short maxcharacters;
	short batteryUnused;
	short id_x;
	short id_y;
	short region_x;
	short region_y;
	short power_x;
	short power_y;
	short home_x;
	short home_y;
	short battery1_x;
	short battery2_x;
	short battery3_x;
	short battery4_x;
	short battery1_y;
	short battery2_y;
	short battery3_y;
	short battery4_y;
	short favorite_x;
	short favorite_y;
	short abc_x;
	short abc_y;
	short list_x;
	short list_y;
	short grid_x;
	short grid_y;
	short carousel_x;
	short carousel_y;
	short count_x;
	short count_y;
	short sortBarOffset;
//	short showPower;
//	short showHome;
	short setting_x;
	short setting_y;
	short install_x;
	short install_y;
	short showHDD;
	short hddInfoAlign;
	short hddInfo_x;
	short hddInfo_y;
	short showGameCnt;
	short gameCntAlign;
	short gameCnt_x;
	short gameCnt_y;
	short showRegion;
	short showBattery;
	short showToolTip;
	//color
	short info_r;
	short info_g;
	short info_b;
	short prompttxt_r;
	short prompttxt_g;
	short prompttxt_b;
	short settingsTxt_r;
	short settingsTxt_g;
	short settingsTxt_b;
	short clock_r;
	short clock_g;
	short clock_b;
	short clock_x;
	short clock_y;
	short clockAlign;
	short sdcard_x;
	short sdcard_y;
	short gameText_r;
	short gameText_g;
	short gameText_b;
	short pagesize;
};

extern struct CFG CFG;
extern struct THEME THEME;
extern u8 ocarinaChoice;
extern u16 playcnt;
extern u8 videoChoice;
extern u8 languageChoice;
extern u8 viChoice;
extern u8 iosChoice;
extern u8 parentalcontrolChoice;
extern u8 fix002;
extern u8 reloadblock;
extern u8 countrystrings;
extern u8 alternatedol;
extern u32 alternatedoloffset;
extern u8 xflip;
extern u8 qboot;
extern u8 sort;
extern u8 fave;
extern u8 wsprompt;
extern u8 keyset;
extern u8 gameDisplay;
extern u16 playcount;
extern u8 favoritevar;
extern char alternatedname[40];


struct Game_CFG
{
	u8 id[8];
	u8 video;
	u8 language;
	u8 ocarina;
	u8 vipatch;
	u8 ios;
	u8 parentalcontrol;
	u8 errorfix002;
	u8 iosreloadblock;
	u8 loadalternatedol;
   u32 alternatedolstart;
	u8 patchcountrystrings;
	char alternatedolname[40];
};
struct Game_NUM
{
	u8 id[8];
	u8 favorite;
	u16 count;
};


void CFG_Default(int widescreen); // -1 = non forced mode
void CFG_Load(void);
void lang_defualt();
struct Game_CFG* CFG_get_game_opt(u8 *id);
struct Game_NUM* CFG_get_game_num(u8 *id);
bool CFG_save_game_opt(u8 *id);
bool CFG_save_game_num(u8 *id);
bool CFG_reset_all_playcounters();
bool CFG_forget_game_opt(u8 *id);
bool CFG_forget_game_num(u8 *id);


enum {
	ConsoleLangDefault=0,
	jap,
	eng,
	ger,
	fren,
	esp,
	it,
	dut,
	schin,
	tchin,
	kor,
	settings_language_max // always the last entry
};

enum {
    systemdefault=0,
    discdefault,
	patch,
	pal50,
	pal60,
	ntsc,
	settings_video_max // always the last entry
};

enum {
    off=0,
	on,
	settings_off_on_max // always the last entry
};
enum {
	anti=2,
};


enum {
    GameID,
	GameRegion,
	Both,
	Neither,
	settings_sinfo_max // always the last entry
};

enum {
	i249=0,
	i222,
	i223
};

enum {
	ios249=0,
	ios222,
	settings_cios_max // always the last entry
};

enum {
	hr12=0,
	hr24,
	Off,
	settings_clock_max // always the last entry
};
enum {
	all=0,
	pcount,
};

enum {
	RumbleOff=0,
	RumbleOn,
	settings_rumble_max // always the last entry
};

enum {
	TooltipsOff=0,
	TooltipsOn,
	settings_tooltips_max // always the last entry
};

enum {
	min3=1,
	min5,
	min10,
	min20,
	min30,
	min60,
	settings_screensaver_max // always the last entry
};

enum {
    no=0,
	yes,
	sysmenu,
	wtf,
	disk3d,
	settings_xflip_max // always the last entry
};
enum {
	us=0,
	dvorak,
	euro,
	azerty,
	settings_keyset_max // always the last entry
};
enum {
	list,
	grid,
	carousel,
	settings_display_max
};
enum {
	scrollDefault,
	scrollMarquee,
	settings_scrolleffect_max // always the last entry
};
struct SSettings {
	int	    video;
	int	    language;
	int     ocarina;
	int     vpatch;
	int     ios;
	int	    sinfo;
	int	    hddinfo;
	int	    rumble;
	int	    xflip;
	int	    volume;
	int     sfxvolume;
	int     tooltips;
	char 	unlockCode[20];
	int	    parentalcontrol;
	int     cios;
	int	    qboot;
	int	    wsprompt;
	int	    keyset;
	int	    sort;
	int	    fave;
	int     wiilight;
	int		gameDisplay;
	int     patchcountrystrings;
	int     screensaver;
	short	godmode;
	char	covers_path[100];
	char	theme_path[100];
	char	wtheme_path[100];
	char	disc_path[100];
	char	titlestxt_path[100];
	char	language_path[100];
	char	languagefiles_path[100];
	char	oggload_path[100];
	char	ogg_path[150];
	char	dolpath[150];
	char	update_path[150];
	char	homebrewapps_path[150];
	char    selected_homebrew[200];
	char	Cheatcodespath[100];
	char	TxtCheatcodespath[100];
	short   error002;
	int		titlesOverride; // db_titles
	char	db_url[200];
	char	db_language[100];
	int		db_JPtoEN;
	int		gridRows;
};

void CFG_LoadGlobal(void);
bool cfg_save_global(void);

bool OpenXMLDatabase(char* xmlfilepath, char* argdblang, bool argJPtoEN, bool openfile, bool loadtitles, bool freemem);

char *get_title(struct discHdr *header);
u8 get_block(struct discHdr *header);

void CFG_Cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
