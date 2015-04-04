#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>

#include "Controls/DeviceHandler.hpp"
#include "usbloader/wbfs.h"
#include "usbloader/wdvd.h"
#include "usbloader/usbstorage2.h"
#include "usbloader/GameList.h"
#include "GameCube/GCGames.h"
#include "language/gettext.h"
#include "GUI/gui.h"
#include "GUI/gui_numpad.h"
#include "GUI/gui_diskcover.h"
#include "GUI/Text.hpp"
#include "settings/CGameStatistics.h"
#include "settings/GameTitles.h"
#include "network/networkops.h"
#include "network/update.h"
#include "network/http.h"
#include "prompts/PromptWindows.h"
#include "prompts/PromptWindow.hpp"
#include "prompts/gameinfo.h"
#include "themes/CTheme.h"
#include "utils/StringTools.h"
#include "utils/tools.h"
#include "mload/mload.h"
#include "FileOperations/fileops.h"
#include "menu/menus.h"
#include "sys.h"
#include "wpad.h"
#include "wad/wad.h"
#include "zlib.h"
#include "svnrev.h"
#include "audio.h"
#include "language/UpdateLanguage.h"
#include "system/IosLoader.h"
#include "gecko.h"
#include "lstub.h"

static const char * DMLVersions[] =
{
	//! sorted by internal release date. see IosLoader.h
	"",			// Original MIOS
	"r51-",		// old DML r51-
	"r52",		// old DML r52
	"v0.1",		// QuadForce v0.1
	"v1.2",		// DML 1.2
	"v1.4",		// DML 1.4
	"v1.4b",	// DML 1.4b
	"v1.5",		// DML 1.5
	"v2.0.x",	// DM  2.0
	"v2.1",		// DML 2.1
	"v2.2.x",	// DM  2.2
	"v2.2.2",	// DM  2.2 update 2
	"v2.2",		// DML 2.2
	"v2.2.1",	// DML 2.2.1
	"v2.3",		// DML 2.3 (mirror link)
	"v2.3",		// DM  2.3
	"v2.3",		// DML 2.3 (main link)
	"v2.4",		// DM  2.4
	"v2.4",		// DML 2.4
	"v2.5",		// DM  2.5
	"v2.5",		// DML 2.5
	"v2.6.0",	// DM 2.6
	"v2.6",		// DML 2.6
	"v2.6.1",	// DM 2.6 update 1
	"v2.7",		// DM 2.7
	"v2.7",		// DML 2.7
	"v2.8",		// DML 2.8
	"v2.8",		// DM 2.8
	"v2.9",		// DML 2.9
	"v2.9",		// DM 2.9
	"v2.0",		// QuadForce v2.0
	"v3.0",		// QuadForce v3.0
	"v4.0",	// QuadForce v4.0 SD
	"v2.10",	// DML 2.10
	"v2.10",	// DM 2.10
	"v4.1",	// QuadForce v4.1 USB
	"v2.11+",	// DML 2.11
	"v2.11+",	// DM 2.11
};


/****************************************************************************
 * OnScreenNumpad
 *
 * Opens an on-screen numpad window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
int OnScreenNumpad(char * var, u32 maxlen)
{
	int save = -1;

	GuiNumpad numpad(var, maxlen);

	GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetSimpleTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiText okBtnTxt(tr( "OK" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	GuiImage okBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		okBtnTxt.SetWidescreen(Settings.widescreen);
		okBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton okBtn(&okBtnImg, &okBtnImg, 0, 4, 5, -15, &trigA, btnSoundOver, btnSoundClick2, 1);
	okBtn.SetLabel(&okBtnTxt);
	GuiText cancelBtnTxt(tr( "Cancel" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	GuiImage cancelBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		cancelBtnTxt.SetWidescreen(Settings.widescreen);
		cancelBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton cancelBtn(&cancelBtnImg, &cancelBtnImg, 1, 4, -5, -15, &trigA, btnSoundOver, btnSoundClick2, 1);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetTrigger(&trigB);

	numpad.Append(&okBtn);
	numpad.Append(&cancelBtn);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&numpad);
	ResumeGui();

	while (save == -1)
	{
		usleep(50000);

		if (okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if (cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if (save == 1)
	{
		snprintf(var, maxlen, "%s", numpad.GetText());

		// convert all , to . characters
		for(u32 i = 0; i < maxlen; ++i)
		{
			if(var[i] == ',')
				var[i] = '.';
		}
	}

	HaltGui();
	mainWindow->Remove(&numpad);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	gprintf("\t%s", (save == 1 ? "saved" : "discarded"));
	return save;
}

/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
int OnScreenKeyboard(char * var, u32 maxlen, int min, bool hide)
{
	int save = -1;

	gprintf("\nOnScreenKeyboard(%s, %i, %i) \n\tkeyset = %i", var, maxlen, min, Settings.keyset);

	GuiKeyboard keyboard(var, maxlen, min, Settings.keyset);
	keyboard.SetVisibleText(!hide);

	GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetSimpleTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiText okBtnTxt(tr( "OK" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	GuiImage okBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		okBtnTxt.SetWidescreen(Settings.widescreen);
		okBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton okBtn(&okBtnImg, &okBtnImg, 0, 4, 5, 15, &trigA, btnSoundOver, btnSoundClick2, 1);
	okBtn.SetLabel(&okBtnTxt);
	GuiText cancelBtnTxt(tr( "Cancel" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	GuiImage cancelBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		cancelBtnTxt.SetWidescreen(Settings.widescreen);
		cancelBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton cancelBtn(&cancelBtnImg, &cancelBtnImg, 1, 4, -5, 15, &trigA, btnSoundOver, btnSoundClick2, 1);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetTrigger(&trigB);

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&keyboard);
	ResumeGui();

	while (save == -1)
	{
		usleep(50000);

		if (okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if (cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if (save)
	{
		snprintf(var, maxlen, "%s", keyboard.GetText());
	}

	HaltGui();
	mainWindow->Remove(&keyboard);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	gprintf("\t%s", (save ? "saved" : "discarded"));
	return save;
}

/****************************************************************************
 * WindowCredits
 * Display credits
 ***************************************************************************/
