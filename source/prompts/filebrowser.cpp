/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * modified by dimok
 *
 * filebrowser.cpp
 *
 * Generic file routines - reading, writing, browsing
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <sys/dir.h>
#include <malloc.h>

#include "filebrowser.h"
#include "menu.h"

#include "listfiles.h"
#include "language/gettext.h"
#include "PromptWindows.h"
#include "libwiigui/gui.h"
#include "sys.h"

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

BROWSERINFO browser;
BROWSERENTRY * browserList = NULL; // list of files/folders in browser

/****************************************************************************
 * ResetBrowser()
 * Clears the file browser memory, and allocates one initial entry
 ***************************************************************************/
void ResetBrowser() {
    browser.numEntries = 0;
    browser.selIndex = 0;
    browser.pageIndex = 0;

    // Clear any existing values
    if (browserList != NULL) {
        free(browserList);
        browserList = NULL;
    }
    // set aside space for 1 entry
    browserList = (BROWSERENTRY *)malloc(sizeof(BROWSERENTRY));
    memset(browserList, 0, sizeof(BROWSERENTRY));
}

/****************************************************************************
 * UpdateDirName()
 * Update curent directory name for file browser
 ***************************************************************************/
int UpdateDirName() {
    int size=0;
    char * test;
    char temp[1024];

    /* current directory doesn't change */
    if (strcmp(browserList[browser.selIndex].filename,".") == 0) {
        return 0;
    }
    /* go up to parent directory */
    else if (strcmp(browserList[browser.selIndex].filename,"..") == 0) {
        /* determine last subdirectory namelength */
        sprintf(temp,"%s",browser.dir);
        test = strtok(temp,"/");
        while (test != NULL) {
            size = strlen(test);
            test = strtok(NULL,"/");
        }

        /* remove last subdirectory name */
        size = strlen(browser.dir) - size - 1;
        browser.dir[size] = 0;

        return 1;
    }
    /* Open a directory */
    else {
        /* test new directory namelength */
        if ((strlen(browser.dir)+1+strlen(browserList[browser.selIndex].filename)) < MAXPATHLEN) {
            /* update current directory name */
            sprintf(browser.dir, "%s/%s",browser.dir, browserList[browser.selIndex].filename);
            return 1;
        } else {
            return -1;
        }
    }
}

/****************************************************************************
 * FileSortCallback
 *
 * Quick sort callback to sort file entries with the following order:
 *   .
 *   ..
 *   <dirs>
 *   <files>
 ***************************************************************************/
int FileSortCallback(const void *f1, const void *f2) {
    /* Special case for implicit directories */
    if (((BROWSERENTRY *)f1)->filename[0] == '.' || ((BROWSERENTRY *)f2)->filename[0] == '.') {
        if (strcmp(((BROWSERENTRY *)f1)->filename, ".") == 0) {
            return -1;
        }
        if (strcmp(((BROWSERENTRY *)f2)->filename, ".") == 0) {
            return 1;
        }
        if (strcmp(((BROWSERENTRY *)f1)->filename, "..") == 0) {
            return -1;
        }
        if (strcmp(((BROWSERENTRY *)f2)->filename, "..") == 0) {
            return 1;
        }
    }

    /* If one is a file and one is a directory the directory is first. */
    if (((BROWSERENTRY *)f1)->isdir && !(((BROWSERENTRY *)f2)->isdir)) return -1;
    if (!(((BROWSERENTRY *)f1)->isdir) && ((BROWSERENTRY *)f2)->isdir) return 1;

    return stricmp(((BROWSERENTRY *)f1)->filename, ((BROWSERENTRY *)f2)->filename);
}

/***************************************************************************
 * Browse subdirectories
 **************************************************************************/
