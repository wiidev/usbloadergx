#include <string.h>
#include <unistd.h>
#include <fat.h>

#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "fatmounter.h"
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

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);

	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%ssettings_background.png", CFG.theme_path);
	GuiImageData settingsbg(imgPath, settings_background_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiImage settingsbackground(&settingsbg);

	GuiText backBtnTxt(tr("Back") , 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
	backBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage backBtnImg(&btnOutline);
	GuiButton backBtn(&backBtnImg,&backBtnImg, 2, 3, 160, 400, &trigA, &btnSoundOver, &btnClick,1);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetTrigger(&trigB);

	GuiText createBtnTxt(tr("Create") , 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
	createBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage createBtnImg(&btnOutline);
	GuiButton createBtn(&createBtnImg,&createBtnImg, 2, 3, -140, 400, &trigA, &btnSoundOver, &btnClick,1);
	createBtn.SetLabel(&createBtnTxt);

	GCTCheats c;

	char txtfilename[40];
	snprintf(txtfilename,sizeof(txtfilename),"%s%s.txt",Settings.TxtCheatcodespath,gameID);

	int check = c.openTxtfile(txtfilename);
	
	switch(check)
	{
	case -1: WindowPrompt(tr("Error"),tr("Cheatfile is blank"),tr("OK"),NULL,NULL,NULL,-1);
			 break;
	case 0: WindowPrompt(tr("Error"),tr("No Cheatfile found"),tr("OK"),NULL,NULL,NULL,-1);
			break;
	case 1:	
	//WindowPrompt("Opened File","File found for Game","Okay",NULL,NULL,NULL);
	int cntcheats = c.getCnt();
	customOptionList cheatslst(cntcheats);
	GuiCustomOptionBrowser chtBrowser(400, 280, &cheatslst, CFG.theme_path, "bg_options_settings.png", bg_options_settings_png, 1, 90);
	chtBrowser.SetPosition(0, 90);
	chtBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	chtBrowser.SetClickable(true);

	GuiText titleTxt(c.getGameName().c_str(), 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetMaxWidth(350, GuiText::SCROLL);
	titleTxt.SetPosition(12,40);
	
	for(int i = 0; i <= cntcheats; i++) 
	{
	cheatslst.SetValue(i, "%s",c.getCheatName(i).c_str());
	cheatslst.SetName(i, "OFF");
	}

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&settingsbackground);
	w.Append(&titleTxt);
	w.Append(&backBtn);
	w.Append(&createBtn);
	w.Append(&chtBrowser);
	mainWindow->Append(&w);
	ResumeGui();
		
	while(!exit)
	{
	VIDEO_WaitVSync ();

	ret = chtBrowser.GetClickedOption();
	if (ret != -1)
	{
		const char *strCheck = cheatslst.GetName(ret);
		if (strncmp(strCheck,"ON",2) == 0)
		{
		cheatslst.SetName(ret,"%s","OFF");
		}
		else if (strncmp(strCheck,"OFF",3) == 0) 
		{
		cheatslst.SetName(ret,"%s","ON");
		}
	}

	if(createBtn.GetState() == STATE_CLICKED)
	{
		createBtn.ResetState();
		if (cntcheats > 0) 
		{
		int selectednrs[30];
		int x = 0;
		for(int i = 0; i <= cntcheats; i++) 
		{
			const char *strCheck = cheatslst.GetName(i);
			if (strncmp(strCheck,"ON",2) == 0) 
			{
			selectednrs[x] = i;
			x++;
			}
		}

		string chtpath = Settings.Cheatcodespath;
		string gctfname = chtpath + c.getGameID() + ".gct";
		c.createGCT(selectednrs,x,gctfname.c_str());
		WindowPrompt(tr("GCT File created"),NULL,tr("OK"),NULL,NULL,NULL,-1);
		exit = true;
		break;				
		} else WindowPrompt(tr("Error"),tr("Could not create GCT file"),tr("OK"),NULL,NULL,NULL,-1);
		
	}

	if(backBtn.GetState() == STATE_CLICKED)
	{
		backBtn.ResetState();
		exit = true;
		break;
	}
}
HaltGui();
mainWindow->Remove(&w);
ResumeGui();	

break;
}
		
return choice;
}
