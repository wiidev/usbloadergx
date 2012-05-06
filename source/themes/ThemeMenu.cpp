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

#include "ThemeMenu.h"
#include "language/gettext.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "FileOperations/DirList.h"
#include "network/networkops.h"
#include "themes/CTheme.h"
#include "FileOperations/fileops.h"
#include "sys.h"
#include "menu/menus.h"
#include "utils/ShowError.h"
#include "utils/tools.h"
#include "gecko.h"


ThemeMenu::ThemeMenu()
	: FlyingButtonsMenu(tr("Theme Menu"))
{
	delete MainButtonImgData;
	delete MainButtonImgOverData;

	MainButtonImgData = Resources::GetImageData("theme_box.png");
	MainButtonImgOverData = NULL;

	ParentMenu = MENU_SETTINGS;

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

ThemeMenu::~ThemeMenu()
{
	HaltGui();
	for(u32 i = 0; i < MainButton.size(); ++i)
		Remove(MainButton[i]);
	Remove(defaultBtn);

	delete defaultBtn;
	delete defaultBtnTxt;
	delete defaultBtnImg;
	for(int i = 0; i < 4; ++i)
		delete ThemePreviews[i];
}

int ThemeMenu::Execute()
{
	ThemeMenu * Menu = new ThemeMenu();
	mainWindow->Append(Menu);

	Menu->ShowMenu();

	int returnMenu = MENU_NONE;

	while((returnMenu = Menu->MainLoop()) == MENU_NONE);

	delete Menu;

	return returnMenu;
}

int ThemeMenu::MainLoop()
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
			return MENU_THEMEMENU;
		}

		defaultBtn->ResetState();
	}

	return FlyingButtonsMenu::MainLoop();
}

void ThemeMenu::SetMainButton(int position, const char * ButtonText, GuiImageData * imageData, GuiImageData * themeImg)
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

GuiImageData * ThemeMenu::GetImageData(int theme)
{
	char filepath[300];
	snprintf(filepath, sizeof(filepath), "%stheme_preview.png", ThemeList[theme].ImageFolder.c_str());

	return (new GuiImageData(filepath));
}

void ThemeMenu::SetupMainButtons()
{
	ThemeList.clear();

	DirList ThemeDir(Settings.theme_path, ".them", DirList::Files);
	if (ThemeDir.GetFilecount() == 0)
	{
		WindowPrompt(tr( "No themes found." ), 0, "OK");
	}

	for(int i = 0; i < ThemeDir.GetFilecount(); ++i)
	{
		u8 *buffer = NULL;
		u32 filesize;
		gprintf("%i %s\n", i, ThemeDir.GetFilepath(i));
		LoadFileToMem(ThemeDir.GetFilepath(i), &buffer, &filesize);

		if(!buffer) continue;

		buffer[filesize-1] = '\0';

		int size = ThemeList.size();
		ThemeList.resize(size+1);

		ThemeList[size].Filepath = ThemeDir.GetFilepath(i);
		GetNodeText(buffer, "Theme-Title:", ThemeList[size].Title);
		GetNodeText(buffer, "Theme-Team:", ThemeList[size].Team);
		GetNodeText(buffer, "Theme-Version:", ThemeList[size].Version);
		GetNodeText(buffer, "Image-Folder:", ThemeList[size].ImageFolder);

		if(ThemeList[size].Title.size() == 0 && ThemeDir.GetFilename(i))
		{
			ThemeList[size].Title = ThemeDir.GetFilename(i);
			size_t pos = ThemeList[size].Title.rfind('.');
			if(pos != std::string::npos)
				ThemeList[size].Title.erase(pos);
		}

		if(ThemeList[size].ImageFolder.size() == 0)
		{
			ThemeList[size].ImageFolder = ThemeDir.GetFilepath(i);
			size_t pos = ThemeList[size].ImageFolder.rfind('.');
			if(pos != std::string::npos)
				ThemeList[size].ImageFolder.erase(pos);
			ThemeList[size].ImageFolder += '/';
		}
		else
		{
			std::string tempString = ThemeList[size].ImageFolder;
			ThemeList[size].ImageFolder = Settings.theme_path;
			ThemeList[size].ImageFolder += tempString;
			ThemeList[size].ImageFolder += '/';
		}

		SetMainButton(size, ThemeList[size].Title.c_str(), MainButtonImgData, NULL);

		free(buffer);
	}
}

