/****************************************************************************
 * Copyright 2009 The Lemon Man and thanks to luccax, Wiipower, Aurelio and crediar
 * Copyright 2010 Dimok
 * Copyright 2011 FIX94, oggzee
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/

/* ALTERED VERSION (2026, for "Create Wii Menu Channel" in USB Loader GX): a
 * per-game shortcut. The loader path and up to two extra arguments (the game ID)
 * live in a config block that source/channel/fwdchannel.c patches, so one
 * build serves every channel. Device mounting is reduced to libfat's
 * fatMountSimple for SD and the first USB drive.
 *
 * argv passed to the loader: [0] full path, then <arguments> from the
 * loader's meta.xml (like the Homebrew Channel does), then the config args.
 */
#include <gccore.h>
#include <ogc/machine/processor.h>
#include <ogc/usbstorage.h>
#include <sdcard/wiisd_io.h>
#include <fat.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#define EXECUTE_ADDR ((u8 *)0x92000000)
#define BOOTER_ADDR  ((u8 *)0x93000000)
#define ARGS_ADDR    ((u8 *)0x93200000)
#define MAX_EXE_SIZE (BOOTER_ADDR - EXECUTE_ADDR)

/* Config block, found by fwdchannel.c through its magic.
 * Layout: "ULGXFWD1" | path[120] | arg1[64] | arg2[64] = 256 bytes.
 * path is relative to the device root and tried on sd: first, then usb1:. */
#define CFG_MAGIC_LEN 8
#define CFG_PATH_OFS  8
#define CFG_PATH_LEN  120
#define CFG_ARG1_OFS  128
#define CFG_ARG2_OFS  192
#define CFG_ARG_LEN   64
char fwd_cfg[256] __attribute__((used, aligned(32))) =
	"ULGXFWD1" "/apps/usbloader_gx/boot.dol";

#define MAX_CMDLINE 4096
static char cmdline[MAX_CMDLINE];
static int cmdline_len = 1; /* the list ends with a double \0 */
static int cmdline_argc = 0;

typedef void (*entrypoint)(void);
extern void __exception_setreload(int t);
extern void __exception_closeall(void);
#include "app_booter_bin.h" /* app_booter_bin[], app_booter_bin_end[] из data/ */

static bool sd_mounted = false, usb_mounted = false;

static void unmount_all(void)
{
	if (sd_mounted)
		fatUnmount("sd:/");
	if (usb_mounted)
		fatUnmount("usb1:/");
	__io_wiisd.shutdown();
	/* the USB drive is left powered for the loader */
}

