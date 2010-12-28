#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <ogcsys.h>
#include <errno.h>

#include "usbloader/sdhc.h"
#include "usbloader/usbstorage2.h"
#include "fatmounter.h"
#include "wbfs_rw.h"

#include "wbfs_base.h"

s32 Wbfs::done = -1;
s32 Wbfs::total = -1;
u32 Wbfs::nb_sectors;

Wbfs::Wbfs(u32 device, u32 lba, u32 size) :
    hdd(NULL)
{
    this->device = device;
    this->lba = lba;
    this->size = size;
}

void Wbfs::GetProgressValue(s32 * d, s32 * m)
{
    *d = done;
    *m = total;
}

s32 Wbfs::Init(u32 device)
{
    s32 ret;

    switch (device)
    {
        case WBFS_DEVICE_USB:
            /* Initialize USB storage */
            ret = USBStorage2_Init();
            if (ret >= 0)
            {
                /* Setup callbacks */
                readCallback = __ReadUSB;
                writeCallback = __WriteUSB;
                /* Device info */
                /* Get USB capacity */
                nb_sectors = USBStorage2_GetCapacity(&sector_size);
                if (!nb_sectors) return -1;
            }
            else return ret;
            break;
        case WBFS_DEVICE_SDHC:
            /* Initialize SDHC */
            ret = SDHC_Init();

            if (ret)
            {
                /* Setup callbacks */
                readCallback = __ReadSDHC;
                writeCallback = __WriteSDHC;

                /* Device info */
                nb_sectors = 0;
                sector_size = SDHC_SECTOR_SIZE;
            }
            else return -1;
            break;
    }

    return 0;
}

// Default behavior: can't format
s32 Wbfs::Format()
{
    return -1;
}

s32 Wbfs::CheckGame(u8 *discid)
{
    wbfs_disc_t *disc = NULL;

    /* Try to open game disc */
    disc = OpenDisc(discid);
    if (disc)
    {
        /* Close disc */
        CloseDisc(disc);

        return 1;
    }

    return 0;
}

s32 Wbfs::GameSize(u8 *discid, f32 *size)
{
    wbfs_disc_t *disc = NULL;

    u32 sectors;

    /* Open disc */
    disc = OpenDisc(discid);
    if (!disc) return -2;

    /* Get game size in sectors */
    sectors = wbfs_sector_used(disc->p, disc->header);

    /* Copy value */
    *size = (disc->p->wbfs_sec_sz / GB_SIZE) * sectors;

    /* Close disc */
    CloseDisc(disc);

    return 0;
}

wbfs_t *Wbfs::GetHddInfo()
{
    return hdd;
}

bool Wbfs::Mounted()
{
    return hdd == NULL;
}


bool Wbfs::ShowFreeSpace(void)
{
    return true;
}
