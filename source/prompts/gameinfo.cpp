#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "usbloader/wbfs.h"
#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "libwiigui/Text.hpp"
#include "xml/xml.h"
#include "menu.h"
#include "menu/menus.h"
#include "filelist.h"
#include "sys.h"
#include "wpad.h"
#include "FileOperations/fileops.h"
#include "prompts/PromptWindows.h"
#include "themes/CTheme.h"
#include "settings/GameTitles.h"
#include "gameinfo.h"
#include "usbloader/GameList.h"
#include "gecko.h"
#include "xml/WiiTDB.hpp"
#include "utils/ShowError.h"

/****************************************************************************
 * gameinfo
 ***************************************************************************/
int showGameInfo(char *ID)
{
    HaltGui();//put this first to try to get rid of the code dump caused by loading this window at the same time as loading images from the SD card
    mainWindow->SetState(STATE_DISABLED);
    ResumeGui();

    char xmlpath[300];
    snprintf(xmlpath, sizeof(xmlpath), "%swiitdb.xml", Settings.titlestxt_path);

    WiiTDB XML_DB;

    if(!XML_DB.OpenFile(xmlpath))
    {
        ShowError(tr("Could not open wiitdb.xml."));
        return -1;
    }

    XML_DB.SetLanguageCode(Settings.db_language);

    GameXMLInfo GameInfo;

    if(!XML_DB.GetGameXMLInfo(ID, &GameInfo))
    {
        ShowError(tr("Could not find info for this game in the wiitdb.xml."));
        return -1;
    }

    XML_DB.CloseFile();

    bool showmeminfo = false;

    int choice = -1;
    int titley = 10;
    int marginY = titley + 40;
    int indexy = marginY;
    int wifiY = 0;
    int intputX = 200, inputY = -30, txtXOffset = 90;
    u8 nunchuk = 0, classiccontroller = 0, balanceboard = 0, dancepad = 0, guitar = 0, gamecube = 0, wheel = 0,
            motionplus = 0, drums = 0, microphone = 0, zapper = 0, nintendods = 0,
            //vitalitysensor=0,
            wiispeak = 0;
    int newline = 1;
    u8 page = 1;

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
    Text * synopsisTxt = NULL;
    GuiText * genreTitleTxt = NULL;
    GuiText ** genreTxt = NULL;
    GuiText ** wifiTxt = NULL;
    GuiText * wiitdb1Txt = NULL;
    GuiText * wiitdb2Txt = NULL;
    GuiText * wiitdb3Txt = NULL;
    GuiText * memTxt = NULL;

    GuiWindow gameinfoWindow(600, 308);
    gameinfoWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    gameinfoWindow.SetPosition(0, -50);

    GuiWindow gameinfoWindow2(600, 308);
    gameinfoWindow2.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    gameinfoWindow2.SetPosition(0, -50);

    GuiWindow txtWindow(350, 270);
    txtWindow.SetAlignment(ALIGN_CENTRE, ALIGN_RIGHT);
    txtWindow.SetPosition(95, 55);

    GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
    GuiImageData dialogBox1(Resources::GetFile("gameinfo1.png"), Resources::GetFileSize("gameinfo1.png"));
    GuiImageData dialogBox2(Resources::GetFile("gameinfo1a.png"), Resources::GetFileSize("gameinfo1a.png"));
    GuiImageData dialogBox3(Resources::GetFile("gameinfo2.png"), Resources::GetFileSize("gameinfo2.png"));
    GuiImageData dialogBox4(Resources::GetFile("gameinfo2a.png"), Resources::GetFileSize("gameinfo2a.png"));

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
    GuiButton backBtn(0, 0);
    backBtn.SetPosition(-20, -20);
    backBtn.SetTrigger(&trigB);
    gameinfoWindow.Append(&backBtn);

    GuiButton nextBtn(0, 0);
    nextBtn.SetPosition(20, 20);
    nextBtn.SetTrigger(&trigA);
    gameinfoWindow.Append(&nextBtn);

    //buttons for scrolling the synopsis
    GuiButton upBtn(0, 0);
    upBtn.SetPosition(0, 0);
    upBtn.SetTrigger(&trigU);

    GuiButton dnBtn(0, 0);
    dnBtn.SetPosition(0, 0);
    dnBtn.SetTrigger(&trigD);

    GuiButton homeBtn(0, 0);
    homeBtn.SetPosition(0, 0);
    homeBtn.SetTrigger(&trigH);

    // button to save the url for the zip file for poor people without wifi
    GuiButton urlBtn(0, 0);
    urlBtn.SetPosition(0, 0);
    urlBtn.SetTrigger(&trig1);
    gameinfoWindow.Append(&urlBtn);

    char linebuf2[100] = "";

    // enable icons for required accessories
    for (u32 i = 0; i < GameInfo.AccessoirList.size(); ++i)
    {
        if(!GameInfo.AccessoirList[i].Required)
            continue;

        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "classiccontroller") == 0) classiccontroller = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "nunchuk") == 0) nunchuk = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "guitar") == 0) guitar = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "drums") == 0) drums = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "dancepad") == 0) dancepad = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "motionplus") == 0) motionplus = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "wheel") == 0) wheel = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "balanceboard") == 0) balanceboard = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "microphone") == 0) microphone = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "zapper") == 0) zapper = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "nintendods") == 0) nintendods = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "wiispeak") == 0) wiispeak = 1;
        //if (strcmp(GameInfo.AccessoirList[i].Name.c_str(),"vitalitysensor")==0)
        //   vitalitysensor=1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "gamecube") == 0) gamecube = 1;
    }

    // switch icons
    if (nunchuk)
        nunchukImgData = Resources::GetImageData("nunchukR.png");
    else nunchukImgData = Resources::GetImageData("nunchuk.png");

    if (classiccontroller)
        classiccontrollerImgData = Resources::GetImageData("classiccontrollerR.png");
    else classiccontrollerImgData = Resources::GetImageData("classiccontroller.png");

    if (guitar)
        guitarImgData = Resources::GetImageData("guitarR.png");
    else guitarImgData = Resources::GetImageData("guitar.png");

    if (gamecube)
        gamecubeImgData = Resources::GetImageData("gcncontrollerR.png");
    else gamecubeImgData = Resources::GetImageData("gcncontroller.png");

    if (wheel)
        wheelImgData = Resources::GetImageData("wheelR.png");
    else wheelImgData = Resources::GetImageData("wheel.png");

    if (motionplus)
        motionplusImgData = Resources::GetImageData("motionplusR.png");
    else motionplusImgData = Resources::GetImageData("motionplus.png");

    if (drums)
        drumsImgData = Resources::GetImageData("drumsR.png");
    else drumsImgData = Resources::GetImageData("drums.png");

    if (microphone)
        microphoneImgData = Resources::GetImageData("microphoneR.png");
    else microphoneImgData = Resources::GetImageData("microphone.png");

    if (zapper)
        zapperImgData = Resources::GetImageData("zapperR.png");
    else zapperImgData = Resources::GetImageData("zapper.png");

    if (wiispeak)
        wiispeakImgData = Resources::GetImageData("wiispeakR.png");
    else wiispeakImgData = Resources::GetImageData("wiispeak.png");

    if (nintendods)
        nintendodsImgData = Resources::GetImageData("nintendodsR.png");
    else nintendodsImgData = Resources::GetImageData("nintendods.png");

    if (balanceboard)
        balanceboardImgData = Resources::GetImageData("balanceboardR.png");
    else balanceboardImgData = Resources::GetImageData("balanceboard.png");

    if (dancepad)
        dancepadImgData = Resources::GetImageData("dancepadR.png");
    else dancepadImgData = Resources::GetImageData("dancepad.png");

    // look for optional accessories
    for (u32 i = 0; i < GameInfo.AccessoirList.size(); ++i)
    {
        if(GameInfo.AccessoirList[i].Required)
            continue;

        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "classiccontroller") == 0) classiccontroller = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "nunchuk") == 0) nunchuk = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "guitar") == 0) guitar = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "drums") == 0) drums = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "dancepad") == 0) dancepad = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "motionplus") == 0) motionplus = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "wheel") == 0) wheel = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "balanceboard") == 0) balanceboard = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "microphone") == 0) microphone = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "zapper") == 0) zapper = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "nintendods") == 0) nintendods = 1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "wiispeak") == 0) wiispeak = 1;
        //if (strcmp(GameInfo.AccessoirList[i].Name.c_str(),"vitalitysensor")==0)
        //    vitalitysensor=1;
        if (strcmp(GameInfo.AccessoirList[i].Name.c_str(), "gamecube") == 0) gamecube = 1;
    }

    dialogBoxImg1 = new GuiImage(&dialogBox1);
    dialogBoxImg1->SetAlignment(0, 3);
    dialogBoxImg1->SetPosition(-9, 0);

    dialogBoxImg2 = new GuiImage(&dialogBox2);
    dialogBoxImg2->SetAlignment(0, 3);
    dialogBoxImg2->SetPosition(145, 0);

    dialogBoxImg3 = new GuiImage(&dialogBox3);
    dialogBoxImg3->SetAlignment(0, 3);
    dialogBoxImg3->SetPosition(301, 0);

    dialogBoxImg4 = new GuiImage(&dialogBox4);
    dialogBoxImg4->SetAlignment(0, 3);
    dialogBoxImg4->SetPosition(457, 0);

    gameinfoWindow.Append(dialogBoxImg1);
    gameinfoWindow.Append(dialogBoxImg2);
    gameinfoWindow.Append(dialogBoxImg3);
    gameinfoWindow.Append(dialogBoxImg4);

    char imgPath[150];
    snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, ID);
    cover = new GuiImageData(imgPath); //load full id image
    if (!cover->GetImage())
    {
        delete cover;
        cover = Resources::GetImageData("nocover.png");
    }
    delete coverImg;
    coverImg = NULL;

    coverImg = new GuiImage(cover);
    coverImg->SetWidescreen(Settings.widescreen);
    coverImg->SetPosition(15, 30);
    gameinfoWindow.Append(coverImg);

    // # of players
    if (GameInfo.Players > 0)
    {
        if (GameInfo.Players == 1)
            playersImgData = Resources::GetImageData("wiimote1.png");

        else if (GameInfo.Players == 2)
            playersImgData = Resources::GetImageData("wiimote2.png");

        else if (GameInfo.Players == 4)
            playersImgData = Resources::GetImageData("wiimote4.png");

        playersImg = new GuiImage(playersImgData);
        playersImg->SetWidescreen(Settings.widescreen);
        playersImg->SetPosition(intputX, inputY);
        playersImg->SetAlignment(0, 4);
        gameinfoWindow.Append(playersImg);
        intputX += (Settings.widescreen ? playersImg->GetWidth() * .8 : playersImg->GetWidth()) + 5;
    }

    //draw the input types for this game
    if (motionplus == 1)
    {
        motionplusImg = new GuiImage(motionplusImgData);
        motionplusImg->SetWidescreen(Settings.widescreen);
        motionplusImg->SetPosition(intputX, inputY);
        motionplusImg->SetAlignment(0, 4);
        gameinfoWindow.Append(motionplusImg);
        intputX += (Settings.widescreen ? motionplusImg->GetWidth() * .8 : motionplusImg->GetWidth()) + 5;
    }
    if (nunchuk == 1)
    {
        nunchukImg = new GuiImage(nunchukImgData);
        nunchukImg->SetWidescreen(Settings.widescreen);
        nunchukImg->SetPosition(intputX, inputY);
        nunchukImg->SetAlignment(0, 4);
        gameinfoWindow.Append(nunchukImg);
        intputX += (Settings.widescreen ? nunchukImg->GetWidth() * .8 : nunchukImg->GetWidth()) + 5;
    }
    if (classiccontroller == 1)
    {
        classiccontrollerImg = new GuiImage(classiccontrollerImgData);
        classiccontrollerImg->SetWidescreen(Settings.widescreen);
        classiccontrollerImg->SetPosition(intputX, inputY);
        classiccontrollerImg->SetAlignment(0, 4);
        gameinfoWindow.Append(classiccontrollerImg);
        intputX += (Settings.widescreen ? classiccontrollerImg->GetWidth() * .8 : classiccontrollerImg->GetWidth())
                + 5;
    }
    if (gamecube == 1)
    {
        gcImg = new GuiImage(gamecubeImgData);
        gcImg->SetWidescreen(Settings.widescreen);
        gcImg->SetPosition(intputX, inputY);
        gcImg->SetAlignment(0, 4);
        gameinfoWindow.Append(gcImg);
        intputX += (Settings.widescreen ? gcImg->GetWidth() * .8 : gcImg->GetWidth()) + 5;
    }
    if (wheel == 1)
    {
        wheelImg = new GuiImage(wheelImgData);
        wheelImg->SetWidescreen(Settings.widescreen);
        wheelImg->SetPosition(intputX, inputY);
        wheelImg->SetAlignment(0, 4);
        gameinfoWindow.Append(wheelImg);
        intputX += (Settings.widescreen ? wheelImg->GetWidth() * .8 : wheelImg->GetWidth()) + 5;
    }
    if (guitar == 1)
    {
        guitarImg = new GuiImage(guitarImgData);
        guitarImg->SetWidescreen(Settings.widescreen);
        guitarImg->SetPosition(intputX, inputY);
        guitarImg->SetAlignment(0, 4);
        gameinfoWindow.Append(guitarImg);
        intputX += (Settings.widescreen ? guitarImg->GetWidth() * .8 : guitarImg->GetWidth()) + 5;
    }
    if (drums == 1)
    {
        drumsImg = new GuiImage(drumsImgData);
        drumsImg->SetWidescreen(Settings.widescreen);
        drumsImg->SetPosition(intputX, inputY);
        drumsImg->SetAlignment(0, 4);
        gameinfoWindow.Append(drumsImg);
        intputX += (Settings.widescreen ? drumsImg->GetWidth() * .8 : drumsImg->GetWidth()) + 5;
    }
    if (microphone == 1)
    {
        microphoneImg = new GuiImage(microphoneImgData);
        microphoneImg->SetWidescreen(Settings.widescreen);
        microphoneImg->SetPosition(intputX, inputY);
        microphoneImg->SetAlignment(0, 4);
        gameinfoWindow.Append(microphoneImg);
        intputX += (Settings.widescreen ? microphoneImg->GetWidth() * .8 : microphoneImg->GetWidth()) + 5;
    }
    if (zapper == 1)
    {
        zapperImg = new GuiImage(zapperImgData);
        zapperImg->SetWidescreen(Settings.widescreen);
        zapperImg->SetPosition(intputX, inputY);
        zapperImg->SetAlignment(0, 4);
        gameinfoWindow.Append(zapperImg);
        intputX += (Settings.widescreen ? zapperImg->GetWidth() * .8 : zapperImg->GetWidth()) + 5;
    }
    if (wiispeak == 1)
    {
        wiispeakImg = new GuiImage(wiispeakImgData);
        wiispeakImg->SetWidescreen(Settings.widescreen);
        wiispeakImg->SetPosition(intputX, inputY);
        wiispeakImg->SetAlignment(0, 4);
        gameinfoWindow.Append(wiispeakImg);
        intputX += (Settings.widescreen ? wiispeakImg->GetWidth() * .8 : wiispeakImg->GetWidth()) + 5;
    }
    if (nintendods == 1)
    {
        nintendodsImg = new GuiImage(nintendodsImgData);
        nintendodsImg->SetWidescreen(Settings.widescreen);
        nintendodsImg->SetPosition(intputX, inputY);
        nintendodsImg->SetAlignment(0, 4);
        gameinfoWindow.Append(nintendodsImg);
        intputX += (Settings.widescreen ? nintendodsImg->GetWidth() * .8 : nintendodsImg->GetWidth()) + 5;
    }
    if (dancepad == 1)
    {
        dancepadImg = new GuiImage(dancepadImgData);
        dancepadImg->SetWidescreen(Settings.widescreen);
        dancepadImg->SetPosition(intputX, inputY);
        dancepadImg->SetAlignment(0, 4);
        gameinfoWindow.Append(dancepadImg);
        intputX += (Settings.widescreen ? dancepadImg->GetWidth() * .8 : dancepadImg->GetWidth()) + 5;
    }
    if (balanceboard == 1)
    {
        balanceboardImg = new GuiImage(balanceboardImgData);
        balanceboardImg->SetWidescreen(Settings.widescreen);
        balanceboardImg->SetPosition(intputX, inputY);
        balanceboardImg->SetAlignment(0, 4);
        gameinfoWindow.Append(balanceboardImg);
        intputX += (Settings.widescreen ? balanceboardImg->GetWidth() * .8 : balanceboardImg->GetWidth()) + 5;
    }

    // # online players
    if (GameInfo.WifiPlayers > 0)
    {
        if(GameInfo.WifiPlayers == 1)
            wifiplayersImgData = Resources::GetImageData("wifi1.png");

        else if(GameInfo.WifiPlayers == 2)
            wifiplayersImgData = Resources::GetImageData("wifi2.png");

        else if(GameInfo.WifiPlayers == 4)
            wifiplayersImgData = Resources::GetImageData("wifi4.png");

        else if(GameInfo.WifiPlayers == 8)
            wifiplayersImgData =Resources::GetImageData("wifi8.png");

        else if(GameInfo.WifiPlayers == 12)
            wifiplayersImgData = Resources::GetImageData("wifi12.png");

        else if(GameInfo.WifiPlayers == 16)
            wifiplayersImgData = Resources::GetImageData("wifi16.png");

        else if(GameInfo.WifiPlayers == 32)
            wifiplayersImgData = Resources::GetImageData("wifi32.png");

        wifiplayersImg = new GuiImage(wifiplayersImgData);
        wifiplayersImg->SetWidescreen(Settings.widescreen);
        wifiplayersImg->SetPosition(intputX, inputY);
        wifiplayersImg->SetAlignment(0, 4);
        gameinfoWindow.Append(wifiplayersImg);
        intputX += (Settings.widescreen ? wifiplayersImg->GetWidth() * .8 : wifiplayersImg->GetWidth()) + 5;
    }

    // ratings
    if (GameInfo.RatingType >= 0)
    {
        if (GameInfo.RatingType == 1)
        {
            if (strcmp(GameInfo.RatingValue.c_str(), "EC") == 0)
                ratingImgData = Resources::GetImageData("esrb_ec.png");
            else if (strcmp(GameInfo.RatingValue.c_str(), "E") == 0)
                ratingImgData = Resources::GetImageData("esrb_e.png");
            else if (strcmp(GameInfo.RatingValue.c_str(), "E10+") == 0)
                ratingImgData = Resources::GetImageData("esrb_eten.png");
            else if (strcmp(GameInfo.RatingValue.c_str(), "T") == 0)
                ratingImgData = Resources::GetImageData("esrb_t.png");
            else if (strcmp(GameInfo.RatingValue.c_str(), "M") == 0)
                ratingImgData = Resources::GetImageData("esrb_m.png");
            else if (strcmp(GameInfo.RatingValue.c_str(), "AO") == 0)
                ratingImgData = Resources::GetImageData("esrb_ao.png");
            else
                ratingImgData = Resources::GetImageData("norating.png");
        } //there are 2 values here cause some countries are stupid and
        else if (GameInfo.RatingType == 2) //can't use the same as everybody else
        {
            if ((strcmp(GameInfo.RatingValue.c_str(), "3") == 0) || (strcmp(GameInfo.RatingValue.c_str(), "4") == 0))
                ratingImgData = Resources::GetImageData("pegi_3.png");
            else if ((strcmp(GameInfo.RatingValue.c_str(), "7") == 0) || (strcmp(GameInfo.RatingValue.c_str(), "7") == 0))
                ratingImgData = Resources::GetImageData("pegi_7.png");
            else if (strcmp(GameInfo.RatingValue.c_str(), "12") == 0)
                ratingImgData = Resources::GetImageData("pegi_12.png");
            else if ((strcmp(GameInfo.RatingValue.c_str(), "16") == 0) || (strcmp(GameInfo.RatingValue.c_str(), "15") == 0))
                ratingImgData = Resources::GetImageData("pegi_16.png");
            else if (strcmp(GameInfo.RatingValue.c_str(), "18") == 0)
                ratingImgData = Resources::GetImageData("pegi_18.png");
            else
            {
                ratingImgData = Resources::GetImageData("norating.png");
            }
        }
        else if (GameInfo.RatingType == 0)
        {
            if (strcmp(GameInfo.RatingValue.c_str(), "A") == 0)
                ratingImgData = Resources::GetImageData("cero_a.png");
            else if (strcmp(GameInfo.RatingValue.c_str(), "B") == 0)
                ratingImgData = Resources::GetImageData("cero_b.png");
            else if (strcmp(GameInfo.RatingValue.c_str(), "C") == 0)
                ratingImgData = Resources::GetImageData("cero_c.png");
            else if (strcmp(GameInfo.RatingValue.c_str(), "D") == 0)
                ratingImgData = Resources::GetImageData("cero_d.png");
            else if (strcmp(GameInfo.RatingValue.c_str(), "Z") == 0)
                ratingImgData = Resources::GetImageData("cero_z.png");
            else
            {
                ratingImgData = Resources::GetImageData("norating.png");
            }
        }

        else
        {
            ratingImgData = Resources::GetImageData("norating.png");
        }
        ratingImg = new GuiImage(ratingImgData);
        ratingImg->SetWidescreen(Settings.widescreen);
        ratingImg->SetPosition(-25, inputY);
        ratingImg->SetAlignment(1, 4);
        gameinfoWindow.Append(ratingImg);
        intputX += (Settings.widescreen ? ratingImg->GetWidth() * .8 : ratingImg->GetWidth()) + 5;
    }

    // memory info
    if (showmeminfo)
    {
        char meminfotxt[200];
        strlcpy(meminfotxt, MemInfo(), sizeof(meminfotxt));
        memTxt = new GuiText(meminfotxt, 18, ( GXColor ) {0, 0, 0, 255});
        memTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        memTxt->SetPosition(0, 0);
        gameinfoWindow.Append(memTxt);
    }

    // title
    int titlefontsize = 25;
    if (GameInfo.Title.size() > 0)
    {
        titleTxt = new GuiText(GameInfo.Title.c_str(), titlefontsize, ( GXColor ) {0, 0, 0, 255});
        titleTxt->SetMaxWidth(350, SCROLL_HORIZONTAL);
        titleTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
        titleTxt->SetPosition(txtXOffset, 12 + titley);
        gameinfoWindow.Append(titleTxt);
    }

    //date
    snprintf(linebuf2, sizeof(linebuf2), " ");
    if (GameInfo.PublishDate != 0)
    {
        int year = GameInfo.PublishDate >> 16;
        int day = GameInfo.PublishDate & 0xFF;
        int month = (GameInfo.PublishDate >> 8) & 0xFF;
        snprintf(linebuf2, sizeof(linebuf2), "%02i ", day);

        switch (month)
        {
            case 1:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "Jan" ));
                break;
            case 2:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "Feb" ));
                break;
            case 3:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "Mar" ));
                break;
            case 4:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "Apr" ));
                break;
            case 5:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "May" ));
                break;
            case 6:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "June" ));
                break;
            case 7:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "July" ));
                break;
            case 8:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "Aug" ));
                break;
            case 9:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "Sept" ));
                break;
            case 10:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "Oct" ));
                break;
            case 11:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "Nov" ));
                break;
            case 12:
                snprintf(linebuf2, sizeof(linebuf2), "%s%s ", linebuf2, tr( "Dec" ));
                break;
        }

        char linebuf[300];
        snprintf(linebuf, sizeof(linebuf), "%s : %s%i", tr( "Released" ), linebuf2, year);
        releasedTxt = new GuiText(linebuf, 16, ( GXColor ) {0, 0, 0, 255});
        if (releasedTxt->GetWidth() > 300) newline = 2;
        releasedTxt->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
        releasedTxt->SetPosition(-17, 12 + indexy);
        indexy += (20 * newline);
        newline = 1;
        gameinfoWindow.Append(releasedTxt);
    }

    //publisher
    if (GameInfo.Publisher.size() != 0)
    {
        snprintf(linebuf2, sizeof(linebuf2), "%s %s", tr( "Published by" ), GameInfo.Publisher.c_str());
        publisherTxt = new GuiText(linebuf2, 16, ( GXColor ) {0, 0, 0, 255});
        if (publisherTxt->GetWidth() > 250) newline = 2;
        publisherTxt->SetMaxWidth(250, WRAP);
        publisherTxt->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
        publisherTxt->SetPosition(-17, 12 + indexy);
        indexy += (20 * newline);
        newline = 1;
        gameinfoWindow.Append(publisherTxt);
    }

    //developer
    if (GameInfo.Developer.size() != 0 && strcasecmp(GameInfo.Developer.c_str(), GameInfo.Publisher.c_str()) != 0)
    {
        snprintf(linebuf2, sizeof(linebuf2), "%s %s", tr( "Developed by" ), GameInfo.Developer.c_str());
        developerTxt = new GuiText(linebuf2, 16, ( GXColor ) {0, 0, 0, 255});
        if (developerTxt->GetWidth() > 250) newline = 2;
        developerTxt->SetMaxWidth(250, WRAP);
        developerTxt->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
        developerTxt->SetPosition(-17, 12 + indexy);
        indexy += (20 * newline);
        newline = 1;
        gameinfoWindow.Append(developerTxt);
    }

    //genre
    int genreY = marginY;
    if(GameInfo.GenreList.size() > 0)
    {
        genreTitleTxt = new GuiText(tr("Gerne:"), 16, ( GXColor ) {0, 0, 0, 255});
        genreTitleTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        genreTitleTxt->SetPosition(205, 12 + genreY);
        genreY += 20;
        gameinfoWindow.Append(genreTitleTxt);
    }

    genreTxt = new GuiText *[GameInfo.GenreList.size()+1]; //to not alloc a 0 vector
    for (u32 i = 0; i < GameInfo.GenreList.size(); ++i)
    {
        genreTxt[i] = new GuiText(GameInfo.GenreList[i].c_str(), 16, ( GXColor ) {0, 0, 0, 255});
        genreTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        genreTxt[i]->SetPosition(215, 12 + genreY);
        genreY += 20;
        gameinfoWindow.Append(genreTxt[i]);
    }

    //online
    wifiTxt = new GuiText *[GameInfo.WifiFeatureList.size()+1]; //to not alloc a 0 vector
    for (int i = GameInfo.WifiFeatureList.size()-1; i >= 0 && GameInfo.WifiFeatureList.size() > 0; --i)
    {
        if (strcmp(GameInfo.WifiFeatureList[i].c_str(), "Nintendods") == 0)
        {
            snprintf(linebuf2, sizeof(linebuf2), "Nintendo DS");
        }
        else
        {
            snprintf(linebuf2, sizeof(linebuf2), "%s", GameInfo.WifiFeatureList[i].c_str());
        }
        wifiTxt[i] = new GuiText(linebuf2, 16, ( GXColor ) {0, 0, 0, 255});
        wifiTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        wifiTxt[i]->SetPosition(215, 200 + wifiY);
        wifiY -= 20;
        gameinfoWindow.Append(wifiTxt[i]);
    }
    if (GameInfo.WifiFeatureList.size() > 0)
    {
        snprintf(linebuf2, sizeof(linebuf2), "%s:", tr( "WiFi Features" ));
    }
    else
    {
        strcpy(linebuf2, "");
    }
    wifiTxt[0] = new GuiText(linebuf2, 16, ( GXColor ) {0, 0, 0, 255});
    wifiTxt[0]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    wifiTxt[0]->SetPosition(205, 200 + wifiY);
    gameinfoWindow.Append(wifiTxt[0]);

    //synopsis
    int pagesize = 12;
    if (GameInfo.Synopsis.size() != 0)
    {
        synopsisTxt = new Text(GameInfo.Synopsis.c_str(), 16, ( GXColor ) {0, 0, 0, 255});
        synopsisTxt->SetMaxWidth(350);
        synopsisTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        synopsisTxt->SetPosition(0, 0);
        synopsisTxt->SetLinesToDraw(pagesize);
        synopsisTxt->Refresh();

        dialogBoxImg11 = new GuiImage(&dialogBox1);
        dialogBoxImg11->SetAlignment(0, 3);
        dialogBoxImg11->SetPosition(-9, 0);

        dialogBoxImg22 = new GuiImage(&dialogBox2);
        dialogBoxImg22->SetAlignment(0, 3);
        dialogBoxImg22->SetPosition(145, 0);

        dialogBoxImg33 = new GuiImage(&dialogBox3);
        dialogBoxImg33->SetAlignment(0, 3);
        dialogBoxImg33->SetPosition(301, 0);

        dialogBoxImg44 = new GuiImage(&dialogBox4);
        dialogBoxImg44->SetAlignment(0, 3);
        dialogBoxImg44->SetPosition(457, 0);

        gameinfoWindow2.Append(dialogBoxImg11);
        gameinfoWindow2.Append(dialogBoxImg22);
        gameinfoWindow2.Append(dialogBoxImg33);
        gameinfoWindow2.Append(dialogBoxImg44);

        txtWindow.Append(synopsisTxt);
        txtWindow.Append(&upBtn);
        txtWindow.Append(&dnBtn);
        coverImg2 = new GuiImage(cover);
        coverImg2->SetWidescreen(Settings.widescreen);
        coverImg2->SetPosition(15, 30);
        gameinfoWindow2.Append(coverImg2);
        gameinfoWindow2.Append(&txtWindow);
    }

    wiitdb1Txt = new GuiText("http://wiitdb.com", 16, ( GXColor ) {0, 0, 0, 255});
    wiitdb1Txt->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    wiitdb1Txt->SetPosition(40, -15);
    gameinfoWindow.Append(wiitdb1Txt);

    wiitdb2Txt = new GuiText(tr( "If you don't have WiFi, press 1 to get an URL to get your WiiTDB.zip" ), 14, ( GXColor ) {0, 0, 0, 255});
    wiitdb2Txt->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    wiitdb2Txt->SetPosition(202, -15);
    gameinfoWindow.Append(wiitdb2Txt);

    wiitdb3Txt = new GuiText(" ", 14, ( GXColor ) {0, 0, 0, 255});
    wiitdb3Txt->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    wiitdb3Txt->SetPosition(202, -4);
    gameinfoWindow.Append(wiitdb3Txt);

    gameinfoWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 100);

    HaltGui();
    //mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&gameinfoWindow);
    mainWindow->ChangeFocus(&gameinfoWindow);
    ResumeGui();

    bool savedURL = false;

    while (choice == -1)
    {

        VIDEO_WaitVSync();
        if (shutdown == 1)
        {
            wiilight(0);
            Sys_Shutdown();
        }
        else if (reset == 1)
            Sys_Reboot();

        else if ((backBtn.GetState() == STATE_CLICKED) || (backBtn.GetState() == STATE_HELD))
        {
            backBtn.ResetState();
            if (page == 1)
            {
                choice = 1;
                if (synopsisTxt) delete synopsisTxt;
                synopsisTxt = NULL;
                break;
            }
            else if (page == 2)
            {
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
                page = 1;
            }
        }
        else if (((nextBtn.GetState() == STATE_CLICKED) || (nextBtn.GetState() == STATE_HELD)) && GameInfo.Synopsis.size() > 0)
        {
            nextBtn.ResetState();

            if (page == 1)
            {
                HaltGui();
                gameinfoWindow.Remove(&nextBtn);
                gameinfoWindow.Remove(&backBtn);
                gameinfoWindow.Remove(&homeBtn);
                gameinfoWindow.SetVisible(false);
                gameinfoWindow2.SetVisible(true);
                coverImg->SetPosition(15, 30);
                gameinfoWindow2.Append(&nextBtn);
                gameinfoWindow2.Append(&backBtn);
                gameinfoWindow2.Append(&homeBtn);
                mainWindow->Append(&gameinfoWindow2);
                ResumeGui();
                page = 2;
            }
            else
            {
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
                page = 1;
            }

        }
        else if ((upBtn.GetState() == STATE_CLICKED || upBtn.GetState() == STATE_HELD) && page == 2)
        {
            synopsisTxt->PreviousLine();

            usleep(60000);
            if (!((ButtonsHold() & WPAD_BUTTON_UP) || (ButtonsHold() & PAD_BUTTON_UP))) upBtn.ResetState();

        }
        else if ((dnBtn.GetState() == STATE_CLICKED || dnBtn.GetState() == STATE_HELD) && page == 2)
        {
            synopsisTxt->NextLine();

            usleep(60000);
            if (!((ButtonsHold() & WPAD_BUTTON_DOWN) || (ButtonsHold() & PAD_BUTTON_DOWN))) dnBtn.ResetState();
        }
        else if (homeBtn.GetState() == STATE_CLICKED)
        {
            if (page == 1)
            {
                choice = 2;
                if (synopsisTxt) delete synopsisTxt;
                synopsisTxt = NULL;
                break;
            }
            else if (page == 2)
            {
                HaltGui();
                gameinfoWindow2.SetVisible(false);
                gameinfoWindow.SetVisible(true);
                mainWindow->Remove(&gameinfoWindow2);
                ResumeGui();
                page = 1;
            }
        }
        else if (urlBtn.GetState() == STATE_CLICKED && !savedURL)
        {
            wiitdb2Txt->SetText(tr( "Please wait..." ));
            gameinfoWindow.Append(wiitdb2Txt);
            if (save_XML_URL())
            {
                snprintf(linebuf2, sizeof(linebuf2), tr( "Your URL has been saved in %sWiiTDB_URL.txt." ), Settings.update_path);
                wiitdb2Txt->SetText(linebuf2);
                gameinfoWindow.Append(wiitdb2Txt);

                wiitdb3Txt->SetText(tr( "Paste it into your browser to get your WiiTDB.zip." ));
                gameinfoWindow.Append(wiitdb3Txt);
                savedURL = true;
            }
            else
            {
                wiitdb2Txt->SetText(tr( "Could not save." ));
                gameinfoWindow.Append(wiitdb2Txt);
            }
            urlBtn.ResetState();
        }
    }

    gameinfoWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 100);
    while (gameinfoWindow.GetEffect() > 0)
        usleep(100);
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
    delete gamecubeImgData;
    delete ratingImgData;
    delete cover;
    delete releasedTxt;
    delete publisherTxt;
    delete developerTxt;
    delete titleTxt;
    delete synopsisTxt;
    delete genreTitleTxt;
    delete wiitdb1Txt;
    delete wiitdb2Txt;
    delete wiitdb3Txt;
    delete memTxt;
    for (u32 i = 0; i < GameInfo.GenreList.size(); ++i)
        delete genreTxt[i];

    for (u32 i = 0; i < GameInfo.WifiFeatureList.size(); ++i)
        delete wifiTxt[i];

    delete [] genreTxt;
    delete [] wifiTxt;

    ResumeGui();

    if (savedURL) return 3;

    return choice;
}

