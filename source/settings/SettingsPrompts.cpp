#include <gccore.h>
#include <unistd.h>
#include <string.h>

#include "language/gettext.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "settings/cfg.h"
#include "network/URL_List.h"
#include "listfiles.h"
#include "menu/menus.h"
#include "main.h"
#include "fatmounter.h"
#include "filelist.h"
#include "sys.h"
#include "menu.h"


/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;

/****************************************************************************
 * MenuOGG
 ***************************************************************************/
bool MenuOGG() {
    int cnt = 0;
    int ret = 0, choice = 0;
    int scrollon, nothingchanged = 0;
    bool returnhere = false;

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    char imgPath[100];

    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);
    snprintf(imgPath, sizeof(imgPath), "%ssettings_background.png", CFG.theme_path);
    GuiImageData settingsbg(imgPath, settings_background_png);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
    GuiTrigger trigMinus;
    trigMinus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);
    GuiTrigger trigPlus;
    trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);

    char fullpath[150];
    char shortpath[35];
    int countoggs = GetAllDirFiles(Settings.oggload_path);

    if (!strcmp("", Settings.oggload_path)) {
        sprintf(shortpath, "%s", tr("Standard"));
    } else {
        sprintf(shortpath, "%s", Settings.oggload_path);
    }

    GuiText titleTxt(shortpath, 24, (GXColor) {0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    titleTxt.SetPosition(0,0);
    GuiButton pathBtn(300, 50);
    pathBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    pathBtn.SetPosition(0,28);
    pathBtn.SetLabel(&titleTxt);
    pathBtn.SetSoundOver(&btnSoundOver);
    pathBtn.SetSoundClick(btnClick2);
    pathBtn.SetTrigger(&trigA);
    pathBtn.SetEffectGrow();

    GuiImage oggmenubackground(&settingsbg);
    oggmenubackground.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    oggmenubackground.SetPosition(0, 0);

    GuiText backBtnTxt(tr("Back") , 22, THEME.prompttext);
    backBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
    GuiImage backBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        backBtnTxt.SetWidescreen(CFG.widescreen);
        backBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    backBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    backBtn.SetPosition(-180, 400);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetImage(&backBtnImg);
    backBtn.SetSoundOver(&btnSoundOver);
    backBtn.SetSoundClick(btnClick2);
    backBtn.SetTrigger(&trigA);
    backBtn.SetTrigger(&trigB);
    backBtn.SetEffectGrow();

    GuiText defaultBtnTxt(tr("Default") , 22, THEME.prompttext);
    defaultBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
    GuiImage defaultBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        defaultBtnTxt.SetWidescreen(CFG.widescreen);
        defaultBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton defaultBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    defaultBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    defaultBtn.SetPosition(180, 400);
    defaultBtn.SetLabel(&defaultBtnTxt);
    defaultBtn.SetImage(&defaultBtnImg);
    defaultBtn.SetSoundOver(&btnSoundOver);
    defaultBtn.SetSoundClick(btnClick2);
    defaultBtn.SetTrigger(&trigA);
    defaultBtn.SetEffectGrow();

    customOptionList options2(countoggs);

    for (cnt = 0; cnt < countoggs; cnt++) {
        options2.SetValue(cnt, "%s", GetFileName(cnt));
        options2.SetName(cnt,"%i.", cnt+1);
    }

    if (cnt < 9) {
        scrollon = 0;
    } else {
        scrollon = 1;
    }

    GuiCustomOptionBrowser optionBrowser4(396, 280, &options2, CFG.theme_path, "bg_options_settings.png", bg_options_settings_png, scrollon, 10);
    optionBrowser4.SetPosition(0, 90);
    optionBrowser4.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

    snprintf(imgPath, sizeof(imgPath), "%smp3_stop.png", CFG.theme_path);
    GuiImageData stop(imgPath, mp3_stop_png);
    snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_right.png", CFG.theme_path);
    GuiImageData play(imgPath, startgame_arrow_right_png);

    GuiImage playBtnImg(&play);
    playBtnImg.SetWidescreen(CFG.widescreen);
    GuiButton playBtn(play.GetWidth(), play.GetHeight());
    playBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    playBtn.SetPosition(50, 400);
    playBtn.SetImage(&playBtnImg);
    playBtn.SetSoundOver(&btnSoundOver);
    playBtn.SetSoundClick(btnClick2);
    playBtn.SetTrigger(&trigA);
    playBtn.SetTrigger(&trigPlus);
    playBtn.SetEffectGrow();

    GuiImage stopBtnImg(&stop);
    stopBtnImg.SetWidescreen(CFG.widescreen);
    GuiButton stopBtn(stop.GetWidth(), stop.GetHeight());
    stopBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    stopBtn.SetPosition(-15, 400);
    stopBtn.SetImage(&stopBtnImg);
    stopBtn.SetSoundOver(&btnSoundOver);
    stopBtn.SetSoundClick(btnClick2);
    stopBtn.SetTrigger(&trigA);
    stopBtn.SetTrigger(&trigMinus);
    stopBtn.SetEffectGrow();

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&oggmenubackground);
    w.Append(&pathBtn);
    w.Append(&backBtn);
    w.Append(&playBtn);
    w.Append(&stopBtn);
    w.Append(&defaultBtn);
    w.Append(&optionBrowser4);
    mainWindow->Append(&w);

    w.SetEffect(EFFECT_FADE, 20);
    ResumeGui();

    while (w.GetEffect()>0) usleep(50);

    while (!returnhere) {

        if (backBtn.GetState() == STATE_CLICKED) {
            if (nothingchanged == 1 && countoggs > 0) {
                if (strcmp("", Settings.oggload_path) && strcmp("notset", Settings.ogg_path)) {
                    bgMusic->Load(Settings.ogg_path);
                } else {
					bgMusic->Load(bg_music_ogg, bg_music_ogg_size, true);
				}
                bgMusic->Play();
            }
            backBtn.ResetState();
            break;
        }

        if (defaultBtn.GetState() == STATE_CLICKED) {
            choice = WindowPrompt(tr("Loading standard music."),0,tr("OK"), tr("Cancel"));
            if (choice == 1) {
                sprintf(Settings.ogg_path, "notset");
				bgMusic->Load(bg_music_ogg, bg_music_ogg_size, true);
                bgMusic->Play();
                bgMusic->SetVolume(Settings.volume);
                cfg_save_global();
            }
            defaultBtn.ResetState();
			if (countoggs > 0)
				optionBrowser4.SetFocus(1);
        }

        if (pathBtn.GetState() == STATE_CLICKED) {
            w.Remove(&optionBrowser4);
            w.Remove(&backBtn);
            w.Remove(&pathBtn);
            w.Remove(&playBtn);
            w.Remove(&stopBtn);
            w.Remove(&defaultBtn);
            char entered[43] = "";
            strlcpy(entered, Settings.oggload_path, sizeof(entered));
            int result = OnScreenKeyboard(entered,43,0);
            w.Append(&optionBrowser4);
            w.Append(&pathBtn);
            w.Append(&backBtn);
            w.Append(&playBtn);
            w.Append(&stopBtn);
            w.Append(&defaultBtn);
            if ( result == 1 ) {
                int len = (strlen(entered)-1);
                if (entered[len] !='/')
                    strncat (entered, "/", 1);
                strlcpy(Settings.oggload_path, entered, sizeof(Settings.oggload_path));
                WindowPrompt(tr("Backgroundmusic Path changed."),0,tr("OK"));
                if (isInserted(bootDevice)) {
                    if (!strcmp("", Settings.oggload_path)) {
                        sprintf(Settings.ogg_path, "notset");
                        bgMusic->Play();
                    }
                    cfg_save_global();
                    returnhere = true;
                    break;
                } else {
                    WindowPrompt(tr("No SD-Card inserted!"), tr("Insert an SD-Card to save."), tr("OK"));
                }
            }
            if (countoggs > 0) {
                optionBrowser4.SetFocus(1);
            }
            pathBtn.ResetState();
        }

        ret = optionBrowser4.GetClickedOption();

        if (ret>=0) {
            choice = WindowPrompt(tr("Set as backgroundmusic?"),GetFileName(ret),tr("Yes"),tr("No"));
            if (choice == 1) {
                snprintf(fullpath,150,"%s%s",Settings.oggload_path,GetFileName(ret));
                if (!bgMusic->Load(fullpath)) {
                    WindowPrompt(tr("Not supported format!"), tr("Loading standard music."), tr("OK"));
                    sprintf(Settings.ogg_path, "notset");
                } else {
                    snprintf(Settings.ogg_path, sizeof(Settings.ogg_path), "%s", fullpath);
                    cfg_save_global();
                    bgMusic->SetVolume(Settings.volume);
                    nothingchanged = 0;
                }
                bgMusic->Play();
                bgMusic->SetVolume(Settings.volume);
            }
            optionBrowser4.SetFocus(1);
        }

        if (playBtn.GetState() == STATE_CLICKED && countoggs > 0) {
            if (countoggs > 0) {
                ret = optionBrowser4.GetSelectedOption();
                snprintf(fullpath, 150,"%s%s", Settings.oggload_path,GetFileName(ret));
                if (!bgMusic->Load(fullpath)) {
                    WindowPrompt(tr("Not supported format!"), tr("Loading standard music."), tr("OK"));
                }
                bgMusic->Play();
                bgMusic->SetVolume(Settings.volume);
                nothingchanged = 1;
                optionBrowser4.SetFocus(1);
            }
            playBtn.ResetState();
        }

        if (stopBtn.GetState() == STATE_CLICKED) {
            if (countoggs > 0) {
                bgMusic->Stop();
                nothingchanged = 1;
                optionBrowser4.SetFocus(1);
            }
            stopBtn.ResetState();
        }
    }

    w.SetEffect(EFFECT_FADE, -20);
    while (w.GetEffect()>0) usleep(50);

    HaltGui();
    mainWindow->Remove(&w);
    ResumeGui();

    return returnhere;
}

