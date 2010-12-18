#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/iosupport.h>

/* init-globals */
static bool geckoinit = false;

#ifndef NO_DEBUG
#include <stdarg.h>

void gprintf(const char *format, ...)
{
	if (!geckoinit)
        return;

	char * tmp = NULL;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
        u32 level = IRQ_Disable();
        usb_sendbuffer(1, tmp, strlen(tmp));
        IRQ_Restore(level);
	}
	va_end(va);

	if(tmp)
        free(tmp);
}

bool InitGecko()
{
    u32 geckoattached = usb_isgeckoalive(EXI_CHANNEL_1);
    if (geckoattached)
    {
        usb_flush(EXI_CHANNEL_1);
        CON_EnableGecko(1, false);
        geckoinit = true;
        return true;
    }
    else return false;
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

static ssize_t __out_write(struct _reent *r, int fd, const char *ptr, size_t len)
{
    if(geckoinit && ptr)
	{
	    u32 level;
        level = IRQ_Disable();
        usb_sendbuffer(1, ptr, len);
        IRQ_Restore(level);
	}

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
	NULL		// device statvfs_r
};

void USBGeckoOutput()
{
	devoptab_list[STD_OUT] = &gecko_out;
	devoptab_list[STD_ERR] = &gecko_out;
}

#endif /* NO_DEBUG */
