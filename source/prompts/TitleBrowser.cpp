/****************************************************************************
 * TitleBrowser
 * USB Loader GX 2009
 *
 * TitleBrowser.cpp
 ***************************************************************************/
#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "prompts/PromptWindows.h"
#include "filelist.h"
#include "settings/cfg.h"
#include "sys.h"
#include "menu.h"
#include "audio.h"

#include "xml/xml.h"

#include "../wad/title.h"

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern u8 shutdown;
extern u8 reset;


/********************************************************************************
* TitleBrowser- opens a browser with a list of installed Titles
* relies on code from any title deleter.
*********************************************************************************/
int TitleBrowser(u32 type)
{	

	u32 num_titles;
	u32 titles[100] ATTRIBUTE_ALIGN(32);
	u32 num_sys_titles;
	u32 sys_titles[10] ATTRIBUTE_ALIGN(32);
	s32 ret = -1;
	int numtitle;//to get rid of a stupid compile wrning
		//open the database file
	FILE *f;
	char path[100];
	
	sprintf(path,"%s/config/database.txt",bootDevice);
	f = fopen(path, "r");
		
		// Get count of titles of our requested type
		ret = getTitles_TypeCount(type, &num_titles);
		if (ret < 0){
			//printf("\tError! Can't get count of titles! (ret = %d)\n", ret);
			//exit(1);
		}
		
		// Die if we can't handle this many
		if (num_titles > 100){
			//printf("\tError! Too many titles! (%u)\n", num_titles);
			//exit(1);
		}
		
		// Get titles of our requested type
		ret = getTitles_Type(type, titles, num_titles);
		if (ret < 0){
			//printf("\tError! Can't get list of titles! (ret = %d)\n", ret);
			//exit(1);
		}
		
		// Get count of system titles
		ret = getTitles_TypeCount(0x00010002, &num_sys_titles);
		if (ret < 0){
			//printf("\tError! Can't get count of titles! (ret = %d)\n", ret);
			//exit(1);
		}
		
		// Get system titles
		ret = getTitles_Type(0x00010002, sys_titles, num_sys_titles);
		if (ret < 0){
			//printf("\tError! Can't get list of titles! (ret = %d)\n", ret);
			//exit(1);
		}
		

		
	
	customOptionList options3(num_titles+num_sys_titles);
	//write the titles on the option browser
	u32 i = 0;
			
			

			//first add the good stuff
			while (i < num_titles){
			//start from the beginning of the file each loop
			if (f)rewind(f);
				char name[50];
				char text[15];
				strcpy(name,"");//make sure name is empty
				
				//set the title's name, number, ID to text 
				sprintf(text, "%s", titleText(type, titles[i]));
				//getTitle_Name(name, TITLE_ID(type, titles[i]), text);
				
				//get name from database cause i dont like the ADT function
						char line[200];
						char tmp[50];
						snprintf(tmp,50,tmp," ");
						snprintf(name,sizeof(name),"Unknown Title");

						if (!f) {
							sprintf(name,"Unknown--<No DB>");
						}
						else
						{
						while (fgets(line, sizeof(line), f)) {
								if (line[0]== text[0]&&
									line[1]== text[1]&&
									line[2]== text[2])
								{	int j=0;
									for(j=0;(line[j+4]!='\0' || j<51);j++)
										
										tmp[j]=line[j+4];
										snprintf(name,sizeof(name),"%s",tmp);
										break;

								}
							
						}
					
					}
				//set the text to the option browser
				options3.SetName(i, "%s",text);
				options3.SetValue(i, "%s",name);
				//options3.SetValue(i, " (%08x)",titles[i]);//use this line to show the number to call to launch the channel
				//move on to the next title
				i++;
			}
			
			// now add the crappy system titles
			while (i < num_titles+num_sys_titles){
			//start from the beginning of the file each loop
			if (f)rewind(f);
				char name[50];
				char text[15];
				strcpy(name,"");//make sure name is empty
				
				//set the title's name, number, ID to text 
				sprintf(text, "%s", titleText(0x00010002, sys_titles[i-num_titles]));
				//getTitle_Name(name, TITLE_ID(0x00010002, sys_titles[i-num_titles]), text);
				
				//get name from database cause i dont like the ADT function
						char line[200];
						char tmp[50];
						snprintf(tmp,50,tmp," ");
						snprintf(name,sizeof(name),"Unknown Title");

						if (!f) {
							sprintf(name,"Unknown--<No DB>");
						}
						else
						{
						while (fgets(line, sizeof(line), f)) {
								if (line[0]== text[0]&&
									line[1]== text[1]&&
									line[2]== text[2])
								{	int j=0;
									for(j=0;(line[j+4]!='\0' || j<51);j++)
										
										tmp[j]=line[j+4];
										snprintf(name,sizeof(name),"%s",tmp);
										break;

								}
							
						}
					
					}
				//set the text to the option browser
				options3.SetName(i, "%s",text);
				options3.SetValue(i, "%s",name);
				//options3.SetValue(i, " (%08x)",titles[i]);
				//move on to the next title
				i++;
			}
	
	bool exit = false;

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

    GuiText titleTxt(tr("Title Launcher"), 28, (GXColor){0, 0, 0, 255});
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
	if(num_titles > 9)
        scrollbaron = 1;

	GuiCustomOptionBrowser optionBrowser3(396, 280, &options3, CFG.theme_path, "bg_options_gamesettings.png", bg_options_settings_png, num_titles>9?1:0, 200);
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
	numtitle=num_titles;
	while(!exit)
	{
		VIDEO_WaitVSync();

		if(shutdown == 1)
			Sys_Shutdown();
		if(reset == 1)
			Sys_Reboot();

		ret = optionBrowser3.GetClickedOption();

		if(ret > -1) {//if a click happened
		
				char name[50];
				char text[15];
				if (f)rewind(f);
				strcpy(name,"");//make sure name is empty
				
				if (ret<numtitle)
				{
				//set the title's name, number, ID to text 
				sprintf(text, "%s", titleText(type, titles[ret]));
				getTitle_Name(name, TITLE_ID(type, titles[ret]), text);
				
				//get name from database cause i dont like the ADT function
						char line[200];
						char tmp[50];
						snprintf(tmp,50,tmp," ");
						snprintf(name,sizeof(name),"Unknown Title");

						if (!f) {
							sprintf(name,"Unknown--<No DB>");
						}
						else
						{
						while (fgets(line, sizeof(line), f)) {
								if (line[0]== text[0]&&
									line[1]== text[1]&&
									line[2]== text[2])
								{	int j=0;
									for(j=0;(line[j+4]!='\0' || j<51);j++)
										
										tmp[j]=line[j+4];
										snprintf(name,sizeof(name),"%s",tmp);
										break;
								}
						}
					}
					char temp[100];
					 //prompt to boot selected title
					 snprintf(temp, sizeof(temp), "%s : %s",text,name);
					 int  choice = WindowPrompt(tr("Boot?"), temp, tr("OK"), tr("Cancel"));
						if(choice) {//if they say yes
						
				
				//stop all this stuff before starting the channel
					fclose(f);
					CloseXMLDatabase();
					ExitGUIThreads();
					ShutdownAudio();
					StopGX();
						WII_Initialize();
						WII_LaunchTitle(TITLE_ID(type,titles[ret]));
                //this really shouldn't be needed because the title will be booted
					 exit = true;
                break;
            }
				else{
					//if they said no to booting the title
					ret = -1;
					optionBrowser3.ResetState();
					}
					
				}
				else//if they clicked a system title
				{
				//set the title's name, number, ID to text 
				sprintf(text, "%s", titleText(0x00010002, sys_titles[ret-num_titles]));
				getTitle_Name(name, TITLE_ID(0x00010002, sys_titles[ret-num_titles]), text);
				
				//get name from database cause i dont like the ADT function
						char line[200];
						char tmp[50];
						snprintf(tmp,50,tmp," ");
						snprintf(name,sizeof(name),"Unknown Title");

						if (!f) {
							sprintf(name,"Unknown--<No DB>");
						}
						else
						{
						while (fgets(line, sizeof(line), f)) {
								if (line[0]== text[0]&&
									line[1]== text[1]&&
									line[2]== text[2])
								{	int j=0;
									for(j=0;(line[j+4]!='\0' || j<51);j++)
										
										tmp[j]=line[j+4];
										snprintf(name,sizeof(name),"%s",tmp);
										break;
								}
						}
					}
					char temp[100];
					 //prompt to boot selected title
					 snprintf(temp, sizeof(temp), "%s : %s May not boot correctly if your System Menu is not up to date.",text,name);
					 int  choice = WindowPrompt(tr("Boot?"), temp, tr("OK"), tr("Cancel"));
						if(choice) {//if they say yes
						
				
				//stop all this stuff before starting the channel
					fclose(f);
					CloseXMLDatabase();
					ExitGUIThreads();
					ShutdownAudio();
					StopGX();
						WII_Initialize();
						WII_LaunchTitle(TITLE_ID(0x00010002,sys_titles[ret-num_titles]));
                //this really shouldn't be needed because the title will be booted
					 exit = true;
                break;
            }
				else{
					//if they said no to booting the title
					ret = -1;
					optionBrowser3.ResetState();
					}
					
				}
		}

		if (cancelBtn.GetState() == STATE_CLICKED)
		{
			//break the loop and end the function
			exit = true;
			ret = -10;
		}
	}
	
	fclose(f);
	HaltGui();
	mainWindow->Remove(&w);
	ResumeGui();
	
	return ret;
}


