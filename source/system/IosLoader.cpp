#include <gctypes.h>

#include "IosLoader.h"
#include "../fatmounter.h"
#include "../usbloader/usbstorage2.h"
#include "../usbloader/disc.h"
#include "../usbloader/wdvd.h"
#include "../wad/nandtitle.h"
#include "../mload/mload_modules.h"
#include "../settings/CSettings.h"
#include "wad/nandtitle.h"
#include "mload/mload.h"
#include "mload/modules/ehcmodule_2.h"
#include "mload/modules/dip_plugin_2.h"
#include "mload/modules/ehcmodule_3.h"
#include "mload/modules/dip_plugin_3.h"
#include "mload/modules/ehcmodule_5.h"
#include "mload/modules/dip_plugin_249.h"
#include "mload/modules/odip_frag.h"


/******************************************************************************
 * Public Methods:
 ******************************************************************************/
/*
 * Loads CIOS (If possible the one from the settings file).
 * @return 0 if a cios has been successfully loaded. Else a value below 0 is returned.
 */
s32 IosLoader::LoadAppCios()
{
    s32 ret = -1;

    // Unmount fat before reloading IOS.
    SDCard_deInit();
    USBDevice_deInit();
    __io_usbstorage.shutdown(); // libogc usb
    __io_usbstorage2.shutdown(); // cios usb
    USB_Deinitialize(); // main usb handle

    u32 ciosLoadPriority[] = { 250, 249, 222, Settings.cios }; // Descending.
    u32 activeCios = IOS_GetVersion();


    for (u8 i = (sizeof(ciosLoadPriority)/sizeof(ciosLoadPriority[0]))-1; i >= 0; i--)
    {
        u32 cios = ciosLoadPriority[i];

        if (activeCios == cios)
        {
            ret = 0;
            break;
        }

        if ((ret = ReloadIosSafe(cios)) > -1)
        {
            // Remember working cios.
            Settings.cios = cios;
            break;
        }
    }

    // Remount devices after reloading IOS.
    SDCard_Init();
    USBDevice_Init();
    Disc_Init();

    return ret;
}


/*
 * Loads a CIOS before a game start.
 * @return 0 if a cios has been successfully loaded. Else a value below 0 is returned.
 */
s32 IosLoader::LoadGameCios(s32 ios)
{
    if(ios == IOS_GetVersion())
        return 0;

    s32 ret = -1;

    // Unmount fat before reloading IOS.
    SDCard_deInit();
    USBDevice_deInit();
    WDVD_Close();
    __io_usbstorage.shutdown(); // libogc usb
    __io_usbstorage2.shutdown(); // cios usb
    USB_Deinitialize(); // main usb handle

    ret = ReloadIosSafe(ios);

    // Remount devices after reloading IOS.
    SDCard_Init();
    USBDevice_Init();

    return ret;
}

/*
 * Reloads a certain IOS under the condition, that an appropriate version of the IOS is installed.
 * @return a negative value if a safe reload of the ios was not possible.
 */
s32 IosLoader::ReloadIosSafe(s32 ios)
{
    switch (ios)
    {
        case 222:
        {
            s32 ios222rev = NandTitles.VersionOf(0x1000000deULL);
            if (ios222rev == 4 || ios222rev == 5 || ios222rev == 65535) break;
            return -2;
        }
        case 223:
        {
            s32 ios223rev = NandTitles.VersionOf(0x1000000dfULL);
            if (ios223rev == 4 || ios223rev == 5 || ios223rev == 65535) break;
            return -2;
        }
        case 249:
        {
            s32 ios249rev = NandTitles.VersionOf(0x1000000f9ULL);
            if (ios249rev < 9 || ios249rev == 65280) return -2;
            break;
        }
        case 250:
        {
            s32 ios250rev = NandTitles.VersionOf(0x1000000faULL);
            if (ios250rev < 9 || ios250rev == 65280) return -2;
            break;
        }
        default:
            return -3;
    }

    s32 r = IOS_ReloadIOS(ios);
    if (r >= 0) WII_Initialize();

    IosLoader::LoadIOSModules(IOS_GetVersion(), IOS_GetRevision());

    return r;
}

/******************************************************************************
 * Private/Protected Methods:
 ******************************************************************************/
void IosLoader::LoadIOSModules(s32 ios, s32 ios_rev)
{
    //! Hermes IOS
    if(ios == 222 || ios == 223 || ios == 224)
    {
        const u8 * ech_module = NULL;
        int ehc_module_size = 0;
        const u8 * dip_plugin = NULL;
        int dip_plugin_size = 0;

        switch (ios_rev)
        {
            case 2:
                ech_module = ehcmodule_2;
                ehc_module_size = ehcmodule_2_size;
                dip_plugin = dip_plugin_2;
                dip_plugin_size = dip_plugin_2_size;
                break;
            case 3:
                ech_module = ehcmodule_3;
                ehc_module_size = ehcmodule_3_size;
                dip_plugin = dip_plugin_3;
                dip_plugin_size = dip_plugin_3_size;
                break;
            default:
                ech_module = ehcmodule_5;
                ehc_module_size = ehcmodule_5_size;
                dip_plugin = odip_frag;
                dip_plugin_size = odip_frag_size;
                break;
        }

        u8 *ehc_cfg = search_for_ehcmodule_cfg((u8 *) ech_module, ehc_module_size);
        if (ehc_cfg)
        {
            ehc_cfg += 12;
            ehc_cfg[0] = 0; // USB Port 0
        }

        load_modules(ech_module, ehc_module_size, dip_plugin, dip_plugin_size);
    }
    //! Waninkoko IOS
    else if(ios == 249 || ios == 250)
    {
        if(ios_rev >= 18)
        {
            if(mload_init() < 0)
                return;

            mload_module((u8 *) dip_plugin_249, dip_plugin_249_size);
            mload_close();
        }
    }
}
