/****************************************************************************
 * Copyright (C) 2015 Cyan
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
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <ogc/machine/processor.h>

#include "neek.hpp"
#include "memory/mem2.h"
#include "Controls/DeviceHandler.hpp"
#include "FileOperations/fileops.h"
#include "settings/CSettings.h"
#include "sys.h"
#include "gecko.h"

typedef struct {
	u32 hdrsize;
	u32 loadersize;
	u32 elfsize;
	u32 argument;
} ioshdr;

#define MEM_REG_BASE 0xd8b4000
#define MEM_PROT (MEM_REG_BASE + 0x20a)
#define TITLE_ID(x,y)	   (((u64)(x) << 32) | (y))

static u32 *kernel = NULL;

static inline void disable_memory_protection(void) {
	write32(MEM_PROT, read32(MEM_PROT) & 0x0000FFFF);
}

bool neekLoadKernel (const char* nandpath)
{
	gprintf( "NEEK: Loading Kernel.bin... ");
	char kernelPath[30];
	if(isWiiU())
		snprintf(kernelPath, sizeof(kernelPath), "%s:/sneek/vwiikernel.bin", DeviceHandler::GetDevicePrefix(nandpath));
	if(!isWiiU() || !CheckFile(kernelPath))
		snprintf(kernelPath, sizeof(kernelPath), "%s:/sneek/kernel.bin", DeviceHandler::GetDevicePrefix(nandpath));
	if(!CheckFile(kernelPath))
	{
		gprintf("File not found.\n");
		return false;
	}
	
	FILE *f = NULL;
	f = fopen(kernelPath, "rb");
	if(!f)
	{
		gprintf("Failed loading file %s.\n", kernelPath);
		return false;
	}
	
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	rewind(f);
	
	// Allocate kernel to mem2
	kernel = (u32 *) MEM2_alloc(fsize);
	if(!kernel)
	{
		return false;
	}
	
	fread(kernel, 1, fsize, f);
	//((ioshdr*)kernel)->argument = 0x42; // set argument size
	DCFlushRange(kernel, fsize);
	
	gprintf("Loaded to 0x%08x, size: %d\n", kernel, fsize);
	gprintf("NEEK: offset memory address: %08x\n", (u32)kernel - 0x80000000);	// offset
	
	fclose(f);
	return true;
}

/*
static void neekClearKernel(void)
{
	if(kernel)
		MEM2_free(kernel);
	kernel = NULL;
}
*/

int neekBoot(void)
{
	gprintf("Booting S/Uneek !!\n");
	if(kernel == NULL)
	{
		gprintf("Kernel not loaded !! Exiting...\n");
		return -1;
	}
	
	/** boot mini without BootMii IOS code by Crediar. **/
	
	disable_memory_protection();
	unsigned int i = 0x939F02F0;
	unsigned char ES_ImportBoot2[16] =
		{ 0x68, 0x4B, 0x2B, 0x06, 0xD1, 0x0C, 0x68, 0x8B, 0x2B, 0x00, 0xD1, 0x09, 0x68, 0xC8, 0x68, 0x42 };
	
	if( memcmp( (void*)(i), ES_ImportBoot2, sizeof(ES_ImportBoot2) ) != 0 )
		for( i = 0x939F0000; i < 0x939FE000; i+=4 )
			if( memcmp( (void*)(i), ES_ImportBoot2, sizeof(ES_ImportBoot2) ) == 0 )
				break;
	
	if(i >= 0x939FE000)
	{
		gprintf("ES_ImportBoot2 not patched !! Exiting...\n");
		//SYS_ResetSystem( SYS_RETURNTOMENU, 0, 0 );
		return -1;
	}
	
	DCInvalidateRange( (void*)i, 0x20 );
	
	*(vu32*)(i+0x00)        = 0x48034904;   // LDR R0, 0x10, LDR R1, 0x14
	*(vu32*)(i+0x04)        = 0x477846C0;   // BX PC, NOP
	*(vu32*)(i+0x08)        = 0xE6000870;   // SYSCALL
	*(vu32*)(i+0x0C)        = 0xE12FFF1E;   // BLR
	//*(vu32*)(i+0x10)    	= 0x11000000;   // kernel offset from 0x80000000. Kernel loaded to (void *)0x91000000
	*(vu32*)(i+0x10)   		= (u32)kernel - 0x80000000;   // kernel offset
	*(vu32*)(i+0x14)        = 0x0000FF01;   // version
	
	DCFlushRange( (void*)i, 0x20 );
	__IOS_ShutdownSubsystems();
	
	s32 fd = IOS_Open( "/dev/es", 0 );
	
	u8 *buffer = (u8*)memalign( 32, 0x100 );
	memset( buffer, 0, 0x100 );
	
	IOS_IoctlvAsync( fd, 0x1F, 0, 0, (ioctlv*)buffer, NULL, NULL );
	return 0;

}

