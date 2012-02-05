/****************************************************************************
 * USB Loader GX Team
 *
 * Main loadup of the application
 *
 * libwiigui
 * Tantric 2009
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <ogcsys.h>
#include <unistd.h>
#include <locale.h>
#include <wiiuse/wpad.h>
#include <di/di.h>
#include <sys/iosupport.h>

#include "video.h"
#include "menu/menus.h"
#include "memory/mem2.h"
#include "wad/nandtitle.h"
#include "StartUpProcess.h"
#include "sys.h"

extern "C"
{
	extern s32 MagicPatches(s32);
	void __exception_setreload(int t);
}

int main(int argc, char *argv[])
{
	__exception_setreload(20);
	// activate magic access rights
	MagicPatches(1);
	// init video
	InitVideo();
	// video frame buffers must be in mem1
	MEM2_init(48);
	// init gecko
	InitGecko();
	// redirect stdout and stderr to gecko
	USBGeckoOutput();
	NandTitles.Get();
	setlocale(LC_ALL, "en.UTF-8");

	if(StartUpProcess::Run(argc, argv) < 0)
		return -1;

	MainMenu(MENU_DISCLIST);
	return 0;
}
