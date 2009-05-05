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
	short cover_x;
	short cover_y;
	short showID;
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
	short clock_x;
	short clock_y;
	short clockAlign;
	short sdcard_x;
	short sdcard_y;
	short gameText_r;
	short gameText_g;
	short gameText_b;
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


void CFG_Default(int widescreen); // -1 = non forced mode
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
    int     ios;
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
