#include "menu/menus.h"
#include "menu/WDMMenu.hpp"
#include "mload/mload.h"
#include "mload/mload_modules.h"
#include "system/IosLoader.h"
#include "Controls/DeviceHandler.hpp"
#include "usbloader/disc.h"
#include "usbloader/apploader.h"
#include "usbloader/usbstorage2.h"
#include "usbloader/wdvd.h"
#include "usbloader/GameList.h"
#include "settings/Settings.h"
#include "settings/CGameSettings.h"
#include "usbloader/frag.h"
#include "usbloader/wbfs.h"
#include "usbloader/playlog.h"
#include "usbloader/MountGamePartition.h"
#include "usbloader/AlternateDOLOffsets.h"
#include "settings/newtitles.h"
#include "patches/fst.h"
#include "patches/gamepatches.h"
#include "patches/wip.h"
#include "system/IosLoader.h"
#include "banner/OpeningBNR.hpp"
#include "wad/nandtitle.h"
#include "menu/menus.h"
#include "memory/memory.h"
#include "GameBooter.hpp"
#include "sys.h"

//appentrypoint has to be global because of asm
u32 AppEntrypoint = 0;

struct discHdr *dvdheader = NULL;
extern u32 hdd_sector_size;
extern int mountMethod;

int GameBooter::BootGCMode()
{
    ExitApp();
    gprintf("\nLoading BC for GameCube");
    WII_Initialize();
    return WII_LaunchTitle(0x0000000100000100ULL);
}


u32 GameBooter::BootPartition(char * dolpath, u8 videoselected, u8 languageChoice, u8 cheat, u8 vipatch, u8 patchcountrystring,
                              u8 alternatedol, u32 alternatedoloffset, u32 returnTo, u8 fix002)
{
    gprintf("booting partition IOS %u r%u\n", IOS_GetVersion(), IOS_GetRevision());
    entry_point p_entry;
    s32 ret;
    u64 offset;

    /* Find game partition offset */
    ret = __Disc_FindPartition(&offset);
    if (ret < 0)
        return 0;

    /* Open specified partition */
    ret = WDVD_OpenPartition(offset);
    if (ret < 0)
        return 0;

    load_wip_code((u8*) Disc_ID);

    /* If a wip file is loaded for this game this does nothing - Dimok */
    PoPPatch();
    NSMBPatch();

    /* Setup low memory */
    __Disc_SetLowMem();

    /* Run apploader */
    ret = Apploader_Run(&p_entry, dolpath, cheat, videoselected, languageChoice, vipatch, patchcountrystring,
            alternatedol, alternatedoloffset, returnTo, fix002);

    if (ret < 0)
        return 0;

    free_wip();

    return (u32) p_entry;
}

int GameBooter::FindDiscHeader(const char * gameID, struct discHdr &gameHeader)
{
    gameList.LoadUnfiltered();

    if(mountMethod == 0 && !gameList.GetDiscHeader(gameID))
    {
        gprintf("Game was not found: %s\n", gameID);
        return -1;
    }
    else if(mountMethod && !dvdheader)
    {
        gprintf("Error: Loading empty disc header from DVD\n");
        return -1;
    }

    memcpy(&gameHeader, (mountMethod ? dvdheader : gameList.GetDiscHeader(gameID)), sizeof(struct discHdr));

    delete dvdheader;
    dvdheader = NULL;

    return 0;
}

void GameBooter::SetupAltDOL(u8 * gameID, u8 &alternatedol, u32 &alternatedoloffset)
{
    if(alternatedol == ALT_DOL_ON_LAUNCH)
    {
        alternatedol = ALT_DOL_FROM_GAME;
        alternatedoloffset = WDMMenu::GetAlternateDolOffset();
    }
    else if(alternatedol == ALT_DOL_DEFAULT)
    {
        alternatedol = ALT_DOL_FROM_GAME;
        alternatedoloffset = defaultAltDol((char *) gameID);
    }

    if(alternatedol == ALT_DOL_FROM_GAME && alternatedoloffset == 0)
        alternatedol = OFF;
}

int GameBooter::SetupDisc(u8 * gameID)
{
    if (mountMethod)
    {
        gprintf("\tloading DVD\n");
        return Disc_Open();
    }

    int ret = -1;

    if(((IosLoader::IsWaninkokoIOS() && IOS_GetRevision() < 18) ||
        hdd_sector_size != 512) && gameList.GetGameFS(gameID) == PART_FS_WBFS)
    {
        gprintf("Disc_SetUSB...");
        ret = Disc_SetUSB(gameID);
        gprintf("%d\n", ret);
        if(ret < 0) return ret;
    }
    else
    {
        gprintf("Loading fragment list...");
        ret = get_frag_list(gameID);
        gprintf("%d\n", ret);
        if(ret < 0) return ret;
        ret = set_frag_list(gameID);
        if(ret < 0) return ret;
        gprintf("\tUSB set to game\n");
    }

    gprintf("Disc_Open()...");
    ret = Disc_Open();
    gprintf("%d\n", ret);

    return ret;
}

