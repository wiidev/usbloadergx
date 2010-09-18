/****************************************************************************
 * PromptWindows
 * USB Loader GX 2009
 *
 * PromptWindows.h
 ***************************************************************************/

#ifndef _GAMEINFO_H_
#define _GAMEINFO_H_

int showGameInfo( char *ID );
void build_XML_URL( char *XMLurl, int XMLurlsize );
bool save_XML_URL();
bool save_gamelist( int txt );
void MemInfoPrompt();
#endif
