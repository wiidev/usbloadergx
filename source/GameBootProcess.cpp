#include "menu/menus.h"
#include "menu/WDMMenu.hpp"
#include "mload/mload.h"
#include "mload/mload_modules.h"
#include "system/IosLoader.h"
#include "Controls/DeviceHandler.hpp"
#include "usbloader/disc.h"
#include "usbloader/apploader.h"
#include "usbloader/wdvd.h"
#include "usbloader/GameList.h"
#include "settings/Settings.h"
#include "settings/CGameSettings.h"
#include "usbloader/frag.h"
#include "usbloader/wbfs.h"
#include "usbloader/playlog.h"
#include "usbloader/MountGamePartition.h"
#include "settings/newtitles.h"
#include "patches/fst.h"
#include "patches/gamepatches.h"
#include "patches/wip.h"
#include "system/IosLoader.h"
#include "banner/OpeningBNR.hpp"
#include "wad/nandtitle.h"
#include "menu/menus.h"
#include "memory/memory.h"
#include "sys.h"

//appentrypoint has to be global because of asm
u32 AppEntrypoint = 0;

struct discHdr *dvdheader = NULL;
extern int wbfs_part_fs;
extern int mountMethod;


static u32 BootPartition(char * dolpath, u8 videoselected, u8 languageChoice, u8 cheat, u8 vipatch, u8 patchcountrystring,
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

int BootGame(const char * gameID)
{
    if(!gameID || strlen(gameID) < 3)
        return -1;

    if (mountMethod == 2)
    {
        ExitApp();
        gprintf("\nLoading BC for GameCube");
        WII_Initialize();
        return WII_LaunchTitle(0x0000000100000100ULL);
    }

    AppCleanUp();

    gprintf("\tSettings.partition: %d\n", Settings.partition);

    struct discHdr gameHeader;
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

    gameList.clear();

    int ret = 0;

    u8 videoChoice = Settings.videomode;
    u8 languageChoice = Settings.language;
    u8 ocarinaChoice = Settings.ocarina;
    u8 viChoice = Settings.videopatch;
    u8 iosChoice = Settings.cios;
    u8 fix002 = Settings.error002;
    u8 countrystrings = Settings.patchcountrystrings;
    u8 alternatedol = OFF;
    u32 alternatedoloffset = 0;
    u8 reloadblock = OFF;
    u8 returnToLoaderGV = 1;

    GameCFG * game_cfg = GameSettings.GetGameCFG(gameHeader.id);

    if (game_cfg)
    {
        videoChoice = game_cfg->video;
        languageChoice = game_cfg->language;
        ocarinaChoice = game_cfg->ocarina;
        viChoice = game_cfg->vipatch;
        fix002 = game_cfg->errorfix002;
        iosChoice = game_cfg->ios;
        countrystrings = game_cfg->patchcountrystrings;
        alternatedol = game_cfg->loadalternatedol;
        alternatedoloffset = game_cfg->alternatedolstart;
        reloadblock = game_cfg->iosreloadblock;
        returnToLoaderGV = game_cfg->returnTo;
    }

    if(alternatedol == 3)
        alternatedoloffset = WDMMenu::GetAlternateDolOffset();

    if(iosChoice != IOS_GetVersion())
    {
        gprintf("Reloading into game cIOS: %i...\n", iosChoice);
        IosLoader::LoadGameCios(iosChoice);
        if(MountGamePartition(false) < 0)
            return -1;
    }

    if (!mountMethod)
    {
        if(IosLoader::IsWaninkokoIOS() && IOS_GetRevision() < 18)
        {
            gprintf("Disc_SetUSB...");
            ret = Disc_SetUSB(gameHeader.id);
            gprintf("%d\n", ret);
        }
        else
        {
            gprintf("Loading fragment list...");
            ret = get_frag_list(gameHeader.id);
            gprintf("%d\n", ret);
            ret = set_frag_list(gameHeader.id);
            if (ret < 0) Sys_BackToLoader();
            gprintf("\tUSB set to game\n");
        }
    }
    else
    {
        gprintf("\tUSB not set, loading DVD\n");
    }

    gprintf("Disc_Open()...");
    ret = Disc_Open();
    gprintf("%d\n", ret);

    if (ret < 0)
        Sys_BackToLoader();

    gprintf("Loading BCA data...");
    ret = do_bca_code(gameHeader.id);
    gprintf("%d\n", ret);

    if (reloadblock == ON && IosLoader::IsHermesIOS())
    {
        enable_ES_ioctlv_vector();
        if (wbfs_part_fs == PART_FS_WBFS)
            mload_close();
    }

    u32 channel = 0;
    if (returnToLoaderGV)
    {
        int idx = NandTitles.FindU32(Settings.returnTo);
        if (idx >= 0) channel = TITLE_LOWER( NandTitles.At( idx ) );
    }

    //This is temporary
    SetCheatFilepath(Settings.Cheatcodespath);
    SetBCAFilepath(Settings.BcaCodepath);

    gprintf("\tDisc_wiiBoot\n");

    /* Boot partition */
    AppEntrypoint = BootPartition(Settings.dolpath, videoChoice, languageChoice, ocarinaChoice, viChoice, countrystrings,
                        alternatedol, alternatedoloffset, channel, fix002);

    if(AppEntrypoint != 0)
    {
        bool enablecheat = false;

        if (ocarinaChoice)
        {
            // OCARINA STUFF - FISHEARS
            if (ocarina_load_code((u8 *) Disc_ID) > 0)
            {
                ocarina_do_code();
                enablecheat = true;
            }
        }

        shadow_mload();
        WBFS_Close();
        DeviceHandler::DestroyInstance();
        USB_Deinitialize();

        if(Settings.PlaylogUpdate)
            Playlog_Update((char *) Disc_ID, BNRInstance::Instance()->GetIMETTitle(CONF_GetLanguage()));

        gprintf("Jumping to game entrypoint: 0x%08X.\n", AppEntrypoint);

        return Disc_JumpToEntrypoint(videoChoice, enablecheat, WDMMenu::GetDolParameter());
    }

    WDVD_ClosePartition();

    return -1;
}
