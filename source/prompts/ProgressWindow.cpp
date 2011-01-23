/****************************************************************************
 * ProgressWindow
 * USB Loader GX 2009
 *
 * ProgressWindow.cpp
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "menu/menus.h"
#include "sys.h"
#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "prompts/ProgressWindow.h"
#include "usbloader/wbfs.h"
#include "usbloader/utils.h"
#include "themes/CTheme.h"
#include "utils/timer.h"

/*** Variables used only in this file ***/
static lwp_t progressthread = LWP_THREAD_NULL;
static mutex_t ProgressMutex = LWP_MUTEX_NULL;
static ProgressAbortCallback AbortCallback = NULL;
static const char * progressTitle = NULL;
static const char * progressMsg1 = NULL;
static const char * progressMsg2 = NULL;
static Timer ProgressTimer;
static int showProgress = 0;
static s64 progressDone = -1;
static s64 progressTotal = -1;
static bool showTime = false;
static bool showSize = false;
static bool changed = true;

/****************************************************************************
 * StartProgress
 ***************************************************************************/
extern "C" void StartProgress(const char * title, const char * msg1, const char * msg2, bool swSize, bool swTime)
{
    progressTitle = title;
    progressMsg1 = msg1;
    progressMsg2 = msg2;
    showSize = swSize;
    showTime = swTime;
    showProgress = 1;
    ProgressTimer.reset();

    LWP_ResumeThread(progressthread);
}

/****************************************************************************
 * ShowProgress
 *
 * Callbackfunction for updating the progress values
 * Use this function as standard callback
 ***************************************************************************/
extern "C" void ShowProgress(s64 done, s64 total)
{
    LWP_MutexLock(ProgressMutex);
    progressDone = done;
    progressTotal = total;
    if(!done)
    {
        ProgressTimer.reset();
        LWP_ResumeThread(progressthread);
        showProgress = 1;
    }
    changed = true;
    LWP_MutexUnlock(ProgressMutex);
}

void ShowProgress(const char *title, const char *msg1, const char *msg2, s64 done, s64 total, bool swSize, bool swTime)
{
    if (total <= 0)
        return;

    else if (done > total)
        done = total;

    LWP_MutexLock(ProgressMutex);

    progressDone = done;
    progressTotal = total;

    progressTitle = title;
    progressMsg1 = msg1;
    progressMsg2 = msg2;

    showSize = swSize;
    showTime = swTime;

    if(!done)
    {
        ProgressTimer.reset();
        LWP_ResumeThread(progressthread);
        showProgress = 1;
    }

    changed = true;

    LWP_MutexUnlock(ProgressMutex);
}

/****************************************************************************
 * ProgressStop
 ***************************************************************************/
extern "C" void ProgressStop()
{
    showProgress = 0;
	progressTitle = NULL;
	progressMsg1 = NULL;
	progressMsg2 = NULL;
    progressDone = -1;
    progressTotal = -1;
    showTime = false;
    showSize = false;

    // wait for thread to finish
    while (!LWP_ThreadIsSuspended(progressthread))
        usleep(100);
}

/****************************************************************************
 * ProgressSetAbortCallback
 *
 * Set a callback for the cancel button
 ***************************************************************************/
extern "C" void ProgressSetAbortCallback(ProgressAbortCallback callback)
{
    AbortCallback = callback;
}

/****************************************************************************
 * UpdateProgressValues
 ***************************************************************************/
