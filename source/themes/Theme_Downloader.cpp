/****************************************************************************
 * Theme_Downloader
 * USB Loader GX 2009
 *
 * Theme downloader for USB Loader GX
 *
 * Theme_Downloader.cpp
 ***************************************************************************/
#include <string.h>
#include <unistd.h>

#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "homebrewboot/HomebrewBrowse.h"
#include "network/networkops.h"
#include "themes/Theme_List.h"
#include "menu.h"
#include "filelist.h"
#include "listfiles.h"
#include "sys.h"
#include "network/http.h"
#include "ZipFile.h"
#include "gecko.h"

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern GuiImage * bgImg;
extern u8 shutdown;
extern u8 reset;


int DownloadTheme(const char *url, const char *title)
{
    if(!url)
        return 0;

    char filename[255];
    memset(filename, 0, sizeof(filename));

    int filesize = download_request(url, (char *) &filename);

    if(filesize <= 0)
    {
        WindowPrompt(tr("Download request failed."), 0, tr("OK"));
        return 0;
    }

    char path[300];
    char filepath[300];

    snprintf(path, sizeof(path), "%s%s", Settings.theme_downloadpath, title);

    subfoldercreate(path);

    snprintf(filepath, sizeof(filepath), "%s/%s", path, filename);

    FILE *file = fopen(filepath, "wb");
    if(!file)
    {
        WindowPrompt(tr("Download failed."), tr("Can't create file"), tr("OK"));
        return 0;
    }

    u32 done = 0;

    int blocksize = 1024*5;

    u8 *buffer = new u8[blocksize];

    while(done < (u32) filesize)
    {
        if((u32) blocksize > filesize-done)
            blocksize = filesize-done;

        ShowProgress(tr("Downloading file"), 0, (char*) filename, done, filesize, true);

        int ret = network_read(buffer, blocksize);
        if(ret < 0)
        {
            free(buffer);
            fclose(file);
            remove(path);
            ProgressStop();
            WindowPrompt(tr("Download failed."), tr("Transfer failed."), tr("OK"));
            return 0;
        }
        else if (ret == 0)
            break;

        fwrite(buffer, 1, blocksize, file);

        done += ret;
    }

    delete [] buffer;
    fclose(file);

    ProgressStop();

    if(done != (u32) filesize)
    {
        remove(filepath);
        WindowPrompt(tr("Download failed."), tr("Connection lost..."), tr("OK"));
        return 0;
    }

    ZipFile zipfile(filepath);

    int result = zipfile.ExtractAll(path);
    if(result)
    {
        remove(filepath);
        int choice = WindowPrompt(tr("Successfully extracted theme."), tr("Do you want to apply it now?"), tr("Yes"), tr("No"));
        if(choice)
        {
            char real_themepath[1024];
            sprintf(real_themepath, "%s", CFG.theme_path);
            if(SearchFile(path, "GXtheme.cfg", real_themepath) == true)
            {
                char *ptr = strrchr(real_themepath, '/');
                if(ptr)
                {
                    ptr++;
                    ptr[0] = '\0';
                }
                snprintf(CFG.theme_path, sizeof(CFG.theme_path), "%s", real_themepath);
                cfg_save_global();
                CFG_Load();
                CFG_LoadGlobal();
                result = 2;
            }
            else
                WindowPrompt(tr("ERROR: Can't set up theme."), tr("GXtheme.cfg not found in any subfolder."), tr("OK"));
        }
    }
    else
        WindowPrompt(tr("Failed to extract."), tr("Unsupported format, try to extract manually."), tr("OK"));

    return result;
}


