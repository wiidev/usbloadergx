#include <gctypes.h>
#include <ogc/machine/processor.h>

#include "IosLoader.h"
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

/*
 * Buffer variables for the IOS info to avoid loading it several times
 */
static int currentIOS = -1;
static iosinfo_t *currentIOSInfo = NULL;
static int currentMIOS = -1;

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
	if((int) activeCios == Settings.cios)
		return 0;

	u32 ciosLoadPriority[] = { Settings.cios, 222, 249, 250, 245, 246, 247, 248 }; // Ascending.


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
			Settings.cios = cios;
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
	else
	{
		return -33;
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

	u8 *appfile = NULL;
	u32 filesize = 0;

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
				}
				else
				{
					currentMIOS = DIOS_MIOS;
				}
				break;
			}
			else if((*(u32*)(appfile+i)) == 'Quad' && (*(u32*)(appfile+i+4)) == 'Forc')
			{
				currentMIOS = QUADFORCE;
				break;
			}
		}
		free(appfile);
	}

	return currentMIOS;
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

	snprintf(filepath, sizeof(filepath), "/title/%08x/%08x/content/%08x.app", 0x00000001, ios, *(u8 *)((u32)TMD+0x1E7));

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
