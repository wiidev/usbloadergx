/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * menu.h
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#ifndef _MENU_H_
#define _MENU_H_

#include <ogcsys.h>
#include "libwiigui/gui.h"
#include "settings/CSettings.h"
#include "main.h"

void InitGUIThreads(void);
void ExitGUIThreads(void);

int MainMenu(int menuitem);

enum
{
    MENU_EXIT = -1,
    MENU_NONE,
    MENU_SETTINGS,
    MENU_DISCLIST,
    MENU_INSTALL,
    MENU_GAME_SETTINGS,
    MENU_HOMEBREWBROWSE,
    BOOTHOMEBREW,
    MENU_THEMEDOWNLOADER
};

void ResumeGui();
void HaltGui();

extern GuiImageData *pointer[4];
extern GuiImageData *background;
extern GuiImage *bgImg;
extern GuiWindow *mainWindow;
extern GuiText *GameRegionTxt;
extern GuiText *GameIDTxt;
extern GuiImageData *cover;
extern GuiImage *coverImg;
extern FreeTypeGX *fontSystem;

#endif
