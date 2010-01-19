#ifndef _MENUS_H
#define _MENUS_H

#include "libwiigui/gui.h"
#include "language/gettext.h"
#include "prompts/PromptWindows.h"
#include "menu.h"
#include "gecko.h"
#include "filelist.h"
#include "sys.h"

extern void ResumeGui();
extern void HaltGui();
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern u8 shutdown;
extern u8 reset;

int MenuInstall();
int MenuDiscList();
int MenuFormat();
int MenuCheck();

#endif // _MENUS_H