bool GameBooter::LoadOcarina(u8 *gameID)
{
    if (ocarina_load_code(gameID) > 0)
    {
        ocarina_do_code();
        return true;
    }

    return false;
}

int GameBooter::BootGame(const char * gameID)
{
    if(!gameID || strlen(gameID) < 3)
        return -1;

    if (mountMethod == 2)
        return BootGCMode();

    AppCleanUp();

    gprintf("\tSettings.partition: %d\n", Settings.partition);

    struct discHdr gameHeader;

    //! Find disc header in the game list first
    int ret = FindDiscHeader(gameID, gameHeader);
    if(ret < 0)
        return ret;

    //! Setup game configuration from game settings. If no game settings exist use global/default.
    GameCFG * game_cfg = GameSettings.GetGameCFG(gameHeader.id);
    u8 videoChoice = game_cfg->video;
    u8 languageChoice = game_cfg->language;
    u8 ocarinaChoice = game_cfg->ocarina;
    u8 viChoice = game_cfg->vipatch;
    u8 iosChoice = game_cfg->ios;
    u8 fix002 = game_cfg->errorfix002;
    u8 countrystrings = game_cfg->patchcountrystrings;
    u8 alternatedol = game_cfg->loadalternatedol;
    u32 alternatedoloffset = game_cfg->alternatedolstart;
    u8 reloadblock = game_cfg->iosreloadblock;
    u8 returnToLoaderGV = game_cfg->returnTo;

    //! Prepare alternate dol settings
    SetupAltDOL(gameHeader.id, alternatedol, alternatedoloffset);

    //! Setup the return to Loader option
    u32 channel = 0;
    if (returnToLoaderGV)
    {
        int idx = NandTitles.FindU32(Settings.returnTo);
        if (idx >= 0) channel = TITLE_LOWER( NandTitles.At( idx ) );
    }

    //! This is temporary - C <-> C++ transfer
    SetCheatFilepath(Settings.Cheatcodespath);
    SetBCAFilepath(Settings.BcaCodepath);

    //! Reload game settings cIOS for this game
    if(iosChoice != IOS_GetVersion())
    {
        gprintf("Reloading into game cIOS: %i...\n", iosChoice);
        IosLoader::LoadGameCios(iosChoice);
        if(MountGamePartition(false) < 0)
            return -1;
    }

    //! Setup disc in cIOS and open it
    ret = SetupDisc(gameHeader.id);
    if (ret < 0)
        Sys_BackToLoader();

    //! Load BCA data for the game
    gprintf("Loading BCA data...");
    ret = do_bca_code(gameHeader.id);
    gprintf("%d\n", ret);

    //! Setup IOS reload block - only possible on Hermes cIOS
    if (reloadblock == ON && IosLoader::IsHermesIOS())
    {
        enable_ES_ioctlv_vector();
        if (gameList.GetGameFS(gameHeader.id) == PART_FS_WBFS)
            mload_close();
    }

	//! Now we can free up the memory used by the game list
    gameList.clear();

    //! Load main.dol or alternative dol into memory, start the game apploader and get game entrypoint
    gprintf("\tDisc_wiiBoot\n");
    AppEntrypoint = BootPartition(Settings.dolpath, videoChoice, languageChoice, ocarinaChoice, viChoice, countrystrings,
                        alternatedol, alternatedoloffset, channel, fix002);

    //! No entrypoint found...back to HBC/SystemMenu
    if(AppEntrypoint == 0)
    {
        WDVD_ClosePartition();
        Sys_BackToLoader();
    }

    //! Load Ocarina codes
    bool enablecheat = false;
    if (ocarinaChoice)
        enablecheat = LoadOcarina(gameHeader.id);

    //! Shadow mload - Only needed on some games with Hermes v5.1 (Check is inside the function)
    shadow_mload();

    //! Remember game's USB port
    int usbport = USBStorage2_GetPort();

    //! Flush all caches and close up all devices
    WBFS_CloseAll();
    DeviceHandler::DestroyInstance();
    USB_Deinitialize();

    if(Settings.USBPort == 2)
    {
        //! Reset USB port because device handler changes it for cache flushing
        USBStorage2_Init();
        USBStorage2_SetPort(usbport);
        USBStorage2_Deinit();
    }

    //! Modify Wii Message Board to display the game starting here
    if(Settings.PlaylogUpdate)
        Playlog_Update((char *) gameHeader.id, BNRInstance::Instance()->GetIMETTitle(CONF_GetLanguage()));

    //! Jump to the entrypoint of the game - the last function of the USB Loader
    gprintf("Jumping to game entrypoint: 0x%08X.\n", AppEntrypoint);
    return Disc_JumpToEntrypoint(videoChoice, enablecheat, WDMMenu::GetDolParameter());
}
