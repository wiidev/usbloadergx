/****************************************************************************
 * TitleBrowser
 * USB Loader GX 2009
 *
 * TitleBrowser.cpp   *giantpune*
 ***************************************************************************/

#include <dirent.h>
#include <zlib.h>

#include "language/gettext.h"
#include "GUI/gui.h"
#include "GUI/gui_optionbrowser.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "network/networkops.h"
#include "network/http.h"
#include "FileOperations/fileops.h"
#include "themes/CTheme.h"
#include "sys.h"
#include "menu.h"
#include "audio.h"
#include "xml/GameTDB.hpp"
#include "wad/nandtitle.h"
#include "gecko.h"

#include "Controls/DeviceHandler.hpp"
#include "usbloader/NandEmu.h"
extern u8 shutdown;
extern u8 reset;

/********************************************************************************
 * TitleBrowser- opens a browser with a list of installed Titles
 *********************************************************************************/
bool TitleSelector(char output[])
{
	gprintf("TitleSelector()\n");

	s32 num_titles;
	s32 r = -1;
	bool ret = false;
	u64 *titleList = NULL;

	// Get count of titles of the good titles
	num_titles = NandTitles.SetType(0x10001);
	u32 n = num_titles;

	for (u32 i = 0; i < n; i++)
	{
		u64 tid = NandTitles.Next();
		if (!tid)
		{
			break;
		}

		//remove ones not actually installed on the nand
		if (!NandTitles.Exists(tid))
		{
			num_titles--;
		}
	}

	//make a list of just the tids we are adding to the titlebrowser
	titleList = (u64*) memalign(32, num_titles * sizeof(u64));
	if (!titleList)
	{
		gprintf("TitleLister(): out of memory!\n");
		return false;
	}
	OptionList options4;
	//write the titles on the option browser

	std::string Filepath = Settings.titlestxt_path;
	Filepath += "wiitdb.xml";

	GameTDB *XML_DB = new GameTDB(Filepath.c_str());
	XML_DB->SetLanguageCode(Settings.db_language);

	s32 i = 0;
	NandTitles.SetType(0x10001);
	while (i < num_titles)
	{
		u64 tid = NandTitles.Next();
		if (!tid)
		{
			gprintf("shit happened\n");
			break;
		}

		if (!NandTitles.Exists(tid))
			continue;

		char id[5];
		NandTitles.AsciiTID(tid, (char*) &id);

		const char* name = NULL;
		std::string TitleName;

		if(XML_DB->GetTitle(id, TitleName))
			name = TitleName.c_str();
		else
			name = NandTitles.NameOf(tid);
		//gprintf("%016llx: %s: %s\n%p\t%p\n", tid, id, name, &id, name );

		options4.SetName(i, "%s", id);
		options4.SetValue(i, "%s", name ? name : tr( "Unknown" ));
		titleList[i] = tid;
		i++;
	}

	delete XML_DB;
	XML_DB = NULL;

	options4.SetName(i, " ");
	options4.SetValue(i, "%s", tr( "Clear" ));

	bool exit = false;

	GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
	GuiImageData settingsbg(Resources::GetFile("settings_background.png"), Resources::GetFileSize("settings_background.png"));

	GuiImage settingsbackground(&settingsbg);
	GuiButton settingsbackgroundbtn(settingsbackground.GetWidth(), settingsbackground.GetHeight());
	settingsbackgroundbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsbackgroundbtn.SetPosition(0, 0);
	settingsbackgroundbtn.SetImage(&settingsbackground);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiText cancelBtnTxt(tr( "Back" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	cancelBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
	GuiImage cancelBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		cancelBtnTxt.SetWidescreen(Settings.widescreen);
		cancelBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton cancelBtn(&cancelBtnImg, &cancelBtnImg, 2, 3, 180, 400, &trigA, btnSoundOver, btnSoundClick2, 1);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetTrigger(&trigB);

	GuiOptionBrowser optionBrowser4(396, 280, &options4, "bg_options_settings.png");
	optionBrowser4.SetPosition(0, 90);
	optionBrowser4.SetAlignment(ALIGN_CENTER, ALIGN_TOP);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&settingsbackgroundbtn);
	w.Append(&cancelBtn);
	w.Append(&optionBrowser4);
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&w);

	ResumeGui();

	while (!exit)
	{
		VIDEO_WaitVSync();

		if (shutdown == 1) Sys_Shutdown();
		if (reset == 1) Sys_Reboot();

		r = optionBrowser4.GetClickedOption();

		if (r > -1)
		{ //if a click happened
			if (r < num_titles)
			{
				u64 tid = titleList[r];
				sprintf(output, "%08x", (unsigned int)TITLE_LOWER( tid ));
			}
			else output[0] = 0;
			ret = true;
			exit = true;
		}

		else if (cancelBtn.GetState() == STATE_CLICKED)
		{
			//break the loop and end the function
			exit = true;
		}
	}

	HaltGui();
	mainWindow->SetState(STATE_DEFAULT);
	mainWindow->Remove(&w);
	free(titleList);
	ResumeGui();

	return ret;
}

