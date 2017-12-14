#include <gctypes.h>
#include <ogc/machine/processor.h>

#include "IosLoader.h"
#include "sys.h"
#include "Controls/DeviceHandler.hpp"
#include "usbloader/usbstorage2.h"
#include "usbloader/disc.h"
#include "usbloader/wbfs.h"
#include "usbloader/wdvd.h"
#include "wad/nandtitle.h"
#include "mload/mload_modules.h"
#include "settings/CSettings.h"
#include "wad/nandtitle.h"
#include "mload/mload.h"
#include "mload/modules/ehcmodule_5.h"
#include "mload/modules/dip_plugin_249.h"
#include "mload/modules/odip_frag.h"
#include "utils/tools.h"
#include "gecko.h"

#define MEM2_PROT		0x0D8B420A
#define ES_MODULE_START	((u16 *)0x939F0000)
#define ES_MODULE_END	(ES_MODULE_START + 0x4000)
#define ES_HACK_OFFSET	4

extern u32 hdd_sector_size[2];

/*
 * Buffer variables for the IOS info to avoid loading it several times
 */
static int currentIOS = -1;
static iosinfo_t *currentIOSInfo = NULL;
static int currentMIOS = -1;
static int currentDMLVersion = -1;

/******************************************************************************
 * Public Methods:
 ******************************************************************************/
/*
 * Check if the ios passed is a Hermes ios.
 */
bool IosLoader::IsHermesIOS(s32 ios)
{
	return (ios == 222 || ios == 223 || ios == 224 || ios == 225 || ios == 202);
}

/*
 * Check if the ios passed is a Waninkoko ios.
 */
bool IosLoader::IsWaninkokoIOS(s32 ios)
{
	if(ios < 200 || ios > 255)
		return false;

	return !IsHermesIOS(ios);
}

/*
 * Check if the ios passed is a d2x ios.
 */
bool IosLoader::IsD2X(s32 ios)
{
	iosinfo_t *info = GetIOSInfo(ios);
	if(!info)
		return false;

	bool res = (strncasecmp(info->name, "d2x", 3) == 0);

	return res;
}

/*
 * Loads CIOS (If possible the one from the settings file).
 * @return 0 if a cios has been successfully loaded. Else a value below 0 is returned.
 */
s32 IosLoader::LoadAppCios()
{
	u32 activeCios = IOS_GetVersion();
	s32 ret = -1;

	// We have what we need
	if((int) activeCios == Settings.LoaderIOS)
		return 0;

	u8 ciosLoadPriority[] = { Settings.LoaderIOS, 249, 250, 222, 223, 245, 246, 247, 248 }; // Ascending.


	for (u32 i = 0; i < (sizeof(ciosLoadPriority)/sizeof(ciosLoadPriority[0])); ++i)
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
			Settings.LoaderIOS = cios;
			break;
		}
	}

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
	WBFS_CloseAll();
	WDVD_Close();
	DeviceHandler::DestroyInstance();
	USBStorage2_Deinit();

	ret = ReloadIosSafe(ios);

	// Remount devices after reloading IOS.
	DeviceHandler::Instance()->MountSD();
	DeviceHandler::Instance()->MountAllUSB(true);
	Disc_Init();

	return ret;
}

/*
 * Reloads a certain IOS under the condition, that an appropriate version of the IOS is installed.
 * @return a negative value if a safe reload of the ios was not possible.
 */
