#include "launcher.h"
#include "menu.h"
#include "wdvd.h"
#include "riivolution.h"
#include "fwrite.h"
#include "files.h"

#include <ogc/lwp_watchdog.h>
#include <ogc/lwp_threads.h>

#include <malloc.h>

#ifdef DEBUGGER
#include <mega.h>
#endif

// DEFINES
#define PART_OFFSET			0x00040000
#define APP_INFO_OFFSET		0x2440
#define APP_DATA_OFFSET		(APP_INFO_OFFSET + 0x20)

#define min(a,b) ((a)>(b) ? (b) : (a))
#define max(a,b) ((a)>(b) ? (a) : (b))

#define PLAYLOG_FILE "/mnt/isfs/title/00000001/00000002/data/play_rec.dat"

// TYPEDEFS
typedef void (*AppReport) (const char*, ...);
typedef void (*AppEnter) (AppReport);
typedef int  (*AppLoad) (void**, s32*, s32*);
typedef void* (*AppExit) ();
typedef void (*AppStart) (AppEnter*, AppLoad*, AppExit*);

typedef struct {
	u8 revision[16];
	AppStart start;
	u32 loader_size;
	u32 data_size;
	u8 unused[4];
} app_info;

typedef struct {
    u8 zeroes[64]; // padding
    u32 imet; // "IMET"
    u8 unk[8];  // 0x0000060000000003 fixed, unknown purpose
    u32 sizes[3]; // icon.bin, banner.bin, sound.bin
    u32 flag1; // unknown
    s16 names[10][42]; // Japanese, English, German, French, Spanish, Italian, Dutch, unknown, unknown, Korean
    u8 zeroes_2[588]; // padding
    u8 crypto[16]; // MD5 of 0x40 to 0x640 in header. crypto should be all 0's when calculating final MD5
} __attribute__((packed)) IMET;

typedef struct {
	u32 checksum;
	union {
		u32 data[0x1f];
		struct {
			s16 name[42];
			u64 ticks_boot;
			u64 ticks_last;
			u32 title_id;
			u16 title_gid;
			//u8 padding[18];
		} ATTRIBUTE_PACKED;
	};
} playtime_buf_t;

// CONSTS
static const u32 SOStartupCode[] = {
	0x28000021,
	0x3A40FFE4,
};

static const u16 NWC24iCleanupSocketCode[] = {
	0x7c65, 0x1b78,
	0x3860, 0x0000,
	0x3880, 0x0007,
	0x4800
};

static const u32 __OSLaunchMenuCodeNew[] = {
	0x38C10008,
	0x38800002,
	0x38600001,
	0x38A00000
};

static const u32 __OSLaunchMenuCodeOld[] = {
	0x39010010,
	0x38C00002,
	0x38A00001,
	0x38E00000
};

static const u32 __OSLaunchRiiv[] = {
	0x3F400001, // lis r26, 1
	0x3B5A0001, // addi r26, r26, 1 #0x00010001
	0x3F605249, // lis r27, 0x5249
	0x3B7B4956, // addi r27, r27, 0x4956 #0x52494956
	0x38600000  // li r3, 0
};

// GLOBALS
static char GameName[0x40] ATTRIBUTE_ALIGN(32);
static s16 BannerName[43];
bool disk_subsequent_reset = false;
static u32 fstdata[0x40] ATTRIBUTE_ALIGN(32);
static void *app_address = NULL;

static union {
	u32 partition_info[24];
	app_info app;
} ATTRIBUTE_ALIGN(32);

// FUNCTIONS
static int nullprintf(const char* fmt, ...)
{
	return 0;
}

static void* FindInBuffer(void* buffer, s32 size, const void* tofind, s32 findsize)
{
	for (int i = 0; i < size - findsize; i++, buffer = (u8*)buffer + 1) {
		if (!memcmp(buffer, tofind, findsize))
			return buffer;
	}

	return NULL;
}

bool Launcher_DiscInserted()
{
	bool cover;
	if (!WDVD_VerifyCover(&cover))
		return cover;
	return false;
}