int
ParseDirectory() {
    DIR_ITER *dir = NULL;
    char fulldir[MAXPATHLEN];
    char filename[MAXPATHLEN];
    struct stat filestat;

    // reset browser
    ResetBrowser();

    // open the directory
    sprintf(fulldir, "%s%s", browser.rootdir, browser.dir); // add currentDevice to path
    dir = diropen(fulldir);

    // if we can't open the dir, try opening the root dir
    if (dir == NULL) {
        sprintf(browser.dir,"/");
        dir = diropen(browser.rootdir);
        if (dir == NULL) {
            return -1;
        }
    }

    // index files/folders
    int entryNum = 0;

    while (dirnext(dir,filename,&filestat) == 0) {
        if (strcmp(filename,".") != 0) {
            BROWSERENTRY * newBrowserList = (BROWSERENTRY *)realloc(browserList, (entryNum+1) * sizeof(BROWSERENTRY));

            if (!newBrowserList) { // failed to allocate required memory
                ResetBrowser();
                entryNum = -1;
                break;
            } else {
                browserList = newBrowserList;
            }
            memset(&(browserList[entryNum]), 0, sizeof(BROWSERENTRY)); // clear the new entry

            strncpy(browserList[entryNum].filename, filename, MAXJOLIET);

            if (strcmp(filename,"..") == 0) {
                sprintf(browserList[entryNum].displayname, "..");
            } else {
                strcpy(browserList[entryNum].displayname, filename);	// crop name for display
            }

            browserList[entryNum].length = filestat.st_size;
            browserList[entryNum].isdir = (filestat.st_mode & _IFDIR) == 0 ? 0 : 1; // flag this as a dir

            entryNum++;
        }
    }

    // close directory
    dirclose(dir);

    // Sort the file list
    qsort(browserList, entryNum, sizeof(BROWSERENTRY), FileSortCallback);

    browser.numEntries = entryNum;
    return entryNum;
}

/****************************************************************************
 * BrowserChangeFolder
 *
 * Update current directory and set new entry list if directory has changed
 ***************************************************************************/
int BrowserChangeFolder() {
    if (!UpdateDirName())
        return -1;

    ParseDirectory();

    return browser.numEntries;
}

/****************************************************************************
 * BrowseDevice
 * Displays a list of files on the selected device
 ***************************************************************************/
int BrowseDevice(int device) {
    sprintf(browser.dir, "/");
    switch (device) {
    case SD:
        sprintf(browser.rootdir, "SD:");
        break;
    case USB:
        sprintf(browser.rootdir, "USB:");
        break;
    }
    ParseDirectory(); // Parse root directory
    return browser.numEntries;
}


/****************************************************************************
 * MenuBrowseDevice
 ***************************************************************************/