s32 IosLoader::ReloadIosSafe(s32 ios)
{
	if(IsHermesIOS(ios))
	{
		s32 iosRev = NandTitles.VersionOf(TITLE_ID(1, ios));
		if((iosRev < 4 || iosRev > 6) && iosRev != 65535)
			return -11;
	}
	else if(IsWaninkokoIOS(ios))
	{
		s32 iosRev = NandTitles.VersionOf(TITLE_ID(1, ios));
		if((iosRev < 9 || iosRev > 30000) && iosRev != 65535)  //let's see if Waninkoko actually gets to 30
			return -22;
	}
	else if(ios < 200) // use AHB Access
	{
		s32 iosRev = NandTitles.VersionOf(TITLE_ID(1, ios));
		if(!iosRev)
			return -33;
	} 
	else
	{
		return -44;
	}

	s32 r = ReloadIosKeepingRights(ios);
	if (r >= 0) WII_Initialize();

	IosLoader::LoadIOSModules(IOS_GetVersion(), IOS_GetRevision());

	return r;
}

/*
 * Reloads a certain IOS and keeps the AHBPROT flag enabled if available.
 */
s32 IosLoader::ReloadIosKeepingRights(s32 ios)
{
	if (CheckAHBPROT())
	{
		static const u16 ticket_check[] = {
			0x685B,		  // ldr  r3, [r3, #4] ; Get TMD pointer
			0x22EC, 0x0052,  // movs r2, 0x1D8	; Set offset of access rights field in TMD
			0x189B,		  // adds r3, r3, r2   ; Add offset to TMD pointer
			0x681B,		  // ldr  r3, [r3]	 ; Load access rights. We'll hack it with full access rights!!!
			0x4698,		  // mov  r8, r3	   ; Store it for the DVD video bitcheck later
			0x07DB		   // lsls r3, r3, 0x1F ; check AHBPROT bit
		};

		/* Disable MEM 2 protection */
		write16(MEM2_PROT, 2);

		for (u16 *patchme = ES_MODULE_START; patchme < ES_MODULE_END; patchme++)
		{
			if (!memcmp(patchme, ticket_check, sizeof(ticket_check)))
			{
				gprintf("ReloadIos: Found TMD access rights check at %p\n", patchme);

				/* Apply patch */
				patchme[ES_HACK_OFFSET] = 0x23FF; // li r3, 0xFF ; Set full access rights

				/* Flush cache */
				DCFlushRange(patchme+ES_HACK_OFFSET, 2);
				break;
			}
		}
	}

	/* Reload IOS. MEM2 protection is implicitly re-enabled */
	return IOS_ReloadIOS(ios);
}

/*
 * Check if MIOS is DIOS MIOS, DIOS MIOS Lite or official MIOS.
 */
u8 IosLoader::GetMIOSInfo()
{
	if(currentMIOS > -1)
		return currentMIOS;

	currentMIOS = DEFAULT_MIOS;

	if(isWiiU()) // vWii users
		return currentMIOS;

	u8 *appfile = NULL;
	u32 filesize = 0;

	// "title/00000001/00000101/content/0000000b.app" contains DM/DML version and built date, but is not always accurate.
	// so we are looking inside 0000000c.app to find the correct version.
	NandTitle::LoadFileFromNand("/title/00000001/00000101/content/0000000c.app", &appfile, &filesize);

	if(appfile)
	{
		for(u32 i = 0; i < filesize-4; ++i)
		{
			if((*(u32*)(appfile+i)) == 'DIOS' && (*(u32*)(appfile+i+5)) == 'MIOS')
			{
				if((*(u32*)(appfile+i+10)) == 'Lite')
				{
					currentMIOS = DIOS_MIOS_LITE;
					gprintf("DIOS MIOS Lite ");
					currentDMLVersion = GetDMLVersion((char*)(appfile+i+31));
				}
				else
				{
					currentMIOS = DIOS_MIOS;
					gprintf("DIOS MIOS ");
					currentDMLVersion = GetDMLVersion((char*)(appfile+i+27));
				}
				break;
			}
			else if((*(u32*)(appfile+i)) == 'Quad' && (*(u32*)(appfile+i+4)) == 'Forc')
			{
				currentMIOS = QUADFORCE;
				char* QF_version = (char*)(appfile+i+10);
				gprintf("QuadForce v%.1f \n", atof(QF_version));
				if(atof(QF_version) >= 4.0)			currentDMLVersion = DML_VERSION_QUAD_4_0;
				else if(atof(QF_version) == 3.0)	currentDMLVersion = DML_VERSION_QUAD_3_0;
				else if(atof(QF_version) == 2.0)	currentDMLVersion = DML_VERSION_QUAD_2_0;
				else 								currentDMLVersion = DML_VERSION_QUAD_0_1;
				
				break;
			}
			else if((*(u32*)(appfile+i)) == 'GCLo' && (*(u32*)(appfile+i+4)) == 'ader')
			{
				// QuadForce USB v4.1 binary doesn't have QF version, checking:  GCLoader....Built   : %s %s.....May 26 2013.00:15:28
				if((*(u32*)(appfile+i+32)) == 'May ' && (*(u32*)(appfile+i+44)) == '00:1' && (*(u32*)(appfile+i+48)) == '5:28')
				{
					currentMIOS = QUADFORCE_USB;
					gprintf("QuadForce USB v4.1\n");
					currentDMLVersion = DML_VERSION_QUAD_4_1;
					break;
				}
			}
		}
		free(appfile);
	}
	
	if(currentMIOS == DEFAULT_MIOS)
		gprintf("MIOS / cMIOS \n");
	
	return currentMIOS;
}


