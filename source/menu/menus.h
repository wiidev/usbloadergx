#ifndef _MENUS_H
#define _MENUS_H

#include "libwiigui/gui.h"
#include "libwiigui/gui_bgm.h"
#include "language/gettext.h"
#include "prompts/PromptWindows.h"
#include "menu.h"
#include "gecko.h"
#include "filelist.h"
#include "sys.h"

extern u8 shutdown;
extern u8 reset;

int MenuInstall();
int MenuDiscList();
int SelectPartitionMenu();
int MountGamePartition(bool ShowGUI = true);

#endif // _MENUS_H
