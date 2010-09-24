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
#define CFG_VIDEO_PAL50 3  // force PAL
#define CFG_VIDEO_PAL60 4  // force PAL60
#define CFG_VIDEO_NTSC  5  // force NTSC
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

    struct THEME
    {
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
    extern u8 returnToLoaderGV;

    enum
    {
        ConsoleLangDefault = 0, jap, eng, ger, fren, esp, it, dut, schin, tchin, kor, settings_language_max
    // always the last entry
    };

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
            u8 returnTo;
    };
    struct Game_NUM
    {
            u8 id[8];
            u8 favorite;
            u16 count;
    };

    void CFG_DefaultTheme(); // -1 = non forced mode
    void CFG_LoadTheme(bool widescreen, const char * theme_path);
    struct Game_CFG* CFG_get_game_opt(const u8 *id);
    struct Game_NUM* CFG_get_game_num(const u8 *id);
    bool CFG_save_game_opt(u8 *id);
    bool CFG_save_game_num(u8 *id);
    bool CFG_reset_all_playcounters();
    bool CFG_forget_game_opt(u8 *id);
    bool CFG_forget_game_num(u8 *id);

    enum
    {
        systemdefault = 0, discdefault, patch, pal50, pal60, ntsc, settings_video_max
    // always the last entry
    };

    enum
    {
        off = 0, on, settings_off_on_max
    // always the last entry
    };

    enum
    {
        wiilight_off = 0, wiilight_on, wiilight_forInstall, settings_wiilight_max
    // always the last entry
    };

    enum
    {
        GameID, GameRegion, Both, Neither, settings_sinfo_max
    // always the last entry
    };

    enum
    {
        hr12 = 0, hr24, Off, settings_clock_max
    // always the last entry
    };
    enum
    {
        ALL = 0, PLAYCOUNT,
    };

    enum
    {
        RumbleOff = 0, RumbleOn, settings_rumble_max
    // always the last entry
    };

    enum
    {
        TooltipsOff = 0, TooltipsOn, settings_tooltips_max
    // always the last entry
    };

    enum
    {
        min3 = 1, min5, min10, min20, min30, min60, settings_screensaver_max
    // always the last entry
    };

    enum
    {
        no = 0, yes, sysmenu, wtf, disk3d, settings_xflip_max
    // always the last entry
    };
    enum
    {
        us = 0, qwerty, dvorak, euro, azerty, settings_keyset_max
    // always the last entry
    };
    enum
    {
        list, grid, carousel, settings_display_max
    };
    enum
    {
        scrollDefault, scrollMarquee, settings_scrolleffect_max
    // always the last entry
    };
    enum
    {
        install_game_only, install_all, install_all_but_update, settings_partitions_max
    // always the last entry
    };
    enum
    {
        not_install_to_dir, install_to_gameid_name, install_to_name_gameid, settings_installdir_max
    // always the last entry
    };

    char *get_title(struct discHdr *header);
    char *cfg_get_title(u8 *id);
    void title_set(char *id, char *title);
    void titles_default();
    u8 get_block(struct discHdr *header);
    s8 get_pegi_block(struct discHdr *header);

    void CFG_Cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
