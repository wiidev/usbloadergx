/****************************************************************************
 * DiscBrowser
 * USB Loader GX 2009
 *
 * DiscBrowser.h
 ***************************************************************************/

#ifndef _DISCBROWSER_H_
#define _DISCBROWSER_H_

#include <gctypes.h>
#include "usbloader/disc.h"

int DiscBrowse(const char * GameID, char * dolname, int dolname_size);
int autoSelectDol(const char *id, bool force);
int autoSelectDolMenu(const char *id, bool force);
u8 DiscMount(struct discHdr * header);

#endif
