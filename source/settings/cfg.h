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

    extern char bootDevice[10];

//    extern u8 ocarinaChoice;
//    extern u16 playcnt;
//
//    extern u8 videoChoice;
//    extern u8 languageChoice;
//    extern u8 viChoice;
//    extern u8 iosChoice;
//    extern u8 parentalcontrolChoice;
//    extern u8 fix002;
//    extern u8 reloadblock;
//    extern u8 countrystrings;
//    extern u8 alternatedol;
//    extern u32 alternatedoloffset;
//    extern u8 xflip;
//    extern u8 qboot;
//    extern u8 sort;
//    extern u8 fave;
//    extern u8 wsprompt;
//    extern u8 keyset;
//    extern u8 gameDisplay;
    extern char alternatedname[40];
//    extern u8 returnToLoaderGV;

    enum
    {
        ConsoleLangDefault = 0, jap, eng, ger, fren, esp, it, dut, schin, tchin, kor, settings_language_max
    // always the last entry
    };

    void CFG_LoadGameNum(); // -1 = non forced mode

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
