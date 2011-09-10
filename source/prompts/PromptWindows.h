/****************************************************************************
 * PromptWindows
 * USB Loader GX 2009
 *
 * PromptWindows.h
 ***************************************************************************/

#ifndef _PROMPTWINDOWS_H_
#define _PROMPTWINDOWS_H_

#include "GUI/gui.h"

int WindowPrompt(const char *title, const char *msg = NULL, const char * btn1Label = NULL, const char * btn2Label =
		NULL, const char * btn3Label = NULL, const char * btn4Label = NULL, int wait = -1);

void WindowCredits();
int OnScreenKeyboard(char * var, u32 maxlen, int min, bool hide = false);
int OnScreenNumpad(char * var, u32 maxlen);
int WindowExitPrompt();
int DiscWait(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, int IsDeviceWait);
int FormatingPartition(const char *title, int part_num);
bool NetworkInitPrompt();
int WindowScreensaver();
int CodeDownload(const char *id);

#endif
