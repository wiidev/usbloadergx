#ifndef _MENUS_H
#define _MENUS_H

#include <unistd.h>

#include "libwiigui/gui.h"
#include "language/gettext.h"
#include "prompts/PromptWindows.h"
#include "menu.h"
#include "gecko.h"
#include "filelist.h"
#include "sys.h"

extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern u8 checkthreadState;
extern u8 needToReloadGamelist;
extern u8 hddOK;
extern u8 mountMethod;


int MenuInstall();
int MenuDiscList();
int MenuFormat();

extern void ResumeCheck();
extern void HaltCheck();
extern void InitCheckThread();
extern void ExitCheckThread();

#endif // _MENUS_H
