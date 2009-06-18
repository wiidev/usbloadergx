/****************************************************************************
 * PromptWindows
 * USB Loader GX 2009
 *
 * PromptWindows.h
 ***************************************************************************/

#ifndef _PROMPTWINDOWS_H_
#define _PROMPTWINDOWS_H_

#include "usbloader/partition.h"

void WindowCredits();
int OnScreenKeyboard(char * var, u32 maxlen, int min);
int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, const char *btn3Label, const char *btn4Label);
int WindowExitPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, const char *btn3Label, const char *btn4Label);
int GameWindowPrompt();
int DiscWait(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, int IsDeviceWait);
int FormatingPartition(const char *title, partitionEntry *entry);
void SearchMissingImages(int choice2);
int ProgressWindow(const char *title, const char *msg);
int ProgressDownloadWindow(int choice2);
int ProgressUpdateWindow();
char * GetMissingFiles();
void WindowScreensaver();

#endif
