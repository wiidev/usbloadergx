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
	if(!VALID(part))
		return NULL;

	return WbfsList[part]->OpenDisc(discid);
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

s32 WBFS_ReInit(u32 device)
{
	WBFS_CloseAll();
	DeviceHandler::Instance()->UnMountAllUSB();
	DeviceHandler::Instance()->MountAllUSB();
	s32 ret = -1;

	if(Settings.MultiplePartitions)
		ret = WBFS_OpenAll();
	else
		ret = WBFS_OpenPart(Settings.partition);

	return ret;
}

s32 WBFS_OpenAll()
{
	int ret = -1;
	int partCount = DeviceHandler::GetUSBPartitionCount();

	for(int i = 0; i < partCount; ++i)
	{
		if(WBFS_OpenPart(i) == 0)
			ret = 0;
	}

	return ret;
}

s32 WBFS_OpenPart(int part_num)
{
	PartitionHandle * usbHandle = DeviceHandler::Instance()->GetUSBHandleFromPartition(part_num);
	if(!usbHandle || part_num < 0 || part_num >= DeviceHandler::GetUSBPartitionCount())
		return -1;

	// close
	WBFS_Close(part_num);

	if(part_num >= (int) WbfsList.size())
		WbfsList.resize(part_num+1);

	int portPart = DeviceHandler::PartitionToPortPartition(part_num);
	int usbPort = DeviceHandler::PartitionToUSBPort(part_num);

	gprintf("\tWBFS_OpenPart: filesystem: %s, start sector %u, sector count: %u\n", usbHandle->GetFSName(portPart), usbHandle->GetLBAStart(portPart), usbHandle->GetSecCount(portPart));

	if (strncmp(usbHandle->GetFSName(portPart), "FAT", 3) == 0)
	{
		WbfsList[part_num] = new Wbfs_Fat(usbHandle->GetLBAStart(portPart), usbHandle->GetSecCount(portPart), part_num, usbPort);
	}
	else if (strncmp(usbHandle->GetFSName(portPart), "NTFS", 4) == 0)
	{
		WbfsList[part_num] = new Wbfs_Ntfs(usbHandle->GetLBAStart(portPart), usbHandle->GetSecCount(portPart), part_num, usbPort);
	}
	else if (strncmp(usbHandle->GetFSName(portPart), "LINUX", 5) == 0)
	{
		WbfsList[part_num] = new Wbfs_Ext(usbHandle->GetLBAStart(portPart), usbHandle->GetSecCount(portPart), part_num, usbPort);
	}
	else if (strncmp(usbHandle->GetFSName(portPart), "WBFS", 4) == 0)
	{
		WbfsList[part_num] = new Wbfs_Wbfs(usbHandle->GetLBAStart(portPart), usbHandle->GetSecCount(portPart), part_num, usbPort);
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

s32 WBFS_Format(u32 lba, u32 size, u32 port)
{
	Wbfs_Wbfs Part(WBFS_MIN_DEVICE, lba, size, port);

	return Part.Format();
}

s32 WBFS_GetCount(int part_num, u32 *count)
{
	if(!VALID(part_num))
		return -1;

	int ret = WbfsList[part_num]->GetCount(count);

	return ret;
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

s32 WBFS_GetFragList(u8 *id)
{
	int part_num = gameList.GetPartitionNumber(id);
	if(!VALID(part_num))
		return -1;

	return WbfsList[part_num]->GetFragList(id);
}
