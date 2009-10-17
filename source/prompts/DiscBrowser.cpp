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
#include "memory.h"
#include "../wad/title.h"
#include "../patches/dvd_broadway.h"

#define BC 		0x0000000100000100ULL

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern u8 shutdown;
extern u8 reset;
extern u8 mountMethod;

/********************************************************************************
*Disk Browser
*********************************************************************************/
int DiscBrowse(struct discHdr * header) {
    bool exit = false;
    int ret, choice;
    u64 offset;

	HaltGui();
	if (!mountMethod)
	{
		ret = Disc_SetUSB(header->id);
		if (ret < 0) {
			ResumeGui();
			WindowPrompt(tr("ERROR:"), tr("Could not set USB."), tr("OK"));
			return ret;
		}
	}
	
	ret = Disc_Open();
    if (ret < 0) {
		ResumeGui();
        WindowPrompt(tr("ERROR:"), tr("Could not open disc."), tr("OK"));
        return ret;
    }

    ret = __Disc_FindPartition(&offset);
    if (ret < 0) {
		ResumeGui();
        WindowPrompt(tr("ERROR:"), tr("Could not find a WBFS partition."), tr("OK"));
        return ret;
    }

    ret = WDVD_OpenPartition(offset);
    if (ret < 0) {
		ResumeGui();
        WindowPrompt(tr("ERROR:"), tr("Could not open WBFS partition"), tr("OK"));
        return ret;
    }
	
	int *buffer = (int*)allocate_memory(0x20);

    if (buffer == NULL) {
		ResumeGui();
        WindowPrompt(tr("ERROR:"), tr("Not enough free memory."), tr("OK"));
        return -1;
    }

    ret = WDVD_Read(buffer, 0x20, 0x420);
    if (ret < 0) {
		ResumeGui();
        WindowPrompt(tr("ERROR:"), tr("Could not read the disc."), tr("OK"));
        return ret;
    }
	
	void *fstbuffer = allocate_memory(buffer[2]*4);
    FST_ENTRY *fst = (FST_ENTRY *)fstbuffer;

    if (fst == NULL) {
		ResumeGui();
        WindowPrompt(tr("ERROR:"), tr("Not enough free memory."), tr("OK"));
        free(buffer);
        return -1;
    }

    ret = WDVD_Read(fstbuffer, buffer[2]*4, buffer[1]*4);

    if (ret < 0) {
		ResumeGui();
        WindowPrompt(tr("ERROR:"), tr("Could not read the disc."), tr("OK"));
        free(buffer);
        free(fstbuffer);
        return ret;
    }
	ResumeGui();
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
                fstfiles(fst, i)[len-1] =='l') {
            options3.SetName(i, "%i", i);
            options3.SetValue(i, fstfiles(fst, i));
            //options3.SetName(i, fstfiles(fst, i));

            dolfilecount++;
        }
    }

    if (dolfilecount <= 0) {
        WindowPrompt(tr("ERROR"), tr("No DOL file found on disc."), tr("OK"));
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

    GuiText titleTxt(get_title(header), 28, (GXColor) {0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(12,40);
    titleTxt.SetMaxWidth(356, SCROLL_HORIZONTAL);

    GuiImage settingsbackground(&settingsbg);
    GuiButton settingsbackgroundbtn(settingsbackground.GetWidth(), settingsbackground.GetHeight());
    settingsbackgroundbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    settingsbackgroundbtn.SetPosition(0, 0);
    settingsbackgroundbtn.SetImage(&settingsbackground);

    GuiText cancelBtnTxt(tr("Back"), 22, THEME.prompttext);
    cancelBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
    GuiImage cancelBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        //cancelBtnTxt.SetWidescreen(CFG.widescreen);
        cancelBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton cancelBtn(&cancelBtnImg,&cancelBtnImg, 2, 3, 180, 400, &trigA, &btnSoundOver, &btnClick,1);
    cancelBtn.SetScale(0.9);
    cancelBtn.SetLabel(&cancelBtnTxt);
    cancelBtn.SetTrigger(&trigB);

    u8 scrollbaron = 0;
    if (dolfilecount > 9)
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
    while (!exit) {
        VIDEO_WaitVSync();

        if (shutdown == 1)
            Sys_Shutdown();
        if (reset == 1)
            Sys_Reboot();

        ret = optionBrowser3.GetClickedOption();

        if (ret > 0) {
            char temp[100];
            strlcpy(temp, fstfiles(fst, ret), sizeof(temp));
            choice = WindowPrompt(temp, tr("Load this DOL as alternate DOL?"), tr("OK"), tr("Cancel"));
            if (choice) {
                //ret = offsetselect[ret];
                strlcpy(alternatedname, temp, sizeof(alternatedname));
				exit = true;
            }
        }

        if (cancelBtn.GetState() == STATE_CLICKED) {
			ret = 696969;
			exit = true;
        }
    }

    HaltGui();
    mainWindow->Remove(&w);
    ResumeGui();

    //free not needed list buffer anymore
    free(fstbuffer);

    return ret;
}


int autoSelectDol(const char *id, bool force) {

	char id4[10];
	sprintf(id4,"%c%c%c%c",id[0],id[1],id[2],id[3]);
	
////// games that can be forced (always need alt dol)

	//Boogie 
    if (strcmp(id,"RBOP69") == 0) return 675;//previous value was 657
    if (strcmp(id,"RBOE69") == 0) return 675;//starstremr
	
	//Fifa 08
    if (strcmp(id,"RF8E69") == 0) return 439;//from isostar
    if (strcmp(id,"RF8P69") == 0) return 463;//from isostar
    if (strcmp(id,"RF8X69") == 0) return 464;//from isostar
		
	//Madden NFL07
    if (strcmp(id,"RMDP69") == 0) return 39;//from isostar
	
	//Madden NFL08
    if (strcmp(id,"RNFP69") == 0) return 1079;//from isostar
	
	//Medal of Honor: Heroes 2
	if (strcmp(id,"RM2X69") == 0)return 601;//dj_skual
	if (strcmp(id,"RM2P69") == 0)return 517;//MZottel
	if (strcmp(id,"RM2E69") == 0) return 492;//Old8oy	
	
	//Mortal Kombat
    if (strcmp(id,"RKMP5D") == 0) return 290;//from isostar
    if (strcmp(id,"RKME5D") == 0) return 290;//starstremr

	//NBA 08
    if (strcmp(id,"RNBX69") == 0) return 964;//from isostar

	//Pangya! Golf with Style
    if (strcmp(id,"RPYP9B") == 0) return 12490;//from isostar
	
	//Redsteel
    if (strcmp(id,"REDP41") == 0) return 1957;//from isostar
    if (strcmp(id,"REDE41") == 0) return 1957;//starstremr

	//SSX
    if (strcmp(id,"RSXP69") == 0) return 377;//previous value was 337
    if (strcmp(id,"RSXE69") == 0) return 377;//previous value was 337
	
	//Wii Sports Resort, needs alt dol one time only, to show the Motion Plus video
	//if (strcmp(id,"RZTP01") == 0 && CheckForSave(id4)==0) return 952;//from isostar
	//if (strcmp(id,"RZTE01") == 0 && CheckForSave(id4)==0) return 674;//from starstremr
	//as well as Grand Slam Tennis, Tiger Woods 10, Virtual Tennis 2009
	
///// games that can't be forced (alt dol is not always needed)
	if (!force) {
	
		//Grand Slam Tennis
		if (strcmp(id,"R5TP69") == 0) return 1493;//from isostar
		if (strcmp(id,"R5TE69") == 0) return 1493;//starstremr
				
		//Medal of Honor Heroes
		if (strcmp(id,"RMZX69") == 0) return 492;//from isostar
		if (strcmp(id,"RMZP69") == 0) return 492;//from isostar
		if (strcmp(id,"RMZE69") == 0) return 492;//starstremr 
								
		//Tiger Woods 10
		if (strcmp(id,"R9OP69") == 0) return 1991;//from isostar
		if (strcmp(id,"R9OE69") == 0) return 1973;//starstremr
		
		//Virtual Tennis 2009
		if (strcmp(id,"RVUP8P") == 0) return 16426;//from isostar
		if (strcmp(id,"RVUE8P") == 0) return 16405;//from isostar
		
		//Wii Sports Resort
		if (strcmp(id,"RZTP01") == 0) return 952;//from isostar
		if (strcmp(id,"RZTE01") == 0) return 674;//from starstremr
	}
	
    return -1;
}

int autoSelectDolMenu(const char *id, bool force) {

	/*
	char id4[10];
	sprintf(id4,"%c%c%c%c",id[0],id[1],id[2],id[3]);
	
	switch (CheckForSave(id4)) {
		case 0:
			WindowPrompt(tr("NO save"),0,tr("OK"));
			break;
		case 1:
			WindowPrompt(tr("save"),0,tr("OK"));
			break;
		default:
			char test[10];
			sprintf(test,"%d",CheckForSave(id4));
			WindowPrompt(test,0,tr("OK"));
            break;
	}
	return -1;
	*/
	
	//Indiana Jones and the Staff of Kings (Fate of Atlantis)
	if (strcmp(id,"RJ8E64") == 0) {
		int choice = WindowPrompt(tr("Select a DOL"), 0, "Fate of Atlantis", tr("Default"));
		switch (choice) {
			case 1:
				choice = 8; //from starstremr
				break;
			default: // no alt dol
				choice = 0;
				break;
		}
		return choice;
	}
	if (strcmp(id,"RJ8P64") == 0) {
		int choice = WindowPrompt(tr("Select a DOL"), 0, "Fate of Atlantis", tr("Default"));
		switch (choice) {
			case 1:
				choice = 8; //from isostar
				break;
			default: // no alt dol
				choice = 0;
				break;
		}
		return choice;
	}
		
	//Metal Slug Anthology (Metal Slug 6)
	if (strcmp(id,"RMLEH4") == 0) {
		int choice = WindowPrompt(tr("Select a DOL"), 0, "Metal Slug 6", tr("Default"));
		switch (choice) {
			case 1:
				choice = 54; //from lustar
				break;
			default: // no alt dol
				choice = 0;
				break;
		}
		return choice;
	}
	if (strcmp(id,"RMLP7U") == 0) {
		int choice = WindowPrompt(tr("Select a DOL"), 0, "Metal Slug 6", tr("Default"));
		switch (choice) {
			case 1:
				choice = 56; //from isostar
				break;
			default: // no alt dol
				choice = 0;
				break;
		}
		return choice;
	}
		
	//Metroid Prime Trilogy
	if (strcmp(id,"R3ME01") == 0) {
		//do not use any alt dol if there is no save game in the nand
		/*
		if (CheckForSave(id4)==0 && force) {
			WindowPrompt(0,tr("You need to start this game one time to create a save file, then exit and start it again."),tr("OK"));
			return -1;
		}
		*/
		int choice = WindowPrompt(tr("Select a DOL"), 0, "Metroid Prime", "Metroid Prime 2", "Metroid Prime 3", tr("Default"));
        switch (choice) {
            case 1:
				choice = 780;
                break;
            case 2:
				choice = 781;
                break;
			case 3:
				choice = 782;
                break;
            default: // no alt dol
				choice = 0;
                break;
		}
		return choice;
	}
	if (strcmp(id,"R3MP01") == 0) {
		/*
		if (CheckForSave(id4)==0 && force) {
			WindowPrompt(0,tr("You need to start this game one time to create a save file, then exit and start it again."),tr("OK"));
			return -1;
		}
		*/
		int choice = WindowPrompt(tr("Select a DOL"), 0, "Metroid Prime", "Metroid Prime 2", "Metroid Prime 3", tr("Default"));
        switch (choice) {
            case 1:
				choice = 782;
				break;
            case 2:
				choice = 783;
				break;
			case 3:
				choice = 784;
                break;
            default: // no alt dol
				choice = 0;
				break;
		}
		return choice;
	}
	
	//Rampage: Total Destruction (M1.dol=Rampage, jarvos.dol=Rampage World Tour)
	if (strcmp(id,"RPGP5D") == 0) {
		int choice = WindowPrompt(tr("Select a DOL"), 0, "Rampage", "World Tour", tr("Default"));
        switch (choice) {
            case 1:
				choice = 369; //from Ramzee
				break;
            case 2:
				choice = 368; //from Ramzee
				break;
            default: // no alt dol
				choice = 0;
				break;
		}
		return choice;
	}
		
	//The House Of The Dead 2 & 3 Return (only to play 2)
	if (strcmp(id,"RHDE8P") == 0) {
		int choice = WindowPrompt(tr("Select a DOL"), 0, "HotD 2", tr("Default"));
		switch (choice) {
			case 1:
				choice = 149; //from starstremr
				break;
			default: // no alt dol
				choice = 0;
				break;
		}
		return choice;
	}
	if (strcmp(id,"RHDP8P") == 0) {
		int choice = WindowPrompt(tr("Select a DOL"), 0, "HotD 2", tr("Default"));
		switch (choice) {
			case 1:
				choice = 149; //from isostar
				break;
			default: // no alt dol
				choice = 0;
				break;
		}
		return choice;
	}


    return -1;
}


/********************************************************************************
* Mount a DVD, get the type and ID.
*********************************************************************************/
static vu32 dvddone = 0;
static dvddiskid *g_diskID = (dvddiskid*)0x80000000; // If you change this address, the read functions will FAIL!
void __dvd_readidcb(s32 result)
{
	dvddone = result;
}

u8 DiscMount(discHdr *header) {
    int ret;
	HaltGui();
	
	u8 *tmpBuff = (u8 *) malloc(0x60);
	memcpy(tmpBuff, g_diskID, 0x60); // Make a backup of the first 96 bytes at 0x80000000

	ret = bwDVD_LowInit();
	dvddone = 0;
	ret = bwDVD_LowReset(__dvd_readidcb);
	while(ret>=0 && dvddone==0);
	
	dvddone = 0;
	ret = bwDVD_LowReadID(g_diskID, __dvd_readidcb); // Leave this one here, or you'll get an IOCTL error
	while(ret>=0 && dvddone==0);

	dvddone = 0;
	ret = bwDVD_LowUnencryptedRead(g_diskID, 0x60, 0x00, __dvd_readidcb); // Overwrite the g_diskID thing
	while(ret>=0 && dvddone==0);
	
	memcpy(header, g_diskID, 0x60);
	memcpy(g_diskID, tmpBuff, 0x60); // Put the backup back, or games won't load
	free(tmpBuff);

	
	ResumeGui();
	if (dvddone != 1) {
		return 0;
	}
	return (header->magic == 0x5D1C9EA3) ? 1 : 2; // Don't check gamecube magic (0xC2339F3D)
}
