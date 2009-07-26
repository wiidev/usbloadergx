#ifndef __RAMDISC_H
#define __RAMDISC_H

#include <gctypes.h>
#include <ogc/disc_io.h>

#define DEVICE_TYPE_RAM_DISK (('R'<<24)|('A'<<16)|('M'<<8)|'D')

extern const DISC_INTERFACE __io_ramdisk;

/*
initRamDisc initialize a dynamic RAM-disc.
Size is the maximum disksize in a range from 16kB up to 16MB
Padding is the size of blocks to be allocate in a range from 4kB up to Disksize
The RAM-disc is formated in FAT12.
*/
bool initRAMDisc(u32 Size, u32 Padding);
/*
exitRAMDisc destroy all datas
*/
void exitRAMDisc();

/*
NOTE:
if the RAM-disc allready initialized, then initRAMDisc returns with "true" without reinitialize it with the new parameters.

__io_ramdisk.startup() initialize a ramdisc of 8MB with a padding of 64kB

__io_ramdisk.shutdown () will only destroy the RAM-disk, if they from __io_ramdisk.startup () was initialized

if the ramdisc initialized from initRamDisc, then you can remount the filesystem without lost all datas

Example:

fatMount("RAM", &__io_ramdisk, 0, 0, 0);
...
fopen("RAM:/file", ...);
*/
#endif /*__RAMDISC_H*/
