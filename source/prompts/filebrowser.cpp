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
#include <sys/dirent.h>
#include <sys/iosupport.h>
#include <malloc.h>
#include <algorithm>

#include "menu.h"

#include "themes/CTheme.h"
#include "FileOperations/fileops.h"
#include "language/gettext.h"
#include "PromptWindows.h"
#include "GUI/gui_filebrowser.h"
#include "sys.h"
#include "filebrowser.h"

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

static int curDevice = -1;
static std::vector<BROWSERINFO> browsers;
BROWSERINFO *browser = NULL;

/****************************************************************************
 * FileFilterCallbacks
 * return: 1-visible 0-hidden
 ***************************************************************************/
int noDIRS(BROWSERENTRY *Entry, void* Args)
{
	return !Entry->isdir;
}
int noFILES(BROWSERENTRY *Entry, void* Args)
{
	return Entry->isdir;
}
int noEXT(BROWSERENTRY *Entry, void* Args)
{
	if (!Entry->isdir)
	{
		char *cptr = strrchr(Entry->displayname, '.');
		if (cptr && cptr != Entry->displayname) *cptr = 0;
	}
	return 1;
}

void ResetBrowser(BROWSERINFO *browser);
/****************************************************************************
 * InitBrowsers()
 * Clears the file browser memory, and allocates one initial entry
 ***************************************************************************/
int InitBrowsers()
{
	curDevice = -1;
	browsers.clear();
	browser = NULL;
	char rootdir[ROOTDIRLEN];
	for (int i = 3; i < STD_MAX; i++)
	{
		if (strcmp(devoptab_list[i]->name, "stdnull") && devoptab_list[i]->write_r != NULL)
		{
			snprintf(rootdir, sizeof(rootdir), "%s:/", devoptab_list[i]->name);
			if ( DIR *dir = opendir( rootdir ) )
			{
				closedir(dir);
				BROWSERINFO browser;
				memset(&browser, 0, sizeof(BROWSERINFO));
				strcpy(browser.rootdir, rootdir);
				ResetBrowser(&browser);
				browsers.push_back(browser);
			}
		}
	}
	if (!browsers.size()) return -1;
	curDevice = 0;
	browser = &browsers[curDevice];
	return 0;
}
/****************************************************************************
 * ResetBrowser()
 * Clears the file browser memory, and allocates one initial entry
 ***************************************************************************/
void ResetBrowser(BROWSERINFO *browser)
{
	browser->pageIndex = 0;
	browser->browserList.clear();
	/*
	 // Clear any existing values
	 if (browser->browserList != NULL) {
	 free(browser->browserList);
	 browser->browserList = NULL;
	 }
	 // set aside space for 1 entry
	 browser->browserList = (BROWSERENTRY *)malloc(sizeof(BROWSERENTRY));
	 if(browser->browserList)
	 memset(browser->browserList, 0, sizeof(BROWSERENTRY));
	 */
}

/****************************************************************************
 * FileSortCallback
 *
 * sort callback to sort file entries with the following order:
 *   .
 *   ..
 *   <dirs>
 *   <files>
 ***************************************************************************/
//int FileSortCallback(const void *f1, const void *f2) {
bool operator<(const BROWSERENTRY &f1, const BROWSERENTRY &f2)
{
	/* Special case for implicit directories */
	if (f1.filename[0] == '.' || f2.filename[0] == '.')
	{
		if (strcmp(f1.filename, ".") == 0)
		{
			return true;
		}
		if (strcmp(f2.filename, ".") == 0)
		{
			return false;
		}
		if (strcmp(f1.filename, "..") == 0)
		{
			return true;
		}
		if (strcmp(f2.filename, "..") == 0)
		{
			return false;
		}
	}

	/* If one is a file and one is a directory the directory is first. */
	if (f1.isdir && !(f2.isdir)) return true;
	if (!(f1.isdir) && f2.isdir) return false;

	return stricmp(f1.filename, f2.filename) < 0;
}

int ParseFilter(FILTERCASCADE *Filter, BROWSERENTRY* Entry)
{
	while (Filter)
	{
		if (Filter->filter && Filter->filter(Entry, Filter->filter_args) == 0) return 0;
		Filter = Filter->next;
	}
	return 1;
}
/***************************************************************************
 * Browse subdirectories
 **************************************************************************/
