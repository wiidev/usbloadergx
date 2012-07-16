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

// Devolution config
u8 *loader_bin = NULL;
static DEVO_CGF *DEVO_CONFIG = (DEVO_CGF*)0x80000020;

extern "C"
{
	syssram* __SYS_LockSram();
	u32 __SYS_UnlockSram(u32 write);
	u32 __SYS_SyncSram(void);
}

int GameBooter::BootGCMode(struct discHdr *gameHdr)
{
	const char *RealPath = GCGames::Instance()->GetPath((const char *) gameHdr->id);

	// check the settings
	GameCFG * game_cfg = GameSettings.GetGameCFG(gameHdr->id);
	u8 videoChoice = game_cfg->video == INHERIT ? Settings.videomode : game_cfg->video;
	u8 languageChoice = game_cfg->language == INHERIT ? 6 : game_cfg->language;
	u8 ocarinaChoice = game_cfg->ocarina == INHERIT ? Settings.ocarina : game_cfg->ocarina;
	u8 GCMode = game_cfg->GameCubeMode == INHERIT ? Settings.GameCubeMode : game_cfg->GameCubeMode;
	u8 dmlProgressivePatch = game_cfg->DMLProgPatch == INHERIT ? Settings.DMLProgPatch : game_cfg->DMLProgPatch;
	u8 dmlNMMChoice = game_cfg->DMLNMM == INHERIT ? Settings.DMLNMM : game_cfg->DMLNMM;
	u8 dmlActivityLEDChoice = game_cfg->DMLActivityLED == INHERIT ? Settings.DMLActivityLED : game_cfg->DMLActivityLED;
	u8 dmlPADHookChoice = game_cfg->DMLPADHOOK == INHERIT ? Settings.DMLPADHOOK : game_cfg->DMLPADHOOK;
	u8 dmlNoDiscChoice = game_cfg->DMLNoDisc == INHERIT ? Settings.DMLNoDisc : game_cfg->DMLNoDisc;
	u8 dmlDebugChoice = game_cfg->DMLDebug == INHERIT ? Settings.DMLDebug : game_cfg->DMLDebug;
	u8 devoMCEmulation = game_cfg->DEVOMCEmulation == INHERIT ? Settings.DEVOMCEmulation : game_cfg->DEVOMCEmulation;
	
	
	if(GCMode == GC_MODE_DEVOLUTION)
	{
	
		if(gameHdr->type == TYPE_GAME_GC_DISC)
		{
			WindowPrompt(tr("Error:"), tr("To run GameCube games from Disc you need to set the GameCube mode to MIOS in the game settings."), tr("OK"));
			return 0;
		}
		
		
		// Check if Devolution is available
		char DEVO_loader_path[100];
		snprintf(DEVO_loader_path, sizeof(DEVO_loader_path), "%sloader.bin", Settings.DEVOLoaderPath);
 		FILE *f = fopen(DEVO_loader_path, "rb");
 		if(f)
 		{
 			fseek(f, 0, SEEK_END);
 			u32 size = ftell(f);
 			rewind(f);
 			loader_bin = (u8*)MEM2_alloc(size);
 			fread(loader_bin, 1, size, f);
			fclose(f);
 		}
		else
		{
			WindowPrompt(tr("Error:"), tr("To run GameCube games with Devolution you need the loader.bin file in your Devolution Path."), tr("OK"));
			return 0;
		}

		// Get the Game's data
		char game_partition[5];
		snprintf(game_partition, sizeof(game_partition), DeviceHandler::GetDevicePrefix(RealPath));
		
		char disc1[100];
		//char disc2[100];
		char DEVO_memCard[100];
		snprintf(disc1, sizeof(disc1), RealPath);
		snprintf(DEVO_memCard, sizeof(DEVO_memCard), RealPath); // Set memory card folder to Disc1 folder
		char *ptr = strrchr(DEVO_memCard, '/');
		if(ptr) *ptr = 0; 

		// Make sure the directory exists
		char devoPath[20];
		snprintf(devoPath, sizeof(devoPath), "%s:/apps/gc_devo", game_partition);
		CreateSubfolder(devoPath);
		
		// Get the starting cluster (and device ID) for the ISO file 1
		struct stat st1;
		stat(disc1, &st1);
		
		// Get the starting cluster for the ISO file 2
		//struct stat st2;
		//stat(disc2, &st2);
		
		// setup Devolution
		memset(DEVO_CONFIG, 0, sizeof(*DEVO_CONFIG));
		DEVO_CONFIG->signature = DEVO_SIG;
		DEVO_CONFIG->version = DEVO_VERSION;
		DEVO_CONFIG->device_signature = st1.st_dev;
		DEVO_CONFIG->disc1_cluster = st1.st_ino;			// set starting cluster for first disc ISO file
		//DEVO_CONFIG->disc2_cluster = st2.st_ino;			// set starting cluster for second disc ISO file
		

		// check memory card
		if(devoMCEmulation == DEVO_MC_OFF)
		{
			DEVO_CONFIG->memcard_cluster = 0;
			snprintf(DEVO_memCard, sizeof(DEVO_memCard), "Original");
		}
		else 
		{
			if(devoMCEmulation == DEVO_MC_INDIVIDUAL)
			{
				snprintf(DEVO_memCard, sizeof(DEVO_memCard), "%s/memcard_%s.bin", DEVO_memCard, (const char *) gameHdr->id);
			}
			else // same for all games
			{
				snprintf(DEVO_memCard, sizeof(DEVO_memCard), "%s:/apps/gc_devo/memcard.bin", game_partition);
			}
			
			// check if file doesn't exist or is less than 16MB
			struct stat st;
			if (stat(DEVO_memCard, &st) == -1 || st.st_size < 16<<20)
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
					if (stat(DEVO_memCard, &st)==-1 || st.st_size < 16<<20)
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
			DEVO_CONFIG->memcard_cluster = st.st_ino;
		}

		// setup video mode
		Disc_SelectVMode(VIDEO_MODE_DISCDEFAULT, false);
		Disc_SetVMode();

		
		// read 32 bytes of disc 1 to the start of MEM1
		FILE *iso_file = fopen(disc1, "rb");
		u8 *lowmem = (u8*)0x80000000;
		fread(lowmem, 1, 32, iso_file);
		fclose(iso_file);


		// flush disc ID and Devolution config out to memory
		DCFlushRange(lowmem, 64);
		puts((const char*)loader_bin + 4);

		gprintf("DEVO: Loading game: %s\n", disc1);
		gprintf("DEVO: Memory Card: %s\n", DEVO_memCard);
		ExitApp();
		LAUNCH_DEVO();
	}
	
	int currentMIOS = IosLoader::GetMIOSInfo();
	// DIOS MIOS
	if(currentMIOS == DIOS_MIOS)
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
		
		// Check DML NoDisc setting
		if(dmlNoDiscChoice)
		{
			WindowPrompt(tr("Warning:"), tr("The No Disc setting is not used anymore by DIOS MIOS v2. Now you need to place a disc in your drive."), tr("OK"));
		}
		
		// Check current GCT location
		if((ocarinaChoice) && strncmp(Settings.GameCubePath, Settings.Cheatcodespath, 4) != 0) // Checking "USBx"
		{
			int choice = WindowPrompt(tr("Warning:"), tr("The GCT Cheatcodes Path and this game are not on the same partition. Run the game without Ocarina?"), tr("OK"), tr("Cancel"));
			if(choice == 0)
				return false;
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
		
		// Check current GCT location
		if((ocarinaChoice) && strncmp(Settings.Cheatcodespath, "SD", 2) != 0)
		{
			int choice = WindowPrompt(tr("Warning:"), tr("The GCT Cheatcodes Path must be on SD card. Run the game without Ocarina?"), tr("OK"), tr("Cancel"));
			if(choice == 0)
				return false;
		}
	}

	// MIOS
	else if(gameHdr->type == TYPE_GAME_GC_DISC) // Launch disc based games from real MIOS
	{
		ExitApp();
		gprintf("\nLoading BC for GameCube");
		WII_Initialize();
		return WII_LaunchTitle(0x0000000100000100ULL);
	}
	else
	{
		WindowPrompt(tr("Error:"), tr("You need to install DIOS MIOS to run GameCube games from USB or DIOS MIOS Lite to run them from SD card"), tr("OK"));
		return 0;
	}


	const char *gcPath = strchr(RealPath, '/');
	if(!gcPath) gcPath = "";

	char gamePath[255];
	snprintf(gamePath, sizeof(gamePath), "%s", gcPath);

	ExitApp();
	gprintf("\nLoading BC for GameCube\n");

	// Game ID
	memcpy((u8 *)Disc_ID, gameHdr->id, 6);
	DCFlushRange((u8 *)Disc_ID, 6);

	*(vu32*)0xCC003024 |= 7;

	Disc_SelectVMode(videoChoice, dmlProgressivePatch);
	Disc_SetVMode();

	DML_CFG *dml_config = (DML_CFG *) DML_CONFIG_ADDRESS;
	memset(dml_config, 0, sizeof(DML_CFG));

	// Magic and version for DML
	dml_config->Magicbytes = DML_MAGIC;
	dml_config->Version = DML_VERSION;

	// Select disc source
	if((gameHdr->type == TYPE_GAME_GC_IMG) || (gameHdr->type == TYPE_GAME_GC_EXTRACTED))
	{
		dml_config->Config |= DML_CFG_GAME_PATH;
		strncpy(dml_config->GamePath, gamePath, sizeof(dml_config->GamePath));
		// use no disc patch
		if(dmlNoDiscChoice)
			dml_config->Config |= DML_CFG_NODISC;

		gprintf("DML: Loading game %s\n", dml_config->GamePath);
	}
	else
	{
		dml_config->Config |= DML_CFG_BOOT_DISC;
	}

	// setup cheat and path
	if(ocarinaChoice)
	{
		dml_config->Config |= DML_CFG_CHEATS | DML_CFG_CHEAT_PATH;
		const char *CheatPath = strchr(Settings.Cheatcodespath, '/');
		if(!CheatPath) CheatPath = "";
		snprintf(dml_config->CheatPath, sizeof(dml_config->CheatPath), "%s%.6s.gct", CheatPath, (char *)gameHdr->id);

		gprintf("DML: Loading cheat %s\n", dml_config->CheatPath);
	}

	// other DMl configs
	if(dmlPADHookChoice)
		dml_config->Config |= DML_CFG_PADHOOK;
	if(dmlActivityLEDChoice)
		dml_config->Config |= DML_CFG_ACTIVITY_LED;
	if(dmlNMMChoice)
		dml_config->Config |= dmlNMMChoice == ON ? DML_CFG_NMM : DML_CFG_NMM_DEBUG;
	if(dmlDebugChoice)
		dml_config->Config |= dmlDebugChoice == ON ? DML_CFG_DEBUGGER : DML_CFG_DEBUGWAIT;

	// internal DML video mode methods
	bool PAL60 = CONF_GetEuRGB60() > 0;
	u32 tvmode = CONF_GetVideo();
	u8 *diskid = (u8 *) Disc_ID;

	switch(videoChoice)
	{
		case VIDEO_MODE_SYSDEFAULT:
			if(tvmode == CONF_VIDEO_NTSC)
				dml_config->VideoMode = DML_VID_FORCE_NTSC;
			else if(PAL60)
			{
				if(CONF_GetProgressiveScan() > 0)
				{
					dml_config->VideoMode = DML_VID_FORCE_PROG;
				}
				else
					dml_config->VideoMode = DML_VID_FORCE_PAL60;
			}
			else
				dml_config->VideoMode = DML_VID_FORCE_PAL50;
			break;
		case VIDEO_MODE_DISCDEFAULT: // DEFAULT (DISC/GAME)
			switch (diskid[3])
			{
				// PAL
				case 'D':
				case 'F':
				case 'P':
				case 'X':
				case 'Y':
					if(tvmode == CONF_VIDEO_NTSC) // Force PAL output (576i) for NTSC consoles.
						dml_config->VideoMode = DML_VID_FORCE_PAL50;
					else if(PAL60)
					{
						if(CONF_GetProgressiveScan() > 0)
						{
							dml_config->VideoMode = DML_VID_FORCE_PROG;
						}
						else
							dml_config->VideoMode = DML_VID_FORCE_PAL60;
					}
					else
						dml_config->VideoMode = DML_VID_FORCE_PAL50;
					break;
				// NTSC
				case 'E':
				case 'J':
					dml_config->VideoMode = DML_VID_FORCE_NTSC;
					break;
				default:
					dml_config->VideoMode = DML_VID_DML_AUTO;
					break;
			}
			break;
		case VIDEO_MODE_PAL50:
			dml_config->VideoMode = DML_VID_FORCE_PAL50 | DML_VID_FORCE;
			break;
		case VIDEO_MODE_PAL60:
			dml_config->VideoMode = DML_VID_FORCE_PAL60 | DML_VID_FORCE;
			break;
		case VIDEO_MODE_NTSC:
			dml_config->VideoMode = DML_VID_FORCE_NTSC | DML_VID_FORCE;
			break;
		case VIDEO_MODE_PAL480P:
		case VIDEO_MODE_NTSC480P:
			dml_config->VideoMode = DML_VID_FORCE_PROG | DML_VID_FORCE;
			break;
		default:
			dml_config->VideoMode = DML_VID_DML_AUTO;
			break;
	}
	
	if(dmlProgressivePatch)
		dml_config->VideoMode |= DML_VID_PROG_PATCH;


	DCFlushRange(dml_config, sizeof(DML_CFG));
	memcpy((u8*)DML_CONFIG_ADDRESS_V1_2, dml_config, sizeof(DML_CFG));
	DCFlushRange((u8*)DML_CONFIG_ADDRESS_V1_2, sizeof(DML_CFG));

	// print the config set for DML
	gprintf("DML: setup configuration 0x%X\n", dml_config->Config);
	gprintf("DML: setup video mode 0x%X\n", dml_config->VideoMode);

	syssram *sram = __SYS_LockSram();
	if(dmlProgressivePatch) {
		sram->flags |= 0x80; //set progressive flag
	}
	else {
		sram->flags &= 0x7F; //clear progressive flag
	}
	// setup video mode flags
	if (*Video_Mode == VI_NTSC) {
		sram->flags &= ~1;	// Clear bit 0 to set the video mode to NTSC
		sram->ntd &= 0xBF; //clear pal60 flag
	}
	else {
		sram->flags |= 1;	// Set bit 0 to set the video mode to PAL
		sram->ntd |= 0x40; //set pal60 flag
	}
	
	// Set language flag
	if(languageChoice <= GC_DUTCH)
	{
		sram->lang = languageChoice;
	}	
	else // console default
	{
		sram->lang = GC_ENGLISH;
		if(CONF_GetLanguage() >= CONF_LANG_ENGLISH && CONF_GetLanguage() <= CONF_LANG_DUTCH)
		{
			sram->lang = CONF_GetLanguage()-1;
		}
	}
	gprintf("DML: setup language 0x%X\n", sram->lang);

	__SYS_UnlockSram(1); // 1 -> write changes

	while(!__SYS_SyncSram())
		usleep(100);

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
	Disc_SelectVMode(videoselected, true);

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
		Disc_SelectVMode(videoChoice, true);
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
	gamepatches(videoChoice, aspectChoice, languageChoice, countrystrings, viChoice, sneekChoice, Hooktype, fix002, returnToChoice);

	//! Load Code handler if needed
	load_handler(Hooktype, WiirdDebugger, Settings.WiirdDebuggerPause);

	//! Jump to the entrypoint of the game - the last function of the USB Loader
	gprintf("Jumping to game entrypoint: 0x%08X.\n", AppEntrypoint);
	return Disc_JumpToEntrypoint(Hooktype, WDMMenu::GetDolParameter());
}