const u32* Launcher_GetFstData()
{
	return fstdata;
}

const s16* Launcher_GetGameNameWide()
{
	BannerName[42] = 0;
	if (RVL_GetFST()) {
		if (BannerName[0])
			return BannerName;
		DiscNode* node = RVL_FindNode("/opening.bnr");
		if (node != NULL) {
			static IMET imet ATTRIBUTE_ALIGN(32);
			if (!WDVD_LowRead(&imet, sizeof(imet), (u64)node->DataOffset << 2) && imet.imet == 0x494D4554) {
				memcpy(BannerName, imet.names[CONF_GetLanguage()], 42*sizeof(BannerName[0]));
				if (BannerName[0]) {
					return BannerName;
				}
			}
		}
	}

	// couldn't find Banner title, using Disc title instead
	for (int i = 0; i < 42; i++)
		BannerName[i] = GameName[i];
	return BannerName;
}

const char* Launcher_GetGameName()
{
	if (!RVL_GetFST())
		return GameName;

	static char gamename[42];
	const s16* name = Launcher_GetGameNameWide();
	// Find the last non-null char
	int end = 40;
	while (!name[end])
		end--;
	for (int i = 0; i <= end; i++)
		gamename[i] = (char)name[i] ? (char)name[i] : ' ';
	gamename[end + 1] = '\0';

	return gamename;
}

LauncherStatus::Enum Launcher_Init()
{
	int fd = WDVD_Init();
	if (fd < 0)
		return LauncherStatus::IosError;

	if (!WDVD_Reset())
		return LauncherStatus::IosError;

	return LauncherStatus::OK;
}

LauncherStatus::Enum Launcher_ReadDisc()
{
	u32 i;
	int j;

	BannerName[0] = 0;

	if (!Launcher_DiscInserted()) {
		disk_subsequent_reset = true;
		return LauncherStatus::NoDisc;
	}

	if (disk_subsequent_reset && !WDVD_Reset()) // In case of re-inserted discs, needs to be called again
		return LauncherStatus::IosError;

	disk_subsequent_reset = true;

	if (WDVD_LowReadDiskId())
		return LauncherStatus::ReadError;

	if (WDVD_LowUnencryptedRead(GameName, 0x40, 0x20))
		return LauncherStatus::ReadError;

	memset(partition_info, 0, sizeof(partition_info));
	// make sure there is at least one primary partition
	if (WDVD_LowUnencryptedRead(partition_info, 0x20, PART_OFFSET) || partition_info[0]==0)
		return LauncherStatus::ReadError;

	// read primary partition table
	if (WDVD_LowUnencryptedRead(partition_info + 8, max(4, min(8, partition_info[0])) * 8, (u64)partition_info[1] << 2))
		return LauncherStatus::ReadError;
	for (i = 0; i < partition_info[0]; i++)
		if (partition_info[i * 2 + 8 + 1] == 0)
			break;

	// make sure we found a game partition
	if (i >= partition_info[0])
		return LauncherStatus::ReadError;

	if (WDVD_LowOpenPartition((u64)partition_info[i * 2 + 8] << 2))
		return LauncherStatus::ReadError;

	return LauncherStatus::OK;
}

LauncherStatus::Enum Launcher_RVL()
{
	if (WDVD_LowRead(fstdata, 0x40, 0x420))
		return LauncherStatus::ReadError;

	fstdata[2] <<= 2;
	u8* fstbuffer = (u8*)memalign(32, fstdata[2]);
	if (!fstbuffer)
		return LauncherStatus::OutOfMemory;

	if (WDVD_LowRead(fstbuffer, fstdata[2], (u64)fstdata[1] << 2))
		return LauncherStatus::ReadError;

	RVL_SetFST(fstbuffer, fstdata[2]);
	BannerName[0] = 0;
	free(fstbuffer);
	return LauncherStatus::OK;
}

