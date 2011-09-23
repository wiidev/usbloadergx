#include <string.h>
#include <unistd.h>

#include "GUI/gui.h"
#include "GUI/gui_optionbrowser.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "themes/CTheme.h"
#include "FileOperations/fileops.h"
#include "menu.h"
#include "sys.h"
#include "gct.h"

/****************************************************************************
 * CheatMenu
 ***************************************************************************/
int CheatMenu(const char * gameID)
{
	int choice = 0;
	bool exit = false;
	int ret = 1;

	GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
	GuiImageData settingsbg(Resources::GetFile("settings_background.png"), Resources::GetFileSize("settings_background.png"));
	GuiImage settingsbackground(&settingsbg);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiText backBtnTxt(tr( "Back" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	backBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
	GuiImage backBtnImg(&btnOutline);
	GuiButton backBtn(&backBtnImg, &backBtnImg, 2, 3, -140, 400, &trigA, NULL, btnSoundClick2, 1);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetTrigger(&trigB);

	GuiText createBtnTxt(tr( "Create" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	createBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
	GuiImage createBtnImg(&btnOutline);
	GuiButton createBtn(&createBtnImg, &createBtnImg, 2, 3, 160, 400, &trigA, NULL, btnSoundClick2, 1);
	createBtn.SetLabel(&createBtnTxt);

	char txtfilename[55];
	snprintf(txtfilename, sizeof(txtfilename), "%s%s.txt", Settings.TxtCheatcodespath, gameID);

	GCTCheats c;
	int check = c.openTxtfile(txtfilename);

	int download = 0;

	switch (check)
	{
		case -1:
			WindowPrompt(tr( "Error" ), tr( "Cheatfile is blank" ), tr( "OK" ));
			break;
		case 0:
			download = WindowPrompt(tr( "Error" ), tr( "No Cheatfile found" ), tr( "Download Now" ), tr( "Cancel" ));
			if (download == 1)
			{
				download = CodeDownload(gameID);
				if (download < 0 || c.openTxtfile(txtfilename) != 1)
					break;
			}
			else
				break;
		case 1:
			int cntcheats = c.getCnt();
			OptionList cheatslst;
			GuiOptionBrowser chtBrowser(400, 280, &cheatslst, "bg_options_settings.png");
			chtBrowser.SetPosition(0, 90);
			chtBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
			chtBrowser.SetClickable(true);

			GuiText titleTxt(c.getGameName().c_str(), 28, ( GXColor ) {0, 0, 0, 255});
			titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
			titleTxt.SetMaxWidth(350, SCROLL_HORIZONTAL);
			titleTxt.SetPosition(12, 40);

			for (int i = 0; i < cntcheats; i++)
			{
				cheatslst.SetValue(i, "%s", c.getCheatName(i).c_str());
				cheatslst.SetName(i, "OFF");
			}

			HaltGui();
			GuiWindow w(screenwidth, screenheight);
			w.Append(&settingsbackground);
			w.Append(&titleTxt);
			w.Append(&backBtn);
			w.Append(&createBtn);
			w.Append(&chtBrowser);
			mainWindow->SetState(STATE_DISABLED);
			mainWindow->ChangeFocus(&w);
			mainWindow->Append(&w);
			ResumeGui();

			while (!exit)
			{
				VIDEO_WaitVSync();

				ret = chtBrowser.GetClickedOption();
				if (ret >= 0)
				{
					const char *strCheck = cheatslst.GetName(ret);
					if (strCheck && strncmp(strCheck, "ON", 2) == 0)
					{
						cheatslst.SetName(ret, "%s", "OFF");
					}
					else if (strCheck && strncmp(strCheck, "OFF", 3) == 0)
					{
						cheatslst.SetName(ret, "%s", "ON");
					}
				}

				if (createBtn.GetState() == STATE_CLICKED)
				{
					createBtn.ResetState();
					if (cntcheats > 0)
					{
						int selectednrs[30];
						int x = 0;
						for (int i = 0; i < cntcheats; i++)
						{
							const char *strCheck = cheatslst.GetName(i);
							if (strCheck && strncmp(strCheck, "ON", 2) == 0)
							{
								selectednrs[x] = i;
								x++;
							}
						}
						if (x == 0)
						{
							WindowPrompt(tr( "Error" ), tr( "No cheats were selected" ), tr( "OK" ));
						}
						else
						{
							CreateSubfolder(Settings.Cheatcodespath);
							string chtpath = Settings.Cheatcodespath;
							string gctfname = chtpath + c.getGameID() + ".gct";
							c.createGCT(selectednrs, x, gctfname.c_str());
							WindowPrompt(tr( "GCT File created" ), NULL, tr( "OK" ));
							exit = true;
							break;
						}
					}
					else WindowPrompt(tr( "Error" ), tr( "Could not create GCT file" ), tr( "OK" ));
				}

				if (backBtn.GetState() == STATE_CLICKED)
				{
					backBtn.ResetState();
					exit = true;
					break;
				}
			}
			HaltGui();
			mainWindow->SetState(STATE_DEFAULT);
			mainWindow->Remove(&w);
			ResumeGui();
			break;
	}

	return choice;
}
