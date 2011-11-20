/****************************************************************************
 * Copyright (C) 2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "menu/menus.h"
#include "menu/WDMMenu.hpp"
#include "mload/mload.h"
#include "mload/mload_modules.h"
#include "system/IosLoader.h"
#include "Controls/DeviceHandler.hpp"
#include "Channels/channels.h"
#include "usbloader/disc.h"
#include "usbloader/apploader.h"
#include "usbloader/usbstorage2.h"
#include "usbloader/wdvd.h"
#include "usbloader/GameList.h"
#include "settings/CGameSettings.h"
#include "usbloader/frag.h"
#include "usbloader/wbfs.h"
#include "usbloader/playlog.h"
#include "usbloader/MountGamePartition.h"
#include "usbloader/AlternateDOLOffsets.h"
#include "settings/newtitles.h"
#include "network/Wiinnertag.h"
#include "patches/patchcode.h"
#include "patches/gamepatches.h"
#include "patches/wip.h"
#include "patches/bca.h"
#include "system/IosLoader.h"
#include "banner/OpeningBNR.hpp"
#include "wad/nandtitle.h"
#include "menu/menus.h"
#include "memory/memory.h"
#include "GameBooter.hpp"
#include "NandEmu.h"
#include "SavePath.h"
#include "sys.h"

//appentrypoint has to be global because of asm
u32 AppEntrypoint = 0;

struct discHdr *dvdheader = NULL;
extern int mountMethod;

int GameBooter::BootGCMode()
{
	ExitApp();
	gprintf("\nLoading BC for GameCube");
	WII_Initialize();
	return WII_LaunchTitle(0x0000000100000100ULL);
}


u32 GameBooter::BootPartition(char * dolpath, u8 videoselected, u8 alternatedol, u32 alternatedoloffset)
{
	gprintf("booting partition IOS %u r%u\n", IOS_GetVersion(), IOS_GetRevision());
	entry_point p_entry;
	s32 ret;
	u64 offset;

	/* Find game partition offset */
	ret = Disc_FindPartition(&offset);
	if (ret < 0)
		return 0;

	/* Open specified partition */
	ret = WDVD_OpenPartition(offset);
	if (ret < 0)
		return 0;

	/* Setup low memory */
	Disc_SetLowMem();

	/* Setup video mode */
	Disc_SelectVMode(videoselected);

	/* Run apploader */
	ret = Apploader_Run(&p_entry, dolpath, alternatedol, alternatedoloffset);

	if (ret < 0)
		return 0;

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

void GameBooter::SetupNandEmu(u8 NandEmuMode, const char *NandEmuPath, struct discHdr &gameHeader)
{
	if(NandEmuMode && strchr(NandEmuPath, '/'))
	{
		//! Create save game path and title.tmd for not existing saves
		CreateSavePath(&gameHeader);

		gprintf("Enabling Nand Emulation on: %s\n", NandEmuPath);
		Set_FullMode(NandEmuMode == 2);
		Set_Path(strchr(NandEmuPath, '/'));

		//! Set which partition to use (USB only)
		if(strncmp(NandEmuPath, "usb", 3) == 0)
			Set_Partition(atoi(NandEmuPath+3)-1);
		//! Unmount SD since NAND Emu mount fails otherwise
		else if(strncmp(NandEmuPath, "sd", 2) == 0)
			DeviceHandler::Instance()->UnMountSD();

		Enable_Emu(strncmp(NandEmuPath, "usb", 3) == 0 ? EMU_USB : EMU_SD);

		//! Remount SD again after activating NAND emu
		if(strncmp(NandEmuPath, "sd", 2) == 0)
			DeviceHandler::Instance()->MountSD();
	}
}

