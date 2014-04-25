#include <dirent.h>
#include <unistd.h>

#include "FileOperations/fileops.h"
#include "Controls/DeviceHandler.hpp"
#include "wad/nandtitle.h"
#include "system/IosLoader.h"
#include "menu/menus.h"
#include "wpad.h"
#include "usbloader/wbfs.h"
#include "usbloader/GameList.h"
#include "settings/GameTitles.h"
#include "xml/GameTDB.hpp"
#include "utils/ShowError.h"

static int FindGamePartition()
{
	int partCount = DeviceHandler::GetUSBPartitionCount();

	// Loop through all WBFS partitions first to check them in case IOS249 Rev < 18
	for(int i = 0; i < partCount; ++i)
	{
		if(DeviceHandler::GetFilesystemType(USB1+i) != PART_FS_WBFS)
			continue;

		if (WBFS_OpenPart(i) == 0)
		{
			Settings.partition = i;
			return 0;
		}
	}

	int firstValidPartition = -1;

	if(IosLoader::IsWaninkokoIOS() && NandTitles.VersionOf(TITLE_ID(1, IOS_GetVersion())) < 18)
		return -1;

	// Loop through FAT/NTFS/EXT partitions, and find the first partition with games on it (if there is one)
	for(int i = 0; i < partCount; ++i)
	{
		if(DeviceHandler::GetFilesystemType(USB1+i) != PART_FS_NTFS &&
		   DeviceHandler::GetFilesystemType(USB1+i) != PART_FS_FAT &&
		   DeviceHandler::GetFilesystemType(USB1+i) != PART_FS_EXT)
		{
			continue;
		}

		if (WBFS_OpenPart(i) != 0)
			continue;

		u32 count;
		// Get the game count...
		WBFS_GetCount(i, &count);

		if (count > 0)
		{
			Settings.partition = i;
			return 0;
		}

		if(firstValidPartition < 0)
			firstValidPartition = i;

		WBFS_Close(i);
	}

	if(firstValidPartition >= 0)
	{
		Settings.partition = firstValidPartition;
		return 0;
	}

	return -1;
}

static int PartitionChoice()
{
	int ret = -1;

	int choice = WindowPrompt(tr( "No WBFS or FAT/NTFS/EXT partition found" ), tr( "You can select or format a partition or use the channel loader mode." ), tr( "Select" ), tr( "Format" ), tr( "Channels" ));
	if (choice == 0)
	{
		Settings.LoaderMode = MODE_NANDCHANNELS;
		return 0;
	}
	else if(choice == 1)
	{
		int part_num = SelectPartitionMenu();
		if(part_num >= 0)
		{
			if(IosLoader::IsWaninkokoIOS() && NandTitles.VersionOf(TITLE_ID(1, IOS_GetVersion())) < 18 &&
			   (DeviceHandler::GetFilesystemType(USB1+part_num) == PART_FS_NTFS ||
				DeviceHandler::GetFilesystemType(USB1+part_num) == PART_FS_FAT ||
				DeviceHandler::GetFilesystemType(USB1+part_num) == PART_FS_EXT))
			{
				WindowPrompt(tr("Warning:"), tr("You are trying to select a FAT32/NTFS/EXT partition with cIOS 249 Rev < 18. This is not supported. Continue on your own risk."), tr("OK"));
			}

			ret = WBFS_OpenPart(part_num);

			Settings.partition = part_num;
			Settings.Save();
		}
	}
	else if(choice == 2)
	{
		while(ret < 0 || ret == -666)
		{
			int part_num = SelectPartitionMenu();
			if(part_num >= 0)
				ret = FormatingPartition(tr( "Formatting, please wait..." ), part_num);
		}
	}

	return ret;
}

/****************************************************************************
 * MountGamePartition
 ***************************************************************************/
int MountGamePartition(bool ShowGUI)
{
	s32 ret = -1;
	gprintf("MountGamePartition()\n");

	s32 wbfsinit = WBFS_Init(WBFS_DEVICE_USB);

	if(Settings.LoaderMode & MODE_WIIGAMES)
	{
		if (wbfsinit < 0)
		{
			if(ShowGUI)
				ShowError("%s %s", tr( "USB Device not initialized." ), tr("Switching to channel list mode."));

			Settings.LoaderMode &= ~MODE_WIIGAMES;
			Settings.LoaderMode |= MODE_NANDCHANNELS;
		}
		else
		{
			if(Settings.MultiplePartitions)
				ret = WBFS_OpenAll();
			else if(!Settings.FirstTimeRun)
				ret = WBFS_OpenPart(Settings.partition);

			if(ret < 0)
				ret = FindGamePartition();

			if(ret < 0)
			{
				if(ShowGUI)
					PartitionChoice();
				else
					Settings.LoaderMode = MODE_NANDCHANNELS;
			}
		}
	}

	gprintf("\tDisc_Init\n");
	ret = Disc_Init();
	if (ret < 0)
	{
		if(ShowGUI)
			WindowPrompt(tr( "Error !" ), tr( "Could not initialize DIP module!" ), tr( "OK" ));
		Sys_LoadMenu();
	}

	gprintf("LoadTitlesFromGameTDB\n");
	//! gameList is loaded in GameTitles.LoadTitlesFromGameTDB after cache file load
	//! for speed up purpose. If titles override active, load game list here.
	if(Settings.titlesOverride)
		GameTitles.LoadTitlesFromGameTDB(Settings.titlestxt_path);
	else
		gameList.LoadUnfiltered();

	return ret;
}
