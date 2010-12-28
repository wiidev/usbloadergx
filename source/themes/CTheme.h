#ifndef _THEME_H_
#define _THEME_H_

#include <string>
#include <stdio.h>
#include <gctypes.h>
#include "Resources.h"
#include "gettheme.h"

class Theme
{
    public:
        //!Set Default
        static void SetDefault();
        //!Load
        static bool Load(const char * path);
        //!Load font data
        static bool LoadFont(const char *path);
        //!Clear all image/font/theme data and free the memory
        static void CleanUp();

        //!Enable tooltips: special case treaded because it is called every frame
        static bool ShowTooltips;
    private:
        //!Clear the font data and free the memory
        static void ClearFontData();
};

#endif
