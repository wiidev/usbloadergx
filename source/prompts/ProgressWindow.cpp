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
#include "GUI/gui.h"
#include "prompts/ProgressWindow.h"
#include "usbloader/wbfs.h"
#include "themes/CTheme.h"
#include "utils/timer.h"
#include "utils/tools.h"

extern float gamesize;
extern int install_abort_signal;

/*** Variables used only in this file ***/
static const float fFilterTime = 15.0f; // seconds
static lwp_t progressthread = LWP_THREAD_NULL;
static bool CancelEnabled = false;
static bool progressCanceled = false;
static char progressTitle[75];
static char progressMsg1[75];
static char progressMsg2[75];
static Timer ProgressTimer;
static int showProgress = 0;
static s64 progressDone = 0.0f;
static s64 progressTotal = 0.0f;
static float fLastProgressDone = 0.0f;
static float fLastElapsedTime = 0.0f;
static float fSpeed = 0.0f;
static bool showTime = false;
static bool showSize = false;
static bool changed = true;
static bool changedMessages = true;

/****************************************************************************
 * StartProgress
 ***************************************************************************/
extern "C" void StartProgress(const char * title, const char * msg1, const char * msg2, bool swSize, bool swTime)
{
	if(title)
		strncpy(progressTitle, title, sizeof(progressTitle)-1);
	else
		progressTitle[0] = '\0';

	if(msg1)
		strncpy(progressMsg1, msg1, sizeof(progressMsg1)-1);
	else
		progressMsg1[0] = '\0';

	if(msg2)
		strncpy(progressMsg2, msg2, sizeof(progressMsg2)-1);
	else
		progressMsg2[0] = '\0';

	fSpeed = 0.0f;
	fLastElapsedTime = 0.0f;
	fLastProgressDone = 0.0f;
	progressTotal = progressDone = 0.0f;
	progressCanceled = false;
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
	if (done > total)
		done = total;

	if(!done)
	{
		fLastElapsedTime = 0.0f;
		fLastProgressDone = 0.0f;
		progressTotal = progressDone = 0.0f;
		ProgressTimer.reset();
		LWP_ResumeThread(progressthread);
		showProgress = 1;
	}

	if(total >= 0)
	{
		progressDone = done;
		progressTotal = total;
	}
	changed = true;
}

void ShowProgress(const char *msg2, s64 done, s64 total)
{
	if(msg2)
		strncpy(progressMsg2, msg2, sizeof(progressMsg2)-1);
	else
		progressMsg2[0] = '\0';

	ShowProgress(done, total);
	changedMessages = true;
}

void ShowProgress(const char *title, const char *msg1, const char *msg2, s64 done, s64 total, bool swSize, bool swTime)
{
	if(title)
		strncpy(progressTitle, title, sizeof(progressTitle)-1);
	else
		progressTitle[0] = '\0';

	if(msg1)
		strncpy(progressMsg1, msg1, sizeof(progressMsg1)-1);
	else
		progressMsg1[0] = '\0';

	if(msg2)
		strncpy(progressMsg2, msg2, sizeof(progressMsg2)-1);
	else
		progressMsg2[0] = '\0';

	showSize = swSize;
	showTime = swTime;

	ShowProgress(done, total);
	changedMessages = true;
}

/****************************************************************************
 * ProgressStop
 ***************************************************************************/
extern "C" void ProgressStop()
{
	progressCanceled = false;
	showProgress = 0;
	progressTitle[0] = 0;
	progressMsg1[0] = 0;
	progressMsg2[0] = 0;
	showTime = false;
	showSize = false;

	// wait for thread to finish
	while (!LWP_ThreadIsSuspended(progressthread))
		usleep(100);
}

/****************************************************************************
 * ProgressCancelEnable
 *
 * Enable/disable the progress cancel button
 ***************************************************************************/
extern "C" void ProgressCancelEnable(bool enable)
{
	CancelEnabled = enable;
}

/****************************************************************************
 * ProgressCanceled
 ***************************************************************************/
extern "C" bool ProgressCanceled()
{
	return progressCanceled;
}

/****************************************************************************
 * UpdateProgressValues
 ***************************************************************************/
