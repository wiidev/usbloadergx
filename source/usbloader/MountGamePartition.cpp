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
#include "xml/WiiTDB.hpp"

static int FindGamePartition()
{
    PartitionHandle * usbHandle = DeviceHandler::Instance()->GetUSBHandle();
    // Loop through all WBFS partitions first to check them in case IOS249 Rev < 18
    for(int i = 0; i < usbHandle->GetPartitionCount(); ++i)
    {
        if(strncmp(usbHandle->GetFSName(i), "WBFS", 4) != 0)
            continue;

        if (WBFS_OpenPart(i) == 0)
        {
            Settings.partition = i;
            return 0;
        }
    }


    if(IosLoader::IsWaninkokoIOS() && NandTitles.VersionOf(TITLE_ID(1, IOS_GetVersion())) < 18)
        return -1;

    // Loop through FAT/NTFS/EXT partitions, and find the first partition with games on it (if there is one)
    for(int i = 0; i < usbHandle->GetPartitionCount(); ++i)
    {
        if(strncmp(usbHandle->GetFSName(i), "NTFS", 4) != 0 &&
           strncmp(usbHandle->GetFSName(i), "FAT", 3) != 0 &&
           strncmp(usbHandle->GetFSName(i), "LINUX", 5) != 0)
        {
            continue;
        }

        if (WBFS_OpenPart(i) != 0)
            continue;

        u32 count;
        // Get the game count...
        WBFS_GetCount(&count);

        if (count > 0)
        {
            Settings.partition = i;
            return 0;
        }

        WBFS_Close();
    }

    return -1;
}

static int PartitionChoice()
{
    int ret = -1;
    PartitionHandle * usbHandle = DeviceHandler::Instance()->GetUSBHandle();

    int choice = WindowPrompt(tr( "No WBFS or FAT/NTFS/EXT partition found" ),
            tr( "You need to select or format a partition" ), tr( "Select" ), tr( "Format" ), tr( "Return" ));

    if (choice == 0)
    {
        Sys_LoadMenu();
    }
    else if(choice == 1)
    {
        int part_num = SelectPartitionMenu();
        if(part_num >= 0)
        {
            if(IosLoader::IsWaninkokoIOS() && NandTitles.VersionOf(TITLE_ID(1, IOS_GetVersion())) < 18 &&
               (strncmp(usbHandle->GetFSName(part_num), "NTFS", 4) == 0 ||
                strncmp(usbHandle->GetFSName(part_num), "FAT", 3) == 0 ||
                strncmp(usbHandle->GetFSName(part_num), "LINUX", 5) == 0))
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
    gprintf("MountGamePartition()\n");

    s32 wbfsinit = MountWBFS(ShowGUI);
    if (wbfsinit < 0)
    {
        if(ShowGUI) WindowPrompt(tr( "Error !" ), tr( "USB Device not found" ), tr( "OK" ));
        Sys_LoadMenu();
    }

    s32 ret = WBFS_OpenPart(Settings.partition);
    if(ret < 0)
        ret = FindGamePartition();

    if (ret < 0 && ShowGUI)
        ret = PartitionChoice();

    if(ret < 0)
        Sys_LoadMenu();

    gprintf("\tDisc_Init\n");
    ret = Disc_Init();
    if (ret < 0)
    {
        if(ShowGUI)
            WindowPrompt(tr( "Error !" ), tr( "Could not initialize DIP module!" ), tr( "OK" ));
        Sys_LoadMenu();
    }

    gprintf("\tOpenXMLDatabase\n");

    GameTitles.LoadTitlesFromWiiTDB(Settings.titlestxt_path);

    return ret;
}