int ParseDirectory(const char* Path, int Flags, FILTERCASCADE *Filter)
{
	DIR *dir = NULL;
	char fulldir[MAXPATHLEN];
	char filename[MAXPATHLEN];
	struct stat filestat;
	unsigned int i;

	if (curDevice == -1) if (InitBrowsers()) return -1; // InitBrowser fails

	if (Path) // note in this codeblock use filename temporary
	{
		strlcpy(fulldir, Path, sizeof(fulldir));
		if (*fulldir && fulldir[strlen(fulldir) - 1] != '/') // a file
		{
			char * chrp = strrchr(fulldir, '/');
			if (chrp) chrp[1] = 0;
		}
		if (strchr(fulldir, ':') == NULL) // Path has no device device
		{
			getcwd(filename, sizeof(filename)); // save the current working dir
			if (*fulldir == 0) // if path is empty
				strlcpy(fulldir, filename, sizeof(fulldir)); //  we use the current working dir
			else
			{ // path is not empty
				if (chdir(fulldir)) // sets the path to concatenate and validate
				{
					if (Flags & (FB_TRYROOTDIR | FB_TRYSTDDEV))
					{
						if (chdir("/") && !(Flags & FB_TRYSTDDEV)) // try to set root if is needed
							return -1;
					}
				}
				if (getcwd(fulldir, sizeof(fulldir))) return -1; // gets the concatenated current working dir
				chdir(filename); // restore the saved cwd
			}
		}
		for (i = 0; i < browsers.size(); i++) // searchs the browser who match the path
		{
			if (strnicmp(fulldir, browsers[i].rootdir, strlen(browsers[i].rootdir) - 1 /*means without trailing '/'*/)
					== 0)
			{
				browser = &browsers[curDevice];
				break;
			}
		}
		if (i != browsers.size()) // found browser
		{
			curDevice = i;
			browser = &browsers[curDevice];
			strcpy(browser->dir, &fulldir[strlen(browser->rootdir)]);
		}
		else if (Flags & FB_TRYSTDDEV)
		{
			curDevice = 0;
			browser = &browsers[curDevice]; // when no browser was found and
			browser->dir[0] = 0; // we alowed try StdDevice and try RootDir
			strlcpy(fulldir, browser->rootdir, sizeof(fulldir)); // set the first browser with root-dir
		}
		else return -1;
	}
	else snprintf(fulldir, sizeof(fulldir), "%s%s", browser->rootdir, browser->dir);

	// reset browser
	ResetBrowser(browser);

	// open the directory
	if ((dir = opendir(fulldir)) == NULL)
	{
		if (Flags & FB_TRYROOTDIR)
		{
			snprintf(fulldir, sizeof(fulldir), browser->rootdir);
			browser->dir[0] = 0;
			if ((dir = opendir(browser->rootdir)) == NULL) return -1;
		}
		else return -1;
	}

	struct dirent *dirent = NULL;
	
	// Adds parent directory ".." manually if in a subdirectory to fix NTFS folder browsing.
	if (strcmp(fulldir, browser->rootdir) != 0)
	{
		snprintf(filename, sizeof(filename), "..");
		
		BROWSERENTRY newEntry;
		memset(&newEntry, 0, sizeof(BROWSERENTRY)); // clear the new entry
		strlcpy(newEntry.filename, filename, sizeof(newEntry.filename));
		strlcpy(newEntry.displayname, filename, sizeof(newEntry.displayname));
		newEntry.isdir = 1; // flag this as a dir
		if (ParseFilter(Filter, &newEntry)) browser->browserList.push_back(newEntry);
	}

	while ((dirent = readdir(dir)) != 0)
	{
		snprintf(filename, sizeof(filename), "%s/%s", fulldir, dirent->d_name);
		if(stat(filename, &filestat) != 0)
			continue;

		snprintf(filename, sizeof(filename), dirent->d_name);

		if (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0)
		{
			BROWSERENTRY newEntry;
			memset(&newEntry, 0, sizeof(BROWSERENTRY)); // clear the new entry
			strlcpy(newEntry.filename, filename, sizeof(newEntry.filename));
			strlcpy(newEntry.displayname, filename, sizeof(newEntry.displayname));
			newEntry.length = filestat.st_size;
			newEntry.isdir = (filestat.st_mode & S_IFDIR) == 0 ? 0 : 1; // flag this as a dir
			if (ParseFilter(Filter, &newEntry)) browser->browserList.push_back(newEntry);
		}
	}

	// close directory
	closedir(dir);

	// Sort the file list
	std::sort(browser->browserList.begin(), browser->browserList.end());
	return 0;
}
int ParseDirectory(int Device, int Flags, FILTERCASCADE *Filter)
{
	if (Device >= 0 && Device < (int) browsers.size())
	{
		int old_curDevice = curDevice;
		curDevice = Device;
		browser = &browsers[curDevice];
		if (ParseDirectory((char*) NULL, Flags, Filter) == 0) return 0;
		curDevice = old_curDevice;
		browser = &browsers[old_curDevice];
	}
	return -1;
}

/****************************************************************************
 * BrowseDevice
 * Displays a list of files on the selected path
 ***************************************************************************/