/****************************************************************************
 * MenuLanguageSelect
 ***************************************************************************/
int MenuLanguageSelect() {
    int cnt = 0;
    int ret = 0, choice = 0;
    int scrollon;
    int returnhere = 0;

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    char imgPath[100];

    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);
    snprintf(imgPath, sizeof(imgPath), "%ssettings_background.png", CFG.theme_path);
    GuiImageData settingsbg(imgPath, settings_background_png);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    char fullpath[100];
    int countfiles = GetAllDirFiles(Settings.languagefiles_path);

    if (!strcmp("", Settings.languagefiles_path)) {
        sprintf(fullpath, "%s", tr("Standard"));
    } else {
        sprintf(fullpath, "%s", Settings.languagefiles_path);
    }

    GuiText titleTxt(fullpath, 24, (GXColor) {0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    titleTxt.SetPosition(0,0);
    GuiButton pathBtn(300, 50);
    pathBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    pathBtn.SetPosition(0,28);
    pathBtn.SetLabel(&titleTxt);
    pathBtn.SetSoundOver(&btnSoundOver);
    pathBtn.SetSoundClick(btnClick2);
    pathBtn.SetTrigger(&trigA);
    pathBtn.SetEffectGrow();

    GuiImage oggmenubackground(&settingsbg);
    oggmenubackground.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    oggmenubackground.SetPosition(0, 0);

    GuiText backBtnTxt(tr("Back") , 22, THEME.prompttext);
    backBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
    GuiImage backBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        backBtnTxt.SetWidescreen(CFG.widescreen);
        backBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    backBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    backBtn.SetPosition(-190, 400);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetImage(&backBtnImg);
    backBtn.SetSoundOver(&btnSoundOver);
    backBtn.SetSoundClick(btnClick2);
    backBtn.SetTrigger(&trigA);
    backBtn.SetTrigger(&trigB);
    backBtn.SetEffectGrow();

    GuiText defaultBtnTxt(tr("Default") , 22, THEME.prompttext);
    defaultBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
    GuiImage defaultBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        defaultBtnTxt.SetWidescreen(CFG.widescreen);
        defaultBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton defaultBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    defaultBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    defaultBtn.SetPosition(190, 400);
    defaultBtn.SetLabel(&defaultBtnTxt);
    defaultBtn.SetImage(&defaultBtnImg);
    defaultBtn.SetSoundOver(&btnSoundOver);
    defaultBtn.SetSoundClick(btnClick2);
    defaultBtn.SetTrigger(&trigA);
    defaultBtn.SetEffectGrow();

    GuiText updateBtnTxt(tr("Update Files") , 22, THEME.prompttext);
    updateBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
    GuiImage updateBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        updateBtnTxt.SetWidescreen(CFG.widescreen);
        updateBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton updateBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    updateBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    updateBtn.SetPosition(0, 400);
    updateBtn.SetLabel(&updateBtnTxt);
    updateBtn.SetImage(&updateBtnImg);
    updateBtn.SetSoundOver(&btnSoundOver);
    updateBtn.SetSoundClick(btnClick2);
    updateBtn.SetTrigger(&trigA);
    updateBtn.SetEffectGrow();

    customOptionList options2(countfiles);

    for (cnt = 0; cnt < countfiles; cnt++) {
        char filename[64];
        strlcpy(filename, GetFileName(cnt), sizeof(filename));
        char *dot = strchr(filename, '.');
        if (dot) *dot='\0';
        options2.SetName(cnt, "%s", filename);
        options2.SetValue(cnt, NULL);

    }

    if (cnt < 9) {
        scrollon = 0;
    } else {
        scrollon = 1;
    }

    GuiCustomOptionBrowser optionBrowser4(396, 280, &options2, CFG.theme_path, "bg_options_settings.png", bg_options_settings_png, scrollon, 10);
    optionBrowser4.SetPosition(0, 90);
    optionBrowser4.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&oggmenubackground);
    w.Append(&pathBtn);
    w.Append(&backBtn);
    w.Append(&defaultBtn);
    w.Append(&updateBtn);
    w.Append(&optionBrowser4);
    mainWindow->Append(&w);

    w.SetEffect(EFFECT_FADE, 20);
    ResumeGui();

    while (w.GetEffect()>0) usleep(50);

    while (!returnhere) {

        if (backBtn.GetState() == STATE_CLICKED) {

            backBtn.ResetState();
            break;
        }

        else if (defaultBtn.GetState() == STATE_CLICKED) {
            choice = WindowPrompt(tr("Loading standard language."),0,tr("OK"), tr("Cancel"));
            if (choice == 1) {
                sprintf(Settings.language_path, "notset");
                cfg_save_global();
                gettextCleanUp();
				HaltGui();
                CFG_Load();
				ResumeGui();
                returnhere = 2;
            }
            defaultBtn.ResetState();
			//optionBrowser4.SetFocus(1); // commented out to prevent crash
        }

        else if (updateBtn.GetState() == STATE_CLICKED) {
            choice = WindowPrompt(tr("Update all Language Files"),tr("Do you wish to update/download all language files?"),tr("OK"), tr("Cancel"));
            if (choice == 1) {

                bool network = true;
                if (!IsNetworkInit()) {
                    network = NetworkInitPrompt();
                }

                if (network) {
                    const char URL[60] = "http://usbloader-gui.googlecode.com/svn/trunk/Languages/";
                    char fullURL[300];
                    FILE *pfile;

                    URL_List LinkList(URL);
                    int listsize = LinkList.GetURLCount();

                    subfoldercreate(Settings.languagefiles_path);

                    for (int i = 0; i < listsize; i++) {

                        ShowProgress(tr("Updating Language Files:"), 0, LinkList.GetURL(i), i, listsize-1);

                        if (strcasecmp(".lang", strrchr(LinkList.GetURL(i), '.')) == 0) {

                            snprintf(fullURL, sizeof(fullURL), "%s%s", URL, LinkList.GetURL(i));

                            struct block file = downloadfile(fullURL);

                            if (file.data && file.size) {
                                char filepath[300];

                                snprintf(filepath, sizeof(filepath), "%s%s", Settings.languagefiles_path, LinkList.GetURL(i));
                                pfile = fopen(filepath, "wb");
                                fwrite(file.data, 1, file.size, pfile);
                                fclose(pfile);

                            }

                            free(file.data);
                        }
                    }
                    ProgressStop();
                    returnhere = 1;
                    break;
                }
            }
			updateBtn.ResetState();
			//optionBrowser4.SetFocus(1); // commented out to prevent crash
        }

        else if (pathBtn.GetState() == STATE_CLICKED) {
            w.Remove(&optionBrowser4);
            w.Remove(&backBtn);
            w.Remove(&pathBtn);
            w.Remove(&defaultBtn);
            char entered[43] = "";
            strlcpy(entered, Settings.languagefiles_path, sizeof(entered));
            int result = OnScreenKeyboard(entered,43,0);
            w.Append(&optionBrowser4);
            w.Append(&pathBtn);
            w.Append(&backBtn);
            w.Append(&defaultBtn);
            if ( result == 1 ) {
                int len = (strlen(entered)-1);
                if (entered[len] !='/')
                    strncat (entered, "/", 1);
                strlcpy(Settings.languagefiles_path, entered, sizeof(Settings.languagefiles_path));
                WindowPrompt(tr("Languagepath changed."),0,tr("OK"));
                if (isInserted(bootDevice)) {
                    cfg_save_global();
                    returnhere = 1;
                    break;
                } else {
                    WindowPrompt(tr("No SD-Card inserted!"), tr("Insert an SD-Card to save."), tr("OK"));
                }
            }
            if (countfiles > 0) {
                optionBrowser4.SetFocus(1);
            }
            pathBtn.ResetState();
        }

        ret = optionBrowser4.GetClickedOption();

        if (ret>=0) {
            choice = WindowPrompt(tr("Do you want to change language?"), 0, tr("Yes"), tr("Cancel"));
            if (choice == 1) {
                if (isInserted(bootDevice)) {
                    snprintf(Settings.language_path, sizeof(Settings.language_path), "%s%s", Settings.languagefiles_path, GetFileName(ret));
                    cfg_save_global();
                    if (!checkfile(Settings.language_path)) {
                        sprintf(Settings.language_path, tr("not set"));
                        WindowPrompt(tr("File not found."),tr("Loading standard language."),tr("OK"));
                    }
                    gettextCleanUp();
					HaltGui();
					CFG_Load();
					ResumeGui();
                    returnhere = 2;
                    break;
                } else {
                    WindowPrompt(tr("No SD-Card inserted!"), tr("Insert an SD-Card to save."), tr("OK"), 0,0,0,-1);
                }
            }
            optionBrowser4.SetFocus(1);
        }

    }

    w.SetEffect(EFFECT_FADE, -20);
    while (w.GetEffect()>0) usleep(50);

    HaltGui();
    mainWindow->Remove(&w);
    ResumeGui();

    return returnhere;
}

