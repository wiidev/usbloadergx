/****************************************************************************
 * TitleBrowser
 * USB Loader GX 2009
 *
 * TitleBrowser.cpp   *giantpune*
 ***************************************************************************/

#include <dirent.h>


#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "network/networkops.h"
#include "network/http.h"
#include "filelist.h"
#include "listfiles.h"
#include "settings/cfg.h"
#include "sys.h"
#include "menu.h"
#include "audio.h"
#include "wad/wad.h"
#include "xml/xml.h"
#include "../wad/title.h"
#include "../usbloader/getentries.h"
#include "../usbloader/utils.h"
#include "../gecko.h"

#define typei 0x00010001

struct discHdr * titleList=NULL;
//discHdr ** titleList;
u32 titleCnt;
extern u32 infilesize;
extern u32 uncfilesize;
extern char wiiloadVersion[2];
#include <zlib.h>
#include "settings/cfg.h"
#include "unzip/unzip.h"
#include "unzip/miniunz.h"

extern struct discHdr * gameList;
extern u32 gameCnt;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern u8 shutdown;
extern u8 reset;
extern u32 infilesize;
extern wchar_t *gameFilter;


/********************************************************************************
* TitleBrowser- opens a browser with a list of installed Titles
* relies on code from any title deleter.
*********************************************************************************/
int TitleBrowser(u32 type) {

	u32 num_titles;
	u32 titles[100] ATTRIBUTE_ALIGN(32);
	u32 num_sys_titles;
	u32 sys_titles[10] ATTRIBUTE_ALIGN(32);
	s32 ret = -1;
	int numtitle;//to get rid of a stupid compile wrning
	//open the database file
	FILE *f;
	char path[100];

	ISFS_Initialize();

	sprintf(path,"%s/config/database.txt",bootDevice);
	f = fopen(path, "r");

	// Get count of titles of our requested type
	ret = getTitles_TypeCount(type, &num_titles);
	if (ret < 0) {
		//printf("\tError! Can't get count of titles! (ret = %d)\n", ret);
		//exit(1);
	}

	// Get titles of our requested type
	ret = getTitles_Type(type, titles, num_titles);
	if (ret < 0) {
		//printf("\tError! Can't get list of titles! (ret = %d)\n", ret);
		//exit(1);
	}

	// Get count of system titles
	ret = getTitles_TypeCount(0x00010002, &num_sys_titles);
	if (ret < 0) {
		//printf("\tError! Can't get count of titles! (ret = %d)\n", ret);
		//exit(1);
	}

	// Get system titles
	ret = getTitles_Type(0x00010002, sys_titles, num_sys_titles);
	if (ret < 0) {
		//printf("\tError! Can't get list of titles! (ret = %d)\n", ret);
		//exit(1);
	}


	//this array will hold all the names for the titles so we only have to get them one time
	char name[num_titles+num_sys_titles][50];

	customOptionList options3(num_titles+num_sys_titles+1);
	//write the titles on the option browser
	u32 i = 0;



	//first add the good stuff
	while (i < num_titles) {
		//start from the beginning of the file each loop
		if (f)rewind(f);
		//char name[50];
		char text[15];
		strcpy(name[i],"");//make sure name is empty
		u8 found=0;
		//set the title's name, number, ID to text
		sprintf(text, "%s", titleText(type, titles[i]));

		//get name from database cause i dont like the ADT function
		char line[200];
		char tmp[50];
		snprintf(tmp,50," ");
		
		//check if the content.bin is on the SD card for that game
		//if there is content.bin,then the game is on the SDmenu and not the wii
		sprintf(line,"SD:/private/wii/title/%s/content.bin",text);
		if (!checkfile(line))
			{
				if (f) {
					while (fgets(line, sizeof(line), f)) {
						if (line[0]== text[0]&&
								line[1]== text[1]&&
								line[2]== text[2]) {
							int j=0;
							found=1;
							for (j=0;(line[j+4]!='\0' || j<51);j++)

								tmp[j]=line[j+4];
							snprintf(name[i],sizeof(name[i]),"%s",tmp);
							//break;
						}
					}
				}
				if (!found) {
					if (getName00(name[i], TITLE_ID(type, titles[i]),CONF_GetLanguage()*2)>=0)
						found=2;

					if (!found) {
						if (getNameBN(name[i], TITLE_ID(type, titles[i]))>=0)
							found=3;

						if (!found)
							snprintf(name[i],sizeof(name[i]),"Unknown Title (%08x)",titles[i]);
					}
				}

				//set the text to the option browser
				options3.SetName(i, "%s",text);
				options3.SetValue(i, "%s",name[i]);
				//options3.SetValue(i, " (%08x) %s",titles[i],name[i]);//use this line to show the number to call to launch the channel
				//move on to the next title
			}
        i++;
    }

    // now add the crappy system titles
    while (i < num_titles+num_sys_titles) {
        //start from the beginning of the file each loop
        if (f)rewind(f);
        //char name[50];
        char text[15];
        strcpy(name[i],"");//make sure name is empty
        u8 found=0;
        //set the title's name, number, ID to text
        sprintf(text, "%s", titleText(0x00010002, sys_titles[i-num_titles]));

        //get name from database cause i dont like the ADT function
        char line[200];
        char tmp[50];
        snprintf(tmp,50," ");
        //snprintf(name[i],sizeof(name[i]),"Unknown Title");
        if (f) {
            while (fgets(line, sizeof(line), f)) {
                if (line[0]== text[0]&&
                        line[1]== text[1]&&
                        line[2]== text[2]) {
                    int j=0;
                    found=1;
                    for (j=0;(line[j+4]!='\0' || j<51);j++)

                        tmp[j]=line[j+4];
                    snprintf(name[i],sizeof(name[i]),"%s",tmp);
                    break;
                }
            }
        }
        if (!found) {
            if (getName00(name[i], TITLE_ID(0x00010002, sys_titles[i-num_titles]))>=0)
                found=2;

            if (!found) {
                if (getNameBN(name[i], TITLE_ID(0x00010002, sys_titles[i-num_titles]))>=0)
                    found=3;

                if (!found)
                    snprintf(name[i],sizeof(name[i]),"Unknown Title (%08x)",sys_titles[i-num_titles]);
            }
        }

        //set the text to the option browser
        options3.SetName(i, "%s",text);
        options3.SetValue(i, "%s",name[i]);
        //options3.SetValue(i, " (%08x) %s",titles[i],name[i]);//use this line to show the number to call to launch the channel
        //move on to the next title
        i++;
    }
    if (i == num_titles+num_sys_titles) {
        options3.SetName(i, " ");
        options3.SetValue(i, "%s",tr("Wii Settings"));
    }
    //we have all the titles we need so close the database and stop poking around in the wii
    fclose(f);

    //get rid of our footprints in there
    Uninstall_FromTitle(TITLE_ID(1, 0));
    ISFS_Deinitialize();
    bool exit = false;

    if (IsNetworkInit())
        ResumeNetworkWait();

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

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

    GuiText titleTxt(tr("Title Launcher"), 28, (GXColor) {0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(12,40);
    titleTxt.SetMaxWidth(356, GuiText::SCROLL);

    GuiImage settingsbackground(&settingsbg);
    GuiButton settingsbackgroundbtn(settingsbackground.GetWidth(), settingsbackground.GetHeight());
    settingsbackgroundbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    settingsbackgroundbtn.SetPosition(0, 0);
    settingsbackgroundbtn.SetImage(&settingsbackground);

    GuiText cancelBtnTxt(tr("Back"), 22, THEME.prompttext);
    cancelBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
    GuiImage cancelBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        cancelBtnTxt.SetWidescreen(CFG.widescreen);
        cancelBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton cancelBtn(&cancelBtnImg,&cancelBtnImg, 2, 3, 180, 400, &trigA, &btnSoundOver, btnClick2,1);
    cancelBtn.SetScale(0.9);
    cancelBtn.SetLabel(&cancelBtnTxt);
    cancelBtn.SetTrigger(&trigB);

    u8 scrollbaron = 0;
    if (num_titles > 9)
        scrollbaron = 1;

    GuiCustomOptionBrowser optionBrowser3(396, 280, &options3, CFG.theme_path, "bg_options_gamesettings.png", bg_options_settings_png, num_titles+num_sys_titles>9?1:0, 200);
    optionBrowser3.SetPosition(0, 90);
    optionBrowser3.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

    snprintf(imgPath, sizeof(imgPath), "%sWifi_btn.png", CFG.theme_path);
    GuiImageData wifiImgData(imgPath, Wifi_btn_png);
    GuiImage wifiImg(&wifiImgData);
    if (Settings.wsprompt == yes) {
        wifiImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton wifiBtn(wifiImg.GetWidth(), wifiImg.GetHeight());
    wifiBtn.SetImage(&wifiImg);
    wifiBtn.SetPosition(100, 400);
    wifiBtn.SetEffectGrow();
    wifiBtn.SetAlpha(80);
    wifiBtn.SetTrigger(&trigA);

    GuiTrigger trigZ;
    trigZ.SetButtonOnlyTrigger(-1, WPAD_NUNCHUK_BUTTON_Z | WPAD_CLASSIC_BUTTON_ZL, PAD_TRIGGER_Z);

    GuiButton screenShotBtn(0,0);
    screenShotBtn.SetPosition(0,0);
    screenShotBtn.SetTrigger(&trigZ);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&screenShotBtn);
    w.Append(&settingsbackgroundbtn);
    w.Append(&titleTxt);
    w.Append(&cancelBtn);
    w.Append(&wifiBtn);
    w.Append(&optionBrowser3);

    mainWindow->Append(&w);


    int tmp=num_titles+num_sys_titles;
    ResumeGui();
    numtitle=num_titles;
    while (!exit) {
        VIDEO_WaitVSync();

        if (shutdown == 1)
            Sys_Shutdown();
        if (reset == 1)
            Sys_Reboot();

        else if (wifiBtn.GetState() == STATE_CLICKED) {

                ResumeNetworkWait();
                wifiBtn.ResetState();
        }

        if (IsNetworkInit()) {
                wifiBtn.SetAlpha(255);
        }

        ret = optionBrowser3.GetClickedOption();

        if (ret > -1) {//if a click happened

            //char name[50];
            char text[15];
            if (f)rewind(f);
            //strcpy(name,"");//make sure name is empty

            if (ret<numtitle) {
                //set the title's name, number, ID to text
                sprintf(text, "%s", titleText(type, titles[ret]));

                char temp[100];
                //prompt to boot selected title
                snprintf(temp, sizeof(temp), "%s : %s",text,name[ret]);
                int  choice = WindowPrompt(tr("Boot?"), temp, tr("OK"), tr("Cancel"));
                if (choice) {//if they say yes


                    //stop all this stuff before starting the channel

                    CloseXMLDatabase();
                    ExitGUIThreads();
                    ShutdownAudio();
                    StopGX();
                    WII_Initialize();
                    WII_LaunchTitle(TITLE_ID(type,titles[ret]));
                    //this really shouldn't be needed because the title will be booted
                    exit = true;
                    break;
                } else {
                    //if they said no to booting the title
                    ret = -1;
                    optionBrowser3.ResetState();
                }

            } else { //if they clicked a system title
                if (ret == tmp) {
                    CloseXMLDatabase();
                    ExitGUIThreads();
                    ShutdownAudio();
                    StopGX();
                    WII_Initialize();
                    WII_ReturnToSettings();

                } else {
                    //set the title's name, number, ID to text
                    sprintf(text, "%s", titleText(0x00010002, sys_titles[ret-num_titles]));

                    char temp[112];
                    //prompt to boot selected title
					snprintf(temp, sizeof(temp), tr("%s : %s May not boot correctly if your System Menu is not up to date."),text,name[ret]);
                    int  choice = WindowPrompt(tr("Boot?"), temp, tr("OK"), tr("Cancel"));
                    if (choice) {//if they say yes


                        //stop all this stuff before starting the channel

                        CloseXMLDatabase();
                        ExitGUIThreads();
                        ShutdownAudio();
                        StopGX();
                        WII_Initialize();
                        WII_LaunchTitle(TITLE_ID(0x00010002,sys_titles[ret-num_titles]));
                        //this really shouldn't be needed because the title will be booted
                        exit = true;
                        break;
                    } else {
                        //if they said no to booting the title
                        ret = -1;
                        optionBrowser3.ResetState();
                    }
                }
            }
        }

        if(infilesize > 0) {

                char filesizetxt[50];
                char temp[50];
                char filepath[100];
//				u32 read = 0;
				
				//make sure there is a folder for this to be saved in
				struct stat st;
                snprintf(filepath, sizeof(filepath), "%s/wad/", bootDevice);
				if (stat(filepath, &st) != 0) {
						if (subfoldercreate(filepath) != 1) {
							WindowPrompt(tr("Error !"),tr("Can't create directory"),tr("OK"));
						}
					}
				snprintf(filepath, sizeof(filepath), "%s/wad/tmp.tmp", bootDevice);
				

                if (infilesize < MB_SIZE)
                    snprintf(filesizetxt, sizeof(filesizetxt), tr("Incoming file %0.2fKB"), infilesize/KB_SIZE);
                else
                    snprintf(filesizetxt, sizeof(filesizetxt), tr("Incoming file %0.2fMB"), infilesize/MB_SIZE);

                snprintf(temp, sizeof(temp), tr("Load file from: %s ?"), GetIncommingIP());

                int choice = WindowPrompt(filesizetxt, temp, tr("OK"), tr("Cancel"));
		gprintf("\nchoice:%d",choice);

		if (choice == 1) {

			u32 read = 0;
			u8 *temp = NULL;
			int len = NETWORKBLOCKSIZE;
			temp = (u8 *) malloc(infilesize);

						bool error = false;
						u8 *ptr = temp;
						gprintf("\nrecieving shit");
			while (read < infilesize) {

			    ShowProgress(tr("Receiving file from:"), GetIncommingIP(), NULL, read, infilesize, true);

			    if (infilesize - read < (u32) len)
				len = infilesize-read;
			    else
				len = NETWORKBLOCKSIZE;

			    int result = network_read(ptr, len);

			    if (result < 0) {
				WindowPrompt(tr("Error while transfering data."), 0, tr("OK"));
				error = true;
				break;
			    }
			    if (!result) {
				gprintf("\n!RESULT");
				break;
							}
			    ptr += result;
			    read += result;
			}
			ProgressStop();

						char filename[101];
						char tmptxt[200];



						//bool installWad=0;
						if (!error) {
						    gprintf("\nno error yet");

							network_read((u8*) &filename, 100);
							gprintf("\nfilename: %s",filename);

							// Do we need to unzip this thing?
							if (wiiloadVersion[0] > 0 || wiiloadVersion[1] > 4) {
							    gprintf("\nusing newer wiiload version");

								if (uncfilesize != 0) { // if uncfilesize == 0, it's not compressed
								    gprintf("\ntrying to uncompress");
									// It's compressed, uncompress
									u8 *unc = (u8 *) malloc(uncfilesize);
									uLongf f = uncfilesize;
									error = uncompress(unc, &f, temp, infilesize) != Z_OK;
									uncfilesize = f;

									free(temp);
									temp = unc;
								}
							}

							if (!error) {
								sprintf(tmptxt,"%s",filename);
								//if we got a wad
								if (strcasestr(tmptxt,".wad")) {
								    FILE *file = fopen(filepath, "wb");
								    fwrite(temp, 1, (uncfilesize>0?uncfilesize:infilesize), file);
								    fclose(file);

								    sprintf(tmptxt,"%s/wad/%s",bootDevice,filename);
								    if (checkfile(tmptxt))remove(tmptxt);
								    rename(filepath, tmptxt);

								    //check and make sure the wad we just saved is the correct size
								    u32 lSize;
								    file = fopen(tmptxt, "rb");

								    // obtain file size:
								    fseek (file , 0 , SEEK_END);
								    lSize = ftell (file);

								    rewind (file);
								    if (lSize==(uncfilesize>0?uncfilesize:infilesize)) {
									gprintf("\nsize is ok");
									int pick = WindowPrompt(tr(" Wad Saved as:"), tmptxt, tr("Install"),tr("Uninstall"),tr("Cancel"));
									//install or uninstall it
									if (pick==1)
										{
											HaltGui();
											w.Remove(&titleTxt);
											w.Remove(&cancelBtn);
											w.Remove(&wifiBtn);
											w.Remove(&optionBrowser3);
											ResumeGui();

											Wad_Install(file);

											HaltGui();
											w.Append(&titleTxt);
											w.Append(&cancelBtn);
											w.Append(&wifiBtn);
											w.Append(&optionBrowser3);
											ResumeGui();

										}
									if (pick==2)Wad_Uninstall(file);
								    }
								    else gprintf("\nBad size");
								    //close that beast, we're done with it
								    fclose (file);

								    //do we want to keep the file in the wad folder
								    if (WindowPrompt(tr("Delete ?"), tmptxt, tr("Delete"),tr("Keep"))!=0)
									remove(tmptxt);
								    }
								else {
								    WindowPrompt(tr("ERROR:"), tr("Not a WAD file."), tr("OK"));
								    }
							}
						}



			if (error || read != infilesize) {
			    WindowPrompt(tr("Error:"), tr("No data could be read."), tr("OK"));


			}
			if(temp)free(temp);
		}



		CloseConnection();
                ResumeNetworkWait();
        }

        if (cancelBtn.GetState() == STATE_CLICKED) {
            //break the loop and end the function
            exit = true;
            ret = -10;
        }
	else if (screenShotBtn.GetState() == STATE_CLICKED) {
			gprintf("\n\tscreenShotBtn clicked");
			screenShotBtn.ResetState();
			ScreenShot();
			gprintf("...It's easy, mmmmmmKay");
		    }
    }

    CloseConnection();
    if (IsNetworkInit())
        HaltNetworkThread();

    fclose(f);
    HaltGui();
    mainWindow->Remove(&w);
    ResumeGui();

    return ret;
}



