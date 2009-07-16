/****************************************************************************
 * DiscBrowser
 * USB Loader GX 2009
 *
 * DiscBrowser.h
 ***************************************************************************/
#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "prompts/PromptWindows.h"
#include "filelist.h"
#include "menu.h"
#include "usbloader/disc.h"
#include "usbloader/fstfile.h"
#include "usbloader/wdvd.h"
#include "main.h"
#include "sys.h"
#include "settings/cfg.h"

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern u8 shutdown;
extern u8 reset;

/********************************************************************************
*Game specific settings
*********************************************************************************/
int DiscBrowse(struct discHdr * header)
{
	bool exit = false;
	int ret, choice;
	u64 offset;

	
	ret = Disc_SetUSB(header->id);
	if(ret < 0) {
	    WindowPrompt(tr("ERROR:"), tr("Could not set USB."), tr("OK"));
        return ret;
	}
	

   ret = Disc_Open();
	if(ret < 0) {
	    WindowPrompt(tr("ERROR:"), tr("Could not open disc."), tr("OK"));
        return ret;
	}
	
	

	ret = __Disc_FindPartition(&offset);
	if (ret < 0) {
	    WindowPrompt(tr("ERROR:"), tr("Could not find a WBFS partition."), tr("OK"));
		return ret;
	}
	
	ret = WDVD_OpenPartition(offset);
    if (ret < 0) {
	    WindowPrompt(tr("ERROR:"), tr("Could not open WBFS partition"), tr("OK"));
		return ret;
    }
	
    int *buffer = (int*)memalign(32, 0x20);
	
	if (buffer == NULL)
	{
		WindowPrompt(tr("ERROR:"), tr("Not enough free memory."), tr("OK"));
		return -1;
	}
	
	ret = WDVD_Read(buffer, 0x20, 0x420);
	if (ret < 0) {
		WindowPrompt(tr("ERROR:"), tr("Could not read the disc."), tr("OK"));
		return ret;
	}
	
	void *fstbuffer = memalign(32, buffer[2]*4);
	FST_ENTRY *fst = (FST_ENTRY *)fstbuffer;

	if (fst == NULL)
	{
		WindowPrompt(tr("ERROR:"), tr("Not enough free memory."), tr("OK"));
		free(buffer);
		return -1;
	}
	
	ret = WDVD_Read(fstbuffer, buffer[2]*4, buffer[1]*4);

	if (ret < 0) {
		WindowPrompt(tr("ERROR:"), tr("Could not read the disc."), tr("OK"));
		free(buffer);
		free(fstbuffer);
		return ret;
	}
	
	free(buffer);
	
	WDVD_Reset();
	//Disc_SetUSB(NULL);
	WDVD_ClosePartition();
	
	u32 discfilecount = fst[0].filelen;
	u32 dolfilecount = 0;
	//int offsetselect[20];
	
	customOptionList options3(discfilecount);
	
	for (u32 i = 0; i < discfilecount; i++) {
			
			//don't add files that aren't .dol to the list
			int len = (strlen(fstfiles(fst, i)));
			if (fstfiles(fst, i)[len-4] =='.' &&
				fstfiles(fst, i)[len-3] =='d' &&
				fstfiles(fst, i)[len-2] =='o' &&
				fstfiles(fst, i)[len-1] =='l')
				{
					options3.SetName(i, "%i", i);
					options3.SetValue(i, fstfiles(fst, i));
					//options3.SetName(i, fstfiles(fst, i));
					
				dolfilecount++;
				}
	}
	
    if(dolfilecount <= 0) {
        WindowPrompt(tr("ERROR"), tr("No dol file found on disc."), tr("OK"));
        free(fstbuffer);
        return -1;
    }

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);

	char imgPath[100];

	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sgamesettings_background.png", CFG.theme_path);
	GuiImageData settingsbg(imgPath, settings_background_png);

    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiText titleTxt(get_title(header), 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(12,40);
	titleTxt.SetMaxWidth(356, GuiText::SCROLL);

    GuiImage settingsbackground(&settingsbg);
	GuiButton settingsbackgroundbtn(settingsbackground.GetWidth(), settingsbackground.GetHeight());
	settingsbackgroundbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsbackgroundbtn.SetPosition(0, 0);
	settingsbackgroundbtn.SetImage(&settingsbackground);

    GuiText cancelBtnTxt(tr("Back"), 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
	cancelBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage cancelBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
        cancelBtnTxt.SetWidescreen(CFG.widescreen);
        cancelBtnImg.SetWidescreen(CFG.widescreen);
    }
	GuiButton cancelBtn(&cancelBtnImg,&cancelBtnImg, 2, 3, 180, 400, &trigA, &btnSoundOver, &btnClick,1);
	cancelBtn.SetScale(0.9);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetTrigger(&trigB);

	u8 scrollbaron = 0;
	if(dolfilecount > 9)
        scrollbaron = 1;

	GuiCustomOptionBrowser optionBrowser3(396, 280, &options3, CFG.theme_path, "bg_options_gamesettings.png", bg_options_settings_png, dolfilecount>9?1:0, 200);
	optionBrowser3.SetPosition(0, 90);
	optionBrowser3.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	
   HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&settingsbackgroundbtn);
    w.Append(&titleTxt);
	w.Append(&cancelBtn);
    w.Append(&optionBrowser3);

	mainWindow->Append(&w);

	ResumeGui();
	
	while(!exit)
	{
		VIDEO_WaitVSync();

		if(shutdown == 1)
			Sys_Shutdown();
		if(reset == 1)
			Sys_Reboot();

		ret = optionBrowser3.GetClickedOption();

		if(ret > 0) {
		    char temp[100];
		    strncpy(temp, fstfiles(fst, ret), sizeof(temp));
            choice = WindowPrompt(temp, tr("Load this dol as alternate dol?"), tr("OK"), tr("Cancel"));
            if(choice) {
                //ret = offsetselect[ret];
                snprintf(alternatedname, sizeof(alternatedname), "%s",  temp);
                exit = true;
                break;
            }
		}

		if (cancelBtn.GetState() == STATE_CLICKED)
		{
			exit = true;
			ret = 696969;
			//break;
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
	ResumeGui();

    //free not needed list buffer anymore
	free(fstbuffer);

	return ret;
}


int autoSelectDol(const char *id)
{	
	//these are the game IDs without the 4th character
	// if it turns out that the offset is different for different regions
	// the 4th char can be added
	if (strcmp(id,"RHD8P") == 0) return 149;
	if (strcmp(id,"RSX69") == 0) return 337;
	if (strcmp(id,"RED41") == 0) return 1957;
	if (strcmp(id,"RM269") == 0) return 492;
	if (strcmp(id,"RKM5D") == 0) return 290;
	if (strcmp(id,"RJ864") == 0) return 8;
	//if (strcmp(id,"") == 0) return ; //blank line for more dols
	
	return -1;
}

