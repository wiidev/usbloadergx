#include <gccore.h>
#include <stdio.h>
#include <string.h>

/* init-globals */
bool geckoinit = false;
bool textVideoInit = false;

#ifndef NO_DEBUG
#include <stdarg.h>

//using the gprintf from crediar because it is smaller than mine
void gprintf( const char *str, ... )
{
	if (!(geckoinit))return;

	char astr[4096];

	va_list ap;
	va_start(ap,str);

	vsprintf( astr, str, ap );

	va_end(ap);

	//usb_sendbuffer( 1, astr, strlen(astr) );
	usb_sendbuffer_safe( 1, astr, strlen(astr) );
}

bool InitGecko()
{
	u32 geckoattached = usb_isgeckoalive(EXI_CHANNEL_1);
	if (geckoattached)
	{
		usb_flush(EXI_CHANNEL_1);
		return true;
	}
	else return false;
}

static char ascii(char s)
{
    if(s < 0x20)
	return '.';
    if(s > 0x7E)
	return '.';
    return s;
}

void hexdump(void *d, int len)
{
    u8 *data;
    int i, off;
    data = (u8*)d;

    gprintf("\n       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  0123456789ABCDEF");
    gprintf("\n====  ===============================================  ================\n");

    for (off=0; off<len; off += 16)
    {
	gprintf("%04x  ",off);
	for(i=0; i<16; i++)
	    if((i+off)>=len)
		gprintf("   ");
	else
	    gprintf("%02x ",data[off+i]);

	gprintf(" ");
	for(i=0; i<16; i++)
	    if((i+off)>=len)
		gprintf(" ");
	    else
		gprintf("%c",ascii(data[off+i]));
	gprintf("\n");
    }
}



#endif /* NO_DEBUG */
