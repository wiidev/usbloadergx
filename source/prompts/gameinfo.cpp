#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "usbloader/wbfs.h"
#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "../xml/xml.h"
#include "menu.h"
#include "filelist.h"
#include "sys.h"
#include "wpad.h"
#include "fatmounter.h"
#include "listfiles.h"
#include "prompts/PromptWindows.h"
#include "gameinfo.h"
#include "usbloader/getentries.h"


/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern u8 shutdown;
extern u8 reset;
extern struct gameXMLinfo gameinfo;
extern struct gameXMLinfo gameinfo_reset;
extern u32 gameCnt;
extern struct discHdr * gameList;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();


/****************************************************************************
* gameinfo
***************************************************************************/
int showGameInfo(char *ID) {
    HaltGui();//put this first to try to get rid of the code dump caused by loading this window at the same time as loading images from the SD card
    mainWindow->SetState(STATE_DISABLED);
    ResumeGui();

    bool databaseopened = true;
    if (databaseopened) {

        LoadGameInfoFromXML(ID, Settings.db_language);

        bool showmeminfo = false;

        int choice=-1;
        int titley=10;
        int marginY=titley+40;
        int indexy = marginY;
        int wifiY=0;
        int intputX=200, inputY=-30, txtXOffset=90;
        u8 nunchuk=0,
			classiccontroller=0,
            balanceboard=0,
            dancepad=0,
            guitar=0,
            gamecube=0,
            wheel=0,
            motionplus=0,
            drums=0,
			microphone=0,
			zapper=0,
			nintendods=0,
			//vitalitysensor=0,
			wiispeak=0;
        int newline=1;
        u8 page=1;

        GuiImageData * playersImgData = NULL;
        GuiImage * playersImg = NULL;

        GuiImageData * wifiplayersImgData = NULL;
        GuiImage * wifiplayersImg = NULL;
        GuiImage * ratingImg = NULL;

        GuiImage * classiccontrollerImg = NULL;
        GuiImage * nunchukImg = NULL;
        GuiImage * guitarImg = NULL;
        GuiImage * drumsImg = NULL;
        GuiImage * dancepadImg = NULL;
        GuiImage * motionplusImg = NULL;
        GuiImage * wheelImg = NULL;
        GuiImage * balanceboardImg = NULL;
        GuiImage * microphoneImg = NULL;
        GuiImage * zapperImg = NULL;
        GuiImage * nintendodsImg = NULL;
        GuiImage * wiispeakImg = NULL;
        //GuiImage * vitalitysensorImg = NULL;
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
        GuiImageData * nunchukImgData = NULL;
        GuiImageData * guitarImgData = NULL;
        GuiImageData * drumsImgData = NULL;
        GuiImageData * motionplusImgData = NULL;
        GuiImageData * wheelImgData = NULL;
        GuiImageData * balanceboardImgData = NULL;
        GuiImageData * dancepadImgData = NULL;
        GuiImageData * microphoneImgData = NULL;
        GuiImageData * zapperImgData = NULL;
        GuiImageData * nintendodsImgData = NULL;
        GuiImageData * wiispeakImgData = NULL;
        //GuiImageData * vitalitysensorImgData = NULL;
        GuiImageData * gamecubeImgData = NULL;
        GuiImageData * ratingImgData = NULL;
        GuiImageData * cover = NULL;

        GuiText * releasedTxt = NULL;
        GuiText * publisherTxt = NULL;
        GuiText * developerTxt = NULL;
        GuiText * titleTxt = NULL;
        GuiText * synopsisTxt = NULL;
        GuiText ** genreTxt = NULL;
        GuiText ** wifiTxt = NULL;
        GuiText * wiitdb1Txt = NULL;
        GuiText * wiitdb2Txt = NULL;
        GuiText * wiitdb3Txt = NULL;
        GuiText * memTxt = NULL;

        GuiWindow gameinfoWindow(600,308);
        gameinfoWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        gameinfoWindow.SetPosition(0, -50);

        GuiWindow gameinfoWindow2(600,308);
        gameinfoWindow2.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
        gameinfoWindow2.SetPosition(0, -50);

        GuiWindow txtWindow(350,270);
        txtWindow.SetAlignment(ALIGN_CENTRE, ALIGN_RIGHT);
        txtWindow.SetPosition(95, 55);

        GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
        GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
        char imgPath[100];
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

        GuiTrigger trig1;
        trig1.SetButtonOnlyTrigger(-1, WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_X, 0);
        GuiTrigger trigA;
        trigA.SetButtonOnlyTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
        GuiTrigger trigB;
        trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
        GuiTrigger trigU;
        trigU.SetButtonOnlyTrigger(-1, WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP, PAD_BUTTON_UP);
        GuiTrigger trigD;
        trigD.SetButtonOnlyTrigger(-1, WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN, PAD_BUTTON_DOWN);
        GuiTrigger trigH;
        trigH.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

        //buttons for changing between synopsis and other info
        GuiButton backBtn(0,0);
        backBtn.SetPosition(-20,-20);
        backBtn.SetTrigger(&trigB);
        gameinfoWindow.Append(&backBtn);

        GuiButton nextBtn(0,0);
        nextBtn.SetPosition(20,20);
        nextBtn.SetTrigger(&trigA);
        gameinfoWindow.Append(&nextBtn);

        //buttons for scrolling the synopsis
        GuiButton upBtn(0,0);
        upBtn.SetPosition(0,0);
        upBtn.SetTrigger(&trigU);

        GuiButton dnBtn(0,0);
        dnBtn.SetPosition(0,0);
        dnBtn.SetTrigger(&trigD);

        GuiButton homeBtn(0,0);
        homeBtn.SetPosition(0,0);
        homeBtn.SetTrigger(&trigH);

        // button to save the url for the zip file for poor people without wifi
        GuiButton urlBtn(0,0);
        urlBtn.SetPosition(0,0);
        urlBtn.SetTrigger(&trig1);
        gameinfoWindow.Append(&urlBtn);

        char linebuf[XML_SYNOPSISLEN] = "";
        char linebuf2[100] = "";

        // enable icons for required accessories
        for (int i=1;i<=XML_ELEMMAX;i++) {
            if (strcmp(gameinfo.accessoriesReq[i],"classiccontroller")==0)
                classiccontroller=1;
            if (strcmp(gameinfo.accessoriesReq[i],"nunchuk")==0)
                nunchuk=1;
            if (strcmp(gameinfo.accessoriesReq[i],"guitar")==0)
                guitar=1;
            if (strcmp(gameinfo.accessoriesReq[i],"drums")==0)
                drums=1;
            if (strcmp(gameinfo.accessoriesReq[i],"dancepad")==0)
                dancepad=1;
            if (strcmp(gameinfo.accessoriesReq[i],"motionplus")==0)
                motionplus=1;
            if (strcmp(gameinfo.accessoriesReq[i],"wheel")==0)
                wheel=1;
            if (strcmp(gameinfo.accessoriesReq[i],"balanceboard")==0)
                balanceboard=1;
			if (strcmp(gameinfo.accessoriesReq[i],"microphone")==0)
                microphone=1;
			if (strcmp(gameinfo.accessoriesReq[i],"zapper")==0)
                zapper=1;
			if (strcmp(gameinfo.accessoriesReq[i],"nintendods")==0)
                nintendods=1;
			if (strcmp(gameinfo.accessoriesReq[i],"wiispeak")==0)
                wiispeak=1;
			//if (strcmp(gameinfo.accessoriesReq[i],"vitalitysensor")==0)
            //   vitalitysensor=1;
            if (strcmp(gameinfo.accessoriesReq[i],"gamecube")==0)
                gamecube=1;
        }

        // switch icons
        if (nunchuk) nunchukImgData = new GuiImageData(nunchukR_png);
        else nunchukImgData = new GuiImageData(nunchuk_png);

        if (classiccontroller) classiccontrollerImgData = new GuiImageData(classiccontrollerR_png);
        else classiccontrollerImgData = new GuiImageData(classiccontroller_png);

        if (guitar) guitarImgData = new GuiImageData(guitarR_png);
        else guitarImgData = new GuiImageData(guitar_png);

        if (gamecube) gamecubeImgData = new GuiImageData(gcncontrollerR_png);
        else gamecubeImgData = new GuiImageData(gcncontroller_png);

        if (wheel) wheelImgData = new GuiImageData(wheelR_png);
        else wheelImgData = new GuiImageData(wheel_png);

        if (motionplus) motionplusImgData = new GuiImageData(motionplusR_png);
        else motionplusImgData = new GuiImageData(motionplus_png);

        if (drums) drumsImgData = new GuiImageData(drumsR_png);
        else drumsImgData = new GuiImageData(drums_png);

        if (microphone) microphoneImgData = new GuiImageData(microphoneR_png);
        else microphoneImgData = new GuiImageData(microphone_png);

        if (zapper) zapperImgData = new GuiImageData(zapperR_png);
        else zapperImgData = new GuiImageData(zapper_png);

		if (wiispeak) wiispeakImgData = new GuiImageData(wiispeakR_png);
        else wiispeakImgData = new GuiImageData(wiispeak_png);

		if (nintendods) nintendodsImgData = new GuiImageData(nintendodsR_png);
        else nintendodsImgData = new GuiImageData(nintendods_png);

		//if (vitalitysensor) vitalitysensorImgData = new GuiImageData(vitalitysensorR_png);
        //else vitalitysensorImgData = new GuiImageData(vitalitysensor_png);

        if (balanceboard) balanceboardImgData = new GuiImageData(balanceboardR_png);
        else balanceboardImgData = new GuiImageData(balanceboard_png);

        if (dancepad) dancepadImgData = new GuiImageData(dancepadR_png);
        else dancepadImgData = new GuiImageData(dancepad_png);

        // look for optional accessories
        for (int i=1;i<=XML_ELEMMAX;i++) {
            if (strcmp(gameinfo.accessories[i],"classiccontroller")==0)
                classiccontroller=1;
            if (strcmp(gameinfo.accessories[i],"nunchuk")==0)
                nunchuk=1;
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
			if (strcmp(gameinfo.accessories[i],"zapper")==0)
                zapper=1;
			if (strcmp(gameinfo.accessories[i],"nintendods")==0)
                nintendods=1;
			if (strcmp(gameinfo.accessories[i],"wiispeak")==0)
                wiispeak=1;
			//if (strcmp(gameinfo.accessories[i],"vitalitysensor")==0)
            //    vitalitysensor=1;
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
        if (!cover->GetImage()) {
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
            if (atoi(gameinfo.players)>1) {
                playersImgData= new GuiImageData(Wiimote2_png);
            }
            if (atoi(gameinfo.players)>2) {
                playersImgData= new GuiImageData(Wiimote4_png);
            }

            playersImg = new GuiImage(playersImgData);
            playersImg->SetWidescreen(CFG.widescreen);
            playersImg->SetPosition(intputX , inputY);
            playersImg->SetAlignment(0,4);
            gameinfoWindow.Append(playersImg);
            intputX += (CFG.widescreen ? playersImg->GetWidth() * .8 : playersImg->GetWidth())+5;
        }

        //draw the input types for this game
        if (motionplus==1) {
            motionplusImg = new GuiImage(motionplusImgData);
            motionplusImg->SetWidescreen(CFG.widescreen);
            motionplusImg->SetPosition(intputX , inputY);
            motionplusImg->SetAlignment(0,4);
            gameinfoWindow.Append(motionplusImg);
            intputX += (CFG.widescreen ? motionplusImg->GetWidth() * .8 : motionplusImg->GetWidth())+5;
        }
        if (nunchuk==1) {
            nunchukImg = new GuiImage(nunchukImgData);
            nunchukImg->SetWidescreen(CFG.widescreen);
            nunchukImg->SetPosition(intputX , inputY);
            nunchukImg->SetAlignment(0,4);
            gameinfoWindow.Append(nunchukImg);
            intputX += (CFG.widescreen ? nunchukImg->GetWidth() * .8 : nunchukImg->GetWidth())+5;
        }
        if (classiccontroller==1) {
            classiccontrollerImg = new GuiImage(classiccontrollerImgData);
            classiccontrollerImg->SetWidescreen(CFG.widescreen);
            classiccontrollerImg->SetPosition(intputX , inputY);
            classiccontrollerImg->SetAlignment(0,4);
            gameinfoWindow.Append(classiccontrollerImg);
            intputX += (CFG.widescreen ? classiccontrollerImg->GetWidth() * .8 : classiccontrollerImg->GetWidth())+5;
        }
        if (gamecube==1) {
            gcImg = new GuiImage(gamecubeImgData);
            gcImg->SetWidescreen(CFG.widescreen);
            gcImg->SetPosition(intputX , inputY);
            gcImg->SetAlignment(0,4);
            gameinfoWindow.Append(gcImg);
            intputX += (CFG.widescreen ? gcImg->GetWidth() * .8 : gcImg->GetWidth())+5;
        }
        if (wheel==1) {
            wheelImg = new GuiImage(wheelImgData);
            wheelImg->SetWidescreen(CFG.widescreen);
            wheelImg->SetPosition(intputX , inputY);
            wheelImg->SetAlignment(0,4);
            gameinfoWindow.Append(wheelImg);
            intputX += (CFG.widescreen ? wheelImg->GetWidth() * .8 : wheelImg->GetWidth())+5;
        }
        if (guitar==1) {
            guitarImg = new GuiImage(guitarImgData);
            guitarImg->SetWidescreen(CFG.widescreen);
            guitarImg->SetPosition(intputX , inputY);
            guitarImg->SetAlignment(0,4);
            gameinfoWindow.Append(guitarImg);
            intputX += (CFG.widescreen ? guitarImg->GetWidth() * .8 : guitarImg->GetWidth())+5;
        }
        if (drums==1) {
            drumsImg = new GuiImage(drumsImgData);
            drumsImg->SetWidescreen(CFG.widescreen);
            drumsImg->SetPosition(intputX , inputY);
            drumsImg->SetAlignment(0,4);
            gameinfoWindow.Append(drumsImg);
            intputX += (CFG.widescreen ? drumsImg->GetWidth() * .8 : drumsImg->GetWidth())+5;
        }
        if (microphone==1) {
            microphoneImg = new GuiImage(microphoneImgData);
            microphoneImg->SetWidescreen(CFG.widescreen);
            microphoneImg->SetPosition(intputX , inputY);
            microphoneImg->SetAlignment(0,4);
            gameinfoWindow.Append(microphoneImg);
            intputX += (CFG.widescreen ? microphoneImg->GetWidth() * .8 : microphoneImg->GetWidth())+5;
        }
		if (zapper==1) {
            zapperImg = new GuiImage( zapperImgData);
            zapperImg->SetWidescreen(CFG.widescreen);
            zapperImg->SetPosition(intputX , inputY);
            zapperImg->SetAlignment(0,4);
            gameinfoWindow.Append(zapperImg);
            intputX += (CFG.widescreen ? zapperImg->GetWidth() * .8 : zapperImg->GetWidth())+5;
        }
		if (wiispeak==1) {
            wiispeakImg = new GuiImage(wiispeakImgData);
            wiispeakImg->SetWidescreen(CFG.widescreen);
            wiispeakImg->SetPosition(intputX , inputY);
            wiispeakImg->SetAlignment(0,4);
            gameinfoWindow.Append(wiispeakImg);
            intputX += (CFG.widescreen ? wiispeakImg->GetWidth() * .8 : wiispeakImg->GetWidth())+5;
        }
		if (nintendods==1) {
            nintendodsImg = new GuiImage(nintendodsImgData);
            nintendodsImg->SetWidescreen(CFG.widescreen);
            nintendodsImg->SetPosition(intputX , inputY);
            nintendodsImg->SetAlignment(0,4);
            gameinfoWindow.Append(nintendodsImg);
            intputX += (CFG.widescreen ? nintendodsImg->GetWidth() * .8 : nintendodsImg->GetWidth())+5;
        }
		/*
		if (vitalitysensor==1) {
            vitalitysensorImg = new GuiImage(vitalitysensorImgData);
            vitalitysensorImg->SetWidescreen(CFG.widescreen);
            vitalitysensorImg->SetPosition(intputX , inputY);
            vitalitysensorImg->SetAlignment(0,4);
            gameinfoWindow.Append(vitalitysensorImg);
            intputX += (CFG.widescreen ? vitalitysensorImg->GetWidth() * .8 : vitalitysensorImg->GetWidth())+5;
        }
		*/
        if (dancepad==1) {
            dancepadImg = new GuiImage(dancepadImgData);
            dancepadImg->SetWidescreen(CFG.widescreen);
            dancepadImg->SetPosition(intputX , inputY);
            dancepadImg->SetAlignment(0,4);
            gameinfoWindow.Append(dancepadImg);
            intputX += (CFG.widescreen ? dancepadImg->GetWidth() * .8 : dancepadImg->GetWidth())+5;
        }
        if (balanceboard==1) {
            balanceboardImg = new GuiImage(balanceboardImgData);
            balanceboardImg->SetWidescreen(CFG.widescreen);
            balanceboardImg->SetPosition(intputX , inputY);
            balanceboardImg->SetAlignment(0,4);
            gameinfoWindow.Append(balanceboardImg);
            intputX += (CFG.widescreen ? balanceboardImg->GetWidth() * .8 : balanceboardImg->GetWidth())+5;
        }

        // # online players
        if ((strcmp(gameinfo.wifiplayers,"") != 0) && (strcmp(gameinfo.wifiplayers,"0") != 0)) {
            wifiplayersImgData = new GuiImageData(wifi1_png);
            if (atoi(gameinfo.wifiplayers)>1) {
                wifiplayersImgData= new GuiImageData(wifi2_png);
            }
            if (atoi(gameinfo.wifiplayers)>2) {
                wifiplayersImgData= new GuiImageData(wifi4_png);
            }
            if (atoi(gameinfo.wifiplayers)>4) {
                //wifiplayersImgData= new GuiImageData(wifi6_png);
                wifiplayersImgData= new GuiImageData(wifi8_png);
            }
			/*
			if (atoi(gameinfo.wifiplayers)>6) {
                wifiplayersImgData= new GuiImageData(wifi8_png);
            }
			*/
            if (atoi(gameinfo.wifiplayers)>8) {
            	wifiplayersImgData= new GuiImageData(wifi12_png);
			}
            if (atoi(gameinfo.wifiplayers)>12) {
            	wifiplayersImgData= new GuiImageData(wifi16_png);
			}
            if (atoi(gameinfo.wifiplayers)>16) {
            	wifiplayersImgData= new GuiImageData(wifi32_png);
			}
            wifiplayersImg = new GuiImage(wifiplayersImgData);
            wifiplayersImg->SetWidescreen(CFG.widescreen);
            wifiplayersImg->SetPosition(intputX , inputY);
            wifiplayersImg->SetAlignment(0,4);
            gameinfoWindow.Append(wifiplayersImg);
            intputX += (CFG.widescreen ? wifiplayersImg->GetWidth() * .8 : wifiplayersImg->GetWidth())+5;
        }

        // ratings
        if (strcmp(gameinfo.ratingtype,"") !=0) {
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
                else {
                    ratingImgData = new GuiImageData(norating_png);
                }
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
                else {
                    ratingImgData = new GuiImageData(norating_png);
                }
            } else if (strcmp(gameinfo.ratingtype,"CERO")==0) {
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
                else {
                    ratingImgData = new GuiImageData(norating_png);
                }
            }

            else {
                ratingImgData = new GuiImageData(norating_png);
            }
            ratingImg = new GuiImage(ratingImgData);
            ratingImg->SetWidescreen(CFG.widescreen);
            ratingImg->SetPosition(-25 , inputY);
            ratingImg->SetAlignment(1,4);
            gameinfoWindow.Append(ratingImg);
            intputX += (CFG.widescreen ? ratingImg->GetWidth() * .8 : ratingImg->GetWidth())+5;
        }

        // memory info
        if (showmeminfo) {
            char meminfotxt[200];
            strlcpy(meminfotxt,MemInfo(),sizeof(meminfotxt));
            snprintf(linebuf, sizeof(linebuf), "%s",meminfotxt);
            memTxt = new GuiText(linebuf, 18, (GXColor) {0,0,0, 255});
            memTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            memTxt->SetPosition(0,0);
            gameinfoWindow.Append(memTxt);
        }

        // title
        int titlefontsize=25;
        if (strcmp(gameinfo.title,"") != 0) {
            snprintf(linebuf, sizeof(linebuf), "%s",gameinfo.title);
            titleTxt = new GuiText(linebuf, titlefontsize, (GXColor) {0,0,0, 255});
            titleTxt->SetMaxWidth(350, GuiText::SCROLL);
            //while (titleTxt->GetWidth()>250) { titleTxt->SetFontSize(titlefontsize-=2); }
            titleTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
            titleTxt->SetPosition(txtXOffset,12+titley);
            gameinfoWindow.Append(titleTxt);
        }

        //date
        snprintf(linebuf2, sizeof(linebuf2), " ");
        if (strcmp(gameinfo.day,"") != 0) {
            snprintf(linebuf2, sizeof(linebuf2), "%s ", gameinfo.day);
        }
        if (strcmp(gameinfo.month,"") != 0) {
            switch (atoi(gameinfo.month)) {
            case 1:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("Jan"));
                break;
            case 2:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("Feb"));
                break;
            case 3:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("Mar"));
                break;
            case 4:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("Apr"));
                break;
            case 5:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("May"));
                break;
            case 6:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("June"));
                break;
            case 7:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("July"));
                break;
            case 8:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("Aug"));
                break;
            case 9:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("Sept"));
                break;
            case 10:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("Oct"));
                break;
            case 11:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("Nov"));
                break;
            case 12:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr("Dec"));
                break;
            }
        }
        if (strcmp(gameinfo.year,"") != 0) {
            snprintf(linebuf, sizeof(linebuf), "%s : %s%s", tr("Released"), linebuf2, gameinfo.year);
            releasedTxt = new GuiText(linebuf, 16, (GXColor) {0,0,0, 255});
            if (releasedTxt->GetWidth()>300) newline=2;
            releasedTxt->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
            releasedTxt->SetPosition(-17,12+indexy);
            indexy+=(20 * newline);
            newline=1;
            gameinfoWindow.Append(releasedTxt);
        }

        //publisher
        if (strcmp(gameinfo.publisher,"") != 0) {
            snprintf(linebuf, sizeof(linebuf), "%s %s", tr("Published by"), gameinfo.publisher);
            publisherTxt = new GuiText(linebuf, 16, (GXColor) {0,0,0, 255});
            if (publisherTxt->GetWidth()>250) newline=2;
            publisherTxt->SetMaxWidth(250,GuiText::WRAP);
            publisherTxt->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
            publisherTxt->SetPosition(-17,12+indexy);
            indexy+=(20 * newline);
            newline=1;
            gameinfoWindow.Append(publisherTxt);
        }

        //developer
        if (strcmp(gameinfo.developer,"") != 0 && strcasecmp(gameinfo.developer,gameinfo.publisher) != 0)	{
            snprintf(linebuf, sizeof(linebuf), "%s %s", tr("Developed by"), gameinfo.developer);
            developerTxt = new GuiText(linebuf, 16, (GXColor) {0,0,0, 255});
            if (developerTxt->GetWidth()>250) newline=2;
            developerTxt->SetMaxWidth(250,GuiText::WRAP);
            developerTxt->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
            developerTxt->SetPosition(-17,12+indexy);
            indexy+=(20 * newline);
            newline=1;
            gameinfoWindow.Append(developerTxt);
        }

        //genre
        int genreY = marginY;
        genreTxt = new GuiText * [gameinfo.genreCnt + 1];
        for (int i=1;i<=gameinfo.genreCnt;i++) {
            snprintf(linebuf, sizeof(linebuf), "%s", gameinfo.genresplit[i]);
            genreTxt[i] = new GuiText(linebuf, 16, (GXColor) {0,0,0, 255});
            genreTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            genreTxt[i]->SetPosition(205,12+genreY);
            genreY+=20;
            gameinfoWindow.Append(genreTxt[i]);
        }

        //online
        wifiTxt = new GuiText * [gameinfo.wifiCnt + 1];
        for (int i=gameinfo.wifiCnt;i>=1;i--) {
            if (strcmp(gameinfo.wififeatures[i],"Nintendods") == 0) {
                snprintf(linebuf, sizeof(linebuf), "Nintendo DS");
            } else {
                snprintf(linebuf, sizeof(linebuf), "%s",gameinfo.wififeatures[i]);
            }
            wifiTxt[i] = new GuiText(linebuf, 16, (GXColor) {0,0,0, 255});
            wifiTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            wifiTxt[i]->SetPosition(215,200+wifiY);
            wifiY-=20;
            gameinfoWindow.Append(wifiTxt[i]);
        }
        if (strcmp(gameinfo.wififeatures[1],"") !=0) {
            snprintf(linebuf, sizeof(linebuf), "%s:",tr("WiFi Features"));
		} else {
            strcpy(linebuf,"");
        }		
		wifiTxt[0] = new GuiText(linebuf, 16, (GXColor) {0,0,0, 255});
        wifiTxt[0]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        wifiTxt[0]->SetPosition(205,200+wifiY);
        gameinfoWindow.Append(wifiTxt[0]);

        //synopsis
        int pagesize=12;
        if (strcmp(gameinfo.synopsis,"") !=0)	{
            snprintf(linebuf, sizeof(linebuf), "%s", gameinfo.synopsis);
            synopsisTxt = new GuiText(linebuf, 16, (GXColor) {0,0,0, 255});
            synopsisTxt->SetMaxWidth(350,GuiText::WRAP);
            synopsisTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            synopsisTxt->SetPosition(0,0);
            synopsisTxt->SetNumLines(pagesize);
            //synopsisTxt->SetFirstLine(12);

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
            txtWindow.Append(&upBtn);
            txtWindow.Append(&dnBtn);
            coverImg2 = new GuiImage(cover);
            coverImg2->SetWidescreen(CFG.widescreen);
            coverImg2->SetPosition(15,30);
            gameinfoWindow2.Append(coverImg2);
            gameinfoWindow2.Append(&txtWindow);
        }

        snprintf(linebuf, sizeof(linebuf), "http://wiitdb.com");
        //snprintf(linebuf, sizeof(linebuf), tr("Don't bother the USB Loader GX Team about errors in this file."));
        wiitdb1Txt = new GuiText(linebuf, 16, (GXColor) {0,0,0, 255});
        wiitdb1Txt->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        wiitdb1Txt->SetPosition(40,-15);
        gameinfoWindow.Append(wiitdb1Txt);
        snprintf(linebuf, sizeof(linebuf), tr("If you don't have WiFi, press 1 to get an URL to get your WiiTDB.zip"));
        wiitdb2Txt = new GuiText(linebuf, 14, (GXColor) {0,0,0, 255});
        wiitdb2Txt->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        wiitdb2Txt->SetPosition(202,-15);
        gameinfoWindow.Append(wiitdb2Txt);
		snprintf(linebuf, sizeof(linebuf), " ");
        wiitdb3Txt = new GuiText(linebuf, 14, (GXColor) {0,0,0, 255});
        wiitdb3Txt->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        wiitdb3Txt->SetPosition(202,-4);
        gameinfoWindow.Append(wiitdb3Txt);

        gameinfoWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 100);
        HaltGui();
        //mainWindow->SetState(STATE_DISABLED);
        mainWindow->Append(&gameinfoWindow);
        mainWindow->ChangeFocus(&gameinfoWindow);
        ResumeGui();

		bool savedURL = false;

        while (choice == -1) {

            VIDEO_WaitVSync();
            if (shutdown == 1) {
                wiilight(0);
                Sys_Shutdown();
            } else if (reset == 1)
                Sys_Reboot();

            else if ((backBtn.GetState()==STATE_CLICKED)||(backBtn.GetState()==STATE_HELD)) {
				backBtn.ResetState();
                if (page==1) {
                    choice=1;
                    synopsisTxt = NULL;
                    break;
                } else if (page==2) {
                    HaltGui();
					gameinfoWindow2.Remove(&nextBtn);
                    gameinfoWindow2.Remove(&backBtn);
                    gameinfoWindow2.Remove(&homeBtn);
                    gameinfoWindow2.SetVisible(false);
                    gameinfoWindow.SetVisible(true);
					gameinfoWindow.Append(&backBtn);
                    gameinfoWindow.Append(&nextBtn);
                    gameinfoWindow.Append(&homeBtn);
                    mainWindow->Remove(&gameinfoWindow2);
                    ResumeGui();
                    page=1;
                }
            } else if (((nextBtn.GetState()==STATE_CLICKED)||(nextBtn.GetState()==STATE_HELD))&&
                       (strcmp(gameinfo.synopsis,"") != 0)) {
                nextBtn.ResetState();
                if (page==1) {
                    HaltGui();
					gameinfoWindow.Remove(&nextBtn);
                    gameinfoWindow.Remove(&backBtn);
                    gameinfoWindow.Remove(&homeBtn);
                    gameinfoWindow.SetVisible(false);
                    gameinfoWindow2.SetVisible(true);
                    coverImg->SetPosition(15,30);
                    gameinfoWindow2.Append(&nextBtn);
                    gameinfoWindow2.Append(&backBtn);
                    gameinfoWindow2.Append(&homeBtn);
                    mainWindow->Append(&gameinfoWindow2);
                    ResumeGui();
                    page=2;
                } else {
                    HaltGui();
					gameinfoWindow2.Remove(&nextBtn);
                    gameinfoWindow2.Remove(&backBtn);
                    gameinfoWindow2.Remove(&homeBtn);
                    gameinfoWindow2.SetVisible(false);
                    gameinfoWindow.SetVisible(true);
                    gameinfoWindow.Append(&backBtn);
                    gameinfoWindow.Append(&nextBtn);
                    gameinfoWindow.Append(&homeBtn);
                    mainWindow->Remove(&gameinfoWindow2);
                    ResumeGui();
                    page=1;
                }

            } else if ((upBtn.GetState()==STATE_CLICKED||upBtn.GetState()==STATE_HELD) && page==2) {
                //int l=synopsisTxt->GetFirstLine()-1;
                if (synopsisTxt->GetFirstLine()>1)
                    synopsisTxt->SetFirstLine(synopsisTxt->GetFirstLine()-1);
                usleep(60000);
                if (!((ButtonsHold() & WPAD_BUTTON_UP)||(ButtonsHold() & PAD_BUTTON_UP)))
                    upBtn.ResetState();
            } else if ((dnBtn.GetState()==STATE_CLICKED||dnBtn.GetState()==STATE_HELD) && page==2
                       &&synopsisTxt->GetTotalLines()>pagesize
                       &&synopsisTxt->GetFirstLine()-1<synopsisTxt->GetTotalLines()-pagesize) {
                int l=0;
                //if(synopsisTxt->GetTotalLines()>pagesize)
                l=synopsisTxt->GetFirstLine()+1;

                //if (l>(synopsisTxt->GetTotalLines()+1)-pagesize)
                //l=(synopsisTxt->GetTotalLines()+1)-pagesize;

                synopsisTxt->SetFirstLine(l);
                usleep(60000);
                if (!((ButtonsHold() & WPAD_BUTTON_DOWN)||(ButtonsHold() & PAD_BUTTON_DOWN)))
                    dnBtn.ResetState();
            } else if (homeBtn.GetState()==STATE_CLICKED) {
                if (page==1) {
                    choice=2;
                    synopsisTxt = NULL;
                    break;
                } else if (page==2) {
                    HaltGui();
                    gameinfoWindow2.SetVisible(false);
                    gameinfoWindow.SetVisible(true);
                    mainWindow->Remove(&gameinfoWindow2);
                    ResumeGui();
                    page=1;
                }
            } else if (urlBtn.GetState()==STATE_CLICKED && !savedURL) {
			    snprintf(linebuf, sizeof(linebuf), tr("Please wait..."));
				wiitdb2Txt->SetText(linebuf);
                gameinfoWindow.Append(wiitdb2Txt);
                if (save_XML_URL()) {
                    snprintf(linebuf, sizeof(linebuf), tr("Your URL has been saved in %sWiiTDB_URL.txt."), Settings.update_path);
                    wiitdb2Txt->SetText(linebuf);
                    gameinfoWindow.Append(wiitdb2Txt);
                    snprintf(linebuf, sizeof(linebuf), tr("Paste it into your browser to get your WiiTDB.zip."));
                    wiitdb3Txt->SetText(linebuf);
                    gameinfoWindow.Append(wiitdb3Txt);
					savedURL = true;
                } else {
					snprintf(linebuf, sizeof(linebuf), tr("Could not save."));
					wiitdb2Txt->SetText(linebuf);
					gameinfoWindow.Append(wiitdb2Txt);
				}
				urlBtn.ResetState();
            }
        }
        if (page==1) {
            gameinfoWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 100);
            while (gameinfoWindow.GetEffect() > 0) usleep(50);
            HaltGui();
            mainWindow->Remove(&gameinfoWindow);
            mainWindow->SetState(STATE_DEFAULT);

            delete playersImgData;
            delete playersImg;

            delete wifiplayersImgData;
            delete wifiplayersImg;
            delete ratingImg;

            delete classiccontrollerImg;
            delete nunchukImg;
            delete guitarImg;
            delete drumsImg;
            delete dancepadImg;
            delete motionplusImg;
            delete wheelImg;
            delete balanceboardImg;
            delete microphoneImg;
            delete zapperImg;
            delete wiispeakImg;
            delete nintendodsImg;
            //delete vitalitysensorImg;
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
            delete nunchukImgData;
            delete guitarImgData;
            delete drumsImgData;
            delete motionplusImgData;
            delete wheelImgData;
            delete balanceboardImgData;
            delete dancepadImgData;
            delete microphoneImgData;
            delete zapperImgData;
            delete wiispeakImgData;
            delete nintendodsImgData;
            //delete vitalitysensorImgData;
            delete gamecubeImgData;
            delete ratingImgData;
            delete cover;
            delete releasedTxt;
            delete publisherTxt;
            delete developerTxt;
            delete titleTxt;
            delete synopsisTxt;
            delete wiitdb1Txt;
            delete wiitdb2Txt;
            delete wiitdb3Txt;
            delete memTxt;
            if (gameinfo.genreCnt>0) {
                for (int i=1;i<=gameinfo.genreCnt;i++) {
                    delete genreTxt[i];
                }
            }
            if (gameinfo.wifiCnt>0) {
                for (int i=0;i<=gameinfo.wifiCnt;i++) {
                    delete wifiTxt[i];
                }
            }
            ResumeGui();
        } else {
            gameinfoWindow2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 100);
            while (gameinfoWindow2.GetEffect() > 0) usleep(50);
            HaltGui();
            mainWindow->Remove(&gameinfoWindow2);
            mainWindow->SetState(STATE_DEFAULT);
            ResumeGui();
        }

		if (savedURL) return 3;
        return choice;

    /* File not found */
    } else {
        return -1;
    }
}

