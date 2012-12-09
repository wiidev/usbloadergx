#ifndef _IOSLOADER_H_
#define _IOSLOADER_H_

#include <gccore.h>
#include <ogc/machine/processor.h>

#define CheckAHBPROT()	(read32(0x0D800064) == 0xFFFFFFFF)

enum MiosInfo
{
	DEFAULT_MIOS,
	DIOS_MIOS,
	DIOS_MIOS_LITE,
	QUADFORCE,
};

enum DMLVersions
{
	DML_VERSION_MIOS = 0,
	DML_VERSION_R51,
	DML_VERSION_R52,
//	DML_VERSION_DML_1_0,
//	DML_VERSION_DML_1_1,
	DML_VERSION_QUAD_0_1, 	// Feb 15 2012 13:19:36 wrong built date.
	DML_VERSION_DML_1_2,	// Apr 24 2012 19:44:08
//	DML_VERSION_DML_1_3,	// Apr 26 2012 
	DML_VERSION_DML_1_4,
	DML_VERSION_DML_1_4b,	// May  7 2012 21:12:47
//	DML_VERSION_QUAD_0_1	// Jun  9 2012 23:13:16	correct built date.
	DML_VERSION_DML_1_5,	// Jun 14 2012 00:05:09
	DML_VERSION_DM_2_0,		// Jun 23 2012 19:43:21
//	DML_VERSION_DM_2_0_1,
//	DML_VERSION_DM_2_0_2,
//	DML_VERSION_DM_2_0_3,
//	DML_VERSION_DM_2_0_3b,
//	DML_VERSION_DM_2_0_3c,
//	DML_VERSION_DM_2_0_3d,
//	DML_VERSION_DM_2_0_4,
//	DML_VERSION_DM_2_0_5,
	DML_VERSION_DM_2_1,		// Jul 17 2012 11:25:35
	DML_VERSION_DM_2_2,		// Jul 18 2012 16:57:47
//	DML_VERSION_DM_2_2_1,
	DML_VERSION_DM_2_2_2,	// Jul 20 2012 14:49:47
	DML_VERSION_DML_2_2,	// Aug  6 2012 15:19:17
	DML_VERSION_DML_2_2_1,	// Aug 13 2012 00:12:46
	DML_VERSION_DML_2_3m,	// Sep 24 2012 13:13:42 (mirror link)
	DML_VERSION_DM_2_3,		// Sep 24 2012 15:51:54 (Main link and Mirror link)
	DML_VERSION_DML_2_3,	// Sep 25 2012 03:03:41 (main link)
	DML_VERSION_DM_2_4,		// Oct 21 2012 22:57:12
	DML_VERSION_DML_2_4,	// Oct 21 2012 22:57:17
	DML_VERSION_DM_2_5,		// Nov  9 2012 21:18:52
	DML_VERSION_DML_2_5,	// Nov  9 2012 21:18:56
	DML_VERSION_DM_2_6_0,	// Dec  1 2012 01:52:53
	DML_VERSION_DML_2_6,	// Dec  1 2012 16:22:29
	DML_VERSION_DM_2_6_1,	// Dec  1 2012 16:42:34
	DML_VERSION_MAX_VERSION,
};

typedef struct _iosinfo_t
{
	u32 magicword;			  //0x1ee7c105
	u32 magicversion;		   // 1
	u32 version;				// Example: 5
	u32 baseios;				// Example: 56
	char name[0x10];			// Example: d2x
	char versionstring[0x10];   // Example: beta2
} __attribute__((packed)) iosinfo_t;

class IosLoader
{
	public:
		static s32 LoadAppCios();
		static s32 LoadGameCios(s32 ios);
		static s32 ReloadIosSafe(s32 ios);
		static s32 ReloadIosKeepingRights(s32 ios);
		static bool IsHermesIOS(s32 ios = IOS_GetVersion());
		static bool IsWaninkokoIOS(s32 ios = IOS_GetVersion());
		static bool IsD2X(s32 ios = IOS_GetVersion());
		static iosinfo_t *GetIOSInfo(s32 ios);
		static u8 GetMIOSInfo(bool checkedOnBoot = false);
		static u8 GetDMLVersion(char* releaseDate = NULL);
	private:
		static void LoadIOSModules(s32 ios, s32 ios_rev);
};

#endif