LauncherStatus::Enum Launcher_CommitRVL(bool dip)
{
	if (dip) {
		File_CreateDir("/riivolution/temp");
		File_CreateFile("/riivolution/temp/fst");
		int tmpfd = File_Open("/riivolution/temp/fst", O_WRONLY | O_TRUNC);
		if (tmpfd < 0)
			return LauncherStatus::IosError;
		File_Write(tmpfd, RVL_GetFST(), RVL_GetFSTSize());
		File_Close(tmpfd);
		fstdata[1] = (u32)(RVL_GetShiftOffset(fstdata[2]) >> 2);
		RVL_AddPatch(RVL_AddFile("/riivolution/temp/fst"), 0, (u64)fstdata[1] << 2, fstdata[2]);
		fstdata[2] >>= 2;
		fstdata[3] = fstdata[2]; // NOTE: Not really sure why
		File_CreateFile("/riivolution/temp/fst.header");
		tmpfd = File_Open("/riivolution/temp/fst.header", O_WRONLY | O_TRUNC);
		if (tmpfd < 0)
			return LauncherStatus::IosError;
		File_Write(tmpfd, fstdata, 0x40);
		File_Close(tmpfd);
		RVL_AddPatch(RVL_AddFile("/riivolution/temp/fst.header"), 0, 0x420, 0x40);
	} else {
		int difference = RVL_GetFSTSize() - fstdata[2];
		if (difference > 0) {
			difference = ROUND_UP(difference, 0x40);
			// TODO: Check that these two are equal first
			*MEM_ARENA1HIGH = (u8*)*MEM_ARENA1HIGH - difference;
			*MEM_FSTADDRESS = (u8*)*MEM_FSTADDRESS - difference;
			*MEM_FSTADDRESS2 = (u8*)*MEM_FSTADDRESS2 - difference;
			*MEM_FSTSIZE = RVL_GetFSTSize();
		}
		memcpy(*MEM_FSTADDRESS, RVL_GetFST(), RVL_GetFSTSize());
	}

	return LauncherStatus::OK;
}

LauncherStatus::Enum Launcher_AddPlaytimeEntry()
{
	u32 titleid = WDVD_GetTMD()->title_id;
	u16 groupid = WDVD_GetTMD()->group_id;

	static playtime_buf_t playtime_buf ATTRIBUTE_ALIGN(32);
	File_CreateFile(PLAYLOG_FILE);
	int d = File_Open(PLAYLOG_FILE, O_WRONLY);
	if (d >= 0) {
		u64 tick_now = gettime();
		memset(&playtime_buf, 0, sizeof(playtime_buf_t));
		s32 i = 0;
		memcpy(playtime_buf.name, Launcher_GetGameNameWide(), sizeof(playtime_buf.name));
		playtime_buf.ticks_boot = tick_now;
		playtime_buf.ticks_last = tick_now;
		playtime_buf.title_id = titleid;
		playtime_buf.title_gid = groupid;
		for (i = 0; i < 0x1f; i++)
			playtime_buf.checksum += playtime_buf.data[i];

		bool ret = sizeof(playtime_buf_t) == File_Write(d, (u8*)&playtime_buf, sizeof(playtime_buf_t));
		File_Close(d);
		if (ret)
			return LauncherStatus::OK;
	}

	return LauncherStatus::IosError;
}

LauncherStatus::Enum Launcher_ScrubPlaytimeEntry()
{
	int d = File_Open(PLAYLOG_FILE, O_WRONLY);
	if (d >= 0) {
		static int zero ATTRIBUTE_ALIGN(32);
		zero = 0;
		bool ret = sizeof(zero) == File_Write(d, &zero, sizeof(zero));
		File_Close(d);
		if (ret)
			return LauncherStatus::OK;
	}

	return LauncherStatus::IosError;
}