/****************************************************************************
 * neekIsNeek2o
 *
 * Detects Kernel.bin format
 *
 * @return values :
 * -1 : kernel.bin not found
 *  0 : sneek   or uneek   kernel.bin
 *  1 : sneek2o or uneek2o kernel.bin
 ***************************************************************************/
int neekIsNeek2o(const char* nandpath)
{
	int found = 0;
	char tempPath[100];
	if(isWiiU())
		snprintf(tempPath, sizeof(tempPath), "%s:/sneek/vwiikernel.bin", DeviceHandler::GetDevicePrefix(nandpath));
	
	if(!isWiiU() || !CheckFile(tempPath))
		snprintf(tempPath, sizeof(tempPath), "%s:/sneek/kernel.bin", DeviceHandler::GetDevicePrefix(nandpath));
	
	if(!CheckFile(tempPath))
		return -1;
	
	u8 *buffer = NULL;
	u32 filesize = 0;
	const char* str = "NEEK2O";
	if(LoadFileToMem(tempPath, &buffer, &filesize))
	{
		for(u32 i = filesize-strlen(str); i > 0; i--)
		{
			if( memcmp(buffer+i, str, strlen(str)) == 0)
			{
				found = 1;
				break;
			}
		}
	}
	else
		return -1;
	
	if(buffer)
		free(buffer);
	
	return found;
}

/****************************************************************************
 * neekPathFormat
 *
 * Convert and trim full path to path format used by neek
 ***************************************************************************/
int neekPathFormat(char* nandpath_out, const char* nandpath_in, u32 len)
{
	const char* neekNandPathTemp = strchr(nandpath_in, '/');
	if(!neekNandPathTemp)
		return -1;
	
	snprintf(nandpath_out, len, "%s", neekNandPathTemp);
	
	if(nandpath_out[strlen(nandpath_out)-1] == '/')
		*(strrchr(nandpath_out, '/')) = '\0';				// remove trailing slash
	
	return 1;
}

/****************************************************************************
 * neek2oSetBootSettings
 *
 * Generate the neek2o autoboot settings
 *
 * neek_config: Pointer to cfg
 * TitleID    : Full path to Channel (00010001 + ABCD) or ID4 for Wii/GC games (00000000 + ID)
 * Magic      : 0x0 for channel, 0x5d1c9ea3 for Wii game, 0xC2339F3D for gamecube and quadforce games
 * returnTo   : true/false to return to another neek channel instead of system menu
 * nandpath   : Set a different temporary Nand Path to use (full path to nand)
 *
 ***************************************************************************/
