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
		//!Load the original Wii System Menu font into memory only. It is not applied.
		static bool loadSystemFont(bool korean);
		//!Reload the main images/sounds for the new theme
		static void Reload();
		//!Clear all image/font/theme data and free the memory
		static void CleanUp();

		//!Enable tooltips: special case treaded because it is called every frame
		static bool ShowTooltips;
	private:
		//!Clear the font data and free the memory
		static void ClearFontData();
};

#endif
