#include <dirent.h>
#include <unistd.h>

#include "menus.h"
#include "wpad.h"
#include "fatmounter.h"
#include "usbloader/getentries.h"
#include "usbloader/wbfs.h"

extern bool load_from_fat;
extern char game_partition[6];
extern char headlessID[8];

/****************************************************************************
 * MenuCheck
 ***************************************************************************/
int MenuCheck() {
	gprintf("\nMenuCheck()");
    int menu = MENU_NONE;
    int i = 0;
    int choice;
    s32 ret2, wbfsinit;
    OptionList options;
    options.length = i;

    VIDEO_WaitVSync ();

    wbfsinit = WBFS_Init(WBFS_DEVICE_USB);
    if (wbfsinit < 0) {
        ret2 = WindowPrompt(tr("No USB Device found."), tr("Do you want to retry for 30 secs?"), "cIOS249", "cIOS222", tr("Back to Wii Menu"));
        SDCard_deInit();
        USBDevice_deInit();
        WPAD_Flush(0);
        WPAD_Disconnect(0);
        WPAD_Shutdown();
        if (ret2 == 1) {
            Settings.cios = ios249;
        } else if (ret2 == 2) {
            Settings.cios = ios222;
        } else {
            Sys_LoadMenu();
        }
        ret2 = DiscWait(tr("No USB Device"), tr("Waiting for USB Device"), 0, 0, 1);
        //reinitialize SD and USB
        Wpad_Init();
        WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
        WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
        if (ret2 < 0) {
            WindowPrompt (tr("Error !"),tr("USB Device not found"), tr("OK"));
            Sys_LoadMenu();
        }
    }

	ret2 = -1;
	memset(game_partition, 0, 6);
	load_from_fat = false;

	extern PartList partitions;
	// Added for slow HDD
	for (int runs = 0; runs < 10; runs++) {
		if (Partition_GetList(WBFS_DEVICE_USB, &partitions) != 0) {
			sleep(1);
			continue;
		}

		if (Settings.partition != -1 && partitions.num > Settings.partition) {
			PartInfo pinfo = partitions.pinfo[Settings.partition];
			ret2 = WBFS_OpenPart(pinfo.fs_type == FS_TYPE_FAT32, pinfo.fat_i, partitions.pentry[Settings.partition].sector, partitions.pentry[Settings.partition].size, (char *) &game_partition);

			if (ret2 == 0)
			{
				load_from_fat = pinfo.fs_type == FS_TYPE_FAT32;
				break;
			}
		}

		if (partitions.wbfs_n != 0) {
			ret2 = WBFS_Open();
			for (int p = 0; p < partitions.num; p++) {
				if (partitions.pinfo[p].fs_type == FS_TYPE_WBFS) {
					Settings.partition = p;
					break;
				}
			}
		} else if (Sys_IsHermes() && partitions.fat_n != 0) {
			// Loop through FAT partitions, and find the first partition with games on it (if there is one)
			u32 count;
			for (int i = 0; i < partitions.num; i++) {
				if (partitions.pinfo[i].fs_type == FS_TYPE_FAT32) {
					if (!WBFS_OpenPart(1, partitions.pinfo[i].fat_i, partitions.pentry[i].sector, partitions.pentry[i].size, (char *) &game_partition)) {
						// Get the game count...
						WBFS_GetCount(&count);

						if (count > 0) {
							load_from_fat = true;
							Settings.partition = i;
							break;
						} else {
							WBFS_Close();
						}
					}
				}
			}
		}

		if (ret2 >= 0 || load_from_fat) {
			cfg_save_global();
			break;
		}
		sleep(1);
	}

    if (ret2 < 0 && !load_from_fat) {
        choice = WindowPrompt(tr("No WBFS or FAT game partition found"),tr("You need to select or format a partition"), tr("Select"), tr("Format"), tr("Return"));
        if (choice == 0) {
            Sys_LoadMenu();
        } else {
			load_from_fat = choice == 1;
            menu = MENU_FORMAT;
        }
    }

    ret2 = Disc_Init();
    if (ret2 < 0) {
        WindowPrompt (tr("Error !"),tr("Could not initialize DIP module!"),tr("OK"));
        Sys_LoadMenu();
    }

    if (shutdown == 1)
        Sys_Shutdown();
    if (reset == 1)
        Sys_Reboot();

    if (wbfsinit < 0) {
        sleep(1);
    }

	// open database if needed, load titles if needed
	OpenXMLDatabase(Settings.titlestxt_path,Settings.db_language, Settings.db_JPtoEN, true, Settings.titlesOverride==1?true:false, true);

    // titles.txt loaded after database to override database titles with custom titles
    //snprintf(pathname, sizeof(pathname), "%stitles.txt", Settings.titlestxt_path);
    //cfg_parsefile(pathname, &title_set);

    //Spieleliste laden
    __Menu_GetEntries(0);

    if (strcmp(headlessID,"")!=0)
		menu = MENU_EXIT;

    if (menu == MENU_NONE)
        menu = MENU_DISCLIST;

    //for HDDs with issues
    if (wbfsinit < 0) {
        sleep(1);
        USBDevice_Init();
        SDCard_Init();
    }

    return menu;
}