bool neek2oSetBootSettings(NEEK_CFG* neek_config, u64 TitleID, u32 Magic, u64 returnto, const char* nandpath )
{
	if(!neek_config)
		return false;
	
	char tmpPath[100];
	memset(neek_config, 0, sizeof(NEEK_CFG));
	
	// Magic and version for DML
	neek_config->magic = NEEK_MAGIC;
	
	// GameID
	if(!Magic)
		neek_config->titleid = TitleID; 			// Set channel ID
	else
	{
		neek_config->gameid = (u32)TitleID; 		// Wii or Gamecube title ID4 to autoboot
		neek_config->gamemagic = Magic; 			// set to 0x5d1c9ea3 for Wii game, 0xC2339F3D for gamecube and quadforce games
		neek_config->config |= NCON_EXT_BOOT_GAME ; // set Disc booting
	}
	
	// Return to
	if(returnto)
	{
		// check if NK2O is installed
		snprintf(tmpPath, sizeof(tmpPath), "%s/title/00010001/4e4b324f/content/title.tmd", nandpath != NULL ? nandpath : Settings.NandEmuChanPath);
		if(CheckFile(tmpPath))
		{
			neek_config->returnto = TITLE_ID(0x00010001, 'NK2O');	// Currently forced to NK2O user channel
			neek_config->config |= NCON_EXT_RETURN_TO;	//  enable "return to" patch
		}
		
		if(isWiiU())
		{
			neek_config->returnto = TITLE_ID(0x00010002, 'HCVA');// Currently forced to "Return to WiiU" system channel
			neek_config->config |= NCON_EXT_RETURN_TO;	//  enable "return to" patch
		}
	}
	
	if(!(neek_config->config & NCON_EXT_RETURN_TO))
	{
		// delete residual "return to" file if last shutdown was unclean.
		snprintf(tmpPath, sizeof(tmpPath), "%s:/sneek/reload.sys", DeviceHandler::GetDevicePrefix(nandpath));
		if(CheckFile(tmpPath))
			RemoveFile(tmpPath);
	}
	
	if(nandpath && strlen(nandpath) > 0)
	{
		// ensure path is correct format
		char neekNandPath[256] = "";
		if(neekPathFormat(neekNandPath, nandpath, sizeof(neekNandPath)))
		{
			snprintf(neek_config->nandpath, sizeof(neek_config->nandpath), "%s", neekNandPath);
			neek_config->config |= NCON_EXT_NAND_PATH ; // Use custom nand path
		//	neek_config->config |= NCON_HIDE_EXT_PATH;  // Set custom nand path as temporary
		}
	}
	
	//set a custom di folder
	//snprintf(neek_config->dipath, sizeof(neek_config->dipath), "/sneek/vwii"); 	// Set path for di.bin and diconfig.bin
	//neek_config->config |= NCON_EXT_DI_PATH; 										// Use custom di path
	
	DCFlushRange(neek_config, sizeof(NEEK_CFG));
	
	//hexdump(neek_config, sizeof(NEEK_CFG));
	
	return true;
}


/****************************************************************************
 * neek2oSetNAND
 *
 * Generates nandcfg.bin if missing and adds missing EmuNAND path
 * Sets default EmuNAND path for neek2o
 *
 * @return values :
 * -1 : error
 *  x : Default path set to EmuNAND number x
 ***************************************************************************/
