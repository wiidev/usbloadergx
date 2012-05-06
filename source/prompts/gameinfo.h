/****************************************************************************
 * PromptWindows
 * USB Loader GX 2009
 *
 * PromptWindows.h
 ***************************************************************************/

#ifndef _GAMEINFO_H_
#define _GAMEINFO_H_

#include "usbloader/disc.h"

int showGameInfo(int selectedGame, struct discHdr *header);
bool save_gamelist(bool bCSV);

#endif
