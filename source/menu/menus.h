#ifndef _MENUS_H
#define _MENUS_H

#include "GUI/gui.h"
#include "language/gettext.h"
#include "prompts/PromptWindows.h"
#include "menu.h"
#include "gecko.h"
#include "sys.h"

extern u8 shutdown;
extern u8 reset;

int MenuInstall();
int SelectPartitionMenu();

#endif // _MENUS_H
