#include <dirent.h>
#include <unistd.h>

#include "FileOperations/fileops.h"
#include "wad/nandtitle.h"
#include "system/IosLoader.h"
#include "menus.h"
#include "wpad.h"
#include "fatmounter.h"
#include "usbloader/wbfs.h"
#include "xml/xml.h"

extern int load_from_fs;
extern char game_partition[6];
extern PartList partitions;

static int FindGamesPartition(PartList * partitions)
{
    if (partitions->wbfs_n != 0)
    {
        WBFS_Open();

		for (int p = 0; p < partitions->num; p++)
        {
            if (partitions->pinfo[p].fs_type == FS_TYPE_WBFS)
            {
                Settings.partition = p;
                load_from_fs = PART_FS_WBFS;
                return 0;
            }
        }
    }


    if(IosLoader::IsWaninkokoIOS() && NandTitles.VersionOf(TITLE_ID(1, IOS_GetVersion())) < 18)
        return -1;

    // Loop through FAT/NTFS partitions, and find the first partition with games on it (if there is one)
    for (int i = 0; i < partitions->num; i++)
    {
        if (partitions->pinfo[i].fs_type == FS_TYPE_FAT32 || partitions->pinfo[i].fs_type == FS_TYPE_NTFS)
        {
            if (!WBFS_OpenPart(partitions->pinfo[i].part_fs, partitions->pinfo[i].index,
                    partitions->pentry[i].sector, partitions->pentry[i].size, (char *) &game_partition))
            {
                u32 count;
                // Get the game count...
                WBFS_GetCount(&count);

                if (count > 0)
                {
                    load_from_fs = partitions->pinfo[i].part_fs;
                    Settings.partition = i;
                    return 0;
                }
                else
                {
                    WBFS_Close();
                }
            }
        }
    }

    return -1;
}

static int PartitionChoice()
{
    int ret = -1;

    int choice = WindowPrompt(tr( "No WBFS or FAT/NTFS partition found" ),
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
            if(IosLoader::IsWaninkokoIOS() && NandTitles.VersionOf(TITLE_ID(1, IOS_GetVersion())) < 18
                && (partitions.pinfo[part_num].part_fs == FS_TYPE_FAT32 || partitions.pinfo[part_num].part_fs == FS_TYPE_NTFS))
                WindowPrompt(tr("Warning:"), tr("You are trying to select a FAT32/NTFS partition with cIOS 249 Rev < 18. This is not supported. Continue on your own risk."), tr("OK"));

            ret = WBFS_OpenPart(partitions.pinfo[part_num].part_fs, partitions.pinfo[part_num].index, partitions.pentry[part_num].sector, partitions.pentry[part_num].size, (char *) &game_partition);

            load_from_fs = partitions.pinfo[part_num].part_fs;
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
                ret = FormatingPartition(tr( "Formatting, please wait..." ), &partitions.pentry[part_num]);
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
        WindowPrompt(tr( "Error !" ), tr( "USB Device not found" ), tr( "OK" ));
        Sys_LoadMenu();
    }

    s32 ret = -1;
    memset(game_partition, 0, 6);
    load_from_fs = -1;

    gprintf("\tPartition_GetList\n");
    // Added for slow HDD
    for (int retries = 10; retries > 0; retries--)
    {
        if (Partition_GetList(WBFS_DEVICE_USB, &partitions) == 0)
            break;

        sleep(1);
    }

    gprintf("\tWBFS_OpenPart: start sector %u, sector count: %u\n", partitions.pentry[Settings.partition].sector, partitions.pentry[Settings.partition].size);
    if (Settings.partition != -1 && partitions.num > Settings.partition)
    {
        PartInfo pinfo = partitions.pinfo[Settings.partition];
        if (!WBFS_OpenPart(pinfo.part_fs, pinfo.index, partitions.pentry[Settings.partition].sector,
                partitions.pentry[Settings.partition].size, (char *) &game_partition))
        {
            ret = 0;
            load_from_fs = pinfo.part_fs;
        }
    }

    if(ret < 0)
        ret = FindGamesPartition(&partitions);

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
    // open database if needed, load titles if needed
    if (CheckFile(Settings.titlestxt_path))
        OpenXMLDatabase(Settings.titlestxt_path, Settings.db_language, Settings.db_JPtoEN, true, Settings.titlesOverride, true);

    gprintf("MountGamePartition() return: %i\n", ret);

    return ret;
}
