#include <gccore.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <ogc/usbstorage.h>
#include <ogc/lwp_watchdog.h>
#include <string.h>
#include <stdio.h>
#include <fat.h>
#include <ntfs.h>
#include <sdcard/wiisd_io.h>
#include <unistd.h>
#include <time.h>
#include "devicemounter.h"

//these are the only stable and speed is good
#define CACHE 8
#define SECTORS 64

typedef struct _PARTITION_RECORD {
    u8 status;                              /* Partition status; see above */
    u8 chs_start[3];                        /* Cylinder-head-sector address to first block of partition */
    u8 type;                                /* Partition type; see above */
    u8 chs_end[3];                          /* Cylinder-head-sector address to last block of partition */
    u32 lba_start;                          /* Local block address to first sector of partition */
    u32 block_count;                        /* Number of blocks in partition */
} __attribute__((__packed__)) PARTITION_RECORD;


typedef struct _MASTER_BOOT_RECORD {
    u8 code_area[446];                      /* Code area; normally empty */
    PARTITION_RECORD partitions[4];         /* 4 primary partitions */
    u16 signature;                          /* MBR signature; 0xAA55 */
} __attribute__((__packed__)) MASTER_BOOT_RECORD;


#define le32(i) (((((u32) i) & 0xFF) << 24) | ((((u32) i) & 0xFF00) << 8) | \
                ((((u32) i) & 0xFF0000) >> 8) | ((((u32) i) & 0xFF000000) >> 24))

int USBDevice_Init()
{
    time_t start = time(0);

    while(start-time(0) < 10) // 10 sec
    {
        if(__io_usbstorage.startup() && __io_usbstorage.isInserted())
            break;

        usleep(200000); // 1/5 sec
    }

    if(!__io_usbstorage.startup() || !__io_usbstorage.isInserted())
        return -1;

    int i;
    MASTER_BOOT_RECORD mbr;
    char BootSector[512];

    __io_usbstorage.readSectors(0, 1, &mbr);

    for(i = 0; i < 4; ++i)
    {
        if(mbr.partitions[i].type == 0)
            continue;

        __io_usbstorage.readSectors(le32(mbr.partitions[i].lba_start), 1, BootSector);

        if(*((u16 *) (BootSector + 0x1FE)) == 0x55AA)
        {
            //! Partition typ can be missleading the correct partition format. Stupid lazy ass Partition Editors.
            if(memcmp(BootSector + 0x36, "FAT", 3) == 0 || memcmp(BootSector + 0x52, "FAT", 3) == 0)
            {
                fatMount(DeviceName[USB1+i], &__io_usbstorage, le32(mbr.partitions[i].lba_start), CACHE, SECTORS);
            }
            else if (memcmp(BootSector + 0x03, "NTFS", 4) == 0)
            {
                ntfsMount(DeviceName[USB1+i], &__io_usbstorage, le32(mbr.partitions[i].lba_start), CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER | NTFS_IGNORE_CASE);
            }
        }
    }

	return -1;
}

void USBDevice_deInit()
{
    int dev;
    char Name[20];

    for(dev = USB1; dev <= USB4; ++dev)
    {
        sprintf(Name, "%s:/", DeviceName[dev]);
        fatUnmount(Name);
        ntfsUnmount(Name, true);
    }
	//Let's not shutdown so it stays awake for the application
	__io_usbstorage.shutdown();
	USB_Deinitialize();
}

int SDCard_Init()
{
    if(!__io_wiisd.startup() || !__io_wiisd.isInserted())
        return -1;

	if (fatMount(DeviceName[SD], &__io_wiisd, 0, CACHE, SECTORS))
		return 1;

	return -1;
}

void SDCard_deInit()
{
    char Name[20];
    sprintf(Name, "%s:/", DeviceName[SD]);
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount(Name);
	//Let's not shutdown so it stays awake for the application
	__io_wiisd.shutdown();
}
