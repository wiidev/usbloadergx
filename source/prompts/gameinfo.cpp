#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "usbloader/wbfs.h"
#include "language/language.h"
#include "libwiigui/gui.h"
#include "../xml/xml.h"
#include "menu.h"
#include "filelist.h"
#include "sys.h"
#include "wpad.h"
#include "fatmounter.h"
#include "listfiles.h"
#include "gameinfo.h"


/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();


/****************************************************************************
* gameinfo
***************************************************************************/
int
showGameInfo(char *ID, u8 *headerID)
{
    //load the xml shit
	char pathname[100];
	snprintf(pathname, sizeof(pathname), "%s%s", Settings.titlestxt_path, "wiitdb.zip");
	// database should always be zipped, while it may be convenient detecting and loading from .xml may lead to confusion:
	bool fileexists = OpenXMLFile(pathname);
	if(!fileexists) {
        snprintf(pathname, sizeof(pathname), "%s%s", Settings.titlestxt_path, "wiitdb.xml");
        fileexists = OpenXMLFile(pathname);
	}
    if(fileexists) {

	int choice = -1;
	//int i = 0;
	int y = 0, y1 = 32;
	int intputX =200, inputY=-30, txtXOffset = 90;
	u8 nunchuk=0,
	classiccontroller=0,
	balanceboard=0,
	dancepad=0,
	guitar=0,
	gamecube=0,
	wheel=0,
	motionplus=0,
	drums=0,
	microphone=0;
	int newline=1;
	u8 page =1;

 //   GuiText * debugTxt = NULL;

    GuiImageData * playersImgData = NULL;
    GuiImage * playersImg = NULL;

    GuiImageData * wifiplayersImgData = NULL;
    GuiImage * wifiplayersImg = NULL;
    GuiImage * ratingImg = NULL;

    GuiImage * classiccontrollerImg = NULL;
    GuiImage * nunchuckImg = NULL;
    GuiImage * guitarImg = NULL;
    GuiImage * drumsImg = NULL;
    GuiImage * dancepadImg = NULL;
    GuiImage * motionplusImg = NULL;
    GuiImage * wheelImg = NULL;
    GuiImage * balanceboardImg = NULL;
    GuiImage * microphoneImg = NULL;
    GuiImage * gcImg = NULL;
    GuiImage * dialogBoxImg1 = NULL;
    GuiImage * dialogBoxImg2 = NULL;
    GuiImage * dialogBoxImg3 = NULL;
    GuiImage * dialogBoxImg4 = NULL;
    GuiImage * dialogBoxImg11 = NULL;
    GuiImage * dialogBoxImg22 = NULL;
    GuiImage * dialogBoxImg33 = NULL;
    GuiImage * dialogBoxImg44 = NULL;
    GuiImage * coverImg = NULL;
    GuiImage * coverImg2 = NULL;

    GuiImageData * classiccontrollerImgData = NULL;
    GuiImageData * nunchuckImgData = NULL;
    GuiImageData * guitarImgData = NULL;
    GuiImageData * drumsImgData = NULL;
    GuiImageData * motionplusImgData = NULL;
    GuiImageData * wheelImgData = NULL;
    GuiImageData * balanceboardImgData = NULL;
    GuiImageData * dancepadImgData = NULL;
    GuiImageData * microphoneImgData = NULL;
    GuiImageData * gamecubeImgData = NULL;
    GuiImageData * ratingImgData = NULL;
    GuiImageData * cover = NULL;

    GuiText * releasedTxt = NULL;
    GuiText * publisherTxt = NULL;
    GuiText * developerTxt = NULL;
    GuiText * titleTxt = NULL;
    GuiText * synopsisTxt = NULL;
    GuiText * genreTxt = NULL;
    GuiText * betaTxt = NULL;
    GuiText * beta1Txt = NULL;
	GuiText ** wifiTxt = NULL;

	GuiWindow gameinfoWindow(600,308);
	gameinfoWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	gameinfoWindow.SetPosition(0, -50);

	GuiWindow gameinfoWindow2(600,308);
	gameinfoWindow2.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	gameinfoWindow2.SetPosition(0, -50);

	GuiWindow txtWindow(350,270);
	txtWindow.SetAlignment(ALIGN_CENTRE, ALIGN_RIGHT);
	txtWindow.SetPosition(85, 50);


	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);
	char imgPath[50];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sgameinfo1_png.png", CFG.theme_path);
	GuiImageData dialogBox1(imgPath, gameinfo1_png);
	snprintf(imgPath, sizeof(imgPath), "%sgameinfo1a_png.png", CFG.theme_path);
	GuiImageData dialogBox2(imgPath, gameinfo1a_png);
	snprintf(imgPath, sizeof(imgPath), "%sgameinfo2_png.png", CFG.theme_path);
	GuiImageData dialogBox3(imgPath, gameinfo2_png);
	snprintf(imgPath, sizeof(imgPath), "%sgameinfo2a_png.png", CFG.theme_path);
	GuiImageData dialogBox4(imgPath, gameinfo2a_png);

	GuiTrigger trigA;
	trigA.SetButtonOnlyTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);


	GuiButton backBtn(0,0);
	backBtn.SetPosition(-20,-20);
	backBtn.SetTrigger(&trigB);
	gameinfoWindow.Append(&backBtn);

	GuiButton nextBtn(0,0);
	nextBtn.SetPosition(20,20);
	nextBtn.SetTrigger(&trigA);
	gameinfoWindow.Append(&nextBtn);
	
	struct Game_CFG *game_cfg = NULL;
	int opt_lang;
	char langtexttmp[11][22] =
	{{"Console Default"},
	{"Japanese"},
	{"English"},
	{"German"},
	{"French"},
	{"Spanish"},
	{"Italian"},
	{"Dutch"},
	{"S. Chinese"},
	{"T. Chinese"},
	{"Korean"}};
	game_cfg = CFG_get_game_opt((u8*)ID);
	if (game_cfg) {
		opt_lang = game_cfg->language;
	} else {
		opt_lang = Settings.language;
	}
	LoadGameInfoFromXML(ID,langtexttmp[opt_lang]);
	char linebuf[1000] = "";
	char linebuf2[100] = "";

	// set images for required input
	for (int i=1;strcmp(gameinfo.accessories_required[i],"") != 0;i++)
			{
			if (strcmp(gameinfo.accessories_required[i],"nunchuk")==0)
			{nunchuckImgData = new GuiImageData(nunchuckR_png);nunchuk=1;}
			else
			nunchuckImgData = new GuiImageData(nunchuck_png);

			if (strcmp(gameinfo.accessories_required[i],"classiccontroller")==0)
			{classiccontrollerImgData = new GuiImageData(classiccontrollerR_png);classiccontroller=1;}
			else
			classiccontrollerImgData = new GuiImageData(classiccontroller_png);

			if (strcmp(gameinfo.accessories_required[i],"guitar")==0)
			{guitarImgData = new GuiImageData(guitarR_png);guitar=1;}
			else
			guitarImgData = new GuiImageData(guitar_png);

			if (strcmp(gameinfo.accessories_required[i],"gamecube")==0)
			{gamecubeImgData = new GuiImageData(gcncontrollerR_png);gamecube=1;}
			else
			gamecubeImgData = new GuiImageData(gcncontroller_png);

			if (strcmp(gameinfo.accessories_required[i],"wheel")==0)
			{wheelImgData = new GuiImageData(wheelR_png);wheel=1;}
			else
			wheelImgData = new GuiImageData(wheel_png);

			if (strcmp(gameinfo.accessories_required[i],"motionplus")==0)
			{motionplusImgData = new GuiImageData(motionplusR_png);motionplus=1;}
			else
			motionplusImgData = new GuiImageData(motionplus_png);

			if (strcmp(gameinfo.accessories_required[i],"drums")==0)
			{drumsImgData = new GuiImageData(drumsR_png);drums=1;}
			else
			drumsImgData = new GuiImageData(drums_png);

			if (strcmp(gameinfo.accessories_required[i],"microphone")==0)
			{microphoneImgData = new GuiImageData(microphoneR_png);microphone=1;}
			else
			microphoneImgData = new GuiImageData(microphone_png);

			if (strcmp(gameinfo.accessories_required[i],"balanceboard")==0)
			{balanceboardImgData = new GuiImageData(balanceboardR_png);balanceboard=1;}
			else
			balanceboardImgData = new GuiImageData(balanceboard_png);

			if (strcmp(gameinfo.accessories_required[i],"dancepad")==0)
			{dancepadImgData = new GuiImageData(dancepadR_png);dancepad=1;}
			else
			dancepadImgData = new GuiImageData(dancepad_png);

			}
	for (int i=1;strcmp(gameinfo.accessories[i],"") != 0;i++)
			{

			if (strcmp(gameinfo.accessories[i],"classiccontroller")==0)
			classiccontroller=1;

			if (strcmp(gameinfo.accessories[i],"nunchuk")==0)
			{nunchuk=1;}

			if (strcmp(gameinfo.accessories[i],"guitar")==0)
			guitar=1;

			if (strcmp(gameinfo.accessories[i],"drums")==0)
			drums=1;

			if (strcmp(gameinfo.accessories[i],"dancepad")==0)
			dancepad=1;

			if (strcmp(gameinfo.accessories[i],"motionplus")==0)
			motionplus=1;

			if (strcmp(gameinfo.accessories[i],"wheel")==0)
			wheel=1;

			if (strcmp(gameinfo.accessories[i],"balanceboard")==0)
			balanceboard=1;

			if (strcmp(gameinfo.accessories[i],"microphone")==0)
			microphone=1;

			if (strcmp(gameinfo.accessories[i],"gamecube")==0)
			gamecube=1;
	}




	dialogBoxImg1 = new GuiImage(&dialogBox1);
	dialogBoxImg1->SetAlignment(0,3);
	dialogBoxImg1->SetPosition(-9,0);

	dialogBoxImg2 = new GuiImage(&dialogBox2);
	dialogBoxImg2->SetAlignment(0,3);
	dialogBoxImg2->SetPosition(145,0);

	dialogBoxImg3 = new GuiImage(&dialogBox3);
	dialogBoxImg3->SetAlignment(0,3);
	dialogBoxImg3->SetPosition(301,0);

	dialogBoxImg4 = new GuiImage(&dialogBox4);
	dialogBoxImg4->SetAlignment(0,3);
	dialogBoxImg4->SetPosition(457,0);


	gameinfoWindow.Append(dialogBoxImg1);
	gameinfoWindow.Append(dialogBoxImg2);
	gameinfoWindow.Append(dialogBoxImg3);
	gameinfoWindow.Append(dialogBoxImg4);

	snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, ID);
	cover = new GuiImageData(imgPath, 0); //load full id image
	if (!cover->GetImage())
	{
		delete cover;
		snprintf(imgPath, sizeof(imgPath), "%snoimage.png", Settings.covers_path);
		cover = new GuiImageData(imgPath, nocover_png); //load no image
	}
		delete coverImg;
		coverImg = NULL;

	coverImg = new GuiImage(cover);
	coverImg->SetWidescreen(CFG.widescreen);
	coverImg->SetPosition(15,30);
	gameinfoWindow.Append(coverImg);

	// # of players
	if (strcmp(gameinfo.players,"") != 0) {
		playersImgData = new GuiImageData(Wiimote1_png);
		if (atoi(gameinfo.players)>1){
			playersImgData= new GuiImageData(Wiimote2_png);}
		if (atoi(gameinfo.players)>2){
			playersImgData= new GuiImageData(Wiimote4_png);}

		playersImg = new GuiImage(playersImgData);
		playersImg->SetWidescreen(CFG.widescreen);
		playersImg->SetPosition(intputX , inputY);
		playersImg->SetAlignment(0,4);
		gameinfoWindow.Append(playersImg);
		intputX += (CFG.widescreen ? playersImg->GetWidth() * .8 : playersImg->GetWidth())+5;
		}

	//draw the inupt types for this game
			if (nunchuk==1){
			nunchuckImg = new GuiImage(nunchuckImgData);
			nunchuckImg->SetWidescreen(CFG.widescreen);
			nunchuckImg->SetPosition(intputX , inputY);
			nunchuckImg->SetAlignment(0,4);
			gameinfoWindow.Append(nunchuckImg);
			intputX += (CFG.widescreen ? nunchuckImg->GetWidth() * .8 : nunchuckImg->GetWidth())+5;}

			if (classiccontroller==1){
			classiccontrollerImg = new GuiImage(classiccontrollerImgData);
			classiccontrollerImg->SetWidescreen(CFG.widescreen);
			classiccontrollerImg->SetPosition(intputX , inputY);
			classiccontrollerImg->SetAlignment(0,4);
			gameinfoWindow.Append(classiccontrollerImg);
			intputX += (CFG.widescreen ? classiccontrollerImg->GetWidth() * .8 : classiccontrollerImg->GetWidth())+5;}

			if (gamecube==1){
			gcImg = new GuiImage(gamecubeImgData);
			gcImg->SetWidescreen(CFG.widescreen);
			gcImg->SetPosition(intputX , inputY);
			gcImg->SetAlignment(0,4);
			gameinfoWindow.Append(gcImg);
			intputX += (CFG.widescreen ? gcImg->GetWidth() * .8 : gcImg->GetWidth())+5;}

			if (wheel==1){
			wheelImg = new GuiImage(wheelImgData);
			wheelImg->SetWidescreen(CFG.widescreen);
			wheelImg->SetPosition(intputX , inputY);
			wheelImg->SetAlignment(0,4);
			gameinfoWindow.Append(wheelImg);
			intputX += (CFG.widescreen ? wheelImg->GetWidth() * .8 : wheelImg->GetWidth())+5;}

			if (guitar==1){
			guitarImg = new GuiImage(guitarImgData);
			guitarImg->SetWidescreen(CFG.widescreen);
			guitarImg->SetPosition(intputX , inputY);
			guitarImg->SetAlignment(0,4);
			gameinfoWindow.Append(guitarImg);
			intputX += (CFG.widescreen ? guitarImg->GetWidth() * .8 : guitarImg->GetWidth())+5;}

			if (drums==1){
			drumsImg = new GuiImage(drumsImgData);
			drumsImg->SetWidescreen(CFG.widescreen);
			drumsImg->SetPosition(intputX , inputY);
			drumsImg->SetAlignment(0,4);
			gameinfoWindow.Append(drumsImg);
			intputX += (CFG.widescreen ? drumsImg->GetWidth() * .8 : drumsImg->GetWidth())+5;}

			if (microphone==1){
			microphoneImg = new GuiImage(microphoneImgData);
			microphoneImg->SetWidescreen(CFG.widescreen);
			microphoneImg->SetPosition(intputX , inputY);
			microphoneImg->SetAlignment(0,4);
			gameinfoWindow.Append(microphoneImg);
			intputX += (CFG.widescreen ? microphoneImg->GetWidth() * .8 : microphoneImg->GetWidth())+5;}

			if (dancepad==1){
			dancepadImg = new GuiImage(dancepadImgData);
			dancepadImg->SetWidescreen(CFG.widescreen);
			dancepadImg->SetPosition(intputX , inputY);
			dancepadImg->SetAlignment(0,4);
			gameinfoWindow.Append(dancepadImg);
			intputX += (CFG.widescreen ? dancepadImg->GetWidth() * .8 : dancepadImg->GetWidth())+5;}

			if (motionplus==1){
			motionplusImg = new GuiImage(motionplusImgData);
			motionplusImg->SetWidescreen(CFG.widescreen);
			motionplusImg->SetPosition(intputX , inputY);
			motionplusImg->SetAlignment(0,4);
			gameinfoWindow.Append(motionplusImg);
			intputX += (CFG.widescreen ? motionplusImg->GetWidth() * .8 : motionplusImg->GetWidth())+5;}




		// # online players
	if ((strcmp(gameinfo.wifiplayers,"") != 0) && (strcmp(gameinfo.wifiplayers,"0") != 0)){
		wifiplayersImgData = new GuiImageData(wifi1_png);
		if (atoi(gameinfo.wifiplayers)>1){
			wifiplayersImgData= new GuiImageData(wifi2_png);}
		if (atoi(gameinfo.wifiplayers)>2){
			wifiplayersImgData= new GuiImageData(wifi4_png);}
		if (atoi(gameinfo.wifiplayers)>4){
			wifiplayersImgData= new GuiImageData(wifi8_png);}
		/*if (atoi(gameinfo.wifiplayers)>8){       uncomment this when we actually have these images and it is needed
			wifiplayersImgData= new GuiImageData(wifi12_png);}
		if (atoi(gameinfo.wifiplayers)>12){
			wifiplayersImgData= new GuiImageData(wifi16_png);}
		if (atoi(gameinfo.wifiplayers)>16){
			wifiplayersImgData= new GuiImageData(wifi32_png);}*/
		wifiplayersImg = new GuiImage(wifiplayersImgData);
		wifiplayersImg->SetWidescreen(CFG.widescreen);
		wifiplayersImg->SetPosition(intputX , inputY);
		wifiplayersImg->SetAlignment(0,4);
		gameinfoWindow.Append(wifiplayersImg);
		intputX += (CFG.widescreen ? wifiplayersImg->GetWidth() * .8 : wifiplayersImg->GetWidth())+5;
		}

	// ratings
	if (strcmp(gameinfo.ratingtype,"") !=0){
	if (strcmp(gameinfo.ratingtype,"ESRB")==0) {
		if (strcmp(gameinfo.ratingvalueESRB,"EC")==0)
			ratingImgData = new GuiImageData(esrb_ec_png);
		else if (strcmp(gameinfo.ratingvalueESRB,"E")==0)
			ratingImgData = new GuiImageData(esrb_e_png);
		else if (strcmp(gameinfo.ratingvalueESRB,"E10+")==0)
			ratingImgData = new GuiImageData(esrb_eten_png);
		else if (strcmp(gameinfo.ratingvalueESRB,"T")==0)
			ratingImgData = new GuiImageData(esrb_t_png);
		else if (strcmp(gameinfo.ratingvalueESRB,"M")==0)
			ratingImgData = new GuiImageData(esrb_m_png);
		else if (strcmp(gameinfo.ratingvalueESRB,"AO")==0)
			ratingImgData = new GuiImageData(esrb_ao_png);
		else {ratingImgData = new GuiImageData(norating_png);}
		}											//there are 2 values here cause some countries are stupid and
	else if (strcmp(gameinfo.ratingtype,"PEGI")==0) {//can't use the same as everybody else
		if ((strcmp(gameinfo.ratingvaluePEGI,"3")==0)||(strcmp(gameinfo.ratingvaluePEGI,"4")==0))
			ratingImgData = new GuiImageData(pegi_3_png);
		else if ((strcmp(gameinfo.ratingvaluePEGI,"7")==0)||(strcmp(gameinfo.ratingvaluePEGI,"7")==0))
			ratingImgData = new GuiImageData(pegi_7_png);
		else if (strcmp(gameinfo.ratingvaluePEGI,"12")==0)
			ratingImgData = new GuiImageData(pegi_12_png);
		else if ((strcmp(gameinfo.ratingvaluePEGI,"16")==0)||(strcmp(gameinfo.ratingvaluePEGI,"15")==0))
			ratingImgData = new GuiImageData(pegi_16_png);
		else if (strcmp(gameinfo.ratingvaluePEGI,"18")==0)
			ratingImgData = new GuiImageData(pegi_18_png);
		else {ratingImgData = new GuiImageData(norating_png);}
		}
	else if (strcmp(gameinfo.ratingtype,"CERO")==0) {
		if (strcmp(gameinfo.ratingvalueCERO,"A")==0)
			ratingImgData = new GuiImageData(cero_a_png);
		else if (strcmp(gameinfo.ratingvalueCERO,"B")==0)
			ratingImgData = new GuiImageData(cero_b_png);
		else if (strcmp(gameinfo.ratingvalueCERO,"C")==0)
			ratingImgData = new GuiImageData(cero_c_png);
		else if (strcmp(gameinfo.ratingvalueCERO,"D")==0)
			ratingImgData = new GuiImageData(cero_d_png);
		else if (strcmp(gameinfo.ratingvalueCERO,"Z")==0)
			ratingImgData = new GuiImageData(cero_z_png);
		else {ratingImgData = new GuiImageData(norating_png);}
		}

	else {ratingImgData = new GuiImageData(norating_png);}
		ratingImg = new GuiImage(ratingImgData);
		ratingImg->SetWidescreen(CFG.widescreen);
		ratingImg->SetPosition(-25 , inputY);
		ratingImg->SetAlignment(1,4);
		gameinfoWindow.Append(ratingImg);
		intputX += (CFG.widescreen ? ratingImg->GetWidth() * .8 : ratingImg->GetWidth())+5;

	}

