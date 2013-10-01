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
	GuiButton backBtn(&backBtnImg, &backBtnImg, 2, 3, -195, 400, &trigA, NULL, btnSoundClick2, 1);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetTrigger(&trigB);

	GuiText updateBtnTxt(tr( "Update" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	updateBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
	GuiImage updateBtnImg(&btnOutline);
	GuiButton updateBtn(&updateBtnImg, &updateBtnImg, 2, 3, 0, 400, &trigA, NULL, btnSoundClick2, 1);
	updateBtn.SetLabel(&updateBtnTxt);

	GuiText createBtnTxt(tr( "Create" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	createBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
	GuiImage createBtnImg(&btnOutline);
	GuiButton createBtn(&createBtnImg, &createBtnImg, 2, 3, 195, 400, &trigA, NULL, btnSoundClick2, 1);
	createBtn.SetLabel(&createBtnTxt);

	char txtfilename[55];
	snprintf(txtfilename, sizeof(txtfilename), "%s%s.txt", Settings.TxtCheatcodespath, gameID);

	GCTCheats gctCheats;
	int check = gctCheats.openTxtfile(txtfilename);

	int download = 0;
	int blankchoice = 0;

	switch (check)
	{
		case -1:
			blankchoice = WindowPrompt(tr( "Error" ), tr( "Cheatfile is blank" ), tr( "Delete" ), tr( "OK" ));
			if(blankchoice)
			{
				char gctPath[200];
				snprintf(gctPath, sizeof(gctPath), "%s%.6s.TXT", Settings.TxtCheatcodespath, gameID);
				RemoveFile(gctPath);
			}
			break;
		case 0:
			download = WindowPrompt(tr( "Error" ), tr( "No Cheatfile found" ), tr( "Download Now" ), tr( "Cancel" ));
			if (download == 1)
			{
				download = CodeDownload(gameID);
				if (download < 0 || gctCheats.openTxtfile(txtfilename) != 1)
					break;
			}
			else
				break;
		case 1:
			int cntcheats = gctCheats.getCnt();
			OptionList cheatslst;
			GuiOptionBrowser chtBrowser(400, 280, &cheatslst, "bg_options_settings.png");
			chtBrowser.SetPosition(0, 90);
			chtBrowser.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
			chtBrowser.SetClickable(true);

			GuiText titleTxt(gctCheats.getGameName().c_str(), 28, ( GXColor ) {0, 0, 0, 255});
			titleTxt.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
			titleTxt.SetMaxWidth(350, SCROLL_HORIZONTAL);
			titleTxt.SetPosition(12, 40);

			char gctPath[200];
			snprintf(gctPath, sizeof(gctPath), "%s%.6s.gct", Settings.Cheatcodespath, gameID);
			u8 *gctBuf = NULL;
			u32 gctSize = 0;
			LoadFileToMem(gctPath, &gctBuf, &gctSize);

			for (int i = 0; i < cntcheats; i++)
			{
				cheatslst.SetValue(i, "%s", gctCheats.getCheatName(i).c_str());
				// search after header and before footer
				if(gctBuf && gctCheats.IsCheatIncluded(i, gctBuf, gctSize))
					cheatslst.SetName(i, tr("ON"));
				else
					cheatslst.SetName(i, tr("OFF"));
			}

			HaltGui();
			GuiWindow w(screenwidth, screenheight);
			w.Append(&settingsbackground);
			w.Append(&titleTxt);
			w.Append(&backBtn);
			w.Append(&updateBtn);
			w.Append(&createBtn);
			w.Append(&chtBrowser);
			mainWindow->SetState(STATE_DISABLED);
			mainWindow->Append(&w);
			ResumeGui();

			while (!exit)
			{
				usleep(100000);

				ret = chtBrowser.GetClickedOption();
				if (ret >= 0)
				{
					const char *strCheck = cheatslst.GetName(ret);
					if (strCheck && strcmp(strCheck, tr("ON")) == 0)
					{
						cheatslst.SetName(ret, "%s", tr("OFF"));
					}
					else if (strCheck && strcmp(strCheck, tr("OFF")) == 0)
					{
						cheatslst.SetName(ret, "%s", tr("ON"));
					}
				}

				if (createBtn.GetState() == STATE_CLICKED)
				{
					if (cntcheats > 0)
					{
						vector<int> vActiveCheats;
						for (int i = 0; i < cntcheats; i++)
						{
							const char *strCheck = cheatslst.GetName(i);
							if (strCheck && strcmp(strCheck, tr("ON")) == 0)
								vActiveCheats.push_back(i);
						}
						if (vActiveCheats.size() == 0)
						{
							if(WindowPrompt(tr( "Error" ), tr( "No cheats were selected! Should the GCT file be deleted?" ), tr("Yes"), tr("Cancel")))
							{
								RemoveFile(gctPath);
								w.Remove(&chtBrowser);
								for (int i = 0; i < gctCheats.getCnt(); i++)
									cheatslst.SetName(i, tr("OFF"));
								w.Append(&chtBrowser);
							}
						}
						else
						{
							CreateSubfolder(Settings.Cheatcodespath);
							gctCheats.createGCT(vActiveCheats, gctPath);
							WindowPrompt(tr( "GCT File created" ), NULL, tr( "OK" ));
						}
					}
					else
						WindowPrompt(tr( "Error" ), tr( "Could not create GCT file" ), tr( "OK" ));

					mainWindow->SetState(STATE_DISABLED);
					w.SetState(STATE_DEFAULT);
					createBtn.ResetState();
				}

				if (backBtn.GetState() == STATE_CLICKED)
				{
					backBtn.ResetState();
					exit = true;
					break;
				}

				if(updateBtn.GetState() == STATE_CLICKED)
				{
					download = CodeDownload(gameID);
					if (download >= 0 && gctCheats.openTxtfile(txtfilename) == 1)
					{
						w.Remove(&chtBrowser);
						cheatslst.ClearList();
						cntcheats = gctCheats.getCnt();
						for (int i = 0; i < cntcheats; i++)
						{
							cheatslst.SetValue(i, "%s", gctCheats.getCheatName(i).c_str());
							// search after header and before footer
							if(gctBuf && gctCheats.IsCheatIncluded(i, gctBuf, gctSize))
								cheatslst.SetName(i, tr("ON"));
							else
								cheatslst.SetName(i, tr("OFF"));
						}
						w.Append(&chtBrowser);
					}
					updateBtn.ResetState();
				}
			}
			if(gctBuf)
				free(gctBuf);
			HaltGui();
			mainWindow->SetState(STATE_DEFAULT);
			mainWindow->Remove(&w);
			ResumeGui();
			break;
	}

	return choice;
}
