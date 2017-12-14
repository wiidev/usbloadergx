#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <sys/iosupport.h>

// #define DEBUG_TO_FILE
// #define WIFI_GECKO // don't keep this for released build

#ifdef WIFI_GECKO
#include <utils/wifi_gecko.h>
#endif

/* init-globals */
static bool geckoinit = false;

void gprintf(const char *format, ...)
{
	#ifndef DEBUG_TO_FILE
		#ifndef WIFI_GECKO
		if (!geckoinit)
			return;
		#endif
	#endif

	static char stringBuf[4096];
	int len;
	va_list va;
	va_start(va, format);
	if((len = vsnprintf(stringBuf, sizeof(stringBuf), format, va)) > 0)
	{
		#ifdef DEBUG_TO_FILE
		FILE *debugF = fopen("sd:/debug.txt", "a");
		if(!debugF)
			debugF = fopen("sd:/debug.txt", "w");
		if(debugF)
		{
			fwrite(stringBuf, 1, strlen(stringBuf), debugF);
			fclose(debugF);
		}
		#else
		usb_sendbuffer(1, stringBuf, len);
		#endif
		
		#ifdef WIFI_GECKO
		wifi_printf(stringBuf);
		#endif
	}
	va_end(va);
}

bool InitGecko()
{
	u32 geckoattached = usb_isgeckoalive(EXI_CHANNEL_1);
	if (geckoattached)
	{
		usb_flush(EXI_CHANNEL_1);
		geckoinit = true;
		return true;
	}

	return false;
}

char ascii(char s)
{
	if (s < 0x20) return '.';
	if (s > 0x7E) return '.';
	return s;
}

void hexdump(void *d, int len)
{
	u8 *data;
	int i, off;
	data = (u8*) d;

	gprintf("\n       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  0123456789ABCDEF");
	gprintf("\n====  ===============================================  ================\n");

	for (off = 0; off < len; off += 16)
	{
		gprintf("%04x  ", off);
		for (i = 0; i < 16; i++)
			if ((i + off) >= len)
				gprintf("   ");
			else gprintf("%02x ", data[off + i]);

		gprintf(" ");
		for (i = 0; i < 16; i++)
			if ((i + off) >= len)
				gprintf(" ");
			else gprintf("%c", ascii(data[off + i]));
		gprintf("\n");
	}
}

static ssize_t __out_write(struct _reent *r, void *fd, const char *ptr, size_t len)
{
	if(len > 0)
		usb_sendbuffer(1, ptr, len);

	return len;
}

static const devoptab_t gecko_out = {
	"stdout",	// device name
	0,			// size of file structure
	NULL,		// device open
	NULL,		// device close
	__out_write,// device write
	NULL,		// device read
	NULL,		// device seek
	NULL,		// device fstat
	NULL,		// device stat
	NULL,		// device link
	NULL,		// device unlink
	NULL,		// device chdir
	NULL,		// device rename
	NULL,		// device mkdir
	0,			// dirStateSize
	NULL,		// device diropen_r
	NULL,		// device dirreset_r
	NULL,		// device dirnext_r
	NULL,		// device dirclose_r
	NULL,		// device statvfs_r
	NULL,		// device ftruncate_r
	NULL,		// device fsync_r
	NULL,		// device deviceData
	NULL,		// device chmod_r
	NULL,		// device fchmod_r
	NULL,		// device rmdir_r
};

void USBGeckoOutput()
{
	devoptab_list[STD_OUT] = &gecko_out;
	devoptab_list[STD_ERR] = &gecko_out;
}