#ifdef RETURN_TO_MENU
static void PatchReturnToMenu(s32 app_section_size, u64 titleID)
{
	u32 *found;

	if ((found = (u32*)FindInBuffer(app_address, app_section_size, __OSLaunchMenuCodeNew, sizeof(__OSLaunchMenuCodeNew))))
	{
		memcpy(found, __OSLaunchRiiv, sizeof(__OSLaunchRiiv));
		found[20] = 0x387A0000; // addi r3, r26, 0
		found[19] = 0x389B0000; // addi r4, r27, 0
		found[26] = 0x387A0000;
		found[25] = 0x389B0000;
	}
	else if ((found = (u32*)FindInBuffer(app_address, app_section_size, __OSLaunchMenuCodeOld, sizeof(__OSLaunchMenuCodeOld))))
	{
		memcpy(found, __OSLaunchRiiv, sizeof(__OSLaunchRiiv));
		found[15] = 0x38BA0000; // addi r5, r26, 0
		found[14] = 0x38DB0000; // addi r6, r27, 0
		found[22] = 0x38BA0000;
		found[21] = 0x38DB0000;
	}
	else
		return;

	if (titleID != 0x0001000152494956llu)
	{
		u16* code = (u16*)found;
		code[1] = titleID >> 48;
		code[3] = (titleID >> 32) & 0xFFFF;
		code[5] = (titleID >> 16) & 0xFFFF;
		code[7] = titleID & 0xFFFF;
	}
}
#endif

static inline void ApplyBinaryPatches(s32 app_section_size)
{
	void* found;

	// DIP
	while ((found = FindInBuffer(app_address, app_section_size, "/dev/di", 7)))
		((u8*)found)[6] = 'o'; // "/dev/di" to "/dev/do"

	// USB_HID
	while ((found = FindInBuffer(app_address, app_section_size, "/dev/usb/hid", 12)))
		((u8*)found)[11] = '0'; // "/dev/usb/hid" to "/dev/usb/hi0"

//	while ((found = FindInBuffer(app_address, app_section_size, "/dev/net/ssl", 12)))
//		((u8*)found)[11] = '0'; // "/dev/net/ssl" to "/dev/net/ss0"

	// prevent NWC from failing to init or shutting down our sockets
	if (ToMount.size()) {
		if ((found = FindInBuffer(app_address, app_section_size, SOStartupCode, sizeof(SOStartupCode))))
			((u16*)found)[3] = 0;

		if ((found = FindInBuffer(app_address, app_section_size, NWC24iCleanupSocketCode, sizeof(NWC24iCleanupSocketCode))))
			((u32*)found)[3] = 0x4E800020;
	}

#ifdef FWRITE_PATCH
	// Apply fwrite patch
	Fwrite_FindPatchLocation((char*)app_address, app_section_size);
#endif

#ifdef RETURN_TO_MENU
	// Return to Riiv is not working properly yet
	if (*(u32*)0x80001808 == 0x48415858) // "HAXX"
		PatchReturnToMenu(app_section_size, 0x00010001af1bf516llu);
	PatchReturnToMenu(app_section_size, 0x000100014a4f4449llu); // "JODI"
	PatchReturnToMenu(app_section_size, 0x0001000152494956llu); // "RIIV"
#endif

	RVL_PatchMemory(&Disc, app_address, app_section_size);
}

LauncherStatus::Enum Launcher_SetVideoMode()
{
	GXRModeObj* vmode;
	u32 tvmode = CONF_GetVideo();
	if (tvmode==CONF_VIDEO_PAL) {
		if (MEM_BASE[3]=='P')
			*MEM_VIDEOMODE = VI_EURGB60;
		else
			*MEM_VIDEOMODE = VI_NTSC;
		if (CONF_GetProgressiveScan() > 0 && VIDEO_HaveComponentCable()) {
			// avoid libogc VIDEO_Configure() fail
			//vmode = &TVEurgb60Hz480Prog;
			vmode = &TVNtsc480Prog;
		} else if (CONF_GetEuRGB60() > 0)
			vmode = &TVEurgb60Hz480IntDf;
		else {
			vmode = &TVPal528IntDf;
			*MEM_VIDEOMODE = VI_PAL;
		}
	} else {
		if (CONF_GetProgressiveScan() > 0 && VIDEO_HaveComponentCable())
			vmode = &TVNtsc480Prog;
		else
			vmode = &TVNtsc480IntDf;
		if (tvmode==CONF_VIDEO_NTSC)
			*MEM_VIDEOMODE = VI_NTSC;
		else
			*MEM_VIDEOMODE = VI_MPAL;
	}

	VIDEO_Configure(vmode);
	VIDEO_SetBlack(true);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();

	return LauncherStatus::OK;
}

