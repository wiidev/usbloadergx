/****************************************************************************
 * Copyright (C) 2014 Cyan
 * Copyright (C) 2011 Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include <unistd.h>
#include <sys/statvfs.h>
#include "HardDriveSM.hpp"
#include "Controls/DeviceHandler.hpp"
#include "settings/CSettings.h"
#include "settings/meta.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "usbloader/GameList.h"
#include "usbloader/wbfs.h"
#include "prompts/ProgressWindow.h"
#include "settings/GameTitles.h"
#include "system/IosLoader.h"
#include "wad/nandtitle.h"
#include "utils/tools.h"

static const char * OnOffText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" )
};

static const char * InstallToText[] =
{
	trNOOP( "None" ),
	trNOOP( "GAMEID_Gamename" ),
	trNOOP( "Gamename [GAMEID]" )
};

static const char * SplitSizeText[] =
{
	trNOOP( "No Splitting" ),
	trNOOP( "Split each 2GB" ),
	trNOOP( "Split each 4GB" ),
};

static inline bool IsValidPartition(int fs_type, int cios)
{
	if (IosLoader::IsWaninkokoIOS(cios) && NandTitles.VersionOf(TITLE_ID(1, cios)) < 18)
	{
		return fs_type == PART_FS_WBFS;
	}
	else
	{
		return fs_type == PART_FS_WBFS || fs_type == PART_FS_FAT || fs_type == PART_FS_NTFS || fs_type == PART_FS_EXT;
	}
}

HardDriveSM::HardDriveSM()
	: SettingsMenu(tr("Hard Drive Settings"), &GuiOptions, MENU_NONE)
{
	int Idx = 0;
	Options->SetName(Idx++, "%s", tr( "Game/Install Partition" ));
	Options->SetName(Idx++, "%s", tr( "Multiple Partitions" ));
	Options->SetName(Idx++, "%s", tr( "USB Port" ));
	Options->SetName(Idx++, "%s", tr( "Mount USB at launch" ));
	Options->SetName(Idx++, "%s", tr( "Install Directories" ));
	Options->SetName(Idx++, "%s", tr( "Game Split Size" ));
	Options->SetName(Idx++, "%s", tr( "Install Partitions" ));
	Options->SetName(Idx++, "%s", tr( "GC Install Compressed" ));
	Options->SetName(Idx++, "%s", tr( "GC Install 32K Aligned" ));
	Options->SetName(Idx++, "%s", tr( "Sync FAT32 FS Info" ));

	OldSettingsPartition = Settings.partition;
	OldSettingsMultiplePartitions = Settings.MultiplePartitions;
	NewSettingsUSBPort = Settings.USBPort;
	oldSettingsUSBAutoMount = Settings.USBAutoMount;

	SetOptionValues();
}

HardDriveSM::~HardDriveSM()
{
	//! if partition has changed, Reinitialize it
	if (Settings.partition != OldSettingsPartition ||
		Settings.MultiplePartitions != OldSettingsMultiplePartitions ||
		Settings.USBPort != NewSettingsUSBPort || 
		Settings.USBAutoMount != oldSettingsUSBAutoMount)
	{
		WBFS_CloseAll();

		if(Settings.USBPort != NewSettingsUSBPort)
		{
			DeviceHandler::Instance()->UnMountAllUSB();
			Settings.USBPort = NewSettingsUSBPort;
			DeviceHandler::Instance()->MountAllUSB();

			if(Settings.partition >= DeviceHandler::GetUSBPartitionCount())
				Settings.partition = 0;

			// set -1 to edit meta.xml arguments
			NewSettingsUSBPort = -1;
		}

		WBFS_Init(WBFS_DEVICE_USB);
		if(Settings.MultiplePartitions)
			WBFS_OpenAll();
		else
			WBFS_OpenPart(Settings.partition);

		//! Reload the new game titles
		gameList.ReadGameList();
		gameList.LoadUnfiltered();
		GameTitles.LoadTitlesFromGameTDB(Settings.titlestxt_path, false);
		
		if(oldSettingsUSBAutoMount != Settings.USBAutoMount || NewSettingsUSBPort == -1)
		{
			// edit meta.xml arguments
			editMetaArguments();
		}
	}
}

void HardDriveSM::SetOptionValues()
{
	int Idx = 0;

	//! Settings: Game/Install Partition
	PartitionHandle * usbHandle = DeviceHandler::Instance()->GetUSBHandleFromPartition(Settings.partition);
	int checkPart = DeviceHandler::PartitionToPortPartition(Settings.partition);

	//! Get the partition name and it's size in GB's
	if(usbHandle)
		Options->SetValue(Idx++, "%s (%.2fGB)", usbHandle->GetFSName(checkPart), usbHandle->GetSize(checkPart)/GB_SIZE);
	else
		Options->SetValue(Idx++, tr("Not Initialized"));

	//! Settings: Multiple Partitions
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.MultiplePartitions] ));

	//! Settings: USB Port
	if(NewSettingsUSBPort == 2)
		Options->SetValue(Idx++, tr("Both Ports"));
	else
		Options->SetValue(Idx++, "%i", NewSettingsUSBPort);

	//! Settings: Auto Mount USB at launch
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.USBAutoMount] ));

	//! Settings: Install directories
	Options->SetValue(Idx++, "%s", tr( InstallToText[Settings.InstallToDir] ));

	//! Settings: Game Split Size
	Options->SetValue(Idx++, "%s", tr( SplitSizeText[Settings.GameSplit] ));

	//! Settings: Install partitions
	if(Settings.InstallPartitions == ONLY_GAME_PARTITION)
		Options->SetValue(Idx++, "%s", tr("Only Game Partition"));
	else if(Settings.InstallPartitions == ALL_PARTITIONS)
		Options->SetValue(Idx++, "%s", tr("All Partitions"));
	else if(Settings.InstallPartitions == REMOVE_UPDATE_PARTITION)
		Options->SetValue(Idx++, "%s", tr("Remove update"));

	//! Settings: GC Install Compressed
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.GCInstallCompressed] ));

	//! Settings: GC Install 32K Aligned
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.GCInstallAligned] ));

	//! Settings: Sync FAT32 FS Info
	Options->SetValue(Idx++, " ");
}

int HardDriveSM::GetMenuInternal()
{
	int ret = optionBrowser->GetClickedOption();

	if (ret < 0)
		return MENU_NONE;

	int Idx = -1;

	//! Settings: Game/Install Partition
	if (ret == ++Idx)
	{
		// Init the USB device if mounted after launch.
		PartitionHandle * usbHandle = DeviceHandler::Instance()->GetUSBHandleFromPartition(Settings.partition);
		if(usbHandle == NULL)
			DeviceHandler::Instance()->MountAllUSB(true);

		// Select the next valid partition, even if that's the same one
		int fs_type = 0;
		int ios = IOS_GetVersion();
		int retries = 20;
		do
		{
			Settings.partition = (Settings.partition + 1) % DeviceHandler::GetUSBPartitionCount();
			fs_type = DeviceHandler::GetFilesystemType(USB1+Settings.partition);
		}
		while (!IsValidPartition(fs_type, ios) && --retries > 0);

		if(fs_type == PART_FS_FAT && Settings.GameSplit == GAMESPLIT_NONE)
			Settings.GameSplit = GAMESPLIT_4GB;
	}

	//! Settings: Multiple Partitions
	else if (ret == ++Idx)
	{
		if (++Settings.MultiplePartitions >= MAX_ON_OFF) Settings.MultiplePartitions = 0;
	}

	//! Settings: USB Port
	else if (ret == ++Idx)
	{
		if(!IosLoader::IsHermesIOS() && !IosLoader::IsD2X())
		{
			WindowPrompt(tr("ERROR:"), tr("USB Port changing is only supported on Hermes cIOS."), tr("OK"));
			NewSettingsUSBPort = 0;
			Settings.USBPort = 0;
		}

		else if (++NewSettingsUSBPort >= 3) // 2 = both ports
			NewSettingsUSBPort = 0;
	}

	//! Settings: Auto mount USB at launch
	else if (ret == ++Idx)
	{
		if (++Settings.USBAutoMount >= MAX_ON_OFF) Settings.USBAutoMount = 0;
	}

	//! Settings: Install directories
	else if (ret == ++Idx)
	{
		if (++Settings.InstallToDir >= INSTALL_TO_MAX) Settings.InstallToDir = 0;
	}

	//! Settings: Game Split Size
	else if (ret == ++Idx)
	{
		if (++Settings.GameSplit >= GAMESPLIT_MAX)
		{
			if(DeviceHandler::GetFilesystemType(USB1+Settings.partition) == PART_FS_FAT)
				Settings.GameSplit = GAMESPLIT_2GB;
			else
				Settings.GameSplit = GAMESPLIT_NONE;
		}
	}

	//! Settings: Install partitions
	else if (ret == ++Idx)
	{
		switch(Settings.InstallPartitions)
		{
			case ONLY_GAME_PARTITION:
				Settings.InstallPartitions = ALL_PARTITIONS;
				break;
			case ALL_PARTITIONS:
				Settings.InstallPartitions = REMOVE_UPDATE_PARTITION;
				break;
			default:
			case REMOVE_UPDATE_PARTITION:
				Settings.InstallPartitions = ONLY_GAME_PARTITION;
				break;
		}
	}

	//! Settings: GC Install Compressed
	else if (ret == ++Idx)
	{
		if (++Settings.GCInstallCompressed >= MAX_ON_OFF) Settings.GCInstallCompressed = 0;
	}

	//! Settings: GC Install 32K Aligned
	else if (ret == ++Idx)
	{
		if (++Settings.GCInstallAligned >= MAX_ON_OFF) Settings.GCInstallAligned = 0;
	}

	//! Settings: Sync FAT32 FS Info
	else if (ret == ++Idx )
	{
		int choice = WindowPrompt(0, tr("Do you want to sync free space info sector on all FAT32 partitions?"), tr("Yes"), tr("Cancel"));
		if(choice)
		{
			StartProgress(tr("Synchronizing..."), tr("Please wait..."), 0, false, false);
			int partCount = DeviceHandler::GetUSBPartitionCount();
			for(int i = 0; i < partCount; ++i)
			{
				ShowProgress(i, partCount);
				if(DeviceHandler::GetFilesystemType(USB1+i) == PART_FS_FAT)
				{
					PartitionHandle *usb = DeviceHandler::Instance()->GetUSBHandleFromPartition(i);
					if(!usb) continue;
					struct statvfs stats;
					char drive[20];
					snprintf(drive, sizeof(drive), "%s:/", usb->MountName(i));
					memset(&stats, 0, sizeof(stats));
					memcpy(&stats.f_flag, "SCAN", 4);
					statvfs(drive, &stats);
				}
			}
			ProgressStop();
		}
	}

	SetOptionValues();

	return MENU_NONE;
}
