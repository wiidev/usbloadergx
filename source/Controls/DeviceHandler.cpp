/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <sdcard/wiisd_io.h>
#include <sdcard/gcsd.h>
#include "settings/CSettings.h"
#include "usbloader/usbstorage2.h"
#include "DeviceHandler.hpp"
#include "usbloader/wbfs.h"
#include "system/IosLoader.h"

DeviceHandler * DeviceHandler::instance = NULL;

DeviceHandler::~DeviceHandler()
{
	UnMountAll();
}

DeviceHandler * DeviceHandler::Instance()
{
	if (instance == NULL)
	{
		instance = new DeviceHandler();
	}
	return instance;
}

void DeviceHandler::DestroyInstance()
{
	if(instance)
	{
		delete instance;
	}
	instance = NULL;
}

bool DeviceHandler::MountAll()
{
	bool result = false;

	for(u32 i = SD; i < MAXDEVICES; i++)
	{
		if(Mount(i))
			result = true;
	}

	return result;
}

void DeviceHandler::UnMountAll()
{
	for(u32 i = SD; i < MAXDEVICES; i++)
		UnMount(i);

	if(sd)
		delete sd;
	if(usb0)
		delete usb0;
	if(usb1)
		delete usb1;

	sd = NULL;
	usb0 = NULL;
	usb1 = NULL;
}

bool DeviceHandler::Mount(int dev)
{
	if(dev == SD)
		return MountSD();

	else if(dev >= USB1 && dev <= USB8)
		return MountUSB(dev-USB1);

	return false;
}

bool DeviceHandler::IsInserted(int dev)
{
	if(dev == SD)
		return SD_Inserted() && sd->IsMounted(0);

	else if(dev >= USB1 && dev <= USB8)
	{
		int portPart = PartitionToPortPartition(dev-USB1);
		PartitionHandle *usb = instance->GetUSBHandleFromPartition(dev-USB1);
		if(usb)
			return usb->IsMounted(portPart);
	}

	return false;
}

void DeviceHandler::UnMount(int dev)
{
	if(dev == SD)
		UnMountSD();

	else if(dev >= USB1 && dev <= USB8)
		UnMountUSB(dev-USB1);
}

bool DeviceHandler::MountSD()
{
	if(!sd)
		sd = new PartitionHandle(&__io_wiisd);

	if(sd->GetPartitionCount() < 1)
	{
		delete sd;
		sd = NULL;
		return false;
	}

	//! Mount only one SD Partition
	return sd->Mount(0, DeviceName[SD], true);
}

static inline bool USBSpinUp()
{
	bool started0 = false;
	bool started1 = false;
	int retries = 400;

	const DISC_INTERFACE * handle0 = NULL;
	const DISC_INTERFACE * handle1 = NULL;
	if(Settings.USBPort == 0 || Settings.USBPort == 2)
		handle0 = DeviceHandler::GetUSB0Interface();
	if(Settings.USBPort == 1 || Settings.USBPort == 2)
		handle1 = DeviceHandler::GetUSB1Interface();

	// wait 20 sec for the USB to spin up...stupid slow ass HDD
	do
	{
		if(handle0)
			started0 = (handle0->startup() && handle0->isInserted());

		if(handle1)
			started1 = (handle1->startup() && handle1->isInserted());

		if(   (!handle0 || started0)
		   && (!handle1 || started1)) {
			break;
		}
		usleep(50000);
	}
	while(--retries > 0);

	return (started0 || started1);
}

bool DeviceHandler::MountUSB(int pos)
{
	if(!usb0 && !usb1)
		return false;

	if(pos >= GetUSBPartitionCount())
		return false;

	int portPart = PartitionToPortPartition(pos);

	if(PartitionToUSBPort(pos) == 0 && usb0)
		return usb0->Mount(portPart, DeviceName[USB1+pos]);
	else if(usb1)
		return usb1->Mount(portPart, DeviceName[USB1+pos]);

	return false;
}

bool DeviceHandler::MountAllUSB(bool spinup)
{
	if(spinup && !USBSpinUp())
		return false;

	if(!usb0 && (Settings.USBPort == 0 || Settings.USBPort == 2))
		usb0 = new PartitionHandle(GetUSB0Interface());
	if(!usb1 && (Settings.USBPort == 1 || Settings.USBPort == 2) && IOS_GetVersion() >= 200)
		usb1 = new PartitionHandle(GetUSB1Interface());

	if(usb0 && usb0->GetPartitionCount() < 1)
	{
		delete usb0;
		usb0 = NULL;
	}
	if(usb1 && usb1->GetPartitionCount() < 1)
	{
		delete usb1;
		usb1 = NULL;
	}

	bool result = false;
	int partCount = GetUSBPartitionCount();

	for(int i = 0; i < partCount; i++)
	{
		if(MountUSB(i))
			result = true;
	}

	return result;
}

