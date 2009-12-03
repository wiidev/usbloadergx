/****************************************************************************
 * PromptWindows
 * USB Loader GX 2009
 *
 * PromptWindows.h
 ***************************************************************************/

#ifndef _PROMPTWINDOWS_H_
#define _PROMPTWINDOWS_H_

#include "usbloader/partition_usbloader.h"

int WindowPrompt(const char *title, const char *msg = NULL, const char *btn1Label = NULL,
                 const char *btn2Label = NULL, const char *btn3Label = NULL,
                 const char *btn4Label = NULL, int wait = -1);

void WindowCredits();
int OnScreenKeyboard(char * var, u32 maxlen, int min);
int WindowExitPrompt();
int GameWindowPrompt();
int DiscWait(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, int IsDeviceWait);
int FormatingPartition(const char *title, partitionEntry *entry);
bool SearchMissingImages(int choice2);
int ProgressDownloadWindow(int choice2);
int ProgressUpdateWindow();
bool NetworkInitPrompt();
char * GetMissingFiles();
int WindowScreensaver();
int CodeDownload(const char *id);
int HBCWindowPrompt(const char *name, const char *coder, const char *version,
                    const char *release_date, const char *long_description,
                    const char *iconPath, u64 filesize);


#endif
