#include "menu/menus.h"
#include "mload/mload.h"
#include "mload/mload_modules.h"
#include "usbloader/disc.h"
#include "usbloader/GameList.h"
#include "settings/Settings.h"
#include "settings/CGameSettings.h"
#include "usbloader/frag.h"
#include "usbloader/wbfs.h"
#include "settings/newtitles.h"
#include "patches/fst.h"
#include "wad/nandtitle.h"

struct discHdr *dvdheader = NULL;
extern int load_from_fs;
extern int mountMethod;

int BootGame(const char * gameID)
{
    if(!gameID || strlen(gameID) < 3)
        return -1;

    gprintf("\tSettings.partition: %d\n", Settings.partition);

    gameList.LoadUnfiltered();

    struct discHdr *header = gameList.GetDiscHeader(gameID);
    if(!header)
    {
        gprintf("Game was not found: %s\n", gameID);
        return -1;
    }

    int ret = 0;
    header = (mountMethod ? dvdheader : header);

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

    GameCFG * game_cfg = GameSettings.GetGameCFG(header->id);

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

    if (!mountMethod)
    {
        gprintf("Loading fragment list...");
        ret = get_frag_list(header->id);
        gprintf("%d\n", ret);

        gprintf("Setting fragment list...");
        ret = set_frag_list(header->id);
        gprintf("%d\n", ret);

        ret = Disc_SetUSB(header->id);
        if (ret < 0) Sys_BackToLoader();
        gprintf("\tUSB set to game\n");
    }
    else
    {
        gprintf("\tUSB not set, loading DVD\n");
    }

    ret = Disc_Open();

    if (ret < 0) Sys_BackToLoader();

    if (dvdheader) delete dvdheader;

    gprintf("Loading BCA data...");
    ret = do_bca_code(header->id);
    gprintf("%d\n", ret);

    if (reloadblock == ON && Sys_IsHermes())
    {
        enable_ES_ioctlv_vector();
        if (load_from_fs == PART_FS_WBFS)
        {
            mload_close();
        }
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

    shadow_mload();

    ret = Disc_WiiBoot(Settings.dolpath, videoChoice, languageChoice, ocarinaChoice, viChoice, countrystrings,
                        alternatedol, alternatedoloffset, channel, fix002);

    if (ret < 0)
        Sys_LoadMenu();

    //should never get here
    printf("Returning entry point: 0x%0x\n", ret);

    return ret;
}
