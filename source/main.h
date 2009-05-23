/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * demo.h
 ***************************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "FreeTypeGX.h"

extern struct SSettings Settings;

void ExitApp();
void DefaultSettings();
extern int ExitRequested;
extern FreeTypeGX *fontSystem;
extern bool netcheck;
extern int Net_Init(char *ip);

#endif