int neek2oSetNAND(const char* nandpath)
{
	// format path string for neek
	char neekNandPath[256] = "";
	if(neekPathFormat(neekNandPath, nandpath, sizeof(neekNandPath)) < 0)
		return -1;
	
	FILE *f = NULL;
	u32 ret = -1;
	bool found = false;
	u32 i = 0;
	
	char nandconfigPath[100];
	snprintf(nandconfigPath, sizeof(nandconfigPath), "%s:/sneek/nandcfg.bin", DeviceHandler::GetDevicePrefix(nandpath));
	
	// vWii neek2o - different filename but not the same format?
	//if(isWiiU())
	//	snprintf(nandconfigPath, sizeof(nandconfigPath), "%s:/sneek/vwiincfg.bin", DeviceHandler::GetDevicePrefix(nandpath));

#ifdef DEBUG
	gprintf("nandconfigPath : %s\n", nandconfigPath);
#endif

	// create the file if it doesn't exist
	if(!CheckFile(nandconfigPath) || FileSize(nandconfigPath) < NANDCONFIG_HEADER_SIZE+1)
	{
		u8* nandConfigHeader[NANDCONFIG_HEADER_SIZE];
		memset(nandConfigHeader, 0, NANDCONFIG_HEADER_SIZE);
		
		f = fopen(nandconfigPath, "wb");
		if(!f)
		{
			gprintf("Failed creating file %s.\n", nandconfigPath);
			return -1;
		}
		
		// create an empty header with 0 NAND
		fwrite(nandConfigHeader, 1, NANDCONFIG_HEADER_SIZE, f);
		fclose(f);
	}
	
	f = fopen(nandconfigPath, "rb");
	if(!f)
	{
		gprintf("Failed loading file %s.\n", nandconfigPath);
		return -1;
	}
	
	fseek(f , 0 , SEEK_END);
	u32 filesize = ftell(f);
	rewind(f);
	
	/* Allocate memory */
	NandConfig *nandCfg = (NandConfig *) MEM2_alloc(filesize);
	if (!nandCfg)
	{
		fclose (f);
		return -1;
	}
	
	// Read the file
	ret = fread (nandCfg, 1, filesize, f);
	if(ret != filesize)
	{
		gprintf("Failed loading file %s to Mem.\n", nandconfigPath);
		fclose (f);
		MEM2_free(nandCfg);
		return -1;
	}

#ifdef DEBUG
	hexdump(nandCfg, NANDCONFIG_HEADER_SIZE);
#endif

	// don't parse if wrong file
	if(nandCfg->NandCnt > NANDCONFIG_MAXNAND || nandCfg->NandSel > nandCfg->NandCnt) // wrong header, delete file.
	{
		fclose(f);
		MEM2_free(nandCfg);
		RemoveFile(nandconfigPath); // delete corrupted file.
		return -1;
	}

#ifdef DEBUG
	// List found nands from the file
	gprintf("NandCnt = %d\n", nandCfg->NandCnt);
	gprintf("NandSel = %d\n", nandCfg->NandSel);
	for( i = 0 ; i < nandCfg->NandCnt ; i++)
	{
		gprintf("Path %d = %s %s\n", i, &nandCfg->Nands[i], strcmp((const char *)&nandCfg->Nands[i], neekNandPath) == 0 ? "found" : "");
	}
#endif

	for( i = 0 ; i < nandCfg->NandCnt ; i++)
	{
		if(strcmp((const char *)&nandCfg->Nands[i], neekNandPath) == 0)
		{
			found = true;
			break;
		}
	}
	
	if(found) // NAND path already present in nandcfg.bin
	{
		// set selected nand in header if different
		if(nandCfg->NandSel != i)
		{
			nandCfg->NandSel = i;
			nandCfg->Padding1 = i; // same value?
			DCFlushRange(nandCfg, NANDCONFIG_HEADER_SIZE);
#ifdef DEBUG
			gprintf("new nandCfg->sel = %d", nandCfg->NandSel);
			hexdump(nandCfg, NANDCONFIG_HEADER_SIZE);
#endif
			freopen(nandconfigPath, "wb", f);
			ret = fwrite(nandCfg, sizeof(char), filesize, f); // Write full file
		}
	}
	else // new NAND path to nandcfg.bin
	{
		NandInfo * newNand = (NandInfo *) MEM2_alloc(sizeof(NandInfo));
		if(newNand)
		{
			memset(newNand, 0, sizeof(NandInfo));
			snprintf(newNand->Path, sizeof(newNand->Path), neekNandPath);
			snprintf(newNand->Name, sizeof(newNand->Name), strlen(neekNandPath) == 0 ? "root" : strrchr(neekNandPath, '/')+1);
			snprintf(newNand->DiPath, sizeof(newNand->DiPath), "/sneek");
			DCFlushRange(newNand, sizeof(NandInfo));
#ifdef DEBUG
			gprintf("new nandCfg");
			hexdump(newNand, sizeof(NandInfo));
#endif
			nandCfg->NandCnt++;
			nandCfg->NandSel = ++i;
			
			// prevent NAND selection bigger than number of NANDs.
			if(nandCfg->NandSel >= nandCfg->NandCnt)
			{
				nandCfg->NandSel = nandCfg->NandCnt -1;
				i--;
			}
			
			freopen(nandconfigPath, "wb", f);
			ret = fwrite(nandCfg, sizeof(char), filesize, f); 	// Write full file
			ret = fwrite(newNand,1,sizeof(NandInfo),f); 		// append new NANDInfo
			if(ret != sizeof(NandInfo))
				gprintf("Writing new NAND info failed\n");
			
			MEM2_free(newNand);
		}
	}
	
	// verify the header is correctly written
	freopen(nandconfigPath, "rb", f);
	ret = fread (nandCfg, 1, NANDCONFIG_HEADER_SIZE, f);
	if(ret != NANDCONFIG_HEADER_SIZE)
	{
		gprintf("Failed loading file %s to Mem.\n", nandconfigPath);
		fclose (f);
		MEM2_free(nandCfg);
		return -1;
	}
	ret = nandCfg->NandSel;
	fclose (f);
	MEM2_free(nandCfg);
#ifdef DEBUG
	gprintf("verify header:\n");
		hexdump(nandCfg, NANDCONFIG_HEADER_SIZE);
#endif
	if(ret == i)
		return ret ;
	
	return -1;
}
