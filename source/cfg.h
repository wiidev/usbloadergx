#ifndef _CFG_H_
#define _CFG_H_

#include <gctypes.h>
#include "disc.h"

#ifdef __cplusplus
extern "C"
{
#endif

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

extern char *cfg_path;
//extern char *cfg_images_path;

struct CFG
{
//	char *background;
//	short covers;
//	short simple;
//	short video;
//	short language;
//	short ocarina;
//	short vipatch;
//	short home;
//	short download;
//	short installdownload;
//	short hidesettingmenu;
//	short savesettings;
	short widescreen;
	short parentalcontrol;
	short maxcharacters;
	short godmode;
	char unlockCode[20];
	char covers_path[100];
	char theme_path[100];
	char disc_path[100];
};

struct THEME
{
	int selection_x;
	int selection_y;
	int selection_w;
	int selection_h;
	int cover_x;
	int cover_y;
	short showID;
	int id_x;
	int id_y;
	int region_x;
	int region_y;
	int power_x;
	int power_y;
	int home_x;
	int home_y;
	int battery1_x;
	int battery2_x;
	int battery3_x;
	int battery4_x;
	int battery1_y;
	int battery2_y;
	int battery3_y;
	int battery4_y;
//	short showPower;
//	short showHome;
	int setting_x;
	int setting_y;
	int install_x;
	int install_y;
	short showHDD;
	short hddInfoAlign;
	int hddInfo_x;
	int hddInfo_y;
	short showGameCnt;
	short gameCntAlign;
	int gameCnt_x;
	int gameCnt_y;
	short showRegion;
	short showBattery;
	short showToolTip;
	//color
	short info_r;
	short info_g;
	short info_b;
	short clock_x;
	short clock_y;
	short clockAlign;
	int sdcard_x;
	int sdcard_y;
};

extern struct CFG CFG;
extern struct THEME THEME;
extern u8 ocarinaChoice;
extern u8 videoChoice;
extern u8 languageChoice;
extern u8 viChoice;
extern u8 iosChoice;
extern u8 parentalcontrolChoice;

struct Game_CFG
{
	u8 id[8];
	u8 video;
	u8 language;
	u8 ocarina;
	u8 vipatch;
	u8 ios;
	u8 parentalcontrol;
};


void CFG_Default();
void CFG_Load(int argc, char **argv);
struct Game_CFG* CFG_get_game_opt(u8 *id);
bool CFG_save_game_opt(u8 *id);
bool CFG_forget_game_opt(u8 *id);

//Astidof - Begin of modification
enum {
	ConsoleLangDefault,
	jap,
	eng,
	ger,
	fren,
	esp,
	it,
	dut,
	schin,
	tchin,
	kor
};

enum {
    systemdefault,
    discdefault,
	patch,
	pal50,
	pal60,
	ntsc
};

enum {
    off,
	on,
};

enum {
    GameID,
	GameRegion,
	Both,
	Neither,
};

enum {
	i249,
	i222,
};

enum {
	ios249,
	ios222,
};

enum {
	HDDInfo,
	Clock,
};

enum {
	RumbleOff,
	RumbleOn,
};

enum {
	TooltipsOff,
	TooltipsOn,
};

enum {
	v10,
	v20,
	v30,
	v40,
	v50,
	v60,
	v70,
	v80,
	v90,
	v100,
	v0,
};

enum {
	ParentalControlOff,
	ParentalControlLevel1,
	ParentalControlLevel2,
	ParentalControlLevel3
};

struct SSettings {
    int		video;
    int		language;
    int     ocarina;
    int     vpatch;
	int		sinfo;
	int		hddinfo;
	int		rumble;
	int		volume;
	int     tooltips;
	char 	unlockCode[20];
	int		parentalcontrol;
	int     cios;
};

void CFG_LoadGlobal(void);
bool cfg_save_global(void);
//Astidof - End of modification

char *get_title(struct discHdr *header);
u8 get_block(struct discHdr *header);

void CFG_Cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
