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
        //!Enable tooltips: special case treaded because it is called every frame
        static bool ShowTooltips;
};

#endif
