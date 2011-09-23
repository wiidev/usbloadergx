#include "menus.h"
#include "usbloader/usbstorage2.h"
#include "usbloader/wbfs.h"
#include "usbloader/disc.h"
#include "usbloader/utils.h"
#include "usbloader/GameList.h"
#include "prompts/ProgressWindow.h"
#include "themes/CTheme.h"

u64 gamesize = 0;

/****************************************************************************
 * MenuInstall
 ***************************************************************************/

int MenuInstall()
{
	int ios_ver = IOS_GetVersion();
	if(ios_ver < 200)
	{
		char text[100];
		snprintf(text, sizeof(text), "%s %i.", tr("You are currently using IOS"), ios_ver);
		WindowPrompt(text, tr("The game installation is disabled under this IOS because of instability in usb write."), tr("OK"));
		return MENU_DISCLIST;
	}

	gprintf("\nMenuInstall()");

	static struct discHdr headerdisc ATTRIBUTE_ALIGN( 32 );

	Disc_SetUSB(NULL);

	int ret, choice = 0;
	char name[200];

	ret = DiscWait(tr( "Insert Disk" ), tr( "Waiting..." ), tr( "Cancel" ), 0, 0);
	if (ret < 0)
	{
		WindowPrompt(tr( "Error reading Disc" ), 0, tr( "Back" ));
		return MENU_DISCLIST;
	}
	ret = Disc_Open();
	if (ret < 0)
	{
		WindowPrompt(tr( "Could not open Disc" ), 0, tr( "Back" ));
		return MENU_DISCLIST;
	}

	ret = Disc_IsWii();
	if (ret < 0)
	{
		choice = WindowPrompt(tr( "Not a Wii Disc" ), tr( "Insert a Wii Disc!" ), tr( "OK" ), tr( "Back" ));

		if (choice == 1)
			return MENU_INSTALL;
		else
			return MENU_DISCLIST;
	}

	Disc_ReadHeader(&headerdisc);
	snprintf(name, sizeof(name), "%s", headerdisc.title);

	ret = WBFS_CheckGame(headerdisc.id);
	if (ret)
	{
		WindowPrompt(tr( "Game is already installed:" ), name, tr( "Back" ));
		return MENU_DISCLIST;
	}

	f32 freespace, used;

	WBFS_DiskSpace(&used, &freespace);
	gamesize = WBFS_EstimeGameSize();

	char gametxt[50];

	sprintf(gametxt, "%s : %.2fGB", name, gamesize/GB_SIZE);

	wiilight(1);
	choice = WindowPrompt(tr( "Continue to install game?" ), gametxt, tr( "OK" ), tr( "Cancel" ));

	if (choice == 1)
	{
		sprintf(gametxt, "%s", tr( "Installing game:" ));

		if (gamesize/GB_SIZE > freespace)
		{
			char errortxt[50];
			sprintf(errortxt, "%s: %.2fGB, %s: %.2fGB", tr( "Game Size" ), gamesize/GB_SIZE, tr( "Free Space" ), freespace);
			WindowPrompt(tr( "Not enough free space!" ), errortxt, tr( "OK" ));
		}
		else
		{
			StartProgress(gametxt, name, 0, true, true);
			ret = WBFS_AddGame();
			ProgressStop();
			wiilight(0);
			if (ret != 0)
			{
				WindowPrompt(tr( "Install Error!" ), 0, tr( "Back" ));
			}
			else
			{
				ShowProgress(tr("Install finished"), name, tr("Reloading game list now, please wait..."), gamesize, gamesize, true, true);
				gameList.ReadGameList(); //get the entries again
				gameList.FilterList();
				GuiSound * instsuccess = NULL;
				bgMusic->Pause();
				instsuccess = new GuiSound(Resources::GetFile("success.ogg"), Resources::GetFileSize("success.ogg"), Settings.sfxvolume);
				instsuccess->SetVolume(Settings.sfxvolume);
				instsuccess->SetLoop(0);
				instsuccess->Play();
				WindowPrompt(tr( "Successfully installed:" ), name, tr( "OK" ));
				instsuccess->Stop();
				delete instsuccess;
				bgMusic->Resume();
			}
		}
	}

	//Turn off the WiiLight
	wiilight(0);
	gamesize = 0;

	return MENU_DISCLIST;
}