int GameBooter::SetupDisc(u8 * gameID)
{
	if (mountMethod)
	{
		gprintf("\tloading DVD\n");
		return Disc_Open();
	}

	int ret = -1;

	if(IosLoader::IsWaninkokoIOS() && IOS_GetRevision() < 18)
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

void GameBooter::ShutDownDevices(int gameUSBPort)
{
	gprintf("Shutting down devices...\n");
	//! Flush all caches and close up all devices
	WBFS_CloseAll();
	DeviceHandler::DestroyInstance();

	//! Shadow mload - Only needed on some games with Hermes v5.1 (Check is inside the function)
	shadow_mload();

	if(Settings.USBPort == 2)
		//! Reset USB port because device handler changes it for cache flushing
		USBStorage2_SetPort(gameUSBPort);
	USBStorage2_Deinit();
	USB_Deinitialize();
}

int GameBooter::BootGame(const char * gameID)
{
	if(!gameID || strlen(gameID) < 3)
		return -1;

	if (mountMethod == 2)
		return BootGCMode();

	if(Settings.Wiinnertag)
		Wiinnertag::TagGame(gameID);

	AppCleanUp();

	gprintf("\tSettings.partition: %d\n", Settings.partition);

	struct discHdr gameHeader;

	//! Find disc header in the game list first
	int ret = FindDiscHeader(gameID, gameHeader);
	if(ret < 0)
		return ret;

	//! Remember game's USB port
	int partition = gameList.GetPartitionNumber(gameHeader.id);
	int usbport = DeviceHandler::PartitionToUSBPort(partition);

	//! Setup game configuration from game settings. If no game settings exist use global/default.
	GameCFG * game_cfg = GameSettings.GetGameCFG(gameHeader.id);
	u8 videoChoice = game_cfg->video == INHERIT ? Settings.videomode : game_cfg->video;
	u8 languageChoice = game_cfg->language == INHERIT ? Settings.language : game_cfg->language;
	u8 ocarinaChoice = game_cfg->ocarina == INHERIT ? Settings.ocarina : game_cfg->ocarina;
	u8 viChoice = game_cfg->vipatch == INHERIT ? Settings.videopatch : game_cfg->vipatch;
	u8 sneekChoice = game_cfg->sneekVideoPatch == INHERIT ? Settings.sneekVideoPatch : game_cfg->sneekVideoPatch;
	u8 iosChoice = game_cfg->ios == INHERIT ? Settings.cios : game_cfg->ios;
	u8 fix002 = game_cfg->errorfix002 == INHERIT ? Settings.error002 : game_cfg->errorfix002;
	u8 countrystrings = game_cfg->patchcountrystrings == INHERIT ? Settings.patchcountrystrings : game_cfg->patchcountrystrings;
	u8 alternatedol = game_cfg->loadalternatedol;
	u32 alternatedoloffset = game_cfg->alternatedolstart;
	u8 reloadblock = game_cfg->iosreloadblock == INHERIT ? Settings.BlockIOSReload : game_cfg->iosreloadblock;
	u8 NandEmuMode = game_cfg->NandEmuMode == INHERIT ? Settings.NandEmuMode : game_cfg->NandEmuMode;
	u8 Hooktype = game_cfg->Hooktype == INHERIT ? Settings.Hooktype : game_cfg->Hooktype;
	u8 WiirdDebugger = game_cfg->WiirdDebugger == INHERIT ? Settings.WiirdDebugger : game_cfg->WiirdDebugger;
	u64 returnToChoice = game_cfg->returnTo ? NandTitles.FindU32(Settings.returnTo) : 0;

	if(ocarinaChoice && Hooktype == OFF)
		Hooktype = 1;

	//! Prepare alternate dol settings
	SetupAltDOL(gameHeader.id, alternatedol, alternatedoloffset);

	//! Reload game settings cIOS for this game
	if(iosChoice != IOS_GetVersion())
	{
		gprintf("Reloading into game cIOS: %i...\n", iosChoice);
		IosLoader::LoadGameCios(iosChoice);
		if(MountGamePartition(false) < 0)
			return -1;
	}

	//! Modify Wii Message Board to display the game starting here (before Nand Emu)
	if(Settings.PlaylogUpdate)
		Playlog_Update((char *) gameHeader.id, BNRInstance::Instance()->GetIMETTitle(CONF_GetLanguage()));

	//! Setup NAND emulation
	if(Settings.LoaderMode != LOAD_CHANNELS)
		SetupNandEmu(NandEmuMode, Settings.NandEmuPath, gameHeader);
	else
		SetupNandEmu(Settings.NandEmuChanMode, Settings.NandEmuChanPath, gameHeader);

	// Load wip codes
	load_wip_code(gameHeader.id);

	//! Load Ocarina codes
	if (ocarinaChoice)
		ocarina_load_code(Settings.Cheatcodespath, gameHeader.id);

	//! Setup disc stuff if we load a game
	if(Settings.LoaderMode != LOAD_CHANNELS)
	{
		//! Setup disc in cIOS and open it
		ret = SetupDisc(gameHeader.id);
		if (ret < 0)
			Sys_BackToLoader();

		//! Load BCA data for the game
		gprintf("Loading BCA data...");
		ret = do_bca_code(Settings.BcaCodepath, gameHeader.id);
		gprintf("%d\n", ret);
	}

	//! Setup IOS reload block
	if (IosLoader::IsHermesIOS())
	{
		if(reloadblock == ON)
		{
			enable_ES_ioctlv_vector();
			if (gameList.GetGameFS(gameHeader.id) == PART_FS_WBFS)
				mload_close();
		}

		reloadblock = 0;
	}
	else if(reloadblock == AUTO)
	{
		iosinfo_t * iosinfo = IosLoader::GetIOSInfo(IOS_GetVersion());
		if(!iosinfo || iosinfo->version < 6)
			reloadblock = 0;
	}

	//! Now we can free up the memory used by the game/channel lists
	gameList.clear();
	Channels::DestroyInstance();

	//! Load main.dol or alternative dol into memory, start the game apploader and get game entrypoint
	if(Settings.LoaderMode != LOAD_CHANNELS)
	{
		gprintf("\tGame Boot\n");
		AppEntrypoint = BootPartition(Settings.dolpath, videoChoice, alternatedol, alternatedoloffset);
		// Reading of game is done we can close devices now
		ShutDownDevices(usbport);
	}
	else
	{
		//! shutdown now and avoid later crashs with free if memory gets overwritten by channel
		ShutDownDevices(usbport);
		gprintf("\tChannel Boot\n");
		/* Setup low memory */
		Disc_SetLowMem();
		/* Setup video mode */
		Disc_SelectVMode(videoChoice);
		// Load dol
		AppEntrypoint = Channels::LoadChannel(gameHeader.tid);
	}

	//! No entrypoint found...back to HBC/SystemMenu
	if(AppEntrypoint == 0)
	{
		gprintf("AppEntryPoint is 0, something went wrong\n");
		WDVD_ClosePartition();
		Sys_BackToLoader();
	}

	//! Do all the game patches
	gprintf("Applying game patches...\n");
	gamepatches(videoChoice, languageChoice, countrystrings, viChoice, sneekChoice, Hooktype, fix002, reloadblock, iosChoice, returnToChoice);

	//! Load Code handler if needed
	load_handler(Hooktype, WiirdDebugger, Settings.WiirdDebuggerPause);

	//! Jump to the entrypoint of the game - the last function of the USB Loader
	gprintf("Jumping to game entrypoint: 0x%08X.\n", AppEntrypoint);
	return Disc_JumpToEntrypoint(Hooktype, WDMMenu::GetDolParameter());
}
