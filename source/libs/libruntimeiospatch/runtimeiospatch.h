// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// Copyright (C) 2010		Joseph Jordan <joe.ftpii@psychlaw.com.au>
// Copyright (C) 2012-2013	damysteryman
// Copyright (C) 2012-2015	Christopher Bratusek <nano@jpberlin.de>
// Copyright (C) 2013		DarkMatterCore
// Copyright (C) 2014		megazig
// Copyright (C) 2015		FIX94

#ifndef __RUNTIMEIOSPATCH_H__
#define __RUNTIMEIOSPATCH_H__

/**
 * Version information for Libruntimeiospatch.
 */
#define LIB_RUNTIMEIOSPATCH_VERSION "1.5.2"

//==============================================================================
// HW_RVL header
//==============================================================================
#if defined(HW_RVL) /* defined(HW_RVL) */

/**
 *Returns true when HW_AHBPROT access can be applied
 */
#define AHBPROT_DISABLED (*(vu32*)0xcd800064 == 0xFFFFFFFF)

//==============================================================================
// Error code definitions:
//==============================================================================
#define ERROR_AHBPROT       -5
#define ERROR_PATCH         -7

//==============================================================================
// C++ header
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
/* __cplusplus */

//==============================================================================
// Extra standard declarations
//==============================================================================
//typedef signed int s32;
//==============================================================================

//==============================================================================
// Patchsets:
//==============================================================================
/*
Wii:
    * DI Readlimit
    * ISFS Permissions
    * ES SetUID
    * ES SetIdentify
    * Hash Check (aka Trucha)
    * New Hash Check (aka New Trucha)
    * SSL patches

Sciifii:
    * MEM2 Prot
    * ES OpenTitleContent 1 & 2
    * ES ReadContent Prot
    * ES CloseContent
    * ES TitleVersionCheck
    * ES TitleDeleteCheck

vWii:
   * Kill Anti-SystemTitle-Install 1, 2, 3, 4 & 5
*/


//==============================================================================
// Functions:
//==============================================================================

/**
 * This function can be used to keep HW_AHBPROT access when going to reload IOS
 * @param verbose Flag determing whether or not to print messages on-screen
 * @example 
 *      if(AHBPROT_DISABLED) {
 *          s32 ret;
 *          ret = IosPatch_AHBPROT(false);
 *          if (ret) {
 *              IOS_ReloadIOS(36);
 *          } else {
 *              printf("IosPatch_AHBPROT failed.");
 *          }
 *      }
 * @return Signed 32bit integer representing code
 *      > 0             : Success   - return equals to number of applied patches
 *      ERROR_AHBPROT   : Error     - No HW_AHBPROT access
 */
s32 IosPatch_AHBPROT(bool verbose);


/**
 * This function applies patches on current IOS
 * @see Patchsets
 * @param wii Flag determining whether or not to apply Wii patches.
 * @param sciifii Flag determining whether or not to apply extra Sciifii patches.
 * @param vwii Flag determining whether or not to apply extra vWii patches.
 * @param wiivc Flag determining whether or not to apply WiiVC patches.
 * @param verbose Flag determining whether or not to print messages on-screen.
 * @example if(AHBPROT_DISABLED) IosPatch_FULL(true, false, false, false);
 * @return Signed 32bit integer representing code
 *      > 0             : Success   - return equals to number of applied patches
 *      ERROR_AHBPROT   : Error     - No HW_AHBPROT access
 *      ERROR_PATCH     : Error     - Patching HW_AHBPROT access failed
 */
s32 IosPatch_RUNTIME(bool wii, bool sciifii, bool vwii, bool wiivc, bool verbose);


/**
 * This function combines IosPatch_AHBPROT + IOS_ReloadIOS + IosPatch_RUNTIME
 * @see Patchsets
 * @param wii Flag determining whether or not to apply Wii patches.
 * @param sciifii Flag determining whether or not to apply extra Sciifii patches.
 * @param vwii Flag determining whether or not to apply extra vWii patches.
 * @param wiivc Flag determining whether or not to apply WiiVC patches.
 * @param verbose Flag determining whether or not to print messages on-screen.
 * @param IOS Which IOS to reload into.
 * @example if(AHBPROT_DISABLED) IosPatch_FULL(true, false, false, false, 58);
 * @return Signed 32bit integer representing code
 *      > 0             : Success   - return equals to number of applied patches
 *      ERROR_AHBPROT   : Error     - No HW_AHBPROT access
 *      ERROR_PATCH     : Error     - Patching HW_AHBPROT access failed
 */
s32 IosPatch_FULL(bool wii, bool sciifii, bool vwii, bool wiivc, bool verbose, int IOS);

/**
 * This function patches only SSL certificate check
 * @param verbose Flag determing whether or not to print messages on-screen.
 * @example if(AHBPROT_DISABLED) IosPatch_SSL(true);
 * @return Signed 32bit integer representing code
 *      > 0             : Success   - return equals to number of applied patches
 *      ERROR_AHBPROT   : Error     - No HW_AHBPROT access
 */
s32 IosPatch_SSL(bool verbose);

//==============================================================================
// C++ footer
//==============================================================================
#ifdef __cplusplus
}
#endif /* __cplusplus */

//==============================================================================
// HW_RVL footer
//==============================================================================
#endif /* defined(HW_RVL) */

#endif
