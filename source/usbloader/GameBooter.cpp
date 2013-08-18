/****************************************************************************
 * Copyright (C) 2012-2013 Cyan
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
#include "settings/SettingsEnums.h"
#include "usbloader/frag.h"
#include "usbloader/wbfs.h"
#include "usbloader/playlog.h"
#include "usbloader/MountGamePartition.h"
#include "usbloader/AlternateDOLOffsets.h"
#include "GameCube/GCGames.h"
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
#include "FileOperations/fileops.h"
#include "prompts/ProgressWindow.h"

//appentrypoint has to be global because of asm
u32 AppEntrypoint = 0;

extern u32 hdd_sector_size[2];
extern "C"
{
	syssram* __SYS_LockSram();
	u32 __SYS_UnlockSram(u32 write);
	u32 __SYS_SyncSram(void);
	extern void __exception_closeall();
}

int GameBooter::BootGCMode(struct discHdr *gameHdr)
{
	// check the settings
	GameCFG * game_cfg = GameSettings.GetGameCFG(gameHdr->id);
	u8 GCMode = game_cfg->GameCubeMode == INHERIT ? Settings.GameCubeMode : game_cfg->GameCubeMode;

	// Devolution
	if(GCMode == GC_MODE_DEVOLUTION)
		return BootDevolution(gameHdr);
	
	// DIOS MIOS (Lite) and QuadForce
	int currentMIOS = IosLoader::GetMIOSInfo();
	if(currentMIOS == DIOS_MIOS || currentMIOS == DIOS_MIOS_LITE || currentMIOS == QUADFORCE || currentMIOS == QUADFORCE_USB)
		return BootDIOSMIOS(gameHdr);
	
	// MIOS or Wiigator cMIOS
	if(gameHdr->type == TYPE_GAME_GC_DISC)
	{
		ExitApp();
		gprintf("\nLoading BC for GameCube");
		WII_Initialize();
		return WII_LaunchTitle(0x0000000100000100ULL);
	}

	// Check if Devolution is available
	char DEVO_loader_path[100];
	snprintf(DEVO_loader_path, sizeof(DEVO_loader_path), "%sloader.bin", Settings.DEVOLoaderPath);
	if(CheckFile(DEVO_loader_path))
	{
		WindowPrompt(tr("Error:"), tr("You need to set GameCube Mode to Devolution to launch GameCube games from USB or SD card"), tr("OK"));
		return 0;
	}

	WindowPrompt(tr("Error:"), tr("You need to install Devolution or DIOS MIOS (Lite) to launch GameCube games from USB or SD card"), tr("OK"));

	return 0;
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
	Disc_SelectVMode(videoselected, false, NULL);

	/* Run apploader */
	ret = Apploader_Run(&p_entry, dolpath, alternatedol, alternatedoloffset);

	if (ret < 0)
		return 0;

	return (u32) p_entry;
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
		int partition = -1;

		//! Create save game path and title.tmd for not existing saves
		CreateSavePath(&gameHeader);

		gprintf("Enabling Nand Emulation on: %s\n", NandEmuPath);
		Set_FullMode(NandEmuMode == 2);
		Set_Path(strchr(NandEmuPath, '/'));

		//! Unmount devices to flush data before activating NAND Emu
		if(strncmp(NandEmuPath, "usb", 3) == 0)
		{
			//! Set which partition to use (USB only)
			partition = atoi(NandEmuPath+3)-1;
			Set_Partition(DeviceHandler::PartitionToPortPartition(partition));
			DeviceHandler::Instance()->UnMount(USB1 + partition);
		}
		else
			DeviceHandler::Instance()->UnMountSD();

		Enable_Emu(strncmp(NandEmuPath, "usb", 3) == 0 ? EMU_USB : EMU_SD);

		//! Mount USB to start game, SD is not required
		if(strncmp(NandEmuPath, "usb", 3) == 0)
			DeviceHandler::Instance()->Mount(USB1 + partition);

	}
}