static void UpdateProgressValues(GuiImage *progressbarImg, GuiText *prTxt, GuiText *timeTxt, GuiText *speedTxt, GuiText *sizeTxt)
{
    if(!changed)
        return;

    extern u64 gamesize;

    LWP_MutexLock(ProgressMutex);
    changed = false;
    s64 done = progressDone;
    s64 total = progressTotal;
    u32 speed = 0;

    if(gamesize > 0)
    {
        done = (s64) ((double) done / (double) total * (double) gamesize);
        total = (s64) gamesize;
    }

    //Calculate speed in KB/s
    if (ProgressTimer.elapsed() > 0.0f)
        speed = (u32) (done/ProgressTimer.elapsed());

    LWP_MutexUnlock(ProgressMutex);

    u32 TimeLeft = 0, h = 0, m = 0, s = 0;
    if(speed > 0)
        TimeLeft = (total-done)/speed;

    if(TimeLeft > 0)
    {
        h =  TimeLeft / 3600;
        m = (TimeLeft / 60) % 60;
        s =  TimeLeft % 60;
    }

    float progressPercent = 100.0 * done / total;

    prTxt->SetTextf("%.2f", progressPercent);

    if (Settings.widescreen && Settings.wsprompt)
        progressbarImg->SetSkew(0, 0, static_cast<int> (progressbarImg->GetWidth() * progressPercent * 0.8)
                - progressbarImg->GetWidth(), 0, static_cast<int> (progressbarImg->GetWidth() * progressPercent
                * 0.8) - progressbarImg->GetWidth(), 0, 0, 0);
    else
        progressbarImg->SetSkew(0, 0, static_cast<int> (progressbarImg->GetWidth() * progressPercent)
            - progressbarImg->GetWidth(), 0, static_cast<int> (progressbarImg->GetWidth() * progressPercent)
            - progressbarImg->GetWidth(), 0, 0, 0);

    if (showTime == true)
    {
        timeTxt->SetTextf("%s %d:%02d:%02d", tr( "Time left:" ), h, m, s);
    }

    if (showSize == true)
    {
        if (total < MB_SIZE)
            sizeTxt->SetTextf("%0.2fKB/%0.2fKB", done / KB_SIZE, total / KB_SIZE);
        else if (total > MB_SIZE && total < GB_SIZE)
            sizeTxt->SetTextf("%0.2fMB/%0.2fMB", done / MB_SIZE, total / MB_SIZE);
        else
            sizeTxt->SetTextf("%0.2fGB/%0.2fGB", done / GB_SIZE, total / GB_SIZE);

        speedTxt->SetTextf("%dKB/s", (int) (speed/KB_SIZE));
    }
}

/****************************************************************************
 * ProgressWindow
 *
 * Opens a window, which displays progress to the user. Can either display a
 * progress bar showing % completion, or a throbber that only shows that an
 * action is in progress.
 ***************************************************************************/