void WindowCredits()
{
	gprintf("WindowCredits()\n");

	int angle = 0;
	GuiSound * creditsMusic = NULL;

	bgMusic->Pause();

	creditsMusic = new GuiSound(Resources::GetFile("credits_music.ogg"), Resources::GetFileSize("credits_music.ogg"), 55);
	creditsMusic->SetVolume(60);
	creditsMusic->SetLoop(1);
	creditsMusic->Play();

	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	GuiButton backBtn(0, 0);
	backBtn.SetPosition(-20, -20);
	backBtn.SetTrigger(&trigB);
	
	u32 i = 0;
	int y = 20;
	float oldFontScale = Settings.FontScaleFactor;
	Settings.FontScaleFactor = 1.0f;

	GuiWindow creditsWindow(screenwidth, screenheight);
	GuiWindow creditsWindowBox(580, 448);
	creditsWindowBox.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);

	GuiImageData creditsBox(Resources::GetFile("credits_bg.png"), Resources::GetFileSize("credits_bg.png"));
	GuiImage creditsBoxImg(&creditsBox);
	creditsBoxImg.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	creditsWindowBox.Append(&creditsBoxImg);

	GuiImageData star(Resources::GetFile("little_star.png"), Resources::GetFileSize("little_star.png"));
	GuiImage starImg(&star);
	starImg.SetWidescreen(Settings.widescreen); //added
	starImg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	starImg.SetPosition(505, 350);

	std::vector<GuiText *> txt;

	const u8 *creditsFont = Resources::GetFile("font.ttf");
	u32 creditsFontSize = Resources::GetFileSize("font.ttf");

	GuiText * currentTxt = new GuiText(tr( "Credits" ), 28, ( GXColor ) {255, 255, 255, 255});
	currentTxt->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	currentTxt->SetPosition(0, 8);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);

	char SvnRev[80];
#ifdef FULLCHANNEL
	snprintf(SvnRev, sizeof(SvnRev), "Rev%sc   IOS%u (Rev %u)%s", GetRev(), IOS_GetVersion(), IOS_GetRevision(), (*(vu32*)0xcd800064 == 0xFFFFFFFF)? " + AHB" : "" );
#else
	snprintf(SvnRev, sizeof(SvnRev), "Rev%s   IOS%u (Rev %u)%s", GetRev(), IOS_GetVersion(), IOS_GetRevision(), (*(vu32*)0xcd800064 == 0xFFFFFFFF)? " + AHB" : "" );