//////////debugging line
//	snprintf(linebuf, sizeof(linebuf), "%s%s%s%s%i%i%i",gameinfo.accessories_required[1],gameinfo.accessories[1],gameinfo.accessories[2],gameinfo.accessories[3],gameinfo.accessoryCnt,
//	guitar,microphone);

		/*for (int i=0;strcmp(gameinfo.accessories_required[i+1],"") != 0;i++)
			{
				snprintf(linebuf, sizeof(linebuf), "%s  %s",linebuf, gameinfo.accessories_required[i+1]);
			}*/

//debugTxt = new GuiText(linebuf, 18, (GXColor){0,0,0, 255});
//			debugTxt->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM); debugTxt->SetPosition(0,0);
//			gameinfoWindow.Append(debugTxt);

	if (strcmp(gameinfo.title,"") != 0)
	{snprintf(linebuf, sizeof(linebuf), "%s",gameinfo.title);
	titleTxt = new GuiText(linebuf, 22, (GXColor){0,0,0, 255});
	if (titleTxt->GetWidth()>300)titleTxt->SetFontSize(18);
			titleTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); titleTxt->SetPosition(txtXOffset,12+y);  y+=24;
			gameinfoWindow.Append(titleTxt);}

		//date
	snprintf(linebuf2, sizeof(linebuf2), " ");
	if (strcmp(gameinfo.day,"") != 0)
	{snprintf(linebuf2, sizeof(linebuf2), "%s ", gameinfo.day);}
	if (strcmp(gameinfo.month,"") != 0){
		switch (atoi(gameinfo.month))
		{
			case 1:
				snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.january);
				break;
            case 2:
				snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.february);
				break;
            case 3:
				snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.march);
				break;
			case 4:
				snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.april);
				break;
			case 5:
				snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.may);
				break;
            case 6:
               snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.june);
                break;
            case 7:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.july);
                break;
			case 8:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.august);
                break;
			case 9:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.september);
                break;
			case 10:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.october);
                break;
			case 11:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.november);
                break;
			case 12:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, LANGUAGE.december);
                break;
	}
	}
	if (strcmp(gameinfo.year,"") != 0){newline=1;
	snprintf(linebuf, sizeof(linebuf), "%s : %s%s", LANGUAGE.released, linebuf2, gameinfo.year);
	releasedTxt = new GuiText(linebuf, 16, (GXColor){0,0,0, 255});
	if (releasedTxt->GetWidth()>300) newline=2;
			releasedTxt->SetAlignment(ALIGN_RIGHT, ALIGN_TOP); releasedTxt->SetPosition(-17,12+y);  y+=(20 * newline);newline=1;
			gameinfoWindow.Append(releasedTxt);}
	//genre
	if (strcmp(gameinfo.genre,"") != 0)
	{snprintf(linebuf, sizeof(linebuf), "%s", gameinfo.genre);
	genreTxt = new GuiText(linebuf, 16, (GXColor){0,0,0, 255});
	if (genreTxt->GetWidth()>300) newline=2;
			genreTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP); genreTxt->SetPosition(205,12+y1);  y1+=(25 * newline);newline=1;
			gameinfoWindow.Append(genreTxt);}

	//developer
	if (strcmp(gameinfo.developer,"") != 0 && strcmp(gameinfo.developer,gameinfo.publisher) != 0)
			{snprintf(linebuf, sizeof(linebuf), "%s %s", LANGUAGE.developedby, gameinfo.developer);
			developerTxt = new GuiText(linebuf, 16, (GXColor){0,0,0, 255});
			if (developerTxt->GetWidth()>250) newline=2;
			developerTxt->SetMaxWidth(250,GuiText::WRAP);
			developerTxt->SetAlignment(ALIGN_RIGHT, ALIGN_TOP); developerTxt->SetPosition(-17,12+y); y+=(40 * newline);newline=1;
			gameinfoWindow.Append(developerTxt);}

	//publisher
	if (strcmp(gameinfo.publisher,"") != 0)
	{snprintf(linebuf, sizeof(linebuf), "%s %s", LANGUAGE.publishedby, gameinfo.publisher);
	publisherTxt = new GuiText(linebuf, 16, (GXColor){0,0,0, 255});
			if (publisherTxt->GetWidth()>250) newline=2;
			publisherTxt->SetMaxWidth(250,GuiText::WRAP);
			publisherTxt->SetAlignment(ALIGN_RIGHT, ALIGN_TOP); publisherTxt->SetPosition(-17,12+y);  y+=(20 * newline);newline=1;
			gameinfoWindow.Append(publisherTxt);}

	//don't bother us txt
	snprintf(linebuf, sizeof(linebuf), "Don't bother the USB Loader GX Team about errors in this file.");
	betaTxt = new GuiText(linebuf, 14, (GXColor){0,0,0, 255});
			betaTxt->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM); betaTxt->SetPosition(-17,-20);//
			gameinfoWindow.Append(betaTxt);
	snprintf(linebuf, sizeof(linebuf), "A site will be available in the near nuture to submit changes.");
	beta1Txt = new GuiText(linebuf, 14, (GXColor){0,0,0, 255});
			beta1Txt->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM); beta1Txt->SetPosition(-17,-10);
			gameinfoWindow.Append(beta1Txt);

	wifiTxt = new GuiText * [gameinfo.wifiCnt + 1];
	int wifiY=0;
	for (int i=1;i<=gameinfo.wifiCnt;i++)
		{
			snprintf(linebuf, sizeof(linebuf), "%s",gameinfo.wififeatures[i]);
				wifiTxt[i] = new GuiText(linebuf, 16, (GXColor){0,0,0, 255});
				wifiTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); wifiTxt[i]->SetPosition(215,200+wifiY);  wifiY-=(20 * newline);
				gameinfoWindow.Append(wifiTxt[i]);
		}

	if (strcmp(gameinfo.wififeatures[1],"") != 0){
		snprintf(linebuf, sizeof(linebuf), "%s:",LANGUAGE.wififeatures);
					wifiTxt[0] = new GuiText(linebuf, 16, (GXColor){0,0,0, 255});
					wifiTxt[0]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); wifiTxt[0]->SetPosition(205,200+wifiY);  //wifiY+=(20 * newline);
					gameinfoWindow.Append(wifiTxt[0]);
	}
	//synopsis
	if (strcmp(gameinfo.synopsis,"") != 0)	{
		snprintf(linebuf, sizeof(linebuf), "%s", gameinfo.synopsis);
		synopsisTxt = new GuiText(linebuf, 16, (GXColor){0,0,0, 255});
		synopsisTxt->SetMaxWidth(350,GuiText::WRAP);
		synopsisTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP); synopsisTxt->SetPosition(0,0);
		dialogBoxImg11 = new GuiImage(&dialogBox1);
		dialogBoxImg11->SetAlignment(0,3);
		dialogBoxImg11->SetPosition(-9,0);

		dialogBoxImg22 = new GuiImage(&dialogBox2);
		dialogBoxImg22->SetAlignment(0,3);
		dialogBoxImg22->SetPosition(145,0);

		dialogBoxImg33 = new GuiImage(&dialogBox3);
		dialogBoxImg33->SetAlignment(0,3);
		dialogBoxImg33->SetPosition(301,0);

		dialogBoxImg44 = new GuiImage(&dialogBox4);
		dialogBoxImg44->SetAlignment(0,3);
		dialogBoxImg44->SetPosition(457,0);


		gameinfoWindow2.Append(dialogBoxImg11);
		gameinfoWindow2.Append(dialogBoxImg22);
		gameinfoWindow2.Append(dialogBoxImg33);
		gameinfoWindow2.Append(dialogBoxImg44);

		txtWindow.Append(synopsisTxt);
		coverImg2 = new GuiImage(cover);
		coverImg2->SetWidescreen(CFG.widescreen);
		coverImg2->SetPosition(15,30);
		gameinfoWindow2.Append(coverImg2);
		gameinfoWindow2.Append(&txtWindow);
	}

	gameinfoWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50);
	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&gameinfoWindow);
	mainWindow->ChangeFocus(&gameinfoWindow);
	ResumeGui();

	while(choice == -1)
	{
		VIDEO_WaitVSync();
		if(shutdown == 1)
		{
			wiilight(0);
			Sys_Shutdown();
		}
		if(reset == 1)
			Sys_Reboot();


		if ((backBtn.GetState()==STATE_CLICKED)||(backBtn.GetState()==STATE_HELD)){
		choice=1;
		synopsisTxt = NULL;
		break;}

		else if (((nextBtn.GetState()==STATE_CLICKED)||(nextBtn.GetState()==STATE_HELD))&&
		(strcmp(gameinfo.synopsis,"") != 0)){

			if (page==1){
			nextBtn.ResetState();
			HaltGui();
			gameinfoWindow.SetVisible(false);
			gameinfoWindow2.SetVisible(true);
			coverImg->SetPosition(15,30);

			backBtn.SetClickable(false);
			gameinfoWindow2.Append(&nextBtn);
			mainWindow->Append(&gameinfoWindow2);
			ResumeGui();
			page=2;
			}
			else {
			nextBtn.ResetState();
			HaltGui();
			backBtn.SetClickable(true);
			gameinfoWindow2.SetVisible(false);
			gameinfoWindow.SetVisible(true);
			gameinfoWindow.Append(&backBtn);
			gameinfoWindow.Append(&nextBtn);
			mainWindow->Remove(&gameinfoWindow2);
			ResumeGui();
			page=1;
			}
			nextBtn.ResetState();
		}
	}
	if (page==1){
		gameinfoWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
		while(gameinfoWindow.GetEffect() > 0) usleep(50);
		HaltGui();
		mainWindow->Remove(&gameinfoWindow);
		mainWindow->SetState(STATE_DEFAULT);


		delete playersImgData;
		delete playersImg;

		delete wifiplayersImgData;
		delete wifiplayersImg;
		delete ratingImg;

		delete classiccontrollerImg;
		delete nunchuckImg;
		delete guitarImg;
		delete drumsImg;
		delete dancepadImg;
		delete motionplusImg;
		delete wheelImg;
		delete balanceboardImg;
		delete microphoneImg;
		delete gcImg;
		delete dialogBoxImg1;
		delete dialogBoxImg2;
		delete dialogBoxImg3;
		delete dialogBoxImg4;
		delete dialogBoxImg11;
		delete dialogBoxImg22;
		delete dialogBoxImg33;
		delete dialogBoxImg44;
		delete coverImg;
		delete coverImg2;

		delete classiccontrollerImgData;
		delete nunchuckImgData;
		delete guitarImgData;
		delete drumsImgData;
		delete motionplusImgData;
		delete wheelImgData;
		delete balanceboardImgData;
		delete dancepadImgData;
		delete microphoneImgData;
		delete gamecubeImgData;
		delete ratingImgData;
		delete cover;

		delete releasedTxt;
		delete publisherTxt;
		delete developerTxt;
		delete titleTxt;
		delete synopsisTxt;
		delete genreTxt;
		delete betaTxt;
		delete beta1Txt;
		if (gameinfo.wifiCnt>0){
			for(int i=1; i<=gameinfo.wifiCnt; i++)
			{
				delete wifiTxt[i];
			}
		}
		FreeXMLMemory();
		ResumeGui();
	} else {
		gameinfoWindow2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
		while(gameinfoWindow2.GetEffect() > 0) usleep(50);
		HaltGui();
		mainWindow->Remove(&gameinfoWindow2);
		mainWindow->SetState(STATE_DEFAULT);
		FreeXMLMemory();
		ResumeGui();
	}
	return choice;

    /* File not found */
    } else {
        return -1;
    }
}

