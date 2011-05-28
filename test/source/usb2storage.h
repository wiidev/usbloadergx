/****************************************************************************
* WiiMC
* usb2storage.h -- USB mass storage support, inside starlet
* Copyright (C) 2008 Kwiirk
* Improved for homebrew by rodries and Tantric
* 
* IOS 202 and the ehcimodule must be loaded before using this!
***************************************************************************/

#ifndef __USB2STORAGE_H__
#define __USB2STORAGE_H__

#ifdef __cplusplus
extern "C"
{
#endif

    void USB2Enable(bool e);
    void USB2Storage_Close();
    int GetUSB2LanPort(void);
    void SetUSB2Mode(int mode);
    //mode 0: port0 disabled & port1 disabled
    //mode 1: port0 enabled & port1 disabled
    //mode 2: port0 disabled & port1 enabled
    //mode 3: port0 enabled & port1 enabled

#ifdef __cplusplus

}

#endif

#endif
