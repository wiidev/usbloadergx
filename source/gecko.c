#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

/* init-globals */
bool geckoinit = false;
bool textVideoInit = false;

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
        CON_EnableGecko(1, true);
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

#endif /* NO_DEBUG */