bool save_gamelist(int txt) // save gamelist
{
    mainWindow->SetState(STATE_DISABLED);
    char tmp[200];
    sprintf(tmp, "%s", Settings.update_path);
    struct stat st;
    if (stat(tmp, &st) != 0)
    {
        mkdir(tmp, 0777);
    }
    FILE *f;
    sprintf(tmp, "%sGameList.txt", Settings.update_path);
    if (txt == 1) sprintf(tmp, "%sGameList.csv", Settings.update_path);
    f = fopen(tmp, "w");
    if (!f)
    {
        sleep(1);
        mainWindow->SetState(STATE_DEFAULT);
        return false;
    }
    //make sure that all games are added to the gamelist
    gameList.LoadUnfiltered();

    f32 size = 0.0;
    f32 freespace, used;
    int i;

    WBFS_DiskSpace(&used, &freespace);

    fprintf(f, "# USB Loader Has Saved this file\n");
    fprintf(f, "# This file was created based on your list of games and language settings.\n");
    fclose(f);
    /* Closing and reopening because of a write issue we are having right now */
    f = fopen(tmp, "w");

    if (txt == 0)
    {
        fprintf(f, "# USB Loader Has Saved this file\n");
        fprintf(f, "# This file was created based on your list of games and language settings.\n\n");

        fprintf(f, "%.2fGB %s %.2fGB %s\n\n", freespace, tr( "of" ), (freespace + used), tr( "free" ));
        fprintf(f, "ID     Size(GB)  Name\n");

        for (i = 0; i < gameList.size(); i++)
        {
            struct discHdr* header = gameList[i];
            WBFS_GameSize(header->id, &size);
            if (i < 500)
            {
                fprintf(f, "%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2], header->id[3], header->id[4],
                        header->id[5]);
                fprintf(f, " [%.2f]   ", size);
                fprintf(f, " %s", GameTitles.GetTitle(header));
            }
            fprintf(f, "\n");
        }
    }
    else
    {

        fprintf(f, "\"ID\",\"Size(GB)\",\"Name\"\n");

        for (i = 0; i < gameList.size(); i++)
        {
            struct discHdr* header = gameList[i];
            WBFS_GameSize(header->id, &size);
            if (i < 500)
            {
                fprintf(f, "\"%c%c%c%c%c%c\",\"%.2f\",\"%s\"\n", header->id[0], header->id[1], header->id[2],
                        header->id[3], header->id[4], header->id[5], size, GameTitles.GetTitle(header));
            }
        }
    }
    fclose(f);

    gameList.FilterList();
    mainWindow->SetState(STATE_DEFAULT);
    return true;
}