int BrowseDevice(char * var, int force) {

    int result=-1;
    int i;
    char currentdir[90];
    int curDivice = -1;
    int forced =force;

    if (forced>-1) {
        if (BrowseDevice(forced) > 0) {
            curDivice = forced;
            goto main;
        }
    }

    else if ((!strcasecmp(bootDevice, "USB:"))&&(BrowseDevice(USB) > 0)) {
        curDivice = USB;
        goto main;
    } else 	if ((!strcasecmp(bootDevice, "SD:"))&&(BrowseDevice(SD) > 0)) {
        curDivice = SD;
        goto main;
    } else {
        WindowPrompt(tr("Error"),0,tr("Ok"));
        return -1;
    }

main:
    int menu = MENU_NONE;

    /*
    	GuiText titleTxt("Browse Files", 28, (GXColor){0, 0, 0, 230});
    	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    	titleTxt.SetPosition(70,20);
    */
    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);
    GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);

    GuiImageData folderImgData(folder_png);
    GuiImage folderImg(&folderImgData);
    GuiButton folderBtn(folderImg.GetWidth(), folderImg.GetHeight());
    folderBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    folderBtn.SetPosition(-210, -145);
    folderBtn.SetImage(&folderImg);
    folderBtn.SetTrigger(&trigA);
    folderBtn.SetEffectGrow();
	
	char imgPath[100];
    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);
    GuiText ExitBtnTxt(tr("Cancel"), 24, (GXColor) {0, 0, 0, 255});
    GuiImage ExitBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        ExitBtnTxt.SetWidescreen(CFG.widescreen);
        ExitBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton ExitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    ExitBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    ExitBtn.SetPosition(-40, -35);
    ExitBtn.SetLabel(&ExitBtnTxt);
    ExitBtn.SetImage(&ExitBtnImg);
    ExitBtn.SetTrigger(&trigA);
    ExitBtn.SetTrigger(&trigB);
    ExitBtn.SetEffectGrow();

    GuiText usbBtnTxt((curDivice==SD?"USB":"SD"), 24, (GXColor) {0, 0, 0, 255});
    GuiImage usbBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        usbBtnTxt.SetWidescreen(CFG.widescreen);
        usbBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton usbBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    usbBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    usbBtn.SetPosition(0, -35);
    usbBtn.SetLabel(&usbBtnTxt);
    usbBtn.SetImage(&usbBtnImg);
    usbBtn.SetTrigger(&trigA);
    usbBtn.SetEffectGrow();

    GuiText okBtnTxt(tr("Ok"), 22, (GXColor) {THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
    GuiImage okBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        okBtnTxt.SetWidescreen(CFG.widescreen);
        okBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton okBtn(&okBtnImg,&okBtnImg, 0, 4, 40, -35, &trigA, &btnSoundOver, &btnClick,1);
    okBtn.SetLabel(&okBtnTxt);

    GuiFileBrowser fileBrowser(396, 248);
    fileBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    fileBrowser.SetPosition(0, 120);

    GuiImageData Address(addressbar_textbox_png);
    snprintf(currentdir, sizeof(currentdir), "%s%s", browser.rootdir, browser.dir);
    GuiText AdressText(currentdir, 20, (GXColor) { 0, 0, 0, 255});
    AdressText.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    AdressText.SetPosition(20, 0);
    AdressText.SetMaxWidth(Address.GetWidth()-40, GuiText::SCROLL);
    GuiImage AdressbarImg(&Address);
    GuiButton Adressbar(Address.GetWidth(), Address.GetHeight());
    Adressbar.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    Adressbar.SetPosition(0, fileBrowser.GetTop()-45);
    Adressbar.SetImage(&AdressbarImg);
    Adressbar.SetLabel(&AdressText);

    //save var in case they cancel and return it to them
    snprintf(currentdir, sizeof(currentdir), "%s", var);
    sprintf(var,"%s", browser.rootdir);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&ExitBtn);
//	w.Append(&titleTxt);
    w.Append(&fileBrowser);
    w.Append(&Adressbar);
    w.Append(&okBtn);
    w.Append(&folderBtn);
    w.Append(&usbBtn);
    mainWindow->Append(&w);
    ResumeGui();

    while (menu == MENU_NONE) {
        VIDEO_WaitVSync();

        if (shutdown == 1)
            Sys_Shutdown();

        if (reset == 1)
            Sys_Reboot();

        for (i=0; i<PAGESIZE; i++) {
            if (fileBrowser.fileList[i]->GetState() == STATE_CLICKED) {
                fileBrowser.fileList[i]->ResetState();
                // check corresponding browser entry
                if (browserList[browser.selIndex].isdir) {
                    if (BrowserChangeFolder()) {
                        fileBrowser.ResetState();
                        fileBrowser.fileList[0]->SetState(STATE_SELECTED);
                        fileBrowser.TriggerUpdate();
                        sprintf(var,"%s", browser.rootdir);
                        int len=strlen(browser.rootdir);
                        for (unsigned int i=len;i<strlen(browser.rootdir)+strlen(browser.dir);i++) {
                            var[i]=browser.dir[i-(len-1)];
                        }
                        AdressText.SetTextf("%s", var);
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

        if (ExitBtn.GetState() == STATE_CLICKED) {
            snprintf(var,sizeof(currentdir),"%s", currentdir);
            break;
        }
        if (okBtn.GetState() == STATE_CLICKED) {
            result = 1;
            break;
        } else if (usbBtn.GetState() == STATE_CLICKED) {
            HaltGui();
            mainWindow->Remove(&w);
            ResumeGui();
            result = BrowseDevice(var, (curDivice==SD?USB:SD));
            break;
        } else if (folderBtn.GetState() == STATE_CLICKED) {
            HaltGui();
            mainWindow->Remove(&w);
            ResumeGui();
            char newfolder[100];
            sprintf(newfolder,"%s/",var);

            int result = OnScreenKeyboard(newfolder,100,0);
            if ( result == 1 ) {
                int len = (strlen(newfolder)-1);
                if (newfolder[len] !='/')
                    strncat (newfolder, "/", 1);

                struct stat st;
                if (stat(newfolder, &st) != 0) {
                    if (subfoldercreate(newfolder) != 1) {
                        WindowPrompt(tr("Error !"),tr("Can't create directory"),tr("OK"));
                    }
                }
            }
            result = BrowseDevice(var, (curDivice==SD?SD:USB));
            break;
        }

    }
    HaltGui();
    mainWindow->Remove(&w);
    ResumeGui();

    //}

    return result;
}

