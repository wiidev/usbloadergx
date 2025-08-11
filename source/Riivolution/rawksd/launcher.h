#pragma once

#include <gctypes.h>

//#define YARR
//#define DEBUGGER

#define MEM_BASE			((u8*)0x80000000)
#define MEM_BOOTCODE		((u32*)0x80000020)
#define MEM_VERSION			((u32*)0x80000024)
#define MEM_ARENA1LOW		((u32*)0x80000030)
#define MEM_BUSSPEED		((u32*)0x800000F8)
#define MEM_CPUSPEED		((u32*)0x800000FC)
#define MEM_IOSVERSION		((u32*)0x80003140)
#define MEM_GAMEONLINE		((u32*)0x80003180)
#define MEM_TITLEFLAGS		((u32*)0x80003184)
#define MEM_IOSEXPECTED		((u32*)0x80003188)
#define MEM_VIDEOMODE		((u32*)0x800000CC)
#define MEM_PHYSICALSIZE	((u32*)0x80000028)
#define MEM_VIRTUALSIZE		((u32*)0x800000F0)
#define MEM_BI2				((u32*)0x800000F4)

#define MEM_FSTADDRESS		((void**)0x80000038)
#define MEM_FSTADDRESS2		((void**)0x80003110)
#define MEM_APPLOADER		((u32*)0x81200000)

#define MEM_ARENA1HIGH		((void**)0x80000034)
#define MEM_FSTSIZE			((u32*)0x8000003C)
#define MEM_EXE				((void*)0x80003F00)

namespace LauncherStatus { enum Enum {
	OK = 0,
	NoDisc = -0x100,
	OutOfMemory = -0x400,
	IosError = -0x200,
	ReadError = -0x201
}; }

extern bool disk_subsequent_reset;

LauncherStatus::Enum Launcher_Init();
LauncherStatus::Enum Launcher_ReadDisc();
LauncherStatus::Enum Launcher_RunApploader();
LauncherStatus::Enum Launcher_Launch();
LauncherStatus::Enum Launcher_RVL();
LauncherStatus::Enum Launcher_CommitRVL(bool dip);
LauncherStatus::Enum Launcher_AddPlaytimeEntry();
LauncherStatus::Enum Launcher_SetVideoMode();
LauncherStatus::Enum Launcher_ScrubPlaytimeEntry();
bool Launcher_DiscInserted();
const char* Launcher_GetGameName();
const s16* Launcher_GetGameNameWide();
const u32* Launcher_GetFstData();