LauncherStatus::Enum Launcher_RunApploader()
{
	AppEnter app_enter = NULL;
	AppLoad app_loader = NULL;
	AppExit app_exit = NULL;
	s32 app_section_size = 0;
	s32 app_disc_offset = 0;

	settime(secs_to_ticks(time(NULL) - 946684800));

	// put crap in memory to keep the apploader/dol happy
	*MEM_VIRTUALSIZE = 0x01800000;
	*MEM_PHYSICALSIZE = 0x01800000;
	*MEM_BI2 = 0;
	*MEM_BOOTCODE = 0x0D15EA5E;
	*MEM_VERSION = 1;
	*MEM_ARENA1LOW = 0;
	*MEM_BUSSPEED = 0x0E7BE2C0;
	*MEM_CPUSPEED = 0x2B73A840;
	*MEM_TITLEFLAGS = 0x80000000;
	memcpy(MEM_GAMEONLINE, MEM_BASE, 4);
	DCFlushRange(MEM_BASE, 0x3F00);

	// read the apploader info
	if (WDVD_LowRead(&app, sizeof(app_info), APP_INFO_OFFSET))
		return LauncherStatus::ReadError;
	// read the apploader into memory
	if (WDVD_LowRead(MEM_APPLOADER, app.loader_size + app.data_size, APP_DATA_OFFSET))
		return LauncherStatus::ReadError;
	ICInvalidateRange(MEM_APPLOADER, app.loader_size + app.data_size);
	app.start(&app_enter, &app_loader, &app_exit);
	app_enter((AppReport)nullprintf);

	while (app_loader(&app_address, &app_section_size, &app_disc_offset)) {
		if (WDVD_LowRead(app_address, app_section_size, (u64)app_disc_offset << 2))
			return LauncherStatus::ReadError;
		ApplyBinaryPatches(app_section_size);
		DCFlushRange(app_address, app_section_size);
		app_address = NULL;
		app_section_size = 0;
		app_disc_offset = 0;
	}

#ifdef FWRITE_PATCH
	// Fwrite patch needs to be fixed for SMG+SMG2
	// fwrite patch causes crashes if fwrite is used in a callback (RB1->USB device found)
	if (memcmp(MEM_BASE, "RMG", 3) && memcmp(MEM_BASE, "SB4", 3))
		Fwrite_Patch();
#endif

	// copy the IOS version over the expected IOS version
	memcpy(MEM_IOSEXPECTED, MEM_IOSVERSION, 4);

	*(u32*)0xCD006C00 = 0;	// deinit audio due to libogc fail

	app_address = app_exit();

	return LauncherStatus::OK;
}

LauncherStatus::Enum Launcher_Launch()
{
#ifdef DEBUGGER
	Mega_StartPolling();
#endif
	if (app_address) {
		RVL_Close();
		ISFS_Deinitialize();
		WPAD_Shutdown();
		USB_Deinitialize();
		WDVD_Close();
		__ES_Close();
		SYS_ProtectRange(SYS_PROTECTCHAN3, NULL, 0, SYS_PROTECTRDWR);
		__MaskIrq(IM_MEMADDRESS);
		DCFlushRangeNoSync(MEM_BASE, 0x01800000);
		ICFlashInvalidate();

		SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
		__lwp_thread_stopmultitasking((void(*)())app_address);
	}

	// We shouldn't get to here
	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync(); VIDEO_WaitVSync();
	return LauncherStatus::ReadError;
}
