/****************************************************************************
 * DiscBrowser
 * USB Loader GX 2009
 *
 * DiscBrowser.h
 ***************************************************************************/

#ifndef _DISCBROWSER_H_
#define _DISCBROWSER_H_

int DiscBrowse( struct discHdr * header );
int autoSelectDol( const char *id, bool force );
int autoSelectDolMenu( const char *id, bool force );
u8 DiscMount( struct discHdr * header );

#endif
