#include <gctypes.h>

#include "IosLoader.h"
#include "../fatmounter.h"
#include "../usbloader/usbstorage2.h"
#include "../wad/nandtitle.h"
#include "../mload/mload_modules.h"
#include "../settings/CSettings.h"
#include "wad/nandtitle.h"

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

    // Unmount devices before reloading IOS.
    SDCard_deInit();
    USBDevice_deInit();
    USBStorage2_Deinit();

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
    if (IOS_GetVersion() == 222) load_ehc_module();
    USBDevice_Init();

    return ret;
}

/******************************************************************************
 * Private/Protected Methods:
 ******************************************************************************/

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
    return r;
}
