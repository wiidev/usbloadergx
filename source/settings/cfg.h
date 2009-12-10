#ifndef _CFG_H_
#define _CFG_H_

#include <gctypes.h>

#ifdef __cplusplus
extern "C" {
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

	struct CFG {
		short widescreen;
		char theme_path[100];
	};


	struct THEME {
		short gamelist_x;
		short gamelist_y;
		short gamelist_w;
		short gamelist_h;
		short gamegrid_x;
		short gamegrid_y;
		short gamegrid_w;
		short gamegrid_h;
		short gamecarousel_x;
		short gamecarousel_y;
		short gamecarousel_w;
		short gamecarousel_h;

		short covers_x;
		short covers_y;

		short show_id;
		short id_x;
		short id_y;
		short show_region;
		short region_x;
		short region_y;

		short sdcard_x;
		short sdcard_y;
		short homebrew_x;
		short homebrew_y;
		short power_x;
		short power_y;
		short home_x;
		short home_y;
		short setting_x;
		short setting_y;
		short install_x;
		short install_y;
		GXColor clock;
		short clock_align;
		short clock_x;
		short clock_y;

		GXColor info;
		short show_hddinfo;
		short hddinfo_align;
		short hddinfo_x;
		short hddinfo_y;

		short show_gamecount;
		short gamecount_align;
		short gamecount_x;
		short gamecount_y;

		short show_tooltip;
		int tooltipAlpha;

		GXColor prompttext;
		GXColor settingstext;
		GXColor gametext;
		short pagesize;

	// Toolbar Icons in GameList
	/*
		short favorite_x;
		short favorite_y;
		short search_x;
		short search_y;
		short abc_x;
		short abc_y;
		short count_x;
		short count_y;
		short list_x;
		short list_y;
		short grid_x;
		short grid_y;
		short carousel_x;
		short carousel_y;
		short sortBarOffset;
	*/
	// Toolbar Icons in GameList
		short gamelist_favorite_x;
		short gamelist_favorite_y;
		short gamelist_search_x;
		short gamelist_search_y;
		short gamelist_abc_x;
		short gamelist_abc_y;
		short gamelist_count_x;
		short gamelist_count_y;
		short gamelist_list_x;
		short gamelist_list_y;
		short gamelist_grid_x;
		short gamelist_grid_y;
		short gamelist_carousel_x;
		short gamelist_carousel_y;
		short gamelist_dvd_x;
		short gamelist_dvd_y;
		short gamelist_lock_x;
		short gamelist_lock_y;
	// Toolbar Icons in GameGrid
		short gamegrid_favorite_x;
		short gamegrid_favorite_y;
		short gamegrid_search_x;
		short gamegrid_search_y;
		short gamegrid_abc_x;
		short gamegrid_abc_y;
		short gamegrid_count_x;
		short gamegrid_count_y;
		short gamegrid_list_x;
		short gamegrid_list_y;
		short gamegrid_grid_x;
		short gamegrid_grid_y;
		short gamegrid_carousel_x;
		short gamegrid_carousel_y;
		short gamegrid_dvd_x;
		short gamegrid_dvd_y;
		short gamegrid_lock_x;
		short gamegrid_lock_y;
	// Toolbar Icons in GameCarousel
		short gamecarousel_favorite_x;
		short gamecarousel_favorite_y;
		short gamecarousel_search_x;
		short gamecarousel_search_y;
		short gamecarousel_abc_x;
		short gamecarousel_abc_y;
		short gamecarousel_count_x;
		short gamecarousel_count_y;
		short gamecarousel_list_x;
		short gamecarousel_list_y;
		short gamecarousel_grid_x;
		short gamecarousel_grid_y;
		short gamecarousel_carousel_x;
		short gamecarousel_carousel_y;
		short gamecarousel_dvd_x;
		short gamecarousel_dvd_y;
		short gamecarousel_lock_x;
		short gamecarousel_lock_y;
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


	struct Game_CFG {
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
	struct Game_NUM {
		u8 id[8];
		u8 favorite;
		u16 count;
	};


	void CFG_Default(int widescreen); // -1 = non forced mode
	void CFG_Load(void);
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
		//off=0,
		//on,
		anti=2,
		settings_error002_max // always the last entry
	};


	 enum {
		wiilight_off=0,
		wiilight_on,
		wiilight_forInstall,
		settings_wiilight_max // always the last entry
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
		i223,
		i250,
		settings_ios_max // always the last entry
	};

	enum {
		ios249=0,
		ios222,
		ios223,
		ios250,
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
		qwerty,
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
	enum {
		install_game_only,
		install_all,
		settings_partitions_max // always the last entry
	};
	struct SParental {
		u8 enabled;
		u8 rating;
		u8 pin[4];
		u8 question;
		wchar_t answer[32]; // IS WCHAR!
		u8 is_unlocked;
	};
	struct SSettings {
		u8      video;
		u8      language;
		u8      ocarina;
		u8      vpatch;
		int     ios;
		u8      sinfo;
		u8      hddinfo;
		u8      rumble;
		u8      xflip;
		int	    volume;
		int     sfxvolume;
		int     gamesoundvolume;
		u8      tooltips;
		char 	unlockCode[20];
		u8	    parentalcontrol;
		u8      cios;
		u8	    qboot;
		u8	    wsprompt;
		u8	    keyset;
		u8	    sort;
		u8	    fave;
		u8      wiilight;
		u8		gameDisplay;
		u8      patchcountrystrings;
		u8      screensaver;
		s8		partition;
		short	godmode;
		char	covers_path[100];
		char	covers2d_path[100];
		char	theme_path[100];
		char	wtheme_path[100];
        char	theme_downloadpath[100];
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
		u8		titlesOverride; // db_titles
		char	db_url[200];
		char	db_language[20];
		u8		db_JPtoEN;
		u8		gridRows;
		u8		autonetwork;
		u8		discart;
		short   gamesound;
		u8		marknewtitles;
		char	BcaCodepath[100];
		u8		FatInstallToDir;
		u8		partitions_to_install;
		u8		fullcopy;
		u8		beta_upgrades;
		struct SParental parental;		
	};
	extern struct SSettings Settings;

	void CFG_LoadGlobal(void);
	bool cfg_save_global(void);

	void GetLanguageToLangCode(int *langid, char *langcode);
	bool OpenXMLDatabase(char* xmlfilepath, char* argdblang, bool argJPtoEN, bool openfile, bool loadtitles, bool freemem);

	char *get_title(struct discHdr *header);
	char *cfg_get_title(u8 *id) ;
	void title_set(char *id, char *title);
	void titles_default();
	u8 get_block(struct discHdr *header);
	s8 get_pegi_block(struct discHdr *header);

	void CFG_Cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