static void ProgressWindow(const char *title, const char *msg1, const char *msg2)
{
    GuiWindow promptWindow(472, 320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
    GuiImageData dialogBox(Resources::GetFile("dialogue_box.png"), Resources::GetFileSize("dialogue_box.png"));

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    GuiImage dialogBoxImg(&dialogBox);
    if (Settings.wsprompt)
    {
        dialogBoxImg.SetWidescreen(Settings.widescreen);
    }

    GuiImageData progressbarOutline(Resources::GetFile("progressbar_outline.png"), Resources::GetFileSize("progressbar_outline.png"));

    GuiImage progressbarOutlineImg(&progressbarOutline);
    if (Settings.wsprompt)
    {
        progressbarOutlineImg.SetWidescreen(Settings.widescreen);
    }
    progressbarOutlineImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    progressbarOutlineImg.SetPosition(35, 40);

    GuiImageData progressbarEmpty(Resources::GetFile("progressbar_empty.png"), Resources::GetFileSize("button_dialogue_box.png"));
    GuiImage progressbarEmptyImg(&progressbarEmpty);
    progressbarEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    progressbarEmptyImg.SetPosition(35, 40);
    progressbarEmptyImg.SetTile(100);

    GuiImageData progressbar(Resources::GetFile("progressbar.png"), Resources::GetFileSize("progressbar.png"));
    GuiImage progressbarImg(&progressbar);
    progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    progressbarImg.SetPosition(35, 40);

    GuiText titleTxt(title, 26, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0, 60);

    GuiText msg1Txt(msg1, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    msg1Txt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    if (msg2)
        msg1Txt.SetPosition(0, 100);
    else
        msg1Txt.SetPosition(0, 120);
    msg1Txt.SetMaxWidth(430, DOTTED);

    GuiText msg2Txt(msg2, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    msg2Txt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    msg2Txt.SetPosition(0, 125);
    msg2Txt.SetMaxWidth(430, DOTTED);

    GuiText prsTxt("%", 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    prsTxt.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
    prsTxt.SetPosition(-178, 40);

    GuiText timeTxt((char*) NULL, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    timeTxt.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    timeTxt.SetPosition(280, -50);

    GuiText sizeTxt((char*) NULL, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    sizeTxt.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    sizeTxt.SetPosition(50, -50);

    GuiText speedTxt((char*) NULL, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    speedTxt.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    speedTxt.SetPosition(50, -74);

    GuiText prTxt((char*) NULL, 26, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    prTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    prTxt.SetPosition(210, 40);

    if ((Settings.wsprompt) && (Settings.widescreen)) /////////////adjust for widescreen
    {
        progressbarOutlineImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        progressbarOutlineImg.SetPosition(0, 40);
        progressbarEmptyImg.SetPosition(80, 40);
        progressbarEmptyImg.SetTile(78);
        progressbarImg.SetPosition(80, 40);
        msg1Txt.SetMaxWidth(380, DOTTED);
        msg2Txt.SetMaxWidth(380, DOTTED);

        timeTxt.SetPosition(250, -50);
        timeTxt.SetFontSize(20);
        speedTxt.SetPosition(90, -74);
        speedTxt.SetFontSize(20);
        sizeTxt.SetPosition(90, -50);
        sizeTxt.SetFontSize(20);
    }

    GuiText cancelTxt(tr( "Cancel" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    GuiImage cancelImg(&btnOutline);
    if (Settings.wsprompt)
    {
        cancelTxt.SetWidescreen(Settings.widescreen);
        cancelImg.SetWidescreen(Settings.widescreen);
    }
    GuiButton cancelBtn(&cancelImg, &cancelImg, 2, 4, 0, -45, &trigA, btnSoundOver, btnSoundClick2, 1);
    cancelBtn.SetLabel(&cancelTxt);
    cancelBtn.SetState(STATE_SELECTED);

    usleep(400000); // wait to see if progress flag changes soon
    if (!showProgress) return;

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&progressbarEmptyImg);
    promptWindow.Append(&progressbarImg);
    promptWindow.Append(&progressbarOutlineImg);
    promptWindow.Append(&prTxt);
    promptWindow.Append(&prsTxt);
    if (title) promptWindow.Append(&titleTxt);
    promptWindow.Append(&msg1Txt);
    promptWindow.Append(&msg2Txt);
    if (showTime) promptWindow.Append(&timeTxt);
    if (showSize)
    {
        promptWindow.Append(&sizeTxt);
        promptWindow.Append(&speedTxt);
    }
    if(AbortCallback)
        promptWindow.Append(&cancelBtn);

    HaltGui();
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while (promptWindow.GetEffect() > 0) usleep(100);

    while (showProgress)
    {
        usleep(30000);

        if (shutdown)
            Sys_Shutdown();
        if (reset)
            Sys_Reboot();

        if (changed)
        {
            if (progressMsg1) msg1Txt.SetText(progressMsg1);
            if (progressMsg2) msg2Txt.SetText(progressMsg2);

            UpdateProgressValues(&progressbarImg, &prTxt, &timeTxt, &speedTxt, &sizeTxt);
        }

        if(cancelBtn.GetState() == STATE_CLICKED)
        {
            if(AbortCallback) AbortCallback();
            cancelBtn.ResetState();
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while (promptWindow.GetEffect() > 0) usleep(100);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
}

/****************************************************************************
 * ProgressThread
 ***************************************************************************/
static void * ProgressThread(void *arg)
{
    while (1)
    {
        if (!showProgress) LWP_SuspendThread(progressthread);

        ProgressWindow(progressTitle, progressMsg1, progressMsg2);
        usleep(100);
    }
    return NULL;
}

/****************************************************************************
 * InitProgressThread
 *
 * Startup Progressthread in idle prio
 ***************************************************************************/
void InitProgressThread()
{
    LWP_MutexInit(&ProgressMutex, true);
    LWP_CreateThread(&progressthread, ProgressThread, NULL, NULL, 16384, 70);
}

/****************************************************************************
 * ExitProgressThread
 *
 * Shutdown Progressthread
 ***************************************************************************/
void ExitProgressThread()
{
    LWP_JoinThread(progressthread, NULL);
    progressthread = LWP_THREAD_NULL;
    LWP_MutexUnlock(ProgressMutex);
    LWP_MutexDestroy(ProgressMutex);
}