bool ThemeMenu::GetNodeText(const u8 *buffer, const char *node, std::string &outtext)
{
	const char * nodeText = strcasestr((const char *) buffer, node);
	if(!nodeText)
		return false;

	nodeText += strlen(node);

	while(*nodeText == ' ') nodeText++;

	while(*nodeText != '\0' && *nodeText != '\\' && *nodeText != '\n' && *nodeText != '"')
	{
		outtext.push_back(*nodeText);
		nodeText++;
	}

	return true;
}

void ThemeMenu::AddMainButtons()
{
	HaltGui();
	for(u32 i = 0; i < MainButton.size(); ++i)
		Remove(MainButton[i]);

	int FirstItem = currentPage*4;
	int n = 0;

	for(int i = FirstItem; i < (int) MainButton.size() && i < FirstItem+4; ++i)
	{
		delete ThemePreviews[n];
		ThemePreviews[n] = GetImageData(i);
		MainButtonImgOver[i]->SetImage(ThemePreviews[n]);
		n++;
	}

	FlyingButtonsMenu::AddMainButtons();
}

void ThemeMenu::MainButtonClicked(int button)
{
	//! TODO: Clean me
	const char * title = ThemeList[button].Title.c_str();
	const char * author = ThemeList[button].Team.c_str();
	const char * version = ThemeList[button].Version.c_str();
	GuiImageData *thumbimageData = ThemePreviews[button % 4];

	gprintf("\nTheme_Prompt(%s ,%s)", title, author);
	bool leave = false;

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

	int PositionY = 30;

	GuiText titleTxt(tr( "Theme Title:" ), 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(230, PositionY);
	PositionY += 20;

	GuiText titleTxt2(title, 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	titleTxt2.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt2.SetPosition(230, PositionY);
	titleTxt2.SetMaxWidth(dialogBox.GetWidth() - 220, WRAP);

	if(titleTxt2.GetTextWidth() >= dialogBox.GetWidth() - 220)
		PositionY += 50;
	else
		PositionY += 30;

	GuiText authorTxt(tr( "Author(s):" ), 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	authorTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	authorTxt.SetPosition(230, PositionY);
	PositionY += 20;

	GuiText authorTxt2(author, 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	authorTxt2.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	authorTxt2.SetPosition(230, PositionY);
	authorTxt2.SetMaxWidth(dialogBox.GetWidth() - 220, DOTTED);

	if(authorTxt2.GetTextWidth() >= dialogBox.GetWidth() - 220)
		PositionY += 50;
	else
		PositionY += 30;

	GuiText versionTxt(tr( "Version:" ), 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	versionTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	versionTxt.SetPosition(230, PositionY);

	GuiText versionTxt2(version, 18, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
	versionTxt2.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	versionTxt2.SetPosition(235+versionTxt.GetTextWidth(), PositionY);
	versionTxt2.SetMaxWidth(dialogBox.GetWidth() - 220, DOTTED);

	GuiText applyBtnTxt(tr( "Apply" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	applyBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
	GuiImage applyBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		applyBtnTxt.SetWidescreen(Settings.widescreen);
		applyBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton applyBtn(&applyBtnImg, &applyBtnImg, ALIGN_RIGHT, ALIGN_TOP, -5, 170, &trigA, btnSoundOver, btnSoundClick2, 1);
	applyBtn.SetLabel(&applyBtnTxt);
	applyBtn.SetScale(0.9);

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
	promptWindow.Append(&versionTxt);
	promptWindow.Append(&versionTxt2);
	promptWindow.Append(&applyBtn);
	promptWindow.Append(&backBtn);

	HaltGui();
	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	ResumeGui();

	while (!leave)
	{
		usleep(100);

		if (shutdown)
			Sys_Shutdown();
		else if (reset)
			Sys_Reboot();

		if (applyBtn.GetState() == STATE_CLICKED)
		{
			int choice = WindowPrompt(tr( "Do you want to apply this theme?" ), title, tr( "Yes" ), tr( "Cancel" ));
			if (choice)
			{
				if (Theme::Load(ThemeList[button].Filepath.c_str()))
				{
					snprintf(Settings.theme, sizeof(Settings.theme), ThemeList[button].Filepath.c_str());
					Theme::Reload();
					returnMenu = MENU_THEMEMENU;
					leave = true;
				}
			}
			mainWindow->SetState(STATE_DISABLED);
			promptWindow.SetState(STATE_DEFAULT);
			applyBtn.ResetState();
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
