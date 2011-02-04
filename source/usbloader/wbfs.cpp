#include <vector>
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

#define VALID(x) (x >= 0 && x < (int) WbfsList.size() && WbfsList[x] != NULL)

static std::vector<Wbfs *> WbfsList;

wbfs_disc_t* WBFS_OpenDisc(u8 *discid)
{
    if(!discid) return NULL;

    int part = gameList.GetPartitionNumber(discid);
    if(VALID(part))
        return WbfsList[part]->OpenDisc(discid);

    return NULL;
}

void WBFS_CloseDisc(wbfs_disc_t *disc)
{
    if(!disc) return;

    struct discHdr * header = (struct discHdr *) disc->header;
    int part_num = gameList.GetPartitionNumber(header->id);
    if(!VALID(part_num))
        return;

    WbfsList[part_num]->CloseDisc(disc);
}

s32 WBFS_Init(u32 device)
{
    return Wbfs::Init(device);
}

s32 WBFS_OpenAll()
{
    int ret = -1;

    PartitionHandle * usbHandle = DeviceHandler::Instance()->GetUSBHandle();

    for(int i = 0; i < usbHandle->GetPartitionCount(); ++i)
    {
        if(WBFS_OpenPart(i) == 0)
            ret = 0;
    }

    return ret;
}

s32 WBFS_OpenPart(int part_num)
{
    PartitionHandle * usbHandle = DeviceHandler::Instance()->GetUSBHandle();
    if(part_num < 0 || part_num >= usbHandle->GetPartitionCount())
        return -1;

    // close
    WBFS_Close(part_num);

    if(part_num >= (int) WbfsList.size())
        WbfsList.resize(part_num+1);

    gprintf("\tWBFS_OpenPart: start sector %u, sector count: %u\n", usbHandle->GetLBAStart(part_num), usbHandle->GetSecCount(part_num));

    if (strncmp(usbHandle->GetFSName(part_num), "FAT", 3) == 0)
    {
        WbfsList[part_num] = new Wbfs_Fat(WBFS_MIN_DEVICE, usbHandle->GetLBAStart(part_num), usbHandle->GetSecCount(part_num));
    }
    else if (strncmp(usbHandle->GetFSName(part_num), "NTFS", 4) == 0)
    {
        WbfsList[part_num] = new Wbfs_Ntfs(WBFS_MIN_DEVICE, usbHandle->GetLBAStart(part_num), usbHandle->GetSecCount(part_num));
    }
    else if (strncmp(usbHandle->GetFSName(part_num), "LINUX", 5) == 0)
    {
        WbfsList[part_num] = new Wbfs_Ext(WBFS_MIN_DEVICE, usbHandle->GetLBAStart(part_num), usbHandle->GetSecCount(part_num));
    }
    else if (strncmp(usbHandle->GetFSName(part_num), "WBFS", 4) == 0)
    {
        WbfsList[part_num] = new Wbfs_Wbfs(WBFS_MIN_DEVICE, usbHandle->GetLBAStart(part_num), usbHandle->GetSecCount(part_num));
    }
    else
    {
        return -1;
    }

    if (WbfsList[part_num]->Open() != 0)
    {
        delete WbfsList[part_num];
        WbfsList[part_num] = NULL;
        return -1;
    }

    return 0;
}

bool WBFS_Close(int part_num)
{
    if(!VALID(part_num))
        return false;

    delete WbfsList[part_num];
    WbfsList[part_num] = NULL;

    gameList.RemovePartition(part_num);

    return true;
}

void WBFS_CloseAll()
{
    gameList.clear();

    for(u32 i = 0; i < WbfsList.size(); ++i)
        WBFS_Close(i);
}

s32 WBFS_Format(u32 lba, u32 size)
{
    Wbfs_Wbfs Part(WBFS_MIN_DEVICE, lba, size);

    return Part.Format();
}

s32 WBFS_GetCount(int part_num, u32 *count)
{
    if(!VALID(part_num))
        return -1;

    return WbfsList[part_num]->GetCount(count);
}

s32 WBFS_GetHeaders(int part_num, struct discHdr *outbuf, u32 cnt, u32 len)
{
    if(!VALID(part_num))
        return -1;

    return WbfsList[part_num]->GetHeaders(outbuf, cnt, len);
}

s32 WBFS_CheckGame(u8 *discid)
{
    int part_num = gameList.GetPartitionNumber(discid);
    if(!VALID(part_num))
        return 0;

    return WbfsList[part_num]->CheckGame(discid);
}

s32 WBFS_AddGame(void)
{
    if(!VALID(Settings.partition))
        return -1;

    return WbfsList[Settings.partition]->AddGame();
}

s32 WBFS_RemoveGame(u8 *discid)
{
    int part_num = gameList.GetPartitionNumber(discid);
    if(!VALID(part_num))
        return -1;

    return WbfsList[part_num]->RemoveGame(discid);
}

s32 WBFS_GameSize(u8 *discid, f32 *size)
{
    int part_num = gameList.GetPartitionNumber(discid);
    if(!VALID(part_num))
        return -1;

    return WbfsList[part_num]->GameSize(discid, size);
}

s32 WBFS_DiskSpace(f32 *used, f32 *free)
{
    if(!VALID(Settings.partition))
        return -1;

    return WbfsList[Settings.partition]->DiskSpace(used, free);
}

s32 WBFS_RenameGame(u8 *discid, const void *newname)
{
    int part_num = gameList.GetPartitionNumber(discid);
    if(!VALID(part_num))
        return -1;

    return WbfsList[part_num]->RenameGame(discid, newname);
}

s32 WBFS_ReIDGame(u8 *discid, const void *newID)
{
    int part_num = gameList.GetPartitionNumber(discid);
    if(!VALID(part_num))
        return -1;

    return WbfsList[part_num]->ReIDGame(discid, newID);
}

u64 WBFS_EstimeGameSize(void)
{
    if(!VALID(Settings.partition))
        return 0;

    return WbfsList[Settings.partition]->EstimateGameSize();
}

int WBFS_GetFragList(u8 *id)
{
    int part_num = gameList.GetPartitionNumber(id);
    if(!VALID(part_num))
        return -1;

    return WbfsList[part_num]->GetFragList(id);
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
