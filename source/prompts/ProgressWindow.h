/****************************************************************************
 * ProgressWindow
 * USB Loader GX 2009
 *
 * ProgressWindow.h
 ***************************************************************************/

#ifndef _PROGRESSWINDOW_H_
#define _PROGRESSWINDOW_H_

void InitProgressThread();
void ExitProgressThread();
void SetupGameInstallProgress(char * titl, char * game);
void ShowProgress (const char *title, const char *msg1, char *dynmsg2,
                   f32 done, f32 total, bool swSize = false, bool swTime = false);
void ProgressStop();

#endif
