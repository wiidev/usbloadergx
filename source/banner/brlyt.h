/*
 *  brlyt.h
 *  BannerPlayer
 *
 *  Created by Alex Marshall on 09/03/16.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _BRLYT_H_
#define _BRLYT_H_

#include <gctypes.h>

typedef struct BRLYT_objs
{
	char	type[4];	// The type of object (FourCC from BRLYT file)
	u32	offset;		// Offset into the BRLYT file to find this object.
} BRLYT_object;

void BRLYT_Initialize(char rootpath[]);
int BRLYT_ReadObjects(BRLYT_object** objs);
void BRLYT_Finish();

#endif //_BRLYT_H_