static void back_to_menu(void)
{
	unmount_all();
	/* tell Priiloader to go to the System Menu instead of autobooting */
	*(vu32 *)0x8132FFFB = 0x50756e65;
	DCFlushRange((void *)0x8132FFFB, 4);
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

static int arg_addl(const char *arg, int len)
{
	if (cmdline_len + len + 1 > MAX_CMDLINE)
		return -1;
	memcpy(cmdline + cmdline_len - 1, arg, len);
	cmdline_len += len + 1;
	cmdline[cmdline_len - 2] = 0;
	cmdline[cmdline_len - 1] = 0;
	cmdline_argc++;
	return 0;
}

static FILE *open_on(const char *dev, char *path, size_t size)
{
	snprintf(path, size, "%s:%.*s", dev, CFG_PATH_LEN, fwd_cfg + CFG_PATH_OFS);
	return fopen(path, "rb");
}

static bool mount_usb(void)
{
	time_t start = time(NULL);
	/* a hard drive can take a few seconds to spin up */
	while (time(NULL) - start < 10)
	{
		if (__io_usbstorage.startup() && __io_usbstorage.isInserted())
			return fatMountSimple("usb1", &__io_usbstorage);
		usleep(200000);
	}
	return false;
}

/* Add <arg> entries of <arguments> from meta.xml next to the DOL. */
static void add_meta_args(const char *exe_path)
{
	char meta_path[256];
	const char *slash = strrchr(exe_path, '/');
	struct stat st;
	if (!slash)
		return;
	snprintf(meta_path, sizeof(meta_path), "%.*smeta.xml", (int)(slash - exe_path + 1), exe_path);
	if (stat(meta_path, &st) != 0 || st.st_size <= 0 || st.st_size > 64 * 1024)
		return;

	char *buf = calloc(st.st_size + 1, 1);
	if (!buf)
		return;
	FILE *f = fopen(meta_path, "rb");
	if (!f || fread(buf, 1, st.st_size, f) != (size_t)st.st_size)
	{
		if (f)
			fclose(f);
		free(buf);
		return;
	}
	fclose(f);

	/* drop comments */
	char *p = buf, *e;
	while ((p = strstr(p, "<!--")) != NULL)
	{
		e = strstr(p, "-->");
		if (!e)
		{
			*p = 0;
			break;
		}
		memmove(p, e + 3, strlen(e + 3) + 1);
	}

	char *start = strstr(buf, "<arguments>");
	char *end = start ? strstr(start, "</arguments>") : NULL;
	if (start && end && strstr(buf, "<app"))
	{
		p = start;
		while ((p = strstr(p, "<arg>")) != NULL && p < end)
		{
			p += 5;
			e = strstr(p, "</arg>");
			if (!e || e > end)
				break;
			arg_addl(p, e - p);
			p = e + 6;
		}
	}
	free(buf);
}

int main(int argc, char *argv[])
{
	u32 cookie;
	char path[200];
	FILE *exe = NULL;

	__exception_setreload(0);

	/* black screen while loading */
	VIDEO_Init();
	GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
	void *xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	sd_mounted = fatMountSimple("sd", &__io_wiisd);
	if (sd_mounted)
		exe = open_on("sd", path, sizeof(path));
	if (!exe)
	{
		usb_mounted = mount_usb();
		if (usb_mounted)
			exe = open_on("usb1", path, sizeof(path));
	}
	if (!exe)
		back_to_menu();

	fseek(exe, 0, SEEK_END);
	long size = ftell(exe);
	rewind(exe);
	if (size <= 0 || size > MAX_EXE_SIZE || fread(EXECUTE_ADDR, 1, size, exe) != (size_t)size)
	{
		fclose(exe);
		back_to_menu();
	}
	fclose(exe);
	DCFlushRange(EXECUTE_ADDR, size);

	arg_addl(path, strlen(path));
	add_meta_args(path);
	if (fwd_cfg[CFG_ARG1_OFS])
		arg_addl(fwd_cfg + CFG_ARG1_OFS, strnlen(fwd_cfg + CFG_ARG1_OFS, CFG_ARG_LEN - 1));
	if (fwd_cfg[CFG_ARG2_OFS])
		arg_addl(fwd_cfg + CFG_ARG2_OFS, strnlen(fwd_cfg + CFG_ARG2_OFS, CFG_ARG_LEN - 1));

	unmount_all();

	/* app_booter (from USB Loader GX) copies the DOL sections into place and
	 * hands the __argv at ARGS_ADDR to the loaded program */
	struct __argv args;
	memset(&args, 0, sizeof(args));
	args.argvMagic = ARGV_MAGIC;
	args.commandLine = (char *)ARGS_ADDR + sizeof(args);
	args.length = cmdline_len;
	args.argc = cmdline_argc;
	memcpy(ARGS_ADDR + sizeof(args), cmdline, cmdline_len);
	memcpy(ARGS_ADDR, &args, sizeof(args));
	DCFlushRange(ARGS_ADDR, sizeof(args) + cmdline_len);

	u32 booter_size = app_booter_bin_end - app_booter_bin;
	memcpy(BOOTER_ADDR, app_booter_bin, booter_size);
	DCFlushRange(BOOTER_ADDR, booter_size);
	ICInvalidateRange(BOOTER_ADDR, booter_size);

	entrypoint booter = (entrypoint)BOOTER_ADDR;
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	_CPU_ISR_Disable(cookie);
	__exception_closeall();
	booter();
	_CPU_ISR_Restore(cookie);
	return 0;
}
