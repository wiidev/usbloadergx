#include <ogcsys.h>
#include <unistd.h>
#include <time.h>

#include "Controls/DeviceHandler.hpp"
#include "usbloader/usbstorage2.h"
#include "wbfs.h"
#include "usbloader/wbfs/wbfs_base.h"
#include "usbloader/wbfs/wbfs_wbfs.h"
#include "usbloader/wbfs/wbfs_fat.h"
#include "usbloader/wbfs/wbfs_ntfs.h"
#include "usbloader/wbfs/wbfs_ext.h"

#include "usbloader/GameList.h"
#include "menu/menus.h"
#include "gecko.h"

static Wbfs *current = NULL;
/* WBFS device */
s32 wbfsDev = WBFS_MIN_DEVICE;
u32 wbfs_part_idx = 0;

wbfs_disc_t* WBFS_OpenDisc(u8 *discid)
{
    return current->OpenDisc(discid);
}

void WBFS_CloseDisc(wbfs_disc_t *disc)
{
    current->CloseDisc(disc);
}

wbfs_t *GetHddInfo(void)
{
    return current->GetHddInfo();
}

s32 WBFS_Init(u32 device)
{
    return Wbfs::Init(device);
}

s32 WBFS_OpenPart(int part_num)
{
    PartitionHandle * usbHandle = DeviceHandler::Instance()->GetUSBHandle();
    if(part_num < 0 || part_num > usbHandle->GetPartitionCount())
        return -1;

    // close
    WBFS_Close();

    gprintf("\tWBFS_OpenPart: start sector %u, sector count: %u\n", usbHandle->GetLBAStart(part_num), usbHandle->GetSecCount(part_num));

    if (strncmp(usbHandle->GetFSName(part_num), "FAT", 3) == 0)
    {
        current = new Wbfs_Fat(wbfsDev, usbHandle->GetLBAStart(part_num), usbHandle->GetSecCount(part_num));
    }
    else if (strncmp(usbHandle->GetFSName(part_num), "NTFS", 4) == 0)
    {
        current = new Wbfs_Ntfs(wbfsDev, usbHandle->GetLBAStart(part_num), usbHandle->GetSecCount(part_num));
    }
    else if (strncmp(usbHandle->GetFSName(part_num), "LINUX", 5) == 0)
    {
        current = new Wbfs_Ext(wbfsDev, usbHandle->GetLBAStart(part_num), usbHandle->GetSecCount(part_num));
    }
    else if (strncmp(usbHandle->GetFSName(part_num), "WBFS", 4) == 0)
    {
        current = new Wbfs_Wbfs(wbfsDev, usbHandle->GetLBAStart(part_num), usbHandle->GetSecCount(part_num));
    }
    if (current->Open())
    {
        delete current;
        current = NULL;
        return -1;
    }

    wbfs_part_idx = usbHandle->GetPartitionNum(part_num);

    return 0;
}

bool WBFS_Close(void)
{
    if (current != NULL)
    {
        current->Close();
        delete current;
        current = NULL;
    }

    gameList.clear();

    return 0;
}

bool WBFS_Mounted()
{
    return (current != NULL && current->IsMounted());
}

s32 WBFS_Format(u32 lba, u32 size)
{
    return current->Format();
}

s32 WBFS_GetCount(u32 *count)
{
    return current->GetCount(count);
}

s32 WBFS_GetHeaders(struct discHdr *outbuf, u32 cnt, u32 len)
{
    return current->GetHeaders(outbuf, cnt, len);
}

s32 WBFS_CheckGame(u8 *discid)
{
    return current->CheckGame(discid);
}

s32 WBFS_AddGame(void)
{
    s32 retval = current->AddGame();
    if (retval == 0) gameList.clear();

    return retval;
}

s32 WBFS_RemoveGame(u8 *discid)
{
    s32 retval = current->RemoveGame(discid);
    if (retval == 0) gameList.clear();

    return retval;
}

s32 WBFS_GameSize(u8 *discid, f32 *size)
{
    return current->GameSize(discid, size);
}

s32 WBFS_DiskSpace(f32 *used, f32 *free)
{
    return current->DiskSpace(used, free);
}

s32 WBFS_RenameGame(u8 *discid, const void *newname)
{
    s32 retval = current->RenameGame(discid, newname);
    if (retval == 0) gameList.clear();

    return retval;
}

s32 WBFS_ReIDGame(u8 *discid, const void *newID)
{
    s32 retval = current->ReIDGame(discid, newID);
    if (retval == 0) gameList.clear();

    return retval;
}

f32 WBFS_EstimeGameSize(void)
{
    return current->EstimateGameSize();
}

int WBFS_GetFragList(u8 *id)
{
    return current->GetFragList(id);
}

bool WBFS_ShowFreeSpace(void)
{
    return current->ShowFreeSpace();
}

int MountWBFS(bool ShowGUI)
{
    if(ShowGUI)
        return WBFS_Init(WBFS_DEVICE_USB);

    int ret = -1;
    time_t currTime = time(0);

    while (time(0) - currTime < 30)
    {
        ret = WBFS_Init(WBFS_DEVICE_USB);
        printf("%i...", int(time(0) - currTime));
        if (ret < 0)
            sleep(1);
        else
			break;

        DeviceHandler::Instance()->UnMountAllUSB();
        DeviceHandler::Instance()->MountAllUSB();
    }

    printf("\n");

    return ret;
}
