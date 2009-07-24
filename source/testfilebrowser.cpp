#include <gccore.h>
#include <stdio.h>

#include "language/gettext.h"
#include "prompts/PromptWindows.h"
#include "libwiigui/gui.h"
#include "filebrowser.h"
#include "menu.h"
#include "sys.h"

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/****************************************************************************
 * MenuBrowseDevice
 ***************************************************************************/
int MenuBrowseDevice()
{
	int i;
	char currentdir[50];

	// populate initial directory listing
	if(BrowseDevice(SD) <= 0)
	{
		int choice = WindowPrompt("Error",
		"Unable to load device.",
		"Retry",
		"Change Settings");

		if(choice) {
			return MENU_DISCLIST;
		}
	}

	int menu = MENU_NONE;

	GuiText titleTxt("Browse Files", 28, (GXColor){0, 0, 0, 230});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(70,20);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigPlus;
	trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);
    GuiTrigger trigMinus;
	trigMinus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);

	GuiFileBrowser fileBrowser(552, 248);
	fileBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	fileBrowser.SetPosition(0, 100);

    GuiImageData btnOutline(button_dialogue_box_png);
	GuiText ExitBtnTxt("Exit", 24, (GXColor){0, 0, 0, 255});
	GuiImage ExitBtnImg(&btnOutline);
	GuiButton ExitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	ExitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	ExitBtn.SetPosition(100, -35);
	ExitBtn.SetLabel(&ExitBtnTxt);
	ExitBtn.SetImage(&ExitBtnImg);
	ExitBtn.SetTrigger(&trigA);
	ExitBtn.SetEffectGrow();

	GuiImageData Address(addressbar_textbox_png);
    snprintf(currentdir, sizeof(currentdir), "%s%s", browser.rootdir, browser.dir);
	GuiText AdressText(currentdir, 20, (GXColor) {0, 0, 0, 255});
	AdressText.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	AdressText.SetPosition(20, 0);
	AdressText.SetMaxWidth(Address.GetWidth()-40, GuiText::SCROLL);
	GuiImage AdressbarImg(&Address);
	GuiButton Adressbar(Address.GetWidth(), Address.GetHeight());
	Adressbar.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	Adressbar.SetPosition(60, fileBrowser.GetTop()-45);
	Adressbar.SetImage(&AdressbarImg);
	Adressbar.SetLabel(&AdressText);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&ExitBtn);
	w.Append(&titleTxt);
	w.Append(&fileBrowser);
	w.Append(&Adressbar);
	mainWindow->Append(&w);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();

        if(reset == 1)
            Sys_Reboot();

		for(i=0; i<PAGESIZE; i++)
		{
			if(fileBrowser.fileList[i]->GetState() == STATE_CLICKED)
			{
				fileBrowser.fileList[i]->ResetState();
				// check corresponding browser entry
				if(browserList[browser.selIndex].isdir)
				{
					if(BrowserChangeFolder())
					{
						fileBrowser.ResetState();
						fileBrowser.fileList[0]->SetState(STATE_SELECTED);
						fileBrowser.TriggerUpdate();
                        AdressText.SetTextf("%s%s", browser.rootdir, browser.dir);
					} else {
						menu = MENU_DISCLIST;
						break;
					}
				} else {
					mainWindow->SetState(STATE_DISABLED);
					mainWindow->SetState(STATE_DEFAULT);
				}
			}
		}

        if(ExitBtn.GetState() == STATE_CLICKED)
			menu = MENU_DISCLIST;
	}
	HaltGui();
	mainWindow->Remove(&w);
	ResumeGui();

	return menu;
}