#endif

	char IosInfo[80] = "";
	iosinfo_t * info = IosLoader::GetIOSInfo(IOS_GetVersion());
	if(info)
		snprintf(IosInfo, sizeof(IosInfo), "(%s v%i%s base%i)", info->name, info->version, info->versionstring, info->baseios);

	// Check if DIOS MIOS (Lite) is available 
	char GCInfo[80] = "";
	int currentMIOS = IosLoader::GetMIOSInfo();
	if(currentMIOS == DIOS_MIOS)
		snprintf(GCInfo, sizeof(GCInfo), "DIOS-MIOS %s", DMLVersions[IosLoader::GetDMLVersion()]);
	else if (currentMIOS == DIOS_MIOS_LITE)
		snprintf(GCInfo, sizeof(GCInfo), "DIOS-MIOS Lite %s", DMLVersions[IosLoader::GetDMLVersion()]);
	else if (currentMIOS == QUADFORCE)
		snprintf(GCInfo, sizeof(GCInfo), "QuadForce %s", DMLVersions[IosLoader::GetDMLVersion()]);
	else if (currentMIOS == QUADFORCE_USB)
		snprintf(GCInfo, sizeof(GCInfo), "QuadForce USB %s", DMLVersions[IosLoader::GetDMLVersion()]);
		
	// Check if Devolution is available
	char DEVO_loader_path[100];
	snprintf(DEVO_loader_path, sizeof(DEVO_loader_path), "%sloader.bin", Settings.DEVOLoaderPath);
	FILE *f = fopen(DEVO_loader_path, "rb");
	if(f)
	{
		char version[5];
		fseek(f, 23, SEEK_SET);
		fread(version, 1, 4, f);
		fclose(f);
		char *ptr = strchr(version, ' ');
		if(ptr) *ptr = 0;
		else version[4] = 0;
		snprintf(GCInfo, sizeof(GCInfo), "%s%sDevolution r%d", GCInfo, strlen(GCInfo) > 1 ? "  /  " : "", atoi(version));
	}

	// Check if Nintendont is available
	char GCInfo2[80] = "";
	char NINVersion[7]= "";
	nintendontVersion(Settings.NINLoaderPath, NINVersion, sizeof(NINVersion));
	if(strlen(NINVersion) > 0)
	{
		snprintf(GCInfo2, sizeof(GCInfo2), "Nintendont v%.6s",  NINVersion );
	}
	else
	{
		char NINBuildDate[21] = "";
		if(nintendontBuildDate(Settings.NINLoaderPath, NINBuildDate))
			snprintf(GCInfo2, sizeof(GCInfo2), "Nintendont Built %.20s",  NINBuildDate );
	}

	// Header - Top left
	currentTxt = new GuiText(GCInfo2, 16, ( GXColor ) {255, 255, 255, 255});
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(0, strlen(GCInfo) > 0 ? y-10 : y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);

	// Header - Bottom left
	currentTxt = new GuiText(GCInfo, 16, ( GXColor ) {255, 255, 255, 255});
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(0, y+6);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);

	// Header - Top right
	currentTxt = new GuiText(SvnRev, 16, ( GXColor ) {255, 255, 255, 255});
	currentTxt->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	currentTxt->SetPosition(0, (info || currentMIOS > DEFAULT_MIOS) ? y-10 : y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);

	// Header - Bottom right
	currentTxt = new GuiText(IosInfo, 16, ( GXColor ) {255, 255, 255, 255});
	currentTxt->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	currentTxt->SetPosition(0, y+6);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 34;

	currentTxt = new GuiText("USB Loader GX", 24, ( GXColor ) {255, 255, 255, 255});
	currentTxt->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	currentTxt->SetPosition(0, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 24;

	currentTxt = new GuiText(tr( "Official Site:" ), 20, ( GXColor ) {255, 255, 255, 255});
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(10, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);

	currentTxt = new GuiText("http://code.google.com/p/usbloader-gui/", 20, ( GXColor ) {255, 255, 255, 255});
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(160, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 22;

	GuiText::SetPresets(20, ( GXColor ) {255, 255, 255, 255}, 3000, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP, ALIGN_LEFT, ALIGN_TOP);

	currentTxt = new GuiText(tr( "Coding:" ));
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(10, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);

	currentTxt = new GuiText("Cyan / Dimok / nIxx / giantpune / ardi");
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(160, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 20;

	currentTxt = new GuiText("hungyip84 / DrayX7 / lustar / r-win");
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(160, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 22;

	char text[100];

	currentTxt = new GuiText(tr( "Design:" ));
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(10, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);

	currentTxt = new GuiText("cyrex / NeoRame");
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(160, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 22;

	currentTxt = new GuiText("");
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(10, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);

	currentTxt = new GuiText("");
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(160, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 22;

	currentTxt = new GuiText(tr( "Big thanks to:" ));
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(10, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);

	sprintf(text, "lustar %s", tr( "for GameTDB and hosting covers / disc images" ));
	currentTxt = new GuiText(text);
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(160, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 20;

	sprintf(text, "Cyan and Shano56 %s", tr( "for their work on the wiki page" ));
	currentTxt = new GuiText(text);
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(160, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 20;

	sprintf(text, "Kinyo %s", tr( "and translators for language files updates" ));
	currentTxt = new GuiText(text);
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(160, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 20;

	sprintf(text, "Deak Phreak %s", tr( "for hosting the themes" ));
	currentTxt = new GuiText(text);
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(160, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 24;

	currentTxt = new GuiText(tr( "Special thanks to:" ));
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(10, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 20;

	sprintf(text, "Waninkoko, Kwiirk & Hermes %s", tr( "for the USB Loader source" ));
	currentTxt = new GuiText(text);
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(10, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 20;

	sprintf(text, "Tantric %s", tr( "for his awesome tool LibWiiGui" ));
	currentTxt = new GuiText(text);
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(10, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 20;

	sprintf(text, "Fishears/Nuke %s", tr( "for Ocarina" ));
	currentTxt = new GuiText(text);
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(10, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 20;

	sprintf(text, "WiiPower %s", tr( "for diverse patches" ));
	currentTxt = new GuiText(text);
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(10, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 20;

	sprintf(text, "Oggzee %s", tr( "for FAT/NTFS support" ));
	currentTxt = new GuiText(text);
	currentTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	currentTxt->SetPosition(10, y);
	currentTxt->SetFont(creditsFont, creditsFontSize);
	txt.push_back(currentTxt);
	y += 20;

	for (i = 0; i < txt.size(); i++)
		creditsWindowBox.Append(txt[i]);

	creditsWindow.Append(&creditsWindowBox);
	creditsWindow.Append(&starImg);
	creditsWindow.Append(&backBtn);

	creditsWindow.SetEffect(EFFECT_FADE, 30);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&creditsWindow);
	ResumeGui();

	while(backBtn.GetState() != STATE_CLICKED)
	{
		usleep(12000);

		if (shutdown)
			Sys_Shutdown();
		if (reset)
			Sys_Reboot();

		angle++;
		if (angle > 360) angle = 0;
		starImg.SetAngle(angle);
	}
	backBtn.ResetState();
	
	creditsMusic->Stop();

	delete creditsMusic;

	creditsWindow.SetEffect(EFFECT_FADE, -30);
	while (creditsWindow.GetEffect() > 0)
		usleep(100);
	HaltGui();
	mainWindow->Remove(&creditsWindow);
	mainWindow->SetState(STATE_DEFAULT);
	for (i = 0; i < txt.size(); i++)
	{
		delete txt[i];
		txt[i] = NULL;
	}
	Settings.FontScaleFactor = oldFontScale;
	ResumeGui();

	bgMusic->Resume();
}

/****************************************************************************
 * WindowScreensaver
 * Display screensaver
 ***************************************************************************/
int WindowScreensaver()
{
	gprintf("WindowScreenSaver()\n");
	bool exit = false;

	/* initialize random seed: */
	srand(time(NULL));

	GuiImageData GXlogo(Resources::GetFile("gxlogo.png"), Resources::GetFileSize("gxlogo.png"));//uncomment for themable screensaver
	//GuiImageData GXlogo(gxlogo_png);//comment for themable screensaver
	GuiImage GXlogoImg(&GXlogo);
	GXlogoImg.SetPosition(172, 152);
	GXlogoImg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	GuiImage BackgroundImg(640, 480, ( GXColor ) {0, 0, 0, 255});
	BackgroundImg.SetPosition(0, 0);
	BackgroundImg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	GuiWindow screensaverWindow(screenwidth, screenheight);
	screensaverWindow.Append(&BackgroundImg);
	screensaverWindow.Append(&GXlogoImg);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&screensaverWindow);
	ResumeGui();

	while (!exit)
	{
		if (shutdown)
			Sys_Shutdown();
		if (reset)
			Sys_Reboot();

		if(!ControlActivityTimeout())
			break;

			/* Set random position */
		GXlogoImg.SetPosition((rand() % 345), (rand() % 305));

		sleep(4);
	}

	HaltGui();
	mainWindow->Remove(&screensaverWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return 1;
}

/****************************************************************************
 * WindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice of up to 4 Buttons.
 *
 * Give him 1 Title, 1 Subtitle and 4 Buttons
 * If title/subtitle or one of the buttons is not needed give him a 0 on that
 * place.
 ***************************************************************************/
int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label,
		const char *btn3Label, const char *btn4Label, int wait)
{
	int choice = -1;
	int count = wait;
	gprintf("WindowPrompt( %s, %s, %s, %s, %s, %s, %i ): ", title, msg, btn1Label, btn2Label, btn3Label, btn4Label, wait);

	PromptWindow *Window = new PromptWindow;
	Window->SetTitle(title);
	Window->SetMessageText(msg);
	if(btn1Label)
		Window->AddButton(btn1Label);
	if(btn2Label)
		Window->AddButton(btn2Label);
	if(btn3Label)
		Window->AddButton(btn3Label);
	if(btn4Label)
		Window->AddButton(btn4Label);

	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(Window);
	ResumeGui();

	while (choice == -1)
	{
		VIDEO_WaitVSync();

		if (shutdown)
		{
			wiilight(0);
			Sys_Shutdown();
		}
		if (reset)
			Sys_Reboot();

		choice = Window->GetChoice();

		if (count > 0) count--;
		if (count == 0) choice = 1;
	}

	delete Window;

	mainWindow->SetState(STATE_DEFAULT);
	gprintf(" %i\n", choice);

	return choice;
}

/****************************************************************************
 * WindowExitPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice of up to 4 Buttons.
 *
 * Give him 1 Titel, 1 Subtitel and 4 Buttons
 * If titel/subtitle or one of the buttons is not needed give him a 0 on that
 * place.
 ***************************************************************************/
int WindowExitPrompt()
{
	gprintf("WindowExitPrompt()\n");

	bgMusic->Pause();

	GuiSound * homein = NULL;
	homein = new GuiSound(Resources::GetFile("menuin.ogg"), Resources::GetFileSize("menuin.ogg"), Settings.sfxvolume);
	homein->SetVolume(Settings.sfxvolume);
	homein->SetLoop(0);
	homein->Play();

	GuiSound * homeout = NULL;
	homeout = new GuiSound(Resources::GetFile("menuout.ogg"), Resources::GetFileSize("menuout.ogg"), Settings.sfxvolume);
	homeout->SetVolume(Settings.sfxvolume);
	homeout->SetLoop(0);

	int choice = -1;

	u64 oldstub = getStubDest();
	loadStub();
	if (oldstub != 0x00010001554c4e52ll && oldstub != 0x00010001554e454fll) Set_Stub(oldstub);

	GuiWindow promptWindow(640, 480);
	promptWindow.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	promptWindow.SetPosition(0, 0);
	GuiImageData top(Resources::GetFile("exit_top.png"), Resources::GetFileSize("exit_top.png"));
	GuiImageData topOver(Resources::GetFile("exit_top_over.png"), Resources::GetFileSize("exit_top_over.png"));
	GuiImageData bottom(Resources::GetFile("exit_bottom.png"), Resources::GetFileSize("exit_bottom.png"));
	GuiImageData bottomOver(Resources::GetFile("exit_bottom_over.png"), Resources::GetFileSize("exit_bottom_over.png"));
	GuiImageData button(Resources::GetFile("exit_button.png"), Resources::GetFileSize("exit_button.png"));
	GuiImageData wiimote(Resources::GetFile("wiimote.png"), Resources::GetFileSize("wiimote.png"));
	GuiImageData close(Resources::GetFile("closebutton.png"), Resources::GetFileSize("closebutton.png"));

	GuiImageData battery(Resources::GetFile("battery_white.png"), Resources::GetFileSize("battery_white.png"));
	GuiImageData batteryBar(Resources::GetFile("battery_bar_white.png"), Resources::GetFileSize("battery_bar_white.png"));
	GuiImageData batteryRed(Resources::GetFile("battery_red.png"), Resources::GetFileSize("battery_red.png"));
	GuiImageData batteryBarRed(Resources::GetFile("battery_bar_red.png"), Resources::GetFileSize("battery_bar_red.png"));

	int i = 0, ret = 0, level;
	char txt[3];
	GuiText * batteryTxt[4];
	GuiImage * batteryImg[4];
	GuiImage * batteryBarImg[4];
	GuiButton * batteryBtn[4];

	for (i = 0; i < 4; i++)
	{
		sprintf(txt, "P%d", i + 1);

		batteryTxt[i] = new GuiText(txt, 22, ( GXColor ) {255, 255, 255, 255});
		batteryTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryImg[i] = new GuiImage(&battery);
		batteryImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryImg[i]->SetPosition(36, 0);
		batteryImg[i]->SetTileHorizontal(0);
		batteryBarImg[i] = new GuiImage(&batteryBar);
		batteryBarImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryBarImg[i]->SetPosition(33, 0);

		batteryBtn[i] = new GuiButton(40, 20);
		batteryBtn[i]->SetLabel(batteryTxt[i]);
		batteryBtn[i]->SetImage(batteryBarImg[i]);
		batteryBtn[i]->SetIcon(batteryImg[i]);
		batteryBtn[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryBtn[i]->SetRumble(false);
		batteryBtn[i]->SetAlpha(70);
		batteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);
	}

	batteryBtn[0]->SetPosition(180, 150);
	batteryBtn[1]->SetPosition(284, 150);
	batteryBtn[2]->SetPosition(388, 150);
	batteryBtn[3]->SetPosition(494, 150);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText titleTxt(tr( "HOME Menu" ), 36, ( GXColor ) {255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	titleTxt.SetPosition(-180, 40);
	titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	GuiText closeTxt(tr( "Close" ), 28, ( GXColor ) {0, 0, 0, 255});
	closeTxt.SetPosition(10, 3);
	GuiImage closeImg(&close);
	if (Settings.wsprompt)
	{
		closeTxt.SetWidescreen(Settings.widescreen);
		closeImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton closeBtn(close.GetWidth(), close.GetHeight());
	closeBtn.SetImage(&closeImg);
	closeBtn.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	closeBtn.SetPosition(190, 30);
	closeBtn.SetLabel(&closeTxt);
	closeBtn.SetRumble(false);
	closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	GuiImage btn1Img(&top);
	GuiImage btn1OverImg(&topOver);
	GuiButton btn1(&btn1Img, &btn1OverImg, 0, 3, 0, 0, &trigA, btnSoundOver, btnSoundClick2, 0);
	btn1.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	GuiText btn2Txt(tr( "Homebrew Channel" ), 26, ( GXColor ) {0, 0, 0, 255});
	if (Settings.HomeMenu == HOME_MENU_SYSTEM)
	{
		btn2Txt.SetText(tr( "Wii Menu" ));
	}
	else if (Settings.HomeMenu == HOME_MENU_FULL)
	{
		btn2Txt.SetText(tr( "Exit" ));
	}
	GuiImage btn2Img(&button);
	if (Settings.wsprompt)
	{
		btn2Txt.SetWidescreen(Settings.widescreen);
		btn2Img.SetWidescreen(Settings.widescreen);
	}
	GuiButton btn2(&btn2Img, &btn2Img, 2, 5, -150, 0, &trigA, btnSoundOver, btnSoundClick2, 1);
	btn2.SetLabel(&btn2Txt);
	btn2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50);
	btn2.SetRumble(false);
	btn2.SetPosition(-150, 0);

	GuiText btn3Txt(tr( "Wii Menu" ), 26, ( GXColor ) {0, 0, 0, 255});
	if (Settings.HomeMenu == HOME_MENU_SYSTEM)
	{
	   btn3Txt.SetText(tr( "Reset" ));
	}
	else if (Settings.HomeMenu == HOME_MENU_FULL)
	{
		btn3Txt.SetText(tr( "Shutdown Wii" ));
	}
	GuiImage btn3Img(&button);
	if (Settings.wsprompt)
	{
		btn3Txt.SetWidescreen(Settings.widescreen);
		btn3Img.SetWidescreen(Settings.widescreen);
	}
	GuiButton btn3(&btn3Img, &btn3Img, 2, 5, 150, 0, &trigA, btnSoundOver, btnSoundClick2, 1);
	btn3.SetLabel(&btn3Txt);
	btn3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 50);
	btn3.SetRumble(false);
	btn3.SetPosition(150, 0);

	GuiImage btn4Img(&bottom);
	GuiImage btn4OverImg(&bottomOver);
	GuiButton btn4(&btn4Img, &btn4OverImg, 0, 4, 0, 0, &trigA, btnSoundOver, btnSoundClick2, 0);
	btn4.SetTrigger(&trigB);
	btn4.SetTrigger(&trigHome);
	btn4.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);

	GuiImage wiimoteImg(&wiimote);
	if (Settings.wsprompt)
	{
		wiimoteImg.SetWidescreen(Settings.widescreen);
	}
	wiimoteImg.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	wiimoteImg.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);
	wiimoteImg.SetPosition(50, 210);

	promptWindow.Append(&btn2);
	promptWindow.Append(&btn3);
	promptWindow.Append(&btn4);
	promptWindow.Append(&btn1);
	promptWindow.Append(&closeBtn);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&wiimoteImg);

	promptWindow.Append(batteryBtn[0]);
	promptWindow.Append(batteryBtn[1]);
	promptWindow.Append(batteryBtn[2]);
	promptWindow.Append(batteryBtn[3]);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	ResumeGui();

	while (choice == -1)
	{
		VIDEO_WaitVSync();

		for (i = 0; i < 4; i++)
		{
			if (WPAD_Probe(i, NULL) == WPAD_ERR_NONE) // controller connected
			{
				level = (userInput[i].wpad.battery_level / 100.0) * 4;
				if (level > 4) level = 4;

				if (level <= 1)
				{
					batteryBarImg[i]->SetImage(&batteryBarRed);
					batteryImg[i]->SetImage(&batteryRed);
				}
				else
				{
					batteryBarImg[i]->SetImage(&batteryBar);
				}

				batteryImg[i]->SetTileHorizontal(level);

				batteryBtn[i]->SetAlpha(255);
			}
			else // controller not connected
			{
				batteryImg[i]->SetTileHorizontal(0);
				batteryImg[i]->SetImage(&battery);
				batteryBtn[i]->SetAlpha(70);
			}
		}

		if (shutdown)
		{
			wiilight(0);
			Sys_Shutdown();
		}
		if (reset)
			Sys_Reboot();

		if (btn1.GetState() == STATE_CLICKED)
		{
			choice = 1;
			btn1.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			btn4.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
			btn2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
			btn3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
			titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			wiimoteImg.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);

			for (int i = 0; i < 4; i++)
				batteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);

		}
		else if (btn4.GetState() == STATE_SELECTED)
		{
			wiimoteImg.SetPosition(50, 165);
		}
		else if (btn2.GetState() == STATE_CLICKED)
		{
			if (Settings.HomeMenu == HOME_MENU_SYSTEM)
				Sys_LoadMenu();
			else if (Settings.HomeMenu == HOME_MENU_DEFAULT)
				Sys_LoadHBC();
			else if (Settings.HomeMenu == HOME_MENU_FULL)
			{
				ret = WindowPrompt(tr( "Exit to where?" ), 0, tr( "Homebrew Channel" ), tr( "Wii Menu" ), tr( "Reset" ), tr( "Cancel" ));
				if (ret == 1)
					Sys_LoadHBC();
				else if(ret == 2)
					Sys_LoadMenu();
				else if(ret == 3)
					RebootApp();
			}
			HaltGui();
			mainWindow->SetState(STATE_DISABLED);
			promptWindow.SetState(STATE_DEFAULT);
			ResumeGui();
			btn2.ResetState();
		}
		else if (btn3.GetState() == STATE_CLICKED)
		{
			if (Settings.HomeMenu == HOME_MENU_SYSTEM)
				RebootApp();
			else if (Settings.HomeMenu == HOME_MENU_DEFAULT)
				Sys_LoadMenu();
			else if (Settings.HomeMenu == HOME_MENU_FULL)
			{
				ret = WindowPrompt(tr( "How to Shutdown?" ), 0, tr( "Full shutdown" ), tr( "Standby" ), tr("Cancel"));
				if (ret == 1)
					Sys_ShutdownToStandby();
				else if(ret == 2)
					Sys_ShutdownToIdle();
			}
			HaltGui();
			mainWindow->SetState(STATE_DISABLED);
			promptWindow.SetState(STATE_DEFAULT);
			ResumeGui();
			btn3.ResetState();
		}
		else if (btn4.GetState() == STATE_CLICKED)
		{
			btn1.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			btn4.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
			btn2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
			btn3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
			titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			wiimoteImg.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);

			for (int i = 0; i < 4; i++)
				batteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);

			choice = 0;
		}
		else if (btn4.GetState() != STATE_SELECTED)
		{
			wiimoteImg.SetPosition(50, 210);
		}
	}
	homeout->Play();
	while (btn1.GetEffect() > 0)
		usleep(100);
	while (promptWindow.GetEffect() > 0)
		usleep(100);
	HaltGui();
	homein->Stop();
	delete homein;
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	while (homeout->IsPlaying() > 0)
		usleep(100);
	homeout->Stop();
	delete homeout;

	for(int i = 0; i < 4; ++i)
	{
		delete batteryTxt[i];
		delete batteryImg[i];
		delete batteryBarImg[i];
		delete batteryBtn[i];
	}

	ResumeGui();
	bgMusic->Resume();

	return choice;
}

/****************************************************************************
 * DiscWait
 ***************************************************************************/
int DiscWait(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, int IsDeviceWait)
{
	int ret = 0;
	u32 cover = 0;

	GuiWindow promptWindow(472, 320);
	promptWindow.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);

	GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
	GuiImageData dialogBox(Resources::GetFile("dialogue_box.png"), Resources::GetFileSize("dialogue_box.png"));
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt)
	{
		dialogBoxImg.SetWidescreen(Settings.widescreen);
	}

	GuiText titleTxt(title, 26, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	titleTxt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	titleTxt.SetPosition(0, 60);
	GuiText msgTxt(msg, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	msgTxt.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	msgTxt.SetPosition(0, -40);
	msgTxt.SetMaxWidth(430);

	GuiText btn1Txt(btn1Label, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt)
	{
		btn1Txt.SetWidescreen(Settings.widescreen);
		btn1Img.SetWidescreen(Settings.widescreen);
	}
	GuiButton btn1(&btn1Img, &btn1Img, 1, 5, 0, 0, &trigA, btnSoundOver, btnSoundClick2, 1);

	if (btn2Label)
	{
		btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		btn1.SetPosition(40, -45);
	}
	else
	{
		btn1.SetAlignment(ALIGN_CENTER, ALIGN_BOTTOM);
		btn1.SetPosition(0, -45);
	}

	btn1.SetLabel(&btn1Txt);
	btn1.SetTrigger(&trigB);
	btn1.SetState(STATE_SELECTED);

	GuiText btn2Txt(btn2Label, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	GuiImage btn2Img(&btnOutline);
	if (Settings.wsprompt)
	{
		btn2Txt.SetWidescreen(Settings.widescreen);
		btn2Img.SetWidescreen(Settings.widescreen);
	}
	GuiButton btn2(&btn2Img, &btn2Img, 1, 4, -20, -25, &trigA, btnSoundOver, btnSoundClick2, 1);
	btn2.SetLabel(&btn2Txt);

	if (Settings.wsprompt && Settings.widescreen) /////////////adjust buttons for widescreen
	{
		msgTxt.SetMaxWidth(380);
		if (btn2Label)
		{
			btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
			btn2.SetPosition(-70, -80);
			btn1.SetPosition(70, -80);
		}
		else
		{
			btn1.SetAlignment(ALIGN_CENTER, ALIGN_BOTTOM);
			btn1.SetPosition(0, -80);
		}
	}

	GuiText timerTxt((char*) NULL, 26, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	timerTxt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	timerTxt.SetPosition(0, 160);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);

	if (btn1Label) promptWindow.Append(&btn1);
	if (btn2Label) promptWindow.Append(&btn2);
	if (IsDeviceWait) promptWindow.Append(&timerTxt);

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	ResumeGui();

	if (IsDeviceWait)
	{
		time_t starttime = time(0);
		time_t timenow = starttime;
		do
		{
			gprintf("%i\n", (int) (timenow-starttime));
			ret = WBFS_Init(WBFS_DEVICE_USB);
			if (ret >= 0) break;

			timerTxt.SetTextf("%i %s", (int) (30-(timenow-starttime)), tr( "seconds left" ));
			DeviceHandler::Instance()->UnMountAllUSB();
			DeviceHandler::Instance()->MountAllUSB();
			timenow = time(0);
		}
		while (timenow-starttime < 30);
	}
	else
	{
		while (!(cover & 0x2))
		{
			VIDEO_WaitVSync();
			if (btn1.GetState() == STATE_CLICKED)
			{
				btn1.ResetState();
				break;
			}
			ret = WDVD_GetCoverStatus(&cover);
			if (ret < 0) break;
		}
	}

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while (promptWindow.GetEffect() > 0)
		usleep(100);
	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return ret;
}

/****************************************************************************
 * FormatingPartition
 ***************************************************************************/
int FormatingPartition(const char *title, int part_num)
{
	PartitionHandle * usbHandle = DeviceHandler::Instance()->GetUSBHandleFromPartition(part_num);
	int portPart = DeviceHandler::PartitionToPortPartition(part_num);

	char text[255];
	sprintf(text, "%s: %.2fGB", tr( "Partition" ), usbHandle->GetSize(portPart) / GB_SIZE);
	int choice = WindowPrompt(tr( "Do you want to format:" ), text, tr( "Yes" ), tr( "No" ));
	if (choice == 0)
		return -666;

	int ret;
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

	GuiText titleTxt(title, 26, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	titleTxt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	titleTxt.SetPosition(0, 60);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	ResumeGui();

	VIDEO_WaitVSync();
	ret = WBFS_Format(usbHandle->GetLBAStart(portPart), usbHandle->GetSecCount(portPart), DeviceHandler::PartitionToUSBPort(part_num));

	if (ret < 0)
	{
		WindowPrompt(tr( "Error !" ), tr( "Failed formating" ), tr( "Return" ));
	}
	else
	{
		PartitionFS * partition = usbHandle->GetPartitionRecord(portPart);
		partition->PartitionType = 0xBF;
		partition->FSName = "WBFS";
		sleep(1);
		ret = WBFS_OpenPart(part_num);
		sprintf(text, "%s %s", text, tr( "formatted!" ));
		WindowPrompt(tr( "Success:" ), text, tr( "OK" ));
		if (ret < 0)
		{
			WindowPrompt(tr( "ERROR" ), tr( "Failed to open partition" ), tr( "OK" ));
			Sys_LoadMenu();
		}
	}

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while (promptWindow.GetEffect() > 0)
		usleep(100);
	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return ret;
}

/****************************************************************************
 * NetworkInitPrompt
 ***************************************************************************/
bool NetworkInitPrompt()
{
	gprintf("\nNetworkinitPrompt()");
	if (IsNetworkInit()) return true;

	bool success = true;

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

	GuiText titleTxt(tr( "Initializing Network" ), 26, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	titleTxt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	titleTxt.SetPosition(0, 60);

	char msg[20] = " ";
	GuiText msgTxt(msg, 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	msgTxt.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	msgTxt.SetPosition(0, -40);

	GuiText btn1Txt(tr( "Cancel" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt)
	{
		btn1Txt.SetWidescreen(Settings.widescreen);
		btn1Img.SetWidescreen(Settings.widescreen);
	}
	GuiButton btn1(&btn1Img, &btn1Img, 2, 4, 0, -45, &trigA, btnSoundOver, btnSoundClick2, 1);
	btn1.SetLabel(&btn1Txt);
	btn1.SetState(STATE_SELECTED);

	if ((Settings.wsprompt) && (Settings.widescreen)) /////////////adjust buttons for widescreen
	{
		btn1.SetAlignment(ALIGN_CENTER, ALIGN_BOTTOM);
		btn1.SetPosition(0, -80);
	}

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
	promptWindow.Append(&btn1);

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	ResumeGui();

	int iTimeout = 100 * 200;   // 20s

	ResumeNetworkThread();

	while (!IsNetworkInit())
	{
		usleep(100000);

		if (--iTimeout == 0)
		{
			msgTxt.SetText(tr( "Could not initialize network, time out!" ));
			sleep(3);
			success = false;
			break;
		}

		if (btn1.GetState() == STATE_CLICKED)
		{
			btn1.ResetState();
			success = false;
			break;
		}
	}

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while (promptWindow.GetEffect() > 0)
		usleep(1000);

	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();

	if (IsNetworkInit())
		HaltNetworkThread();

	return success;
}
int CodeDownload(const char *id)
{
	if (!CreateSubfolder(Settings.TxtCheatcodespath))
	{
		WindowPrompt(tr( "Error !" ), tr( "Can't create directory" ), tr( "OK" ));
		return -1;
	}

	int ret = -1;

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

	char title[50];
	sprintf(title, "%s", tr( "Code Download" ));
	GuiText titleTxt(title, 26, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	titleTxt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	titleTxt.SetPosition(0, 50);
	char msg[50];
	sprintf(msg, "%s", tr( "Initializing Network" ));
	GuiText msgTxt(msg, 26, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	msgTxt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	msgTxt.SetPosition(0, 140);
	char msg2[50] = " ";
	GuiText msg2Txt(msg2, 26, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	msg2Txt.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	msg2Txt.SetPosition(0, 50);

	GuiText btn1Txt(tr( "Cancel" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt)
	{
		btn1Txt.SetWidescreen(Settings.widescreen);
		btn1Img.SetWidescreen(Settings.widescreen);
	}
	GuiButton btn1(&btn1Img, &btn1Img, 2, 4, 0, -40, &trigA, btnSoundOver, btnSoundClick2, 1);
	btn1.SetLabel(&btn1Txt);
	btn1.SetState(STATE_SELECTED);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
	promptWindow.Append(&msg2Txt);
	promptWindow.Append(&btn1);

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	ResumeGui();

	while (!IsNetworkInit())
	{
		VIDEO_WaitVSync();

		Initialize_Network();

		if (IsNetworkInit())
		{
			msgTxt.SetText(GetNetworkIP());
		}
		else
		{
			msgTxt.SetText(tr( "Could not initialize network!" ));
		}
		if (btn1.GetState() == STATE_CLICKED)
		{
			btn1.ResetState();
			ret = 0;
			break;
		}
	}

	if (IsNetworkInit())
	{
		char txtpath[250];
		snprintf(txtpath, sizeof(txtpath), "%s%s.txt", Settings.TxtCheatcodespath, id);

		char codeurl[250];
		snprintf(codeurl, sizeof(codeurl), "http://geckocodes.org/txt.php?txt=%s", id);
		//snprintf(codeurl, sizeof(codeurl), "http://geckocodes.org/codes/G/%s.txt", id);
		
		struct block file = downloadfile(codeurl);

		if (file.data != NULL)
		{
			bool validUrl = false;
			if(file.size > 0)
			{
				char *textCpy = new (std::nothrow) char[file.size+1];
				if(textCpy)
				{
					memcpy(textCpy, file.data, file.size);
					textCpy[file.size] = '\0';
					validUrl = (strcasestr(textCpy, "404 Not Found") == 0);
					delete [] textCpy;
				}
			}

			if(!validUrl)
			{
				snprintf(codeurl, sizeof(codeurl), "%s%s", codeurl, tr( " is not on the server." ));
				WindowPrompt(tr( "Error" ), codeurl, tr( "OK" ));
			}
			else
			{
				FILE * pfile = fopen(txtpath, "wb");
				if(pfile)
				{
					fwrite(file.data, 1, file.size, pfile);
					fclose(pfile);
					
					// verify downloaded content - thanks airline38
					pfile = fopen(txtpath, "rb");
					if(pfile)
					{
						char target[4];
						fseek(pfile,0,SEEK_SET);
						fread(target, sizeof(char), 4, pfile);
						fclose(pfile);
						//printf("target=%s  game id=%s\n",target,id);
						if (strncmp(target,id,4)== 0 )
						{
							snprintf(txtpath, sizeof(txtpath), "%s%s", txtpath, tr(" has been Saved.  The text has not been verified.  Some of the code may not work right with each other.  If you experience trouble, open the text in a real text editor for more information." ));
							WindowPrompt(0, txtpath, tr( "OK" ));
							ret = 0;
						}
						else
						{
							RemoveFile(txtpath);
							snprintf(codeurl, sizeof(codeurl), "%s%s", codeurl, tr( " is not on the server." ));
							WindowPrompt(tr( "Error" ), codeurl, tr( "OK" ));
						}
					}
				}
				else
					WindowPrompt(tr("Error"), tr("Could not write file."), tr( "OK" ));
			}
			free(file.data);
		}
		else
		{
			snprintf(codeurl, sizeof(codeurl), "%s%s", codeurl, tr(" could not be downloaded."));
			WindowPrompt(tr( "Error" ), codeurl, tr( "OK" ));
		}
	}


	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while (promptWindow.GetEffect() > 0)
		usleep(100);

	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();

	return ret;
}