static void UpdateProgressValues(GuiImage *progressbarImg, GuiText *prTxt, GuiText *timeTxt, GuiText *speedTxt, GuiText *sizeTxt)
{
	if(!changed)
		return;

	changed = false;
	changedMessages = false;

	float done;
	float total;

	if(gamesize > 0.0f && progressTotal > 0.0f)
	{
		done = ((float) progressDone / (float) progressTotal * gamesize);
		total = gamesize;

		if(progressCanceled)
			install_abort_signal = 1;
	}
	else
	{
		done = (float) progressDone;
		total = (float) progressTotal;
	}

	float fElapsedTime = ProgressTimer.elapsed();
	float fTimeDiff = fElapsedTime - fLastElapsedTime;
	//! initialize filter to current value on 2nd run
	if(fLastProgressDone == 0.0f && done > 0.0f && fTimeDiff > 0.0f) {
		fSpeed = done / fTimeDiff;
	}
	else if(fTimeDiff > 0.0f) {
		//! low pass filtering the speed
		fSpeed += ((done - fLastProgressDone) - fSpeed * fTimeDiff) / fFilterTime;
		//! store current elapse value for next cycle
		fLastElapsedTime = fElapsedTime;
	}

	s32 TimeLeft = 0, h = 0, m = 0, s = 0;
	if(fSpeed > 0.0f)
		TimeLeft = (total-done)/fSpeed;

	if(TimeLeft > 0)
	{
		h =  TimeLeft / 3600;
		m = (TimeLeft / 60) % 60;
		s =  TimeLeft % 60;
	}

	float progressPercent;
	if(total > 0.0f)
	   progressPercent = LIMIT(100.0f * done / total, 0.f, 100.f);
	else
		progressPercent = 100.0f; // total is 0 or below? i guess we are done...

	prTxt->SetTextf("%.2f", progressPercent);

	if (Settings.widescreen && Settings.wsprompt)
		progressbarImg->SetSkew(0, 0, (progressbarImg->GetWidth() * progressPercent
										- progressbarImg->GetWidth()) * Settings.WSFactor,
								0,	(progressbarImg->GetWidth() * progressPercent
										- progressbarImg->GetWidth()) * Settings.WSFactor, 0, 0, 0);
	else
		progressbarImg->SetSkew(0, 0, (progressbarImg->GetWidth() * progressPercent)
			- progressbarImg->GetWidth(), 0, (progressbarImg->GetWidth() * progressPercent)
			- progressbarImg->GetWidth(), 0, 0, 0);

	if (showTime == true)
	{
		timeTxt->SetTextf("%s %d:%02d:%02d", tr( "Time left:" ), (int)h, (int)m, (int)s);
	}

	if (showSize == true)
	{
		if (total < MB_SIZE)
			sizeTxt->SetTextf("%0.2fKB/%0.2fKB", done / KB_SIZE, total / KB_SIZE);
		else if (total > MB_SIZE && total < GB_SIZE)
			sizeTxt->SetTextf("%0.2fMB/%0.2fMB", done / MB_SIZE, total / MB_SIZE);
		else
			sizeTxt->SetTextf("%0.2fGB/%0.2fGB", done / GB_SIZE, total / GB_SIZE);

		speedTxt->SetTextf("%dKB/s", (int) (fSpeed/KB_SIZE));
	}

	//! store current done value for next cycle
	fLastProgressDone = done;
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
	progressCanceled = false;

	usleep(500000); // wait to see if progress flag changes soon
	if (!showProgress) return;

	const int ProgressPosY  = 20;

	GuiWindow promptWindow(472, 320);
	promptWindow.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
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
	progressbarOutlineImg.SetPosition(35, ProgressPosY);

	GuiImageData progressbarEmpty(Resources::GetFile("progressbar_empty.png"), Resources::GetFileSize("button_dialogue_box.png"));
	GuiImage progressbarEmptyImg(&progressbarEmpty);
	progressbarEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarEmptyImg.SetPosition(35, ProgressPosY);
	progressbarEmptyImg.SetTileHorizontal(100);

	GuiImageData progressbar(Resources::GetFile("progressbar.png"), Resources::GetFileSize("progressbar.png"));
	GuiImage progressbarImg(&progressbar);
	progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarImg.SetPosition(35, ProgressPosY);

	GuiText titleTxt(title, 26, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	titleTxt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	titleTxt.SetPosition(0, 50);

	GuiText msg1Txt(msg1, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	msg1Txt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	msg1Txt.SetPosition(0, 90);
	msg1Txt.SetMaxWidth(430, DOTTED);

	GuiText msg2Txt(msg2, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	msg2Txt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	msg2Txt.SetPosition(0, 125);
	msg2Txt.SetMaxWidth(430, DOTTED);

	GuiText prsTxt("%", 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	prsTxt.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	prsTxt.SetPosition(-178, ProgressPosY);

	GuiText timeTxt((char*) NULL, 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	timeTxt.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	timeTxt.SetPosition(280, -50);

	GuiText sizeTxt((char*) NULL, 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	sizeTxt.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	sizeTxt.SetPosition(50, -50);

	GuiText speedTxt((char*) NULL, 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	speedTxt.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	speedTxt.SetPosition(50, -74);

	GuiText prTxt((char*) NULL, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	prTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	prTxt.SetPosition(210, ProgressPosY);

	if ((Settings.wsprompt) && (Settings.widescreen)) /////////////adjust for widescreen
	{
		progressbarOutlineImg.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
		progressbarOutlineImg.SetPosition(0, ProgressPosY);
		progressbarEmptyImg.SetPosition(80, ProgressPosY);
		progressbarEmptyImg.SetTileHorizontal(78);
		progressbarImg.SetPosition(80, ProgressPosY);
		msg1Txt.SetMaxWidth(380, DOTTED);
		msg2Txt.SetMaxWidth(380, DOTTED);

		timeTxt.SetPosition(250, -50);
		speedTxt.SetPosition(90, -74);
		sizeTxt.SetPosition(90, -50);
	}

	GuiText cancelTxt(tr( "Cancel" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	GuiImage cancelImg(&btnOutline);
	const float cancelScale = 0.8f;
	cancelImg.SetScale(cancelScale);
	cancelTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	cancelTxt.SetPosition(cancelImg.GetWidth()/2*cancelScale-cancelTxt.GetTextWidth()/2, 0);
	if (Settings.wsprompt)
	{
		cancelTxt.SetWidescreen(Settings.widescreen);
		cancelImg.SetWidescreen(Settings.widescreen);
	}

	GuiButton cancelBtn(&cancelImg, &cancelImg, ALIGN_LEFT, ALIGN_MIDDLE, 0, 0, &trigA, btnSoundOver, btnSoundClick2, 1);
	cancelBtn.SetLabel(&cancelTxt);
	cancelBtn.SetState(STATE_SELECTED);
	cancelBtn.SetPosition(dialogBoxImg.GetWidth()/2-cancelImg.GetWidth()/2*cancelScale, ProgressPosY + 45);

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
	if(CancelEnabled)
		promptWindow.Append(&cancelBtn);

	HaltGui();
	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	ResumeGui();

	while (promptWindow.GetEffect() > 0) usleep(100);

	while (showProgress)
	{
		usleep(100000);

		if (shutdown)
			Sys_Shutdown();
		if (reset)
			Sys_Reboot();

		if (changed)
		{
			if (changedMessages)
			{
				titleTxt.SetText(progressTitle);
				msg1Txt.SetText(progressMsg1);
				msg2Txt.SetText(progressMsg2);

				if(progressMsg1[0] != '\0' && progressMsg2[0] == '\0') {
					msg1Txt.SetPosition(0, 120);
				}
				else if(progressMsg2[0] != '\0' && progressMsg1[0] == '\0') {
					msg2Txt.SetPosition(0, 120);
				}
				else {
					msg1Txt.SetPosition(0, 90);
					msg2Txt.SetPosition(0, 125);
				}
			}
			UpdateProgressValues(&progressbarImg, &prTxt, &timeTxt, &speedTxt, &sizeTxt);
		}

		if(cancelBtn.GetState() == STATE_CLICKED)
		{
			progressCanceled = true;
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
	LWP_CreateThread(&progressthread, ProgressThread, NULL, NULL, 16384, 60);

	memset(progressTitle, 0, sizeof(progressTitle));
	memset(progressMsg1, 0, sizeof(progressMsg1));
	memset(progressMsg2, 0, sizeof(progressMsg2));
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
}
