/****************************************************************************
 * Copyright (C) 2011
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include <string.h>
#include <unistd.h>

#include "ThemeDownloader.h"
#include "language/gettext.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "FileOperations/DirList.h"
#include "network/networkops.h"
#include "themes/CTheme.h"
#include "FileOperations/fileops.h"
#include "sys.h"
#include "network/FileDownloader.h"
#include "network/http.h"
#include "menu/menus.h"
#include "ZipFile.h"
#include "utils/ShowError.h"
#include "utils/tools.h"
#include "gecko.h"


ThemeDownloader::ThemeDownloader()
	: FlyingButtonsMenu(tr("Theme Downloader"))
{
	ThemeList = NULL;
	delete MainButtonImgData;
	delete MainButtonImgOverData;

	ParentMenu = MENU_SETTINGS;

	ThemeListURL = "http://wii.spiffy360.com/themes.php?xml=1&category=1&adult=";
	if(Settings.godmode)
		ThemeListURL += "1";
	else
		ThemeListURL += "0";

	MainButtonImgData = Resources::GetImageData("theme_box.png");
	MainButtonImgOverData = NULL;

	urlTxt = new GuiText(tr( "Themes by www.spiffy360.com" ), 22, (GXColor) {255, 255, 255, 255});
	urlTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	urlTxt->SetPosition(350, 12);
	Append(urlTxt);

	for(int i = 0; i < 4; ++i)
		ThemePreviews[i] = NULL;


	defaultBtnTxt = new GuiText(tr( "Default" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	defaultBtnTxt->SetMaxWidth(btnOutline->GetWidth() - 30);
	defaultBtnImg = new GuiImage(btnOutline);
	if (Settings.wsprompt)
	{
		defaultBtnTxt->SetWidescreen(Settings.widescreen);
		defaultBtnImg->SetWidescreen(Settings.widescreen);
	}
	defaultBtn = new GuiButton(btnOutline->GetWidth(), btnOutline->GetHeight());
	defaultBtn->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	defaultBtn->SetPosition(-20, 400);
	defaultBtn->SetLabel(defaultBtnTxt);
	defaultBtn->SetImage(defaultBtnImg);
	defaultBtn->SetSoundOver(btnSoundOver);
	defaultBtn->SetSoundClick(btnSoundClick2);
	defaultBtn->SetTrigger(trigA);
	defaultBtn->SetEffectGrow();
	Append(defaultBtn);

	backBtn->SetPosition(-205, 400);
}

ThemeDownloader::~ThemeDownloader()
{
	HaltGui();
	for(u32 i = 0; i < MainButton.size(); ++i)
		Remove(MainButton[i]);
	Remove(urlTxt);
	Remove(defaultBtn);

	delete urlTxt;
	delete defaultBtn;
	delete defaultBtnTxt;
	delete defaultBtnImg;
	delete ThemeList;
	for(int i = 0; i < 4; ++i)
		delete ThemePreviews[i];
}

int ThemeDownloader::Execute()
{
	ThemeDownloader * Menu = new ThemeDownloader();
	mainWindow->Append(Menu);

	Menu->ShowMenu();

	int returnMenu = MENU_NONE;

	while((returnMenu = Menu->MainLoop()) == MENU_NONE);

	delete Menu;

	return returnMenu;
}

int ThemeDownloader::MainLoop()
{
	if(defaultBtn->GetState() == STATE_CLICKED)
	{
		int choice = WindowPrompt(0, tr("Do you want to load the default theme?"), tr("Yes"), tr("Cancel"));
		if(choice)
		{
			HaltGui();
			Theme::SetDefault();
			Theme::Reload();
			ResumeGui();
			return MENU_THEMEDOWNLOADER;
		}

		defaultBtn->ResetState();
	}

	return FlyingButtonsMenu::MainLoop();
}

void ThemeDownloader::SetMainButton(int position, const char * ButtonText, GuiImageData * imageData, GuiImageData * themeImg)
{
	if(position >= (int) MainButton.size())
	{
		MainButtonImg.resize(position+1);
		MainButtonImgOver.resize(position+1);
		MainButtonTxt.resize(position+1);
		MainButton.resize(position+1);
	}

	MainButtonImg[position] = new GuiImage(imageData);
	MainButtonImgOver[position] = new GuiImage(themeImg);
	MainButtonImgOver[position]->SetScale(0.4);
	MainButtonImgOver[position]->SetPosition(50, -45);

	MainButtonTxt[position] = new GuiText(ButtonText, 18, ( GXColor ) {0, 0, 0, 255});
	MainButtonTxt[position]->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	MainButtonTxt[position]->SetPosition(0, 10);
	MainButtonTxt[position]->SetMaxWidth(imageData->GetWidth() - 10, DOTTED);

	MainButton[position] = new GuiButton(imageData->GetWidth(), imageData->GetHeight());
	MainButton[position]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	MainButton[position]->SetSoundOver(btnSoundOver);
	MainButton[position]->SetSoundClick(btnSoundClick);
	MainButton[position]->SetImage(MainButtonImg[position]);
	MainButton[position]->SetImageOver(MainButtonImg[position]);
	MainButton[position]->SetIcon(MainButtonImgOver[position]);
	MainButton[position]->SetLabel(MainButtonTxt[position]);
	MainButton[position]->SetTrigger(trigA);
	MainButton[position]->SetEffectGrow();

	switch(position % 4)
	{
		case 0:
			MainButton[position]->SetPosition(90, 75);
			break;
		case 1:
			MainButton[position]->SetPosition(340, 75);
			break;
		case 2:
			MainButton[position]->SetPosition(90, 230);
			break;
		case 3:
			MainButton[position]->SetPosition(340, 230);
			break;
		default:
			break;
	}
}

GuiImageData * ThemeDownloader::GetImageData(int theme)
{
	GuiImageData * ImageData = NULL;
	char filepath[300];
	snprintf(filepath, sizeof(filepath), "%s/tmp/%s.jpg", Settings.theme_path, ThemeList->GetThemeTitle(theme));

	if (!CheckFile(filepath))
	{
		struct block file = downloadfile(ThemeList->GetImageLink(theme));
		char storepath[300];
		snprintf(storepath, sizeof(storepath), "%s/tmp/", Settings.theme_path);
		CreateSubfolder(storepath);
		if (file.data)
		{
			ImageData = new GuiImageData(file.data, file.size, false);

			FILE *storefile = fopen(filepath, "wb");
			if(storefile)
			{
				fwrite(file.data, 1, file.size, storefile);
				fclose(storefile);
			}
			free(file.data);
		}
	}
	else
		ImageData = new GuiImageData(filepath);

	return ImageData;
}

void ThemeDownloader::SetupMainButtons()
{
	ResumeGui();

	if (!IsNetworkInit() && !NetworkInitPrompt())
	{
		ShowError(tr("Could not initialize network!"));
		return;
	}

	ShowProgress(tr("Downloading pagelist:"), "www.spiffy360.com", tr("Please wait..."), 0, 1);

	if(!CheckConnection(ThemeListURL.c_str()))
	{
		ProgressStop();
		ShowError(tr("Connection to server timed out."));
		return;
	}

	ThemeList = new Theme_List(ThemeListURL.c_str());

	if (ThemeList->GetThemeCount() == 0)
	{
		ProgressStop();
		WindowPrompt(tr( "No themes found on the site." ), 0, "OK");
		returnMenu = MENU_SETTINGS;
	}

	for(int i = 0; i < ThemeList->GetThemeCount(); ++i)
	{
		SetMainButton(i, ThemeList->GetThemeTitle(i), MainButtonImgData, NULL);
	}
}

void ThemeDownloader::AddMainButtons()
{
	HaltGui();
	for(u32 i = 0; i < MainButton.size(); ++i)
		Remove(MainButton[i]);
	ResumeGui();

	int FirstItem = currentPage*4;
	int n = 0;

	for(int i = FirstItem; i < (int) MainButton.size() && i < FirstItem+4; ++i)
	{
		delete ThemePreviews[n];
		ShowProgress(tr("Downloading image:"), 0, ThemeList->GetThemeTitle(i), n, 4);
		ThemePreviews[n] = GetImageData(i);
		MainButtonImgOver[i]->SetImage(ThemePreviews[n]);
		n++;
	}
	ProgressStop();
	HaltGui();

	FlyingButtonsMenu::AddMainButtons();
}

void ThemeDownloader::MainButtonClicked(int button)
{
	//! TODO: Clean me
	const char * title = ThemeList->GetThemeTitle(button);
	const char * author = ThemeList->GetThemeAuthor(button);
	GuiImageData *thumbimageData = ThemePreviews[button % 4];
	const char *downloadlink = ThemeList->GetDownloadLink(button);

	gprintf("\nTheme_Prompt(%s ,%s, %s)", title, author, downloadlink);
	bool leave = false;
	int result = 0;

	GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
	GuiImageData dialogBox(Resources::GetFile("theme_dialogue_box.png"), Resources::GetFileSize("theme_dialogue_box.png"));

	GuiImage dialogBoxImg(&dialogBox);

	GuiWindow promptWindow(dialogBox.GetWidth(), dialogBox.GetHeight());
	promptWindow.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiText titleTxt(tr( "Theme Title:" ), 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(230, 30);

	GuiText titleTxt2(title, 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	titleTxt2.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt2.SetPosition(230, 50);
	titleTxt2.SetMaxWidth(dialogBox.GetWidth() - 220, WRAP);

	GuiText authorTxt(tr( "Author(s):" ), 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	authorTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	authorTxt.SetPosition(230, 100);

	GuiText authorTxt2(author, 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	authorTxt2.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	authorTxt2.SetPosition(230, 120);
	authorTxt2.SetMaxWidth(dialogBox.GetWidth() - 220, DOTTED);

	GuiText downloadBtnTxt(tr( "Download" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	downloadBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
	GuiImage downloadBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		downloadBtnTxt.SetWidescreen(Settings.widescreen);
		downloadBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton downloadBtn(&downloadBtnImg, &downloadBtnImg, ALIGN_RIGHT, ALIGN_TOP, -5, 170, &trigA, btnSoundOver, btnSoundClick2, 1);
	downloadBtn.SetLabel(&downloadBtnTxt);
	downloadBtn.SetScale(0.9);

	GuiText backBtnTxt(tr( "Back" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	backBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
	GuiImage backBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		backBtnTxt.SetWidescreen(Settings.widescreen);
		backBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton backBtn(&backBtnImg, &backBtnImg, ALIGN_RIGHT, ALIGN_TOP, -5, 220, &trigA, btnSoundOver, btnSoundClick2, 1);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetTrigger(&trigB);
	backBtn.SetScale(0.9);

	GuiImage ThemeImage(thumbimageData);
	ThemeImage.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	ThemeImage.SetPosition(20, 10);
	ThemeImage.SetScale(0.8);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&ThemeImage);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&titleTxt2);
	promptWindow.Append(&authorTxt);
	promptWindow.Append(&authorTxt2);
	promptWindow.Append(&downloadBtn);
	promptWindow.Append(&backBtn);

	HaltGui();
	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	ResumeGui();

	while (!leave)
	{
		VIDEO_WaitVSync();

		if (shutdown)
			Sys_Shutdown();
		else if (reset)
			Sys_Reboot();

		if (downloadBtn.GetState() == STATE_CLICKED)
		{
			int choice = WindowPrompt(tr( "Do you want to download this theme?" ), title, tr( "Yes" ), tr( "Cancel" ));
			if (choice)
			{
				result = DownloadTheme(downloadlink, title);
				if (result == 2)
				{
					returnMenu = MENU_THEMEDOWNLOADER;
					leave = true;
				}
			}
			mainWindow->SetState(STATE_DISABLED);
			promptWindow.SetState(STATE_DEFAULT);
			downloadBtn.ResetState();
		}

		else if (backBtn.GetState() == STATE_CLICKED)
		{
			leave = true;
			backBtn.ResetState();
		}
	}

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while (promptWindow.GetEffect() > 0) usleep(100);
	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
}

int ThemeDownloader::DownloadTheme(const char *url, const char *title)
{
	if (!url) return -1;

	if(!CreateSubfolder(Settings.theme_path))
	{
		ShowError(tr("Can't create path: %s"), Settings.theme_path);
		return -1;
	}

	DirList oldDir(Settings.theme_path);

	char filepath[300];
	snprintf(filepath, sizeof(filepath), "%s/TempTheme.zip", Settings.theme_path);

	int ret = DownloadFileToPath(url, filepath, false);
	if(ret < 1024)
	{
		ShowError(tr("Error when downloading file: %i"), ret);
		return -2;
	}

	ZipFile *zipfile = new ZipFile(filepath);

	int result = zipfile->ExtractAll(Settings.theme_path);
	if(result < 0)
	{
		WindowPrompt(tr( "Failed to extract." ), tr( "Unsupported format, try to extract manually TempTheme.zip." ), tr( "OK" ));
		return -3;
	}

	std::string themeDir;
	std::string Filename;
	zipfile->FindFilePart(".them", Filename);
	zipfile->FindFilePart("/", themeDir);

	if(!zipfile->FindFile("theme_preview.png") && themeDir.size() != 0)
	{
		size_t pos = themeDir.find("/");
		if(pos != std::string::npos)
		{
			themeDir.erase(pos);
			char filepath[255];
			char newfilepath[255];
			snprintf(filepath, sizeof(filepath), "%s/tmp/%s.jpg", Settings.theme_path, title);
			snprintf(newfilepath, sizeof(newfilepath), "%s/%s/theme_preview.png", Settings.theme_path, themeDir.c_str());
			CopyFile(filepath, newfilepath);
		}

	}

	delete zipfile;
	remove(filepath);

	int choice = WindowPrompt(tr( "Successfully extracted theme." ), tr( "Do you want to apply it now?" ), tr( "Yes" ), tr( "No" ));
	if (choice == 0)
		return -1;

	if(Filename.size() == 0)
	{
		WindowPrompt(tr( "ERROR: Can't set up theme." ), tr( "The .them file was not found in the zip." ), tr( "OK" ));
		return -1;
	}

	char real_themepath[1024];
	snprintf(real_themepath, sizeof(real_themepath), "%s/%s", Settings.theme_path, Filename.c_str());

	if (Theme::Load(real_themepath))
	{
		snprintf(Settings.theme, sizeof(Settings.theme), real_themepath);
		Theme::Reload();
		result = 2;
	}

	return result;
}