static int Theme_Prompt(const char *title, const char *author, GuiImageData *thumbimageData, const char *downloadlink)
{
    gprintf("\nTheme_Prompt(%s ,%s, <DATA>, %s)",title,author,downloadlink);
    bool leave = false;
    int result = 0;

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    char imgPath[100];
    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);
    snprintf(imgPath, sizeof(imgPath), "%stheme_dialogue_box.png", CFG.theme_path);
    GuiImageData dialogBox(imgPath, theme_dialogue_box_png);

    GuiImage dialogBoxImg(&dialogBox);

    GuiWindow promptWindow(dialogBox.GetWidth(),dialogBox.GetHeight());
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiText titleTxt(tr("Theme Title:"), 18, THEME.prompttext);
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    titleTxt.SetPosition(230, 30);

    GuiText titleTxt2(title, 18, THEME.prompttext);
    titleTxt2.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    titleTxt2.SetPosition(230, 50);
    titleTxt2.SetMaxWidth(dialogBox.GetWidth()-220, GuiText::WRAP);

    GuiText authorTxt(tr("Author:"), 18, THEME.prompttext);
    authorTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    authorTxt.SetPosition(230, 100);

    GuiText authorTxt2(author, 18, THEME.prompttext);
    authorTxt2.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    authorTxt2.SetPosition(230, 120);
    authorTxt2.SetMaxWidth(dialogBox.GetWidth()-220, GuiText::DOTTED);

    GuiText downloadBtnTxt(tr("Download") , 22, THEME.prompttext);
    downloadBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
    GuiImage downloadBtnImg(&btnOutline);
    if (Settings.wsprompt == yes)
    {
        downloadBtnTxt.SetWidescreen(CFG.widescreen);
        downloadBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton downloadBtn(&downloadBtnImg,&downloadBtnImg, ALIGN_RIGHT, ALIGN_TOP, -5, 170, &trigA, &btnSoundOver, btnClick2,1);
    downloadBtn.SetLabel(&downloadBtnTxt);
    downloadBtn.SetScale(0.9);

    GuiText backBtnTxt(tr("Back") , 22, THEME.prompttext);
    backBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
    GuiImage backBtnImg(&btnOutline);
    if (Settings.wsprompt == yes)
    {
        backBtnTxt.SetWidescreen(CFG.widescreen);
        backBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton backBtn(&backBtnImg,&backBtnImg, ALIGN_RIGHT, ALIGN_TOP, -5, 220, &trigA, &btnSoundOver, btnClick2,1);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetTrigger(&trigB);
    backBtn.SetScale(0.9);

    GuiImage ThemeImage(thumbimageData);
    ThemeImage.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    ThemeImage.SetPosition(20, 10);
    ThemeImage.SetScale(0.8);

    ThemeImage.SetScale(0.8);

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&ThemeImage);
    promptWindow.Append(&titleTxt);
    promptWindow.Append(&titleTxt2);
    promptWindow.Append(&authorTxt);
    promptWindow.Append(&authorTxt2);
    promptWindow.Append(&downloadBtn);
    promptWindow.Append(&backBtn);

    HaltGui();
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while (!leave)
    {
        VIDEO_WaitVSync();

        if (shutdown == 1)
            Sys_Shutdown();
        else if (reset == 1)
            Sys_Reboot();

        if (downloadBtn.GetState() == STATE_CLICKED)
        {
            int choice = WindowPrompt(tr("Do you want to download this theme?"), title, tr("Yes"), tr("Cancel"));
            if(choice)
            {
                result = DownloadTheme(downloadlink, title);
                if(result == 2)
                    leave = true;
            }
            mainWindow->SetState(STATE_DISABLED);
            promptWindow.SetState(STATE_DEFAULT);
            mainWindow->ChangeFocus(&promptWindow);
            downloadBtn.ResetState();
        }

        else if (backBtn.GetState() == STATE_CLICKED)
        {
            leave = true;
            backBtn.ResetState();
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while (promptWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    return result;
}


int Theme_Downloader()
{
    int pagesize = 4;
    int menu = MENU_NONE;
    bool listchanged = false;

    char THEME_LINK[70];
    sprintf(THEME_LINK, "http://wii.spiffy360.com/themes.php?xml=1&category=1&adult=%d", Settings.godmode);
    //gprintf("\nTHEME_LINK: %s", THEME_LINK);
    //const char THEME_LINK_ADULT[70] = "http://wii.spiffy360.com/themes.php?xml=1&category=1&adult=1";

    /*** Sound Variables ***/
    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
    GuiSound btnClick1(button_click_pcm, button_click_pcm_size, Settings.sfxvolume);

    /*** Image Variables ***/
    char imgPath[150];
    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);

    snprintf(imgPath, sizeof(imgPath), "%stheme_box.png", CFG.theme_path);
    GuiImageData theme_box_Data(imgPath, theme_box_png);

    snprintf(imgPath, sizeof(imgPath), "%ssettings_background.png", CFG.theme_path);
    GuiImageData bgData(imgPath, settings_background_png);

    snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_left.png", CFG.theme_path);
    GuiImageData arrow_left(imgPath, startgame_arrow_left_png);

    snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_right.png", CFG.theme_path);
    GuiImageData arrow_right(imgPath, startgame_arrow_right_png);

    snprintf(imgPath, sizeof(imgPath), "%sWifi_btn.png", CFG.theme_path);
    GuiImageData wifiImgData(imgPath, Wifi_btn_png);

    snprintf(imgPath, sizeof(imgPath), "%spageindicator.png", CFG.theme_path);
    GuiImageData PageindicatorImgData(imgPath, pageindicator_png);

    GuiImage background(&bgData);

    /*** Trigger Variables ***/
    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
    GuiTrigger trigL;
    trigL.SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
    GuiTrigger trigR;
    trigR.SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
    GuiTrigger trigMinus;
    trigMinus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);
    GuiTrigger trigPlus;
    trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);

    GuiText titleTxt(tr("Theme Downloader"), 28, (GXColor) {0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,40);

    GuiImageData *ImageData[pagesize];
    GuiImage *Image[pagesize];
    GuiImage *theme_box_img[pagesize];
    GuiButton *MainButton[pagesize];
    GuiText *MainButtonTxt[pagesize];
    Theme_List *Theme = NULL;

    /*** Buttons ***/

    for (int i = 0; i < pagesize; i++)
    {
        ImageData[i] = NULL;
        Image[i] = NULL;
        MainButtonTxt[i] = NULL;
        theme_box_img[i] = new GuiImage(&theme_box_Data);

        MainButton[i] = new GuiButton(theme_box_Data.GetWidth(), theme_box_Data.GetHeight());
        MainButton[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        MainButton[i]->SetSoundOver(&btnSoundOver);
        MainButton[i]->SetSoundClick(&btnClick1);
        MainButton[i]->SetImage(theme_box_img[i]);
        MainButton[i]->SetEffectGrow();
        MainButton[i]->SetTrigger(&trigA);
    }

    /*** Positions ***/
	MainButton[0]->SetPosition(90, 75);
	MainButton[1]->SetPosition(340, 75);
	MainButton[2]->SetPosition(90, 230);
	MainButton[3]->SetPosition(340, 230);

    GuiText backBtnTxt(tr("Back") , 22, THEME.prompttext);
    backBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
    GuiImage backBtnImg(&btnOutline);
    if (Settings.wsprompt == yes)
    {
        backBtnTxt.SetWidescreen(CFG.widescreen);
        backBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton backBtn(&backBtnImg,&backBtnImg, 2, 3, -180, 400, &trigA, &btnSoundOver, btnClick2,1);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetTrigger(&trigB);

    GuiButton HomeBtn(1,1);
    HomeBtn.SetTrigger(&trigHome);

    GuiImage GoLeftImg(&arrow_left);
    GuiButton GoLeftBtn(GoLeftImg.GetWidth(), GoLeftImg.GetHeight());
    GoLeftBtn.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    GoLeftBtn.SetPosition(25, -25);
    GoLeftBtn.SetImage(&GoLeftImg);
    GoLeftBtn.SetSoundOver(&btnSoundOver);
    GoLeftBtn.SetSoundClick(btnClick2);
    GoLeftBtn.SetEffectGrow();
    GoLeftBtn.SetTrigger(&trigA);
    GoLeftBtn.SetTrigger(&trigL);
    GoLeftBtn.SetTrigger(&trigMinus);

    GuiImage GoRightImg(&arrow_right);
    GuiButton GoRightBtn(GoRightImg.GetWidth(), GoRightImg.GetHeight());
    GoRightBtn.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
    GoRightBtn.SetPosition(-25, -25);
    GoRightBtn.SetImage(&GoRightImg);
    GoRightBtn.SetSoundOver(&btnSoundOver);
    GoRightBtn.SetSoundClick(btnClick2);
    GoRightBtn.SetEffectGrow();
    GoRightBtn.SetTrigger(&trigA);
    GoRightBtn.SetTrigger(&trigR);
    GoRightBtn.SetTrigger(&trigPlus);

    GuiImage PageindicatorImg(&PageindicatorImgData);
    GuiText PageindicatorTxt(NULL, 22, (GXColor) { 0, 0, 0, 255});
    GuiButton PageIndicatorBtn(PageindicatorImg.GetWidth(), PageindicatorImg.GetHeight());
    PageIndicatorBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    PageIndicatorBtn.SetPosition(110, 400);
    PageIndicatorBtn.SetImage(&PageindicatorImg);
    PageIndicatorBtn.SetLabel(&PageindicatorTxt);
    PageIndicatorBtn.SetSoundOver(&btnSoundOver);
    PageIndicatorBtn.SetSoundClick(&btnClick1);
    PageIndicatorBtn.SetTrigger(&trigA);
    PageIndicatorBtn.SetEffectGrow();

    GuiImage wifiImg(&wifiImgData);
    if (Settings.wsprompt == yes)
    {
        wifiImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton wifiBtn(wifiImg.GetWidth(), wifiImg.GetHeight());
    wifiBtn.SetImage(&wifiImg);
    wifiBtn.SetPosition(500, 400);
    wifiBtn.SetSoundOver(&btnSoundOver);
    wifiBtn.SetSoundClick(&btnClick1);
    wifiBtn.SetEffectGrow();
    wifiBtn.SetTrigger(&trigA);

    GuiWindow w(screenwidth, screenheight);

    HaltGui();
    w.Append(&background);
    mainWindow->Append(&w);
    ResumeGui();

    if(!IsNetworkInit())
        NetworkInitPrompt();

    char url[300];
    int currentpage = 1;
    int currenttheme = 0;

    HaltGui();
    w.RemoveAll();
    w.Append(&background);
    w.Append(&titleTxt);
    w.Append(&backBtn);
    w.Append(&GoLeftBtn);
    w.Append(&GoRightBtn);
    w.Append(&PageIndicatorBtn);
    w.Append(&wifiBtn);
    w.Append(&HomeBtn);
    ResumeGui();

    ShowProgress(tr("Downloading Page List:"), "", (char *) tr("Please wait..."), 0, pagesize);

    Theme = new Theme_List(THEME_LINK);

    int ThemesOnPage = Theme->GetThemeCount();

    if(!ThemesOnPage)
    {
        WindowPrompt(tr("No themes found on the site."), 0, "OK");
        menu = MENU_SETTINGS;
    }

    while(menu == MENU_NONE)
    {
        HaltGui();
        w.RemoveAll();
        w.Append(&background);
        w.Append(&titleTxt);
        w.Append(&backBtn);
        w.Append(&GoLeftBtn);
        w.Append(&GoRightBtn);
        w.Append(&PageIndicatorBtn);
        w.Append(&wifiBtn);
        w.Append(&HomeBtn);
        ResumeGui();

        sprintf(url, "%i", currentpage);
        PageindicatorTxt.SetText(url);

        int n = 0;

        for(int i = currenttheme; (i < (currenttheme+pagesize)); i++)
        {
            ShowProgress(tr("Downloading image:"), 0, (char *) Theme->GetThemeTitle(i), n, pagesize);

            if(MainButtonTxt[n])
                delete MainButtonTxt[n];
            if(ImageData[n])
                delete ImageData[n];
            if(Image[n])
                delete Image[n];

            MainButtonTxt[n] = NULL;
            ImageData[n] = NULL;
            Image[n] = NULL;

            if(i < ThemesOnPage)
            {
                MainButtonTxt[n] = new GuiText(Theme->GetThemeTitle(i), 18, (GXColor) { 0, 0, 0, 255});
                MainButtonTxt[n]->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
                MainButtonTxt[n]->SetPosition(0, 10);
                MainButtonTxt[n]->SetMaxWidth(theme_box_Data.GetWidth()-10, GuiText::DOTTED);

                sprintf(url, "%s", Theme->GetImageLink(i));

                char filepath[300];
                snprintf(filepath, sizeof(filepath), "%s/tmp/%s.jpg", Settings.theme_downloadpath, Theme->GetThemeTitle(i));

                FILE * storefile = fopen(filepath, "rb");

                if(!storefile)
                {
                    struct block file = downloadfile(url);
                    char storepath[300];
                    snprintf(storepath, sizeof(storepath), "%s/tmp/", Settings.theme_downloadpath);
                    subfoldercreate(storepath);
                    if(file.data)
                    {
                        storefile = fopen(filepath, "wb");
                        fwrite(file.data, 1, file.size, storefile);
                        fclose(storefile);
                    }
                    ImageData[n] = new GuiImageData(file.data, file.size);
                    free(file.data);
                }
                else
                {
                    fseek(storefile, 0, SEEK_END);
                    u32 filesize = ftell(storefile);
                    u8 *buffer = (u8*) malloc(filesize);
                    rewind(storefile);
                    fread(buffer, 1, filesize, storefile);
                    fclose(storefile);
                    ImageData[n] = new GuiImageData(buffer, filesize);
                    free(buffer);
                    buffer = NULL;
                }
                Image[n] = new GuiImage(ImageData[n]);
                Image[n]->SetScale(0.4);
                Image[n]->SetPosition(50, -45);
                MainButton[n]->SetIcon(Image[n]);
                MainButton[n]->SetLabel(MainButtonTxt[n]);
            }
            n++;
        }

        ProgressStop();

        HaltGui();
        for(int i = 0; i < pagesize; i++)
        {
            if(MainButtonTxt[i])
                w.Append(MainButton[i]);
        }
        ResumeGui();

        listchanged = false;

        while(!listchanged)
        {
            VIDEO_WaitVSync ();

            if (shutdown == 1)
                Sys_Shutdown();
            else if (reset == 1)
                Sys_Reboot();

            else if (wifiBtn.GetState() == STATE_CLICKED)
            {
                Initialize_Network();
                wifiBtn.ResetState();
            }
            else if (backBtn.GetState() == STATE_CLICKED)
            {
                listchanged = true;
                menu = MENU_SETTINGS;
                backBtn.ResetState();
                break;
            }
            else if (GoRightBtn.GetState() == STATE_CLICKED)
            {
                listchanged = true;
                currenttheme += pagesize;
                currentpage++;
                if(currenttheme >= ThemesOnPage)
                {
                    currentpage = 1;
                    currenttheme = 0;
                }
                GoRightBtn.ResetState();
            }
            else if (GoLeftBtn.GetState() == STATE_CLICKED)
            {
                listchanged = true;
                currenttheme -= pagesize;
                currentpage--;
                if(currenttheme < 0)
                {
                    currentpage = roundup((ThemesOnPage+1.0f)/pagesize);
                    currenttheme = currentpage*pagesize-pagesize;
                }
                GoLeftBtn.ResetState();
            }

            for(int i = 0; i < pagesize; i++)
            {
                if(MainButton[i]->GetState() == STATE_CLICKED)
                {
                    snprintf(url, sizeof(url), "%s", Theme->GetDownloadLink(currenttheme+i));
                    int ret = Theme_Prompt(Theme->GetThemeTitle(currenttheme+i), Theme->GetThemeAuthor(currenttheme+i), ImageData[i], url);
                    MainButton[i]->ResetState();
                    if(ret == 2)
                    {
                        listchanged = true;
                        menu = MENU_THEMEDOWNLOADER;
                    }
                }
            }
        }
    }

    w.SetEffect(EFFECT_FADE, -20);

    while(w.GetEffect() > 0) usleep(100);

    HaltGui();
    mainWindow->Remove(&w);

    for (int i = 0; i < pagesize; i++)
    {
        if(MainButton[i])
            delete MainButton[i];
        if(theme_box_img[i])
            delete theme_box_img[i];
        if(ImageData[i])
            delete ImageData[i];
        if(Image[i])
            delete Image[i];
        if(MainButtonTxt[i])
            delete MainButtonTxt[i];
    }

    if(Theme)
        delete Theme;
    Theme = NULL;

    ResumeGui();

    return menu;
}