int GameBooter::SetupDisc(struct discHdr &gameHeader)
{
	if (gameHeader.type == TYPE_GAME_WII_DISC)
	{
		gprintf("\tloading DVD\n");
		return Disc_Open();
	}

	int ret = -1;

	if(IosLoader::IsWaninkokoIOS() && IOS_GetRevision() < 18)
	{
		gprintf("Disc_SetUSB...");
		ret = Disc_SetUSB(gameHeader.id);
		gprintf("%d\n", ret);
		if(ret < 0) return ret;
	}
	else
	{
		gprintf("Loading fragment list...");
		ret = get_frag_list(gameHeader.id);
		gprintf("%d\n", ret);
		if(ret < 0) return ret;
		ret = set_frag_list(gameHeader.id);
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

int GameBooter::BootGame(struct discHdr *gameHdr)
{
	if(!gameHdr)
		return -1;

	struct discHdr gameHeader;
	memcpy(&gameHeader, gameHdr, sizeof(struct discHdr));

	gprintf("\tBootGame: %.6s\n", gameHeader.id);

	if(Settings.Wiinnertag)
		Wiinnertag::TagGame((const char *) gameHeader.id);

	if(gameHeader.type == TYPE_GAME_GC_IMG || gameHeader.type == TYPE_GAME_GC_DISC  || gameHdr->type == TYPE_GAME_GC_EXTRACTED)
		return BootGCMode(&gameHeader);

	AppCleanUp();

	gprintf("\tSettings.partition: %d\n", Settings.partition);

	s32 ret = -1;

	//! Remember game's USB port
	int partition = gameList.GetPartitionNumber(gameHeader.id);
	int usbport = DeviceHandler::PartitionToUSBPort(partition);

	//! Setup game configuration from game settings. If no game settings exist use global/default.
	GameCFG * game_cfg = GameSettings.GetGameCFG(gameHeader.id);
	u8 videoChoice = game_cfg->video == INHERIT ? Settings.videomode : game_cfg->video;
	u8 videoPatchDolChoice = game_cfg->videoPatchDol == INHERIT ? Settings.videoPatchDol : game_cfg->videoPatchDol;
	u8 aspectChoice = game_cfg->aspectratio == INHERIT ? Settings.GameAspectRatio : game_cfg->aspectratio;
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
	u8 Hooktype = game_cfg->Hooktype == INHERIT ? Settings.Hooktype : game_cfg->Hooktype;
	u8 WiirdDebugger = game_cfg->WiirdDebugger == INHERIT ? Settings.WiirdDebugger : game_cfg->WiirdDebugger;
	u64 returnToChoice = game_cfg->returnTo ? NandTitles.FindU32(Settings.returnTo) : 0;
	u8 NandEmuMode = game_cfg->NandEmuMode == INHERIT ? Settings.NandEmuMode : game_cfg->NandEmuMode;
	const char *NandEmuPath = game_cfg->NandEmuPath.size() == 0 ? Settings.NandEmuPath : game_cfg->NandEmuPath.c_str();
	if(gameHeader.tid != 0)
	{
		NandEmuMode = (gameHeader.type == TYPE_GAME_EMUNANDCHAN)
					  ? (game_cfg->NandEmuMode == INHERIT ? Settings.NandEmuChanMode : game_cfg->NandEmuMode)	//! Emulated nand title
					  : 0;																						//! Real nand title
		NandEmuPath = game_cfg->NandEmuPath.size() == 0 ? Settings.NandEmuChanPath : game_cfg->NandEmuPath.c_str();
	}

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
	{
		BNRInstance::Instance()->Load(&gameHeader);
		Playlog_Update((char *) gameHeader.id, BNRInstance::Instance()->GetIMETTitle(CONF_GetLanguage()));
	}

	//! Load wip codes
	load_wip_code(gameHeader.id);

	//! Load Ocarina codes
	if (ocarinaChoice)
		ocarina_load_code(Settings.Cheatcodespath, gameHeader.id);

	//! Setup NAND emulation
	SetupNandEmu(NandEmuMode, NandEmuPath, gameHeader);

	//! Setup disc stuff if we load a game
	if(gameHeader.tid == 0)
	{
		//! Setup disc in cIOS and open it
		ret = SetupDisc(gameHeader);
		if (ret < 0)
			Sys_BackToLoader();

		//! Load BCA data for the game
		gprintf("Loading BCA data...");
		ret = do_bca_code(Settings.BcaCodepath, gameHeader.id);
		gprintf("%d\n", ret);
	}

	if(IosLoader::IsHermesIOS(iosChoice))
	{
		if(reloadblock == ON)
		{
			//! Setup IOS reload block
			enable_ES_ioctlv_vector();
			if (gameList.GetGameFS(gameHeader.id) == PART_FS_WBFS)
				mload_close();
		}
	}
	else if(IosLoader::IsD2X(iosChoice))
	{
		// Open ES file descriptor for the d2x patches
		static char es_fs[] ATTRIBUTE_ALIGN(32) = "/dev/es";
		int es_fd = IOS_Open(es_fs, 0);
		if(es_fd >= 0)
		{
			// IOS Reload Block
			if(reloadblock != OFF) {
				BlockIOSReload(es_fd, iosChoice);
			}
			// Check if new patch method for return to works otherwise old method will be used
			if(PatchNewReturnTo(es_fd, returnToChoice) >= 0)
				returnToChoice = 0; // Patch successful, no need for old method

			// Close ES file descriptor
			IOS_Close(es_fd);
		}
	}

	//! Now we can free up the memory used by the game/channel lists
	gameList.clear();
	Channels::DestroyInstance();

	//! Load main.dol or alternative dol into memory, start the game apploader and get game entrypoint
	if(gameHeader.tid == 0)
	{
		gprintf("\tGame Boot\n");
		AppEntrypoint = BootPartition(Settings.dolpath, videoChoice, alternatedol, alternatedoloffset);
		// Reading of game is done we can close devices now
		ShutDownDevices(usbport);
	}
	else
	{
		//! shutdown now and avoid later crashs with free if memory gets overwritten by channel
		ShutDownDevices(DeviceHandler::PartitionToUSBPort(std::max(atoi(NandEmuPath+3)-1, 0)));
		gprintf("\tChannel Boot\n");
		/* Setup video mode */
		Disc_SelectVMode(videoChoice, false, NULL);
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
	gamepatches(videoChoice, videoPatchDolChoice, aspectChoice, languageChoice, countrystrings, viChoice, sneekChoice, Hooktype, fix002, returnToChoice);

	//! Load Code handler if needed
	load_handler(Hooktype, WiirdDebugger, Settings.WiirdDebuggerPause);

	//! Jump to the entrypoint of the game - the last function of the USB Loader
	gprintf("Jumping to game entrypoint: 0x%08X.\n", AppEntrypoint);
	return Disc_JumpToEntrypoint(Hooktype, WDMMenu::GetDolParameter());
}

int GameBooter::BootDIOSMIOS(struct discHdr *gameHdr)
{
	const char *RealPath = GCGames::Instance()->GetPath((const char *) gameHdr->id);

	GameCFG * game_cfg = GameSettings.GetGameCFG(gameHdr->id);
	u8 videoChoice = game_cfg->video == INHERIT ? Settings.videomode : game_cfg->video;
	s8 languageChoice = game_cfg->language == INHERIT ? Settings.language - 1 : game_cfg->language;
	u8 ocarinaChoice = game_cfg->ocarina == INHERIT ? Settings.ocarina : game_cfg->ocarina;
	u8 dmlVideoChoice = game_cfg->DMLVideo == INHERIT ? Settings.DMLVideo : game_cfg->DMLVideo;
	u8 dmlProgressivePatch = game_cfg->DMLProgPatch == INHERIT ? Settings.DMLProgPatch : game_cfg->DMLProgPatch;
	u8 dmlNMMChoice = game_cfg->DMLNMM == INHERIT ? Settings.DMLNMM : game_cfg->DMLNMM;
	u8 dmlActivityLEDChoice = game_cfg->DMLActivityLED == INHERIT ? Settings.DMLActivityLED : game_cfg->DMLActivityLED;
	u8 dmlPADHookChoice = game_cfg->DMLPADHOOK == INHERIT ? Settings.DMLPADHOOK : game_cfg->DMLPADHOOK;
	u8 dmlNoDisc2Choice = game_cfg->DMLNoDisc2 == INHERIT ? Settings.DMLNoDisc2 : game_cfg->DMLNoDisc2;
	u8 dmlWidescreenChoice = game_cfg->DMLWidescreen == INHERIT ? Settings.DMLWidescreen : game_cfg->DMLWidescreen;
	u8 dmlScreenshotChoice = game_cfg->DMLScreenshot == INHERIT ? Settings.DMLScreenshot : game_cfg->DMLScreenshot;
	u8 dmlJPNPatchChoice = game_cfg->DMLJPNPatch == INHERIT ? Settings.DMLJPNPatch : game_cfg->DMLJPNPatch;
	u8 dmlDebugChoice = game_cfg->DMLDebug == INHERIT ? Settings.DMLDebug : game_cfg->DMLDebug;
	
	int currentMIOS = IosLoader::GetMIOSInfo();
	// DIOS MIOS
	if(currentMIOS == DIOS_MIOS || currentMIOS == QUADFORCE_USB)
	{
		// Check Main GameCube Path location
		if(strncmp(Settings.GameCubePath, "sd", 2) == 0 || strncmp(DeviceHandler::PathToFSName(Settings.GameCubePath), "FAT", 3) != 0)
		{
			WindowPrompt(tr("Error:"), tr("To run GameCube games with DIOS MIOS you need to set your 'Main GameCube Path' to an USB FAT32 partition."), tr("OK"));
			return 0;
		}

		// Check current game location
		if(strncmp(RealPath, "sd", 2) == 0)
		{
			WindowPrompt(tr("The game is on SD Card."), tr("To run GameCube games with DIOS MIOS you need to place them on an USB FAT32 partition."), tr("OK"));
			// Todo: Add here copySD2USB.
			return 0;
		}

		// Check if the partition is the first partition on the drive
		int part_num = atoi(Settings.GameCubePath+3);
		int portPart = DeviceHandler::PartitionToPortPartition(part_num-USB1);
		int usbport = DeviceHandler::PartitionToUSBPort(part_num-USB1);
		PartitionHandle * usbHandle = DeviceHandler::Instance()->GetUSBHandleFromPartition(part_num-USB1);
		if(usbHandle->GetPartitionNum(portPart))
		{
			WindowPrompt(tr("Error:"), tr("To run GameCube games with DIOS MIOS you need to set your 'Main GameCube Path' on the first partition of the Hard Drive."), tr("OK"));
			return 0;
		}

		// Check if the partition is primary
		if(usbHandle->GetPartitionTableType(portPart) != MBR)
		{
			WindowPrompt(tr("Error:"), tr("To run GameCube games with DIOS MIOS you need to set your 'Main GameCube Path' on a primary partition."), tr("OK"));
			return 0;
		}
		
		// Check HDD sector size. Only 512 bytes/sector is supported by DIOS MIOS
		if(hdd_sector_size[usbport] != BYTES_PER_SECTOR)
		{
			WindowPrompt(tr("Error:"), tr("To run GameCube games with DIOS MIOS you need to use a 512 bytes/sector Hard Drive."), tr("OK"));
			return 0;
		}

		if(usbHandle->GetPartitionClusterSize(usbHandle->GetLBAStart(portPart)) > 32768)
		{
			WindowPrompt(tr("Error:"), tr("To run GameCube games with DIOS MIOS you need to use a partition with 32k bytes/cluster or less."), tr("OK"));
			return 0;
		}
	}

	// DIOS MIOS Lite
	else if(currentMIOS == DIOS_MIOS_LITE || currentMIOS == QUADFORCE)
	{
		if(((gameHdr->type == TYPE_GAME_GC_IMG) || (gameHdr->type == TYPE_GAME_GC_EXTRACTED)) && strncmp(RealPath, "usb", 3) == 0)
		{
			if(!GCGames::Instance()->CopyUSB2SD(gameHdr))
				return 0;

			RealPath = GCGames::Instance()->GetPath((const char *) gameHdr->id);
		}
	}


	// Check DIOS MIOS config for specific versions
	if(currentMIOS != QUADFORCE)
	{
		if(IosLoader::GetDMLVersion() < DML_VERSION_DML_1_2)
		{
			WindowPrompt(tr("Error:"), tr("You need to install DIOS MIOS Lite v1.2 or a newer version."), tr("OK"));
			return 0;
		}
		if(dmlWidescreenChoice && IosLoader::GetDMLVersion() < DML_VERSION_DM_2_1) // DML Force Widescreen setting : added in DM v2.1+, config v1.
		{
			if(Settings.DMLWidescreen) // Display the warning only if set as Global setting. Individual game setting is not displayed.
				WindowPrompt(tr("Warning:"), tr("The Force Widescreen setting requires DIOS MIOS v2.1 or more. This setting will be ignored."), tr("OK"));
			dmlWidescreenChoice = OFF;
		}
		if(dmlNoDisc2Choice && (IosLoader::GetDMLVersion() < DML_VERSION_DM_2_2_2 || IosLoader::GetDMLVersion() > DML_VERSION_DML_2_2_1)) // DML NoDisc+ setting : Added in DM 2.2 upate 2, config v2, removed in DM(L) v2.3
		{
			if(Settings.DMLNoDisc2) // Display the warning only if set as Global setting. Individual game setting is not displayed.
				WindowPrompt(tr("Warning:"), tr("The No Disc+ setting requires DIOS MIOS 2.2 update2. This setting will be ignored."), tr("OK"));
			dmlNoDisc2Choice = false;
		}
	}
	
	// Check Ocarina and cheat file location. the .gct file need to be located on the same partition than the game.
	if(gameHdr->type != TYPE_GAME_GC_DISC && ocarinaChoice && strcmp(DeviceHandler::GetDevicePrefix(RealPath), DeviceHandler::GetDevicePrefix(Settings.Cheatcodespath)) != 0)
	{
		char path[255], destPath[255];
		int res = -1;
		snprintf(path, sizeof(path), "%s%.6s.gct", Settings.Cheatcodespath, (char *)gameHdr->id);
		snprintf(destPath, sizeof(destPath), "%s:/DMLTemp.gct", DeviceHandler::GetDevicePrefix(RealPath));
		
		gprintf("DML: Copying %s to %s \n", path, destPath);
		res = CopyFile(path, destPath);
		if(res < 0)
		{
			gprintf("DML: Couldn't copy the file. ret %d. Ocarina Disabled\n", res);
			RemoveFile(destPath);
			ocarinaChoice = false;
		}
	}

	// Check if game has multi Discs
	bool bootDisc2 = false;
	if(gameHdr->type != TYPE_GAME_GC_DISC && gameHdr->disc_no == 0 && currentMIOS != QUADFORCE)
	{
		char disc2Path[255];
		snprintf(disc2Path, sizeof(disc2Path), "%s", RealPath);
		char *pathPtr = strrchr(disc2Path, '/');
		if(pathPtr) *pathPtr = 0;
		snprintf(disc2Path, sizeof(disc2Path), "%s/disc2.iso", disc2Path);
		if(CheckFile(disc2Path))
		{
			int choice = WindowPrompt(gameHdr->title, tr("This game has multiple discs. Please select the disc to launch."), tr("Disc 1"), tr("Disc 2"), tr("Cancel"));
			if(choice == 0)
				return 0;
			else if(choice == 2)
				bootDisc2 = true;
		}	
	}

	const char *gcPath = strchr(RealPath, '/');
	if(!gcPath) gcPath = "";

	char gamePath[255];
	snprintf(gamePath, sizeof(gamePath), "%s", gcPath);

	if(bootDisc2)
	{
		char *pathPtr = strrchr(gamePath, '/');
		if(pathPtr) *pathPtr = 0;
		snprintf(gamePath, sizeof(gamePath), "%s/disc2.iso", gamePath);
	}

	ExitApp();

	// Game ID
	memcpy((u8 *)Disc_ID, gameHdr->id, 6);
	DCFlushRange((u8 *)Disc_ID, 6);

	// *(vu32*)0xCC003024 |= 7; // DML 1.1- only?

	DML_CFG *dml_config = (DML_CFG *) DML_CONFIG_ADDRESS;
	memset(dml_config, 0, sizeof(DML_CFG));

	// Magic and version for DML
	dml_config->Magicbytes = DML_MAGIC;
	dml_config->Version = IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_2 ? 0x00000002 : 0x00000001;

	// Select disc source
	if((gameHdr->type == TYPE_GAME_GC_IMG) || (gameHdr->type == TYPE_GAME_GC_EXTRACTED))
	{
		dml_config->Config |= DML_CFG_GAME_PATH;
		strncpy(dml_config->GamePath, gamePath, sizeof(dml_config->GamePath));
		// Extended NoDisc patch
		if(dmlNoDisc2Choice && IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_2_2 && IosLoader::GetDMLVersion() < DML_VERSION_DML_2_3m)
			dml_config->Config |= DML_CFG_NODISC2;	// used by v2.2 update2 as an Extended NoDisc patching

		gprintf("DML: Loading game %s\n", dml_config->GamePath);
	}
	else
	{
		dml_config->Config |= DML_CFG_BOOT_DISC;
	}

	// setup cheat and path
	if(ocarinaChoice)
	{
		// Check if the .gct folder is on the same partition than the game, if not load the temporary .gct file.
		if(strcmp(DeviceHandler::GetDevicePrefix(RealPath), DeviceHandler::GetDevicePrefix(Settings.Cheatcodespath)) == 0)
		{
			const char *CheatPath = strchr(Settings.Cheatcodespath, '/');
			if(!CheatPath) CheatPath = "";
			snprintf(dml_config->CheatPath, sizeof(dml_config->CheatPath), "%s%.6s.gct", CheatPath, (char *)gameHdr->id);
		}
		else if(gameHdr->type != TYPE_GAME_GC_DISC)
		{
			snprintf(dml_config->CheatPath, sizeof(dml_config->CheatPath), "DMLTemp.gct");
		}

		dml_config->Config |= DML_CFG_CHEATS | DML_CFG_CHEAT_PATH;
		gprintf("DML: Loading cheat %s\n", dml_config->CheatPath);
	}

	// other DML configs
	if(dmlPADHookChoice)
		dml_config->Config |= DML_CFG_PADHOOK;
	if(dmlActivityLEDChoice)
		dml_config->Config |= DML_CFG_ACTIVITY_LED;
	if(dmlNMMChoice)
		dml_config->Config |= dmlNMMChoice == ON ? DML_CFG_NMM : DML_CFG_NMM_DEBUG;
	if(dmlDebugChoice)
		dml_config->Config |= dmlDebugChoice == ON ? DML_CFG_DEBUGGER : DML_CFG_DEBUGWAIT;
	if(dmlWidescreenChoice)
		dml_config->Config |= DML_CFG_FORCE_WIDE;
	if(dmlScreenshotChoice)
	{
		dml_config->Config |= DML_CFG_SCREENSHOT;
		dml_config->Config |= DML_CFG_PADHOOK;
	}
	if(bootDisc2 && IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_6_0)
		dml_config->Config |= DML_CFG_BOOT_DISC2;


	// Setup Video Mode
	if(dmlVideoChoice == DML_VIDEO_NONE)				// No video mode
	{
		dml_config->VideoMode = DML_VID_NONE;
	}
	else
	{
		if(dmlVideoChoice == DML_VIDEO_AUTO)			// Auto select video mode
		{
			dml_config->VideoMode = DML_VID_DML_AUTO;
			Disc_SelectVMode(VIDEO_MODE_DISCDEFAULT, false, NULL);
		}
		else											// Force user choice
		{
			Disc_SelectVMode(videoChoice, false, &dml_config->VideoMode);
			if(!(dml_config->VideoMode & DML_VID_DML_AUTO))
				dml_config->VideoMode |= DML_VID_FORCE;
		}	
		Disc_SetVMode();
	}
	
	if(dmlProgressivePatch)
		dml_config->VideoMode |= DML_VID_PROG_PATCH;


	DCFlushRange(dml_config, sizeof(DML_CFG));
	memcpy((u8*)DML_CONFIG_ADDRESS_V1_2, dml_config, sizeof(DML_CFG));
	DCFlushRange((u8*)DML_CONFIG_ADDRESS_V1_2, sizeof(DML_CFG));

	// print the config set for DML
	gprintf("DML: setup configuration 0x%X\n", dml_config->Config);
	gprintf("DML: setup video mode 0x%X\n", dml_config->VideoMode);

	// Set Sram flags
	bool progressive = (dml_config->VideoMode & DML_VID_FORCE_PROG) || (dml_config->VideoMode & DML_VID_PROG_PATCH);
	PatchSram(languageChoice, true, progressive);

	/* NTSC-J Patch */	// Thanks to Fix94
	u8 *diskid = (u8 *) Disc_ID;
	if(dmlJPNPatchChoice && diskid[3] == 'J')
		*HW_PPCSPEED = 0x0002A9E0;

	gprintf("\nLoading BC for GameCube\n");
	WII_Initialize();
	return WII_LaunchTitle(0x0000000100000100ULL);
}

int GameBooter::BootDevolution(struct discHdr *gameHdr)
{
	const char *RealPath = GCGames::Instance()->GetPath((const char *) gameHdr->id);

	GameCFG * game_cfg = GameSettings.GetGameCFG(gameHdr->id);
	u8 videoChoice = game_cfg->video == INHERIT ? Settings.videomode : game_cfg->video;
	int languageChoice = game_cfg->language == INHERIT ? Settings.language -1 : game_cfg->language;
	u8 devoMCEmulation = game_cfg->DEVOMCEmulation == INHERIT ? Settings.DEVOMCEmulation : game_cfg->DEVOMCEmulation;
	u8 devoActivityLEDChoice = game_cfg->DEVOActivityLED == INHERIT ? Settings.DEVOActivityLED : game_cfg->DEVOActivityLED;
	u8 devoWidescreenChoice = game_cfg->DEVOWidescreen == INHERIT ? Settings.DEVOWidescreen : game_cfg->DEVOWidescreen;
	u8 devoFZeroAXChoice = game_cfg->DEVOFZeroAX == INHERIT ? Settings.DEVOFZeroAX : game_cfg->DEVOFZeroAX;
	u8 devoTimerFixChoice = game_cfg->DEVOTimerFix == INHERIT ? Settings.DEVOTimerFix : game_cfg->DEVOTimerFix;
	u8 devoDButtonsChoice = game_cfg->DEVODButtons == INHERIT ? Settings.DEVODButtons : game_cfg->DEVODButtons;

	if(gameHdr->type == TYPE_GAME_GC_DISC)
	{
		WindowPrompt(tr("Error:"), tr("To run GameCube games from Disc you need to set the GameCube mode to MIOS in the game settings."), tr("OK"));
		return 0;
	}

	if(!CheckAHBPROT())
	{
		WindowPrompt(tr("Error:"), tr("Devolution requires AHB access! Please launch USBLoaderGX from HBC or from an updated channel or forwarder."), tr("OK"));
		return 0;
	}

	if(strncmp(DeviceHandler::PathToFSName(RealPath), "FAT", 3) != 0)
	{
		WindowPrompt(tr("Error:"), tr("To run GameCube games with Devolution you need to use a FAT32 partition."), tr("OK"));
		return 0;
	}

	// Check if Devolution is available
	u8 *loader_bin = NULL;
	int DEVO_version = 0;
	char DEVO_loader_path[100];
	snprintf(DEVO_loader_path, sizeof(DEVO_loader_path), "%sloader.bin", Settings.DEVOLoaderPath);
	FILE *f = fopen(DEVO_loader_path, "rb");
	if(f)
	{
		fseek(f, 0, SEEK_END);
		u32 size = ftell(f);
		rewind(f);
		loader_bin = (u8*)MEM2_alloc(size);
		if(!loader_bin) 
		{
			fclose(f);
			WindowPrompt(tr("Error:"), tr("Devolution's loader.bin file can't be loaded."), tr("OK"));
			return 0;
		}
		fread(loader_bin, 1, size, f);

		//read Devolution version
		char version[5];
		fseek(f, 23, SEEK_SET);
		fread(version, 1, 4, f);
		char *ptr = strchr(version, ' ');
		if(ptr) *ptr = 0;
		else version[4] = 0;
		DEVO_version = atoi(version);

		fclose(f);
	}
	else
	{
		WindowPrompt(tr("Error:"), tr("To run GameCube games with Devolution you need the loader.bin file in your Devolution Path."), tr("OK"));
		return 0;
	}


	// Devolution config
	DEVO_CGF *devo_config = (DEVO_CGF*)0x80000020;

	char disc1[100];
	char disc2[100];
	bool multiDisc = false;
	char DEVO_memCard[100];
	snprintf(disc1, sizeof(disc1), "%s", RealPath);
	
	snprintf(disc2, sizeof(disc2), "%s", RealPath);
	char *pathPtr = strrchr(disc2, '/');
	if(pathPtr) *pathPtr = 0;
	snprintf(disc2, sizeof(disc2), "%s/disc2.iso", disc2);
	if(CheckFile(disc2))
		multiDisc = true;

	snprintf(DEVO_memCard, sizeof(DEVO_memCard), "%s", RealPath); // Set memory card folder to Disc1 folder
	char *ptr = strrchr(DEVO_memCard, '/');
	if(ptr) *ptr = 0; 

	// Make sure the directory exists
	char devoPath[20];
	snprintf(devoPath, sizeof(devoPath), "%s:/apps/gc_devo", DeviceHandler::GetDevicePrefix(RealPath));
	CreateSubfolder(devoPath);
	
	// Get the starting cluster (and device ID) for the ISO file 1
	struct stat st1;
	stat(disc1, &st1);
	
	// Get the starting cluster for the ISO file 2
	struct stat st2;
	if(multiDisc)
		stat(disc2, &st2);
	
	// setup Devolution
	memset(devo_config, 0, sizeof(*devo_config));
	devo_config->signature = DEVO_SIG;
	devo_config->version = DEVO_CONFIG_VERSION;
	// st1.st_dev doesn't work with our current device type. It returns Wii_UMS 'WUMS' instead of Wii_USB 'WUSB'.
	// Only last two letters are returned by DevkitPro, so we set them manually to Devolution config.
	devo_config->device_signature = st1.st_dev == 'SD' ? 'SD' : 'SB'; // Set device type.
	devo_config->disc1_cluster = st1.st_ino;			// set starting cluster for first disc ISO file
	if(multiDisc)
		devo_config->disc2_cluster = st2.st_ino;		// set starting cluster for second disc ISO file
	
	// Devolution configs
	// use wifi logging if USB gecko is not found in slot B
	// devo_config->options |= DEVO_CFG_WIFILOG;			// removed on Tueidj request
	if(devoWidescreenChoice && DEVO_version >= 188)
		devo_config->options |= DEVO_CFG_WIDE;
	if(!devoActivityLEDChoice && DEVO_version >= 142)
		devo_config->options |= DEVO_CFG_NOLED;				// ON by default
	if(devoFZeroAXChoice && DEVO_version >= 196)
		devo_config->options |= DEVO_CFG_FZERO_AX;
	if(devoTimerFixChoice && DEVO_version >= 196)
		devo_config->options |= DEVO_CFG_TIMER_FIX;
	if(devoDButtonsChoice && DEVO_version >= 200)
		devo_config->options |= DEVO_CFG_D_BUTTONS;
	
	// check memory card
	if(devoMCEmulation == DEVO_MC_OFF)
	{
		devo_config->memcard_cluster = 0;
		snprintf(DEVO_memCard, sizeof(DEVO_memCard), "Original");
	}
	else 
	{
		if(devoMCEmulation == DEVO_MC_INDIVIDUAL)
		{
			snprintf(DEVO_memCard, sizeof(DEVO_memCard), "%s/memcard_%.6s.bin", DEVO_memCard, (const char *) gameHdr->id);
		}
		else // same for all games
		{
			snprintf(DEVO_memCard, sizeof(DEVO_memCard), "%s:/apps/gc_devo/memcard.bin", DeviceHandler::GetDevicePrefix(RealPath));
		}
		
		// check if file doesn't exist or is less than 512KB (59 Blocks)
		struct stat st;
		if (stat(DEVO_memCard, &st) == -1 || st.st_size < 1<<19)
		{
			// need to enlarge or create it
			FILE *f = fopen(DEVO_memCard, "wb");
			if(f)
			{
				// make it 16MB
				ShowProgress(tr("Please wait..."), 0, 0);
				gprintf("Resizing memcard file...\n");
				fseek(f, (16 << 20) - 1, SEEK_SET);
				fputc(0, f);
				fclose(f);
				if (stat(DEVO_memCard, &st)==-1 || st.st_size < 1<<19)
				{
					// it still isn't big enough. Give up.
					st.st_ino = 0;
				}
				ProgressStop();
			}
			else
			{
				// couldn't open or create the memory card file
				st.st_ino = 0;
			}
		}
		devo_config->memcard_cluster = st.st_ino;
	}


	// read 32 bytes of disc 1 to the start of MEM1
	FILE *iso_file = fopen(disc1, "rb");
	if(!iso_file) 
	{
		WindowPrompt(tr("Error:"), tr("File not found."), tr("OK"));
		return 0;
	}
	u8 *lowmem = (u8*)0x80000000;
	fread(lowmem, 1, 32, iso_file);
	fclose(iso_file);
	
	// setup video mode
	Disc_SelectVMode(videoChoice, true, NULL);
	Disc_SetVMode();

	// Set sram flags
	PatchSram(languageChoice, false, false);

	// flush disc ID and Devolution config out to memory
	DCFlushRange(lowmem, 64);
	
	ExitApp();
	IosLoader::ReloadIosKeepingRights(58); // reload IOS 58 with AHBPROT rights
	
	gprintf("DEVO: Loading game: %s\n", disc1);
	gprintf("DEVO: Memory Card: %s\n\n", DEVO_memCard);
	gprintf("%.72s", (const char*)loader_bin + 4);

	u32 cpu_isr;
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	_CPU_ISR_Disable( cpu_isr );
	__exception_closeall();
	LAUNCH_DEVO();
	_CPU_ISR_Restore( cpu_isr );
	return 0;
}

void GameBooter::PatchSram(int language, bool patchVideoMode, bool progressive)
{
	syssram *sram = __SYS_LockSram();

	// Setup language flag
	if(language >= GC_ENGLISH && language <= GC_DUTCH)
	{
		sram->lang = language;
	}
	else // console default
	{
		sram->lang = GC_ENGLISH;
		if(CONF_GetLanguage() >= CONF_LANG_ENGLISH && CONF_GetLanguage() <= CONF_LANG_DUTCH)
		{
			sram->lang = CONF_GetLanguage()-1;
		}
	}
	gprintf("Sram: Language set to 0x%X\n", sram->lang);

	// Setup Video mode flags
	if(patchVideoMode)
	{
		if(progressive)
			sram->flags |= 0x80; //set progressive flag
		else
			sram->flags &= 0x7F; //clear progressive flag

		if (*Video_Mode == VI_NTSC)
		{
			sram->flags &= ~1;	// Clear bit 0 to set the video mode to NTSC
			sram->ntd &= 0xBF; //clear pal60 flag
		}
		else 
		{
			sram->flags |= 1;	// Set bit 0 to set the video mode to PAL
			sram->ntd |= 0x40; //set pal60 flag
		}

		gprintf("Sram: flags set to 0x%X\n", sram->flags);
		gprintf("Sram: ntd set to 0x%X\n", sram->ntd);
	}

	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram())
		usleep(100);


	// Log Sram's first 20 bytes
/*	char srambuff[64];
	sram = __SYS_LockSram();
	memcpy(srambuff, sram, 20);
	__SYS_UnlockSram(0);

	int i;
	gprintf("SRAM Hex View\n\n");
	gprintf("     \t\t 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F\n");
	for (i=0;i<20;i++)
	{
		if( (i%16) == 0 )
			gprintf("\n0x%d0h\t\t", i/16);
		
		gprintf("%02X  ", srambuff[i]);
	}
*/
}
