/****************************************************************************
 * ProgressWindow
 * USB Loader GX 2009
 *
 * ProgressWindow.h
 ***************************************************************************/

#ifndef _PROGRESSWINDOW_H_
#define _PROGRESSWINDOW_H_

typedef void (*ProgressAbortCallback)(void);

void InitProgressThread();
void ExitProgressThread();
void SetupGameInstallProgress(char * titl, char * game);
void ShowProgress(const char *title, const char *msg1, const char *msg2, f32 done, f32 total, bool swSize = false,
        bool swTime = false);
void ProgressStop();
void ProgressSetAbortCallback(ProgressAbortCallback callback);

#ifdef __cplusplus
extern "C"
{
#endif

    void ProgressCallback(s64 gameinstalldone, s64 gameinstalltotal);

#ifdef __cplusplus
}
#endif

#endif
