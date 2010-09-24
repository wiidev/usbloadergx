#ifndef _CTHEME_H_
#define _CTHEME_H_

#include <string>
#include <stdio.h>
#include <gctypes.h>

class CTheme
{
    public:
        //!Constructor
        CTheme();
        //!Destructor
        ~CTheme();
        //!Set Default
        void SetDefault();
        //!Load
        bool Load(const char * path);

        /** Variables **/
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

    protected:
        bool SetSetting(char *name, char *value);
        //!Find the config file in the default paths
        bool FindConfig();

        void ParseLine(char *line);
        void TrimLine(char *dest, char *src, int size);
        bool WideScreen;
};

extern CTheme Theme;

#endif