int TitleBrowser()
{
	u32 num_titles;
	u32 num_sys_titles;
	s32 ret = -1;
	u64 *titleList = NULL;

	// Get count of titles of the good titles
	num_titles = NandTitles.SetType(0x10001);
	u32 n = num_titles;
	for (u32 i = 0; i < n; i++)
	{
		u64 tid = NandTitles.Next();
		if (!tid)
		{
			break;
		}

		//remove ones not actually installed on the nand
		if (!NandTitles.Exists(tid))
		{
			num_titles--;
		}
	}

	// Get count of system titles
	num_sys_titles = NandTitles.SetType(0x10002);
	n = num_sys_titles;
	for (u32 i = 0; i < n; i++)
	{
		u64 tid = NandTitles.Next();
		if (!tid)
		{
			break;
		}
		//these can't be booted anyways
		if (TITLE_LOWER( tid ) == 0x48414741 || TITLE_LOWER( tid ) == 0x48414141 || TITLE_LOWER( tid ) == 0x48414641)
		{
			num_sys_titles--;
			continue;
		}

		//these aren't installed on the nand
		if (!NandTitles.Exists(tid))
		{
			num_sys_titles--;
		}
	}

	//make a list of just the tids we are adding to the titlebrowser
	titleList = (u64*) memalign(32, (num_titles + num_sys_titles) * sizeof(u64));
	if (!titleList)
	{
		gprintf("TitleBrowser(): out of memory!\n");
		return -1;
	}
	OptionList options3;
	//write the titles on the option browser

	std::string Filepath = Settings.titlestxt_path;
	Filepath += "wiitdb.xml";

	GameTDB *XML_DB = new GameTDB(Filepath.c_str());
	XML_DB->SetLanguageCode(Settings.db_language);

	u32 i = 0;
	NandTitles.SetType(0x10001);
	//first add the good stuff
	while (i < num_titles)
	{
		u64 tid = NandTitles.Next();
		if (!tid)
		{
			gprintf("shit happened3\n");
			break;
		}
		gprintf("[ %u ] tid: %016llx\t%s\n", i, tid, NandTitles.NameOf(tid));

		if (!NandTitles.Exists(tid))
		{
			continue;
		}

		char id[5];
		NandTitles.AsciiTID(tid, (char*) &id);

		const char* name = NULL;
		std::string TitleName;

		if(XML_DB->GetTitle(id, TitleName))
			name = TitleName.c_str();
		else
			name = NandTitles.NameOf(tid);

		options3.SetName(i, "%s", id);
		options3.SetValue(i, "%s", name ? name : tr( "Unknown" ));
		titleList[i] = tid;
		i++;
	}

	NandTitles.SetType(0x10002);
	while (i < num_sys_titles + num_titles)
	{
		u64 tid = NandTitles.Next();
		if (!tid)
		{
			break;
		}
		if (TITLE_LOWER( tid ) == 0x48414741 || TITLE_LOWER( tid ) == 0x48414141 || TITLE_LOWER( tid ) == 0x48414641) continue;

		if (!NandTitles.Exists(tid))
		{
			continue;
		}

		char id[5];
		NandTitles.AsciiTID(tid, (char*) &id);

		const char* name = NULL;
		std::string TitleName;

		if(XML_DB->GetTitle(id, TitleName))
			name = TitleName.c_str();
		else
			name = NandTitles.NameOf(tid);

		options3.SetName(i, "%s", id);
		options3.SetValue(i, "%s", name ? name : tr( "Unknown" ));
		titleList[i] = tid;
		i++;
	}

	delete XML_DB;
	XML_DB = NULL;

	if (i == num_titles + num_sys_titles)
	{
		options3.SetName(i, " ");
		options3.SetValue(i, "%s", tr( "Wii Settings" ));
	}

	bool exit = false;
	int total = num_titles + num_sys_titles;

	if (IsNetworkInit()) ResumeNetworkWait();

	GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
	GuiImageData settingsbg(Resources::GetFile("settings_background.png"), Resources::GetFileSize("settings_background.png"));

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiText titleTxt(tr( "Title Launcher" ), 28, ( GXColor )
	{   0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	titleTxt.SetPosition(12, 40);
	titleTxt.SetMaxWidth(356, SCROLL_HORIZONTAL);

	GuiImage settingsbackground(&settingsbg);
	GuiButton settingsbackgroundbtn(settingsbackground.GetWidth(), settingsbackground.GetHeight());
	settingsbackgroundbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsbackgroundbtn.SetPosition(0, 0);
	settingsbackgroundbtn.SetImage(&settingsbackground);

	GuiText cancelBtnTxt(tr( "Back" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	cancelBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
	GuiImage cancelBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		cancelBtnTxt.SetWidescreen(Settings.widescreen);
		cancelBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton cancelBtn(&cancelBtnImg, &cancelBtnImg, 2, 3, 180, 400, &trigA, btnSoundOver, btnSoundClick2, 1);
	cancelBtn.SetScale(0.9);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetTrigger(&trigB);

	GuiOptionBrowser optionBrowser3(396, 280, &options3, "bg_options_settings.png");
	optionBrowser3.SetPosition(0, 90);
	optionBrowser3.SetAlignment(ALIGN_CENTER, ALIGN_TOP);

	GuiImageData wifiImgData(Resources::GetFile("wifi_btn.png"), Resources::GetFileSize("wifi_btn.png"));
	GuiImage wifiImg(&wifiImgData);
	if (Settings.wsprompt)
	{
		wifiImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton wifiBtn(wifiImg.GetWidth(), wifiImg.GetHeight());
	wifiBtn.SetImage(&wifiImg);
	wifiBtn.SetPosition(100, 400);
	wifiBtn.SetEffectGrow();
	wifiBtn.SetAlpha(80);
	wifiBtn.SetTrigger(&trigA);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&settingsbackgroundbtn);
	w.Append(&titleTxt);
	w.Append(&cancelBtn);
	w.Append(&wifiBtn);
	w.Append(&optionBrowser3);
	mainWindow->Append(&w);

	ResumeGui();

	while (!exit)
	{
		VIDEO_WaitVSync();

		if (shutdown == 1) Sys_Shutdown();
		if (reset == 1)
			Sys_Reboot();

		else if (wifiBtn.GetState() == STATE_CLICKED)
		{
			ResumeNetworkWait();
			wifiBtn.ResetState();
		}

		if (IsNetworkInit())
		{
			wifiBtn.SetAlpha(255);
		}

		ret = optionBrowser3.GetClickedOption();

		if (ret > -1)
		{ //if a click happened

			if (ret < total)
			{
				//set the title's name, number, ID to text
				char text[0x100];
				char id[5];
				NandTitles.AsciiTID(titleList[ret], (char*) &id);

				snprintf(text, sizeof(text), "%s : %s", id, NandTitles.NameOf(titleList[ret]));

				//prompt to boot selected title
				if (WindowPrompt(tr( "Boot?" ), text, tr( "OK" ), tr( "Cancel" )))
				{ //if they say yes
					ExitApp();
					WII_Initialize();
					WII_LaunchTitle(titleList[ret]);
					//this really shouldn't be needed because the title will be booted
					exit = true;
					break;
				}
				else
				{
					//if they said no to booting the title
					ret = -1;
					optionBrowser3.ResetState();
				}

			}
			else if (ret == total)
			{ //if they clicked to go to the wii settings
				ExitApp();
				WII_Initialize();
				WII_ReturnToSettings();
			}
		}
		if (cancelBtn.GetState() == STATE_CLICKED)
		{
			//break the loop and end the function
			exit = true;
			ret = -10;
		}
	}

	CloseConnection();
	if (IsNetworkInit()) HaltNetworkThread();

	HaltGui();
	mainWindow->Remove(&w);
	free(titleList);
	ResumeGui();

	return ret;
}

