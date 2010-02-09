#include <gccore.h>
#include <stdio.h>
#include <string.h>

/* init-globals */
bool geckoinit = false;
bool textVideoInit = false;

#ifndef NO_DEBUG
#include <stdarg.h>

//using the gprintf from crediar because it is smaller than mine
void gprintf( const char *str, ... ) {
    if (!(geckoinit))return;

    char astr[4096];

    va_list ap;
    va_start(ap,str);

    vsprintf( astr, str, ap );

    va_end(ap);

    usb_sendbuffer_safe( 1, astr, strlen(astr) );
}

bool InitGecko() {
    u32 geckoattached = usb_isgeckoalive(EXI_CHANNEL_1);
    if (geckoattached) {
        usb_flush(EXI_CHANNEL_1);
        return true;
    } else return false;
}


#endif /* NO_DEBUG */
