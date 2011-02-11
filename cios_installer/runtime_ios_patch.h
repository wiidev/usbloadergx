// Copyright 2010 Joseph Jordan <joe.ftpii@psychlaw.com.au>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#ifndef __RUNTIMEIOSPATCH_H
#define __RUNTIMEIOSPATCH_H

#include <gccore.h>

#define HAVE_AHBPROT ((*(vu32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)

u32 IOSPATCH_Apply();

#endif
