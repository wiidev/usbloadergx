/*-------------------------------------------------------------

di2.h -- Drive Interface library

Written by rodries
Modified from (and supplemental to) original libdi library:

Team Twiizers
Copyright (C) 2008

Erant
marcan


This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/



/*
All buffers in this document need to be 32-byte aligned!
*/

#ifndef __DI2_H__
#define __DI2_H__

#include <stdint.h>
#include <ogc/ipc.h>
#include <ogc/disc_io.h>
#include <di/di.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*
    FUNCTION PROTOTYPES GO HERE!
    */

    int DI2_Init(bool dvdx);
    void DI2_Mount();
    void DI2_Close();
    int DI2_GetStatus();

    int DI2_Identify(DI_DriveID* id);
    int DI2_ReadDiscID(u64 *id);
    int DI2_GetError(uint32_t* error);
    int DI2_GetCoverRegister(uint32_t* status);
    int DI2_Reset();

    int DI2_StopMotor();
    int DI2_Eject();
    int DI2_KillDrive();

    int DI2_ReadDVD(void* buf, uint32_t len, uint32_t lba);
    int DI2_ReadDVDAsync(void* buf, uint32_t len, uint32_t lba, ipccallback ipc_cb);

    int DI2_Read(void *buf, u32 size, u32 offset);
    int DI2_UnencryptedRead(void *buf, u32 size, u32 offset);

    int DI2_ReadDVDConfig(uint32_t* val, uint32_t flag);
    int DI2_ReadDVDCopyright(uint32_t* copyright);
    int DI2_ReadDVDDiscKey(void* buf);
    int DI2_ReadDVDPhysical(void* buf);
    int DI2_ReportKey(int keytype, uint32_t lba, void* buf);

    int DI2_OpenPartition(u32 offset);
    int DI2_ClosePartition(void);

    void DI2_SetDVDMotorStopSecs(int secs);
    unsigned int DI2_GetDVDMotorStopSecs(void);

#ifdef __cplusplus

}

#endif

#endif
