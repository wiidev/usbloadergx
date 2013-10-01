// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// Copyright (C) 2010	Joseph Jordan <joe.ftpii@psychlaw.com.au>
// Copyright (C) 2012	damysteryman
// Copyright (C) 2012	Christopher Bratusek <nano@tuxfamily.org>


#ifndef _RUNTIMEIOSPATCH_H_
	#define _RUNTIMEIOSPATCH_H_

	#define LIB_RUNTIMEIOSPATCH_VERSION "1.3.0"

	#define AHBPROT_DISABLED (*(vu32*)0xcd800064 == 0xFFFFFFFF)

	#define ERROR_AHBPROT	-5
	#define ERROR_PATCH	-7
	#ifdef __cplusplus
	extern "C" {
	#endif

	s32 IosPatch_AHBPROT(bool verbose);
	s32 IosPatch_RUNTIME(bool wii, bool sciifii, bool vwii, bool verbose);
	s32 IosPatch_FULL(bool wii, bool sciifii, bool vwii, bool verbose, int IOS);

	#ifdef __cplusplus
	}
	#endif

#endif
