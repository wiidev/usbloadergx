/****************************************************************************
 * ProgressWindow
 * USB Loader GX 2009
 *
 * ProgressWindow.h
 ***************************************************************************/

#ifndef _PROGRESSWINDOW_H_
#define _PROGRESSWINDOW_H_

typedef void (*ProgressAbortCallback)(void);

#ifdef __cplusplus

void InitProgressThread();
void ExitProgressThread();
void ShowProgress(const char *title, const char *msg1, const char *msg2, s64 done, s64 total, bool swSize = false, bool swTime = false);

extern "C"
{
#endif

void StartProgress(const char * title, const char * msg1, const char * msg2, bool swSize, bool swTime);
void ShowProgress(s64 done, s64 total);
void ProgressSetAbortCallback(ProgressAbortCallback callback);
void ProgressStop();

#ifdef __cplusplus
}
#endif

#endif