bool DeviceHandler::MountUSBPort1(bool spinup)
{
	if(spinup && !USBSpinUp())
		return false;

	if(!usb1 && (Settings.USBPort == 1 || Settings.USBPort == 2) && IOS_GetVersion() >= 200)
		usb1 = new PartitionHandle(GetUSB1Interface());

	if(usb1 && usb1->GetPartitionCount() < 1)
	{
		delete usb1;
		usb1 = NULL;
		return false;
	}

	bool result = false;
	int partCount = GetUSBPartitionCount();
	int partCount0 = 0;
	if(usb0)
		partCount0 = usb0->GetPartitionCount();

	for(int i = partCount0; i < partCount; i++)
	{
		if(MountUSB(i))
			result = true;
	}

	return result;
}

void DeviceHandler::UnMountUSB(int pos)
{
	if(pos >= GetUSBPartitionCount())
		return;

	int portPart = PartitionToPortPartition(pos);

	if(PartitionToUSBPort(pos) == 0 && usb0)
		return usb0->UnMount(portPart);
	else if(usb1)
		return usb1->UnMount(portPart);
}

void DeviceHandler::UnMountAllUSB()
{
	int partCount = GetUSBPartitionCount();

	for(int i = 0; i < partCount; i++)
		UnMountUSB(i);

	delete usb0;
	usb0 = NULL;
	delete usb1;
	usb1 = NULL;
}

int DeviceHandler::PathToDriveType(const char * path)
{
	if(!path)
		return -1;

	for(int i = SD; i < MAXDEVICES; i++)
	{
		if(strncasecmp(path, DeviceName[i], strlen(DeviceName[i])) == 0)
			return i;
	}

	return -1;
}

const char * DeviceHandler::GetFSName(int dev)
{
	if(dev == SD && DeviceHandler::instance->sd)
	{
		return DeviceHandler::instance->sd->GetFSName(0);
	}
	else if(dev >= USB1 && dev <= USB8)
	{
		int partCount0 = 0;
		int partCount1 = 0;
		if(DeviceHandler::instance->usb0)
			partCount0 += DeviceHandler::instance->usb0->GetPartitionCount();
		if(DeviceHandler::instance->usb1)
			partCount1 += DeviceHandler::instance->usb1->GetPartitionCount();

		if(dev-USB1 < partCount0 && DeviceHandler::instance->usb0)
			return DeviceHandler::instance->usb0->GetFSName(dev-USB1);
		else if(DeviceHandler::instance->usb1)
			return DeviceHandler::instance->usb1->GetFSName(dev-USB1-partCount0);
	}

	return "";
}

const char * DeviceHandler::GetDevicePrefix(const char * path)
{
	if(PathToDriveType(path) == -1)
		return "";
	return DeviceName[PathToDriveType(path)];
}

int DeviceHandler::GetFilesystemType(int dev)
{
	if(!instance)
		return -1;

	const char *FSName = GetFSName(dev);
	if(!FSName) return -1;

	if(strncmp(FSName, "WBFS", 4) == 0)
		return PART_FS_WBFS;
	else if(strncmp(FSName, "FAT", 3) == 0)
		return PART_FS_FAT;
	else if(strncmp(FSName, "NTFS", 4) == 0)
		return PART_FS_NTFS;
	else if(strncmp(FSName, "LINUX", 4) == 0)
		return PART_FS_EXT;

	return -1;
}


u16 DeviceHandler::GetUSBPartitionCount()
{
	if(!instance)
		return 0;

	u16 partCount0 = 0;
	u16 partCount1 = 0;
	if(instance->usb0)
		partCount0 = instance->usb0->GetPartitionCount();
	if(instance->usb1)
		partCount1 = instance->usb1->GetPartitionCount();

	return partCount0+partCount1;
}

int DeviceHandler::PartitionToUSBPort(int part)
{
	if(!DeviceHandler::instance)
		return 0;

	u16 partCount0 = 0;
	if(DeviceHandler::instance->usb0)
		partCount0 = instance->usb0->GetPartitionCount();

	if(!instance->usb0 || part >= partCount0)
		return 1;
	else
		return 0;
}

int DeviceHandler::PartitionToPortPartition(int part)
{
	if(!DeviceHandler::instance)
		return 0;

	u16 partCount0 = 0;
	if(instance->usb0)
		partCount0 = instance->usb0->GetPartitionCount();

	if(!instance->usb0 || part >= partCount0)
		return part-partCount0;
	else
		return part;
}

PartitionHandle *DeviceHandler::GetUSBHandleFromPartition(int part) const
{
	if(PartitionToUSBPort(part) == 0)
		return usb0;
	else
		return usb1;
}
