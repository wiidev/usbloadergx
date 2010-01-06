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
#include "settings/cfg.h"
#include "main.h"

void InitGUIThreads(void);
void ExitGUIThreads(void);
void ResumeGui();
void HaltGui();
void menuBootgame(const char *headless);

int MainMenu (int menu);

enum {
    MENU_EXIT = -1,
    MENU_NONE,
    MENU_SETTINGS,
    MENU_DISCLIST,
    MENU_FORMAT,
    MENU_INSTALL,
    MENU_CHECK,
    MENU_GAME_SETTINGS,
    MENU_HOMEBREWBROWSE,
    BOOTHOMEBREW,
    MENU_THEMEDOWNLOADER
};
class GuiImageData;
GuiImageData *LoadCoverImage(struct discHdr *header, bool Prefere3D=true, bool noCover=true);
class GuiSound;
extern GuiSound *btnClick2;
#endif
