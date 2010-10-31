#include <string.h>
#include <unistd.h>

#include <fat.h>
#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "themes/CTheme.h"
#include "fatmounter.h"
#include "FileOperations/fileops.h"
#include "menu.h"
#include "filelist.h"
#include "sys.h"
#include "gct.h"

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/*** Extern variables ***/
extern GuiWindow * mainWindow;

/****************************************************************************
 * CheatMenu
 ***************************************************************************/
int CheatMenu(const char * gameID)
{
    int choice = 0;
    bool exit = false;
    int ret = 1;

    // because destroy GuiSound must wait while sound playing is finished, we use a global sound
    if (!btnClick2) btnClick2 = new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
    //  GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
    GuiImageData settingsbg(Resources::GetFile("settings_background.png"), Resources::GetFileSize("settings_background.png"));
    GuiImage settingsbackground(&settingsbg);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiText backBtnTxt(tr( "Back" ), 22, Theme.prompttext);
    backBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
    GuiImage backBtnImg(&btnOutline);
    GuiButton backBtn(&backBtnImg, &backBtnImg, 2, 3, -140, 400, &trigA, NULL, btnClick2, 1);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetTrigger(&trigB);

    GuiText createBtnTxt(tr( "Create" ), 22, Theme.prompttext);
    createBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
    GuiImage createBtnImg(&btnOutline);
    GuiButton createBtn(&createBtnImg, &createBtnImg, 2, 3, 160, 400, &trigA, NULL, btnClick2, 1);
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
                if (download < 0 || c.openTxtfile(txtfilename) != 1) break;
            }
            else break;
        case 1:
            int cntcheats = c.getCnt();
            OptionList cheatslst;
            GuiCustomOptionBrowser chtBrowser(400, 280, &cheatslst, "bg_options_settings.png", 1, 90);
            chtBrowser.SetPosition(0, 90);
            chtBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
            chtBrowser.SetClickable(true);

            GuiText titleTxt(c.getGameName().c_str(), 28, ( GXColor ) {0, 0, 0, 255});
            titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
            titleTxt.SetMaxWidth(350, SCROLL_HORIZONTAL);
            titleTxt.SetPosition(12, 40);

            for (int i = 0; i <= cntcheats; i++)
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
                if (ret != -1)
                {
                    const char *strCheck = cheatslst.GetName(ret);
                    if (strncmp(strCheck, "ON", 2) == 0)
                    {
                        cheatslst.SetName(ret, "%s", "OFF");
                    }
                    else if (strncmp(strCheck, "OFF", 3) == 0)
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
                        for (int i = 0; i <= cntcheats; i++)
                        {
                            const char *strCheck = cheatslst.GetName(i);
                            if (strncmp(strCheck, "ON", 2) == 0)
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