bool save_XML_URL() // save xml url as as txt file for people without wifi
{
    char tmp[200];
    sprintf(tmp, "%s", Settings.update_path);
    struct stat st;
    if (stat(tmp, &st) != 0)
    {
        mkdir(tmp, 0777);
    }
    FILE *f;
    sprintf(tmp, "%sWiiTDB_URL.txt", Settings.update_path);
    f = fopen(tmp, "w");
    if (!f)
    {
        sleep(1);
        return false;
    }

    char XMLurl[3540];
    build_XML_URL(XMLurl, sizeof(XMLurl));

    fprintf(f, "# USB Loader Has Saved this file\n");
    fprintf(f, "# This URL was created based on your list of games and language settings.\n");
    fclose(f);
    // Closing and reopening because of a write issue we are having right now
    f = fopen(tmp, "w");
    fprintf(f, "# USB Loader Has Saved this file\n");
    fprintf(f, "# This URL was created based on your list of games and language settings.\n");
    fprintf(f,
            "# Copy and paste this URL into your web browser and you should get a zip file that will work for you.\n");
    fprintf(f, "%s\n\n\n ", XMLurl);

    fclose(f);

    return true;
}

void MemInfoPrompt()
{
    char meminfotxt[200];
    strlcpy(meminfotxt, MemInfo(), sizeof(meminfotxt));
    WindowPrompt(0, meminfotxt, tr( "OK" ));
}

void build_XML_URL(char *XMLurl, int XMLurlsize)
{
    gameList.LoadUnfiltered();
    // NET_BUFFER_SIZE in http.c needs to be set to size of XMLurl + headerformat
    char url[3540];
    char filename[10];
    snprintf(url, sizeof(url), "http://wiitdb.com/wiitdb.zip?LANG=%s&ID=", Settings.db_language);
    int i;
    for (i = 0; i < gameList.size(); i++)
    {
        struct discHdr* header = gameList[i];
        if (i < 500)
        {
            snprintf(filename, sizeof(filename), "%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],
                    header->id[3], header->id[4], header->id[5]);
            strncat(url, filename, 6);
            if ((i != gameList.size() - 1) && (i < 500)) strncat(url, ",", 1);
        }
    }
    strlcpy(XMLurl, url, XMLurlsize);
    gameList.FilterList();
}