bool save_gamelist(int txt) { // save gamelist
	mainWindow->SetState(STATE_DISABLED);
    char tmp[200];
    sprintf(tmp, "%s", Settings.update_path);
    struct stat st;
    if (stat(tmp, &st) != 0) {
        mkdir(tmp, 0777);
    }
    FILE *f;
    sprintf(tmp, "%sGameList.txt", Settings.update_path);
	if (txt==1)
	sprintf(tmp, "%sGameList.csv", Settings.update_path);
    f = fopen(tmp, "w");
    if (!f) {
        sleep(1);
		mainWindow->SetState(STATE_DEFAULT);
        return false;
    }
    //make sure that all games are added to the gamelist
    __Menu_GetEntries(1);

    f32 size = 0.0;
	f32 freespace, used;
	unsigned int i;

	WBFS_DiskSpace(&used, &freespace);

	fprintf(f, "# USB Loader Has Saved this file\n");
    fprintf(f, "# This file was created based on your list of games and language settings.\n");
    fclose(f);
    /* Closing and reopening because of a write issue we are having right now */
    f = fopen(tmp, "w");

	if (txt==0) 	{
		fprintf(f, "# USB Loader Has Saved this file\n");
		fprintf(f, "# This file was created based on your list of games and language settings.\n\n");

		fprintf(f, "%.2fGB %s %.2fGB %s\n\n",freespace,tr("of"),(freespace+used),tr("free"));
		fprintf(f, "ID     Size(GB)  Name\n");

		for (i = 0; i < gameCnt ; i++) {
			struct discHdr* header = &gameList[i];
			WBFS_GameSize(header->id, &size);
			if (i<500) {
				fprintf(f, "%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2], header->id[3], header->id[4], header->id[5]);
				fprintf(f, " [%.2f]   ", size);
				fprintf(f, " %s",get_title(header));
			}
			fprintf(f, "\n");
		}
	} else {

	fprintf(f, "\"ID\",\"Size(GB)\",\"Name\"\n");

		for (i = 0; i < gameCnt ; i++) {
			struct discHdr* header = &gameList[i];
			WBFS_GameSize(header->id, &size);
			if (i<500) {
				fprintf(f, "\"%c%c%c%c%c%c\",\"%.2f\",\"%s\"\n", header->id[0], header->id[1], header->id[2], header->id[3], header->id[4], header->id[5], size,get_title(header));
				//fprintf(f, "\"%.2f\",", size);
				//fprintf(f, "\"%s\"",get_title(header));
			}
			//fprintf(f, "\n");
		}
	}
    fclose(f);

    __Menu_GetEntries();
	mainWindow->SetState(STATE_DEFAULT);
    return true;
}


bool save_XML_URL() { // save xml url as as txt file for people without wifi
    char tmp[200];
    sprintf(tmp, "%s", Settings.update_path);
    struct stat st;
    if (stat(tmp, &st) != 0) {
        mkdir(tmp, 0777);
    }
    FILE *f;
    sprintf(tmp, "%sWiiTDB_URL.txt", Settings.update_path);
    f = fopen(tmp, "w");
    if (!f) {
        sleep(1);
        return false;
    }
	
	char XMLurl[3540];
	build_XML_URL(XMLurl,sizeof(XMLurl));
	
    fprintf(f, "# USB Loader Has Saved this file\n");
    fprintf(f, "# This URL was created based on your list of games and language settings.\n");
    fclose(f);
    // Closing and reopening because of a write issue we are having right now
    f = fopen(tmp, "w");
    fprintf(f, "# USB Loader Has Saved this file\n");
    fprintf(f, "# This URL was created based on your list of games and language settings.\n");
    fprintf(f, "# Copy and paste this URL into your web browser and you should get a zip file that will work for you.\n");
    fprintf(f, "%s\n\n\n ", XMLurl);

    fclose(f);

    return true;
}


void MemInfoPrompt()
{
	char meminfotxt[200];
    strlcpy(meminfotxt,MemInfo(),sizeof(meminfotxt));
	WindowPrompt(0,meminfotxt, tr("OK"));
}


void build_XML_URL(char *XMLurl, int XMLurlsize) {
    __Menu_GetEntries(1);
	// NET_BUFFER_SIZE in http.c needs to be set to size of XMLurl + headerformat
    char url[3540];
    char filename[10];
    snprintf(url,sizeof(url),"http://wiitdb.com/wiitdb.zip?LANG=%s&ID=", Settings.db_language);
    unsigned int i;
    for (i = 0; i < gameCnt ; i++) {
        struct discHdr* header = &gameList[i];
        if (i<500) {
			snprintf(filename,sizeof(filename),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);
            strncat(url,filename,6);
            if ((i!=gameCnt-1)&&(i<500))
                strncat(url, ",",1);
        }
    }
	strlcpy(XMLurl,url,XMLurlsize);
	__Menu_GetEntries();
}