u8 IosLoader::GetDMLVersion(char* releaseDate)
{
	if(currentDMLVersion > -1)
		return currentDMLVersion;

	currentDMLVersion = DML_VERSION_MIOS;

	// Older versions - not working with USBloaderGX
	if(strncmp(releaseDate, "t: ", 3) == 0)
	{
		currentMIOS = DEFAULT_MIOS;
		return currentDMLVersion;
	}

	// Current installed version
	gprintf("built on %s\n", releaseDate);

	struct tm time;
	strptime(releaseDate, "%b %d %Y %H:%M:%S", &time);
	time_t unixTime = mktime(&time);

	if(currentMIOS == DIOS_MIOS)
	{
		// Timestamp of DM 2.0
		strptime("Jun 23 2012 19:43:21", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_0_time = mktime(&time);

		// Timestamp of DM 2.1
		strptime("Jul 17 2012 11:25:35", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_1_time = mktime(&time);

		// Timestamp of DM 2.2 initial release
		strptime("Jul 18 2012 16:57:47", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_2_time = mktime(&time);

		// Timestamp of DM 2.2 update2
		strptime("Jul 20 2012 14:49:47", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_2_2_time = mktime(&time);

		// Timestamp of DM 2.3
		strptime("Sep 24 2012 15:51:54", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_3_time = mktime(&time);

		// Timestamp of DM 2.4
		strptime("Oct 21 2012 22:57:12", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_4_time = mktime(&time);

		// Timestamp of DM 2.5
		strptime("Nov  9 2012 21:18:52", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_5_time = mktime(&time);

		// Timestamp of DM 2.6.0
		strptime("Dec  1 2012 01:52:53", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_6_0_time = mktime(&time);

		// Timestamp of DM 2.6.1
		strptime("Dec  1 2012 16:42:34", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_6_1_time = mktime(&time);

		// Timestamp of DM 2.7
		strptime("Feb 20 2013 14:54:33", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_7_time = mktime(&time);

		// Timestamp of DM 2.8
		strptime("Feb 24 2013 14:17:03", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_8_time = mktime(&time);

		// Timestamp of DM 2.9
		strptime("Apr  5 2013 18:29:35", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_9_time = mktime(&time);

		// Timestamp of DM 2.10
		strptime("May 24 2013 21:22:22", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_10_time = mktime(&time);

		// Timestamp of DM 2.11
		strptime("Jul  2 2014 10:31:15", "%b %d %Y %H:%M:%S", &time);
		const time_t dm_2_11_time = mktime(&time);

		if(difftime(unixTime, dm_2_11_time) >= 0) 			currentDMLVersion = DML_VERSION_DM_2_11;
		else if(difftime(unixTime, dm_2_10_time) >= 0) 		currentDMLVersion = DML_VERSION_DM_2_10;
		else if(difftime(unixTime, dm_2_9_time) >= 0) 		currentDMLVersion = DML_VERSION_DM_2_9;
		else if(difftime(unixTime, dm_2_8_time) >= 0) 		currentDMLVersion = DML_VERSION_DM_2_8;
		else if(difftime(unixTime, dm_2_7_time) >= 0) 		currentDMLVersion = DML_VERSION_DM_2_7;
		else if(difftime(unixTime, dm_2_6_1_time) >= 0) 	currentDMLVersion = DML_VERSION_DM_2_6_1;
		else if(difftime(unixTime, dm_2_6_0_time) >= 0) 	currentDMLVersion = DML_VERSION_DM_2_6_0;
		else if(difftime(unixTime, dm_2_5_time) >= 0) 		currentDMLVersion = DML_VERSION_DM_2_5;
		else if(difftime(unixTime, dm_2_4_time) >= 0) 		currentDMLVersion = DML_VERSION_DM_2_4;
		else if(difftime(unixTime, dm_2_3_time) >= 0) 		currentDMLVersion = DML_VERSION_DM_2_3;
		else if(difftime(unixTime, dm_2_2_2_time) >= 0) 	currentDMLVersion = DML_VERSION_DM_2_2_2;
		else if(difftime(unixTime, dm_2_2_time) >= 0) 		currentDMLVersion = DML_VERSION_DM_2_2;
		else if(difftime(unixTime, dm_2_1_time) >= 0) 		currentDMLVersion = DML_VERSION_DM_2_1;
		else if(difftime(unixTime, dm_2_0_time) >= 0) 		currentDMLVersion = DML_VERSION_DM_2_0;	
	}
	else if(currentMIOS == DIOS_MIOS_LITE)
	{
		// Timestamp of DML r52
		strptime("Mar 7 2012 19:36:06", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_r52_time = mktime(&time);

		// Timestamp of DML 1.2
		strptime("Apr 24 2012 19:44:08", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_1_2_time = mktime(&time);

		// Timestamp of DML 1.4b
		strptime("May  7 2012 21:12:47", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_1_4b_time = mktime(&time);

		// Timestamp of DML 1.5
		strptime("Jun 14 2012 00:05:09", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_1_5_time = mktime(&time);

		// Timestamp of DML 2.2 initial release
		strptime("Aug  6 2012 15:19:17", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_2_time = mktime(&time);

		// Timestamp of DML 2.2 update1
		strptime("Aug 13 2012 00:12:46", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_2_1_time = mktime(&time);

		// Timestamp of DML 2.3 mirror link
		strptime("Sep 24 2012 13:13:42", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_3m_time = mktime(&time);

		// Timestamp of DML 2.3 main link
		strptime("Sep 25 2012 03:03:41", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_3_time = mktime(&time);
		// Timestamp of DML 2.4
		strptime("Oct 21 2012 22:57:17", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_4_time = mktime(&time);

		// Timestamp of DML 2.5
		strptime("Nov  9 2012 21:18:56", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_5_time = mktime(&time);

		// Timestamp of DML 2.6
		strptime("Dec  1 2012 16:22:29", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_6_time = mktime(&time);

		// Timestamp of DML 2.7
		strptime("Feb 21 2013 03:13:49", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_7_time = mktime(&time);

		// Timestamp of DML 2.8
		strptime("Feb 24 2013 13:30:29", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_8_time = mktime(&time);

		// Timestamp of DML 2.9
		strptime("Apr  5 2013 18:20:33", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_9_time = mktime(&time);

		// Timestamp of DML 2.10
		strptime("May 24 2013 18:51:58", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_10_time = mktime(&time);

		// Timestamp of DML 2.11
		strptime("Jul  2 2014 10:31:06", "%b %d %Y %H:%M:%S", &time);
		const time_t dml_2_11_time = mktime(&time);

		if(difftime(unixTime, dml_2_11_time) >= 0) 			currentDMLVersion = DML_VERSION_DML_2_11;
		else if(difftime(unixTime, dml_2_10_time) >= 0) 	currentDMLVersion = DML_VERSION_DML_2_10;
		else if(difftime(unixTime, dml_2_9_time) >= 0) 		currentDMLVersion = DML_VERSION_DML_2_9;
		else if(difftime(unixTime, dml_2_8_time) >= 0) 		currentDMLVersion = DML_VERSION_DML_2_8;
		else if(difftime(unixTime, dml_2_7_time) >= 0) 		currentDMLVersion = DML_VERSION_DML_2_7;
		else if(difftime(unixTime, dml_2_6_time) >= 0) 		currentDMLVersion = DML_VERSION_DML_2_6;
		else if(difftime(unixTime, dml_2_5_time) >= 0) 		currentDMLVersion = DML_VERSION_DML_2_5;
		else if(difftime(unixTime, dml_2_4_time) >= 0) 		currentDMLVersion = DML_VERSION_DML_2_4;
		else if(difftime(unixTime, dml_2_3_time) >= 0) 		currentDMLVersion = DML_VERSION_DML_2_3;
		else if(difftime(unixTime, dml_2_3m_time) >= 0) 	currentDMLVersion = DML_VERSION_DML_2_3m;
		else if(difftime(unixTime, dml_2_2_1_time) >= 0) 	currentDMLVersion = DML_VERSION_DML_2_2_1;
		else if(difftime(unixTime, dml_2_2_time) >= 0) 		currentDMLVersion = DML_VERSION_DML_2_2;
		else if(difftime(unixTime, dml_1_5_time) >= 0)		currentDMLVersion = DML_VERSION_DML_1_5;
		else if(difftime(unixTime, dml_1_4b_time) >= 0)		currentDMLVersion = DML_VERSION_DML_1_4b;
		else if(difftime(unixTime, dml_1_2_time) > 0)		currentDMLVersion = DML_VERSION_DML_1_4;
		else if(difftime(unixTime, dml_1_2_time) == 0)		currentDMLVersion = DML_VERSION_DML_1_2;
		else if (difftime(unixTime, dml_r52_time) >= 0) 	currentDMLVersion = DML_VERSION_R52;
		else												currentDMLVersion = DML_VERSION_R51;	
	}

	return currentDMLVersion;

}

/*
 * Check if selected IOS is compatible with Emulated NAND and user's settings
 */
bool IosLoader::is_NandEmu_compatible(const char *NandEmuPath, s32 ios)
{
	// TODO: Check features against cIOS revision
	// rev17: 1st FAT32 partition of a 512 bytes/sector HDD, NandEmuPath must be on root, Full EmuNAND only
	// rev18: adds Partial EmuNAND mode
	// rev21: adds EmuNAND paths
	// rev21 d2x beta: adds partition selection. officially supported in d2x v3
	// d2x v4: adds 4096 bytes/sector support

	// Check IOS
	if(IsD2X(ios))
		return true;

	if(!IsWaninkokoIOS(ios) || NandTitles.VersionOf(TITLE_ID(1, ios)) < 17)
		return false;

	// Check all path restrictions when using rev17+
	if(NandEmuPath)
	{
		// Check if EmuNAND Path location is on root
		const char *NandEmuFolder = strchr(NandEmuPath, '/');
		if(!NandEmuFolder || strlen(NandEmuFolder) > 1)
			return false;

		// Check if EmuNAND partition is on USB devices
		if(strncmp(NandEmuPath, "usb", 3) == 0)
		{
			int part_num = atoi(NandEmuPath+3);
			int usbport = DeviceHandler::PartitionToUSBPort(part_num-USB1);
			
			// Check if this is the first FAT32 partition on the drive
			for(int dev = USB1; dev <= part_num; dev++)
			{
				if(strncmp(DeviceHandler::GetFSName(dev), "FAT", 3) == 0)
				{
					if(dev == part_num)
						break;
					else
						return false;
				}
			}
			
			// Check if the partition is primary
			// EmuNAND works with Primary and Extended partitions, no need to check the PartitionTableType
			
			// Check HDD sector size. Only 512 bytes/sector is supported by d2x < v4
			if(hdd_sector_size[usbport] != BYTES_PER_SECTOR)
				return false;
		}
	}
	return true;
}

/******************************************************************************
 * Private/Protected Methods:
 ******************************************************************************/
void IosLoader::LoadIOSModules(s32 ios, s32 ios_rev)
{
	//! Hermes IOS
	if(IsHermesIOS(ios))
	{
		const u8 * ech_module = NULL;
		int ehc_module_size = 0;
		const u8 * dip_plugin = NULL;
		int dip_plugin_size = 0;

		ech_module = ehcmodule_5;
		ehc_module_size = size_ehcmodule_5;
		dip_plugin = odip_frag;
		dip_plugin_size = odip_frag_size;
		gprintf("Loading ehc v5 and opendip module\n");

		load_modules(ech_module, ehc_module_size, dip_plugin, dip_plugin_size);
	}
	//! Waninkoko IOS
	else if(IsWaninkokoIOS(ios))
	{
		// Init ISFS for d2x check
		ISFS_Initialize();

		iosinfo_t *info = GetIOSInfo(ios);
		if(ios_rev >= 18 && (!info || info->version < 6))
		{
			if(mload_init() >= 0)
			{
				gprintf("Loading dip module for Waninkoko's cios\n");
				mload_module((u8 *) dip_plugin_249, dip_plugin_249_size);
				mload_close();
			}
		}
		ISFS_Deinitialize();
	}
}

/*
 * Reads the ios info struct from the .app file.
 * @return pointer to iosinfo_t on success else NULL. The user is responsible for freeing the buffer.
 */
iosinfo_t *IosLoader::GetIOSInfo(s32 ios)
{
	if(currentIOS == ios && currentIOSInfo)
		return currentIOSInfo;

	if(currentIOSInfo)
	{
		free(currentIOSInfo);
		currentIOSInfo = NULL;
	}

	currentIOS = ios;
	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(0x20);
	u64 TicketID = ((((u64) 1) << 32) | ios);
	u32 TMD_Length;

	s32 ret = ES_GetStoredTMDSize(TicketID, &TMD_Length);
	if (ret < 0)
		return NULL;

	signed_blob *TMD = (signed_blob*) memalign(32, ALIGN32(TMD_Length));
	if (!TMD)
		return NULL;

	ret = ES_GetStoredTMD(TicketID, TMD, TMD_Length);
	if (ret < 0)
	{
		free(TMD);
		return NULL;
	}

	snprintf(filepath, sizeof(filepath), "/title/%08x/%08x/content/%08x.app", 0x00000001, (unsigned int)ios, (unsigned int)(*(u8 *)((u32)TMD+0x1E7)));

	free(TMD);

	u8 *buffer = NULL;
	u32 filesize = 0;

	NandTitle::LoadFileFromNand(filepath, &buffer, &filesize);

	if(!buffer)
		return NULL;

	iosinfo_t *iosinfo = (iosinfo_t *) buffer;

	if(iosinfo->magicword != 0x1ee7c105 || iosinfo->magicversion != 1)
	{
		free(buffer);
		return NULL;
	}

	iosinfo = (iosinfo_t *) realloc(buffer, sizeof(iosinfo_t));
	if(!iosinfo)
		iosinfo = (iosinfo_t *) buffer;

	currentIOSInfo = iosinfo;

	return iosinfo;
}