int BrowseDevice(char * Path, int Path_size, int Flags, FILTERCASCADE *Filter/*=NULL*/)
{
	int result = -1;
	int i;

	if (InitBrowsers() || ParseDirectory(Path, Flags, Filter))
	{
		WindowPrompt(tr( "Error" ), 0, tr( "OK" ));
		return -1;
	}
	int menu = MENU_NONE;

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiImageData folderImgData(Resources::GetFile("icon_folder.png"), Resources::GetFileSize("icon_folder.png"));
	GuiImage folderImg(&folderImgData);
	GuiButton folderBtn(folderImg.GetWidth(), folderImg.GetHeight());
	folderBtn.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	folderBtn.SetPosition(-210, -145);
	folderBtn.SetImage(&folderImg);
	folderBtn.SetTrigger(&trigA);
	folderBtn.SetEffectGrow();

	GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
	GuiText ExitBtnTxt(tr( "Cancel" ), 24, ( GXColor ) {0, 0, 0, 255});
	GuiImage ExitBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		ExitBtnTxt.SetWidescreen(Settings.widescreen);
		ExitBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton ExitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	ExitBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	ExitBtn.SetPosition(-40, -35);
	ExitBtn.SetLabel(&ExitBtnTxt);
	ExitBtn.SetImage(&ExitBtnImg);
	ExitBtn.SetTrigger(&trigA);
	ExitBtn.SetTrigger(&trigB);
	ExitBtn.SetEffectGrow();

	GuiText usbBtnTxt(browsers[(curDevice + 1) % browsers.size()].rootdir, 24, ( GXColor ) {0, 0, 0, 255});
	GuiImage usbBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		usbBtnTxt.SetWidescreen(Settings.widescreen);
		usbBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton usbBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	usbBtn.SetAlignment(ALIGN_CENTER, ALIGN_BOTTOM);
	usbBtn.SetPosition(0, -35);
	usbBtn.SetLabel(&usbBtnTxt);
	usbBtn.SetImage(&usbBtnImg);
	usbBtn.SetTrigger(&trigA);
	usbBtn.SetEffectGrow();

	GuiText okBtnTxt(tr( "OK" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	GuiImage okBtnImg(&btnOutline);
	if (Settings.wsprompt)
	{
		okBtnTxt.SetWidescreen(Settings.widescreen);
		okBtnImg.SetWidescreen(Settings.widescreen);
	}
	GuiButton okBtn(&okBtnImg, &okBtnImg, 0, 4, 40, -35, &trigA, btnSoundOver, btnSoundClick2, 1);
	okBtn.SetLabel(&okBtnTxt);

	GuiFileBrowser fileBrowser(396, 248);
	fileBrowser.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	fileBrowser.SetPosition(0, 120);

	GuiImageData Address(Resources::GetFile("addressbar_textbox.png"), Resources::GetFileSize("addressbar_textbox.png"));
	GuiText AdressText((char*) NULL, 20, ( GXColor ) {0, 0, 0, 255});
	AdressText.SetTextf("%s%s", browser->rootdir, browser->dir);
	AdressText.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	AdressText.SetPosition(20, 0);
	AdressText.SetMaxWidth(Address.GetWidth() - 40, SCROLL_HORIZONTAL);
	GuiImage AdressbarImg(&Address);
	GuiButton Adressbar(Address.GetWidth(), Address.GetHeight());
	Adressbar.SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	Adressbar.SetPosition(0, fileBrowser.GetTop() - 45);
	Adressbar.SetImage(&AdressbarImg);
	Adressbar.SetLabel(&AdressText);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&ExitBtn);
	//  w.Append(&titleTxt);
	w.Append(&fileBrowser);
	w.Append(&Adressbar);
	w.Append(&okBtn);
	if (!(Flags & FB_NOFOLDER_BTN)) w.Append(&folderBtn);
	if (browsers.size() > 1 && !(Flags & FB_NODEVICE_BTN)) w.Append(&usbBtn);
	mainWindow->Append(&w);
	ResumeGui();
	int clickedIndex = -1;
	while (menu == MENU_NONE)
	{
		VIDEO_WaitVSync();

		if (shutdown == 1) Sys_Shutdown();

		if (reset == 1) Sys_Reboot();

		for (i = 0; i < FILEBROWSERSIZE; i++)
		{
			if (fileBrowser.fileList[i]->GetState() == STATE_CLICKED)
			{
				fileBrowser.fileList[i]->ResetState();

				clickedIndex = browser->pageIndex + i;
				bool pathCanged = false;
				// check corresponding browser entry
				if (browser->browserList[clickedIndex].isdir)
				{
					/* go up to parent directory */
					if (strcmp(browser->browserList[clickedIndex].filename, "..") == 0)
					{
						/* remove last subdirectory name */
						int len = strlen(browser->dir);
						while (browser->dir[0] && browser->dir[len - 1] == '/')
							browser->dir[--len] = '\0'; // remove all trailing '/'
						char *cptr = strrchr(browser->dir, '/');
						if (cptr)
							*++cptr = 0;
						else browser->dir[0] = '\0'; // remove trailing dir
						pathCanged = true;
					}
					/* Open a directory */
					/* current directory doesn't change */
					else if (strcmp(browser->browserList[clickedIndex].filename, "."))
					{
						/* test new directory namelength */
						if ((strlen(browser->dir) + strlen(browser->browserList[clickedIndex].filename) + 1/*'/'*/)
								< MAXPATHLEN)
						{
							/* update current directory name */
							sprintf(browser->dir, "%s%s/", browser->dir, browser->browserList[clickedIndex].filename);
							pathCanged = true;
						}
					}
					if (pathCanged)
					{
						LOCK( &fileBrowser );
						ParseDirectory((char*) NULL, Flags, Filter);
						fileBrowser.ResetState();
						fileBrowser.UpdateList();
						AdressText.SetTextf("%s%s", browser->rootdir, browser->dir);
					}
					clickedIndex = -1;
				}
				else /* isFile */
				{
					AdressText.SetTextf("%s%s%s", browser->rootdir, browser->dir,
							browser->browserList[clickedIndex].filename);
				}
			}
		}

		if (ExitBtn.GetState() == STATE_CLICKED)
		{
			result = 0;
			break;
		}
		else if (okBtn.GetState() == STATE_CLICKED)
		{
			if (clickedIndex >= 0)
				snprintf(Path, Path_size, "%s%s%s", browser->rootdir, browser->dir,
						browser->browserList[clickedIndex].filename);
			else snprintf(Path, Path_size, "%s%s", browser->rootdir, browser->dir);
			result = 1;
			break;
		}
		else if (usbBtn.GetState() == STATE_CLICKED)
		{
			usbBtn.ResetState();
			for (u32 i = 1; i < browsers.size(); i++)
			{
				LOCK( &fileBrowser );
				if (ParseDirectory((curDevice + i) % browsers.size(), Flags, Filter) == 0)
				{
					fileBrowser.ResetState();
					fileBrowser.UpdateList();
					AdressText.SetTextf("%s%s", browser->rootdir, browser->dir);
					usbBtnTxt.SetText(browsers[(curDevice + 1) % browsers.size()].rootdir);
					break;
				}
			}
		}
		else if (folderBtn.GetState() == STATE_CLICKED)
		{
			folderBtn.ResetState();

			HaltGui();
			mainWindow->Remove(&w);
			ResumeGui();
			char newfolder[100];
			char oldfolder[100];
			snprintf(newfolder, sizeof(newfolder), "%s%s", browser->rootdir, browser->dir);
			strcpy(oldfolder, newfolder);

			int result = OnScreenKeyboard(newfolder, sizeof(newfolder), strlen(browser->rootdir));
			if (result == 1)
			{
				unsigned int len = strlen(newfolder);
				if (len > 0 && len + 1 < sizeof(newfolder) && newfolder[len - 1] != '/')
				{
					newfolder[len] = '/';
					newfolder[len + 1] = '\0';
				}

				struct stat st;
				if (stat(newfolder, &st) != 0)
				{
					if (WindowPrompt(tr( "Directory does not exist!" ),
							tr( "The entered directory does not exist. Would you like to create it?" ),
							tr( "OK" ), tr( "Cancel" )) == 1) if (CreateSubfolder(newfolder) == false) WindowPrompt(
							tr( "Error !" ), tr( "Can't create directory" ), tr( "OK" ));
				}
				if (ParseDirectory(newfolder, Flags, Filter) == 0)
				{
					fileBrowser.ResetState();
					fileBrowser.UpdateList();
					AdressText.SetTextf("%s%s", browser->rootdir, browser->dir);
					usbBtnTxt.SetText(browsers[(curDevice + 1) % browsers.size()].rootdir);
				}
			}
			HaltGui();
			mainWindow->Append(&w);
			ResumeGui();
		}

	}
	HaltGui();
	mainWindow->Remove(&w);
	ResumeGui();

	//}

	return result;
}

int BrowseDevice(char * Path, int Path_size, int Flags, FILEFILTERCALLBACK Filter, void *FilterArgs)
{
	if (Filter)
	{
		FILTERCASCADE filter = { Filter, FilterArgs, NULL };
		return BrowseDevice(Path, Path_size, Flags, &filter);
	}
	return BrowseDevice(Path, Path_size, Flags);
}
