/* mload.c (for PPC) (c) 2009, Hermes

  This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __MLOAD_H__
#define __MLOAD_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <gccore.h>
#include "unistd.h"

#define MLOAD_MLOAD_THREAD_ID	0x4D4C4400
#define MLOAD_GET_IOS_BASE		0x4D4C4401
#define MLOAD_GET_MLOAD_VERSION 0x4D4C4402

#define MLOAD_LOAD_MODULE		0x4D4C4480
#define MLOAD_RUN_MODULE		0x4D4C4481
#define MLOAD_RUN_THREAD		0x4D4C4482

#define MLOAD_STOP_THREAD		0x4D4C4484
#define MLOAD_CONTINUE_THREAD   0x4D4C4485

#define MLOAD_GET_LOAD_BASE	 0x4D4C4490
#define MLOAD_MEMSET			0x4D4C4491

#define MLOAD_GET_EHCI_DATA		0x4D4C44A0
#define MLOAD_GET_LOG			0x4D4C44A1

#define MLOAD_SET_ES_IOCTLV		0x4D4C44B0

#define MLOAD_GETW				0x4D4C44C0
#define MLOAD_GETH				0x4D4C44C1
#define MLOAD_GETB				0x4D4C44C2
#define MLOAD_SETW				0x4D4C44C3
#define MLOAD_SETH				0x4D4C44C4
#define MLOAD_SETB				0x4D4C44C5

#define MLOAD_SET_LOG_MODE		0x4D4C44D0
#define MLOAD_GET_LOG_BUFFER	0x4D4C44D1

#ifdef __cplusplus
extern "C" {
#endif

// from IOS ELF stripper of neimod

#define getbe32(x) ((adr[x]<<24) | (adr[x+1]<<16) | (adr[x+2]<<8) | (adr[x+3]))

typedef struct
{
		u32		ident0;
		u32		ident1;
		u32		ident2;
		u32		ident3;
		u32		machinetype;
		u32		version;
		u32		entry;
		u32	 phoff;
		u32	 shoff;
		u32		flags;
		u16	 ehsize;
		u16	 phentsize;
		u16	 phnum;
		u16	 shentsize;
		u16	 shnum;
		u16	 shtrndx;
} elfheader;

typedef struct
{
	   u32	  type;
	   u32	  offset;
	   u32	  vaddr;
	   u32	  paddr;
	   u32	  filesz;
	   u32	  memsz;
	   u32	  flags;
	   u32	  align;
} elfphentry;

typedef struct
{
	void *start;
	int prio;
	void *stack;
	int size_stack;
} data_elf;

/*--------------------------------------------------------------------------------------------------------------*/

// to init/test if the device is running

int mload_init();

/*--------------------------------------------------------------------------------------------------------------*/

// to close the device (remember call it when rebooting the IOS!)

int mload_close();

/*--------------------------------------------------------------------------------------------------------------*/

// to get the thread id of mload

int mload_get_thread_id();

/*--------------------------------------------------------------------------------------------------------------*/

// get the base and the size of the memory readable/writable to load modules

int mload_get_load_base(u32 *starlet_base, int *size);

/*--------------------------------------------------------------------------------------------------------------*/

// load and run a module from starlet (it need to allocate MEM2 to send the elf file)
// the module must be a elf made with stripios

int mload_module(void *addr, int len);

/*--------------------------------------------------------------------------------------------------------------*/

// load a module from the PPC
// the module must be a elf made with stripios

int mload_elf(void *my_elf, data_elf *data_elf);

/*--------------------------------------------------------------------------------------------------------------*/

// run one thread (you can use to load modules or binary files)

int mload_run_thread(void *starlet_addr, void *starlet_top_stack, int stack_size, int priority);

/*--------------------------------------------------------------------------------------------------------------*/

// stops one starlet thread

int mload_stop_thread(int id);

/*--------------------------------------------------------------------------------------------------------------*/

// continue one stopped starlet thread

int mload_continue_thread(int id);

/*--------------------------------------------------------------------------------------------------------------*/

// fix starlet address to read/write (uses SEEK_SET, etc as mode)

int mload_seek(int offset, int mode);

/*--------------------------------------------------------------------------------------------------------------*/

// read bytes from starlet (it update the offset)

int mload_read(void* buf, u32 size);

/*--------------------------------------------------------------------------------------------------------------*/

// write bytes from starlet (it update the offset)

int mload_write(const void * buf, u32 size);

/*--------------------------------------------------------------------------------------------------------------*/

// fill a block (similar to memset)

int mload_memset(void *starlet_addr, int set, int len);

/*--------------------------------------------------------------------------------------------------------------*/

// get the ehci datas ( ehcmodule.elf uses this address)

void * mload_get_ehci_data();

/*--------------------------------------------------------------------------------------------------------------*/

// set the dev/es ioctlv in routine

int mload_set_ES_ioctlv_vector(void *starlet_addr);

/*--------------------------------------------------------------------------------------------------------------*/


// to get log buffer
// this function return the size of the log buffer and prepare it to read with mload_read() the datas

int mload_get_log();

/*--------------------------------------------------------------------------------------------------------------*/


// to get IOS base for dev/es  to create the cIOS

int mload_get_IOS_base();

int mload_get_version();

/*--------------------------------------------------------------------------------------------------------------*/

int mload_getw(const void * addr, u32 *dat);
int mload_geth(const void * addr, u16 *dat);
int mload_getb(const void * addr, u8 *dat);

int mload_setw(const void * addr, u32 dat);
int mload_seth(const void * addr, u16 dat);
int mload_setb(const void * addr, u8 dat);

int wanin_mload_get_IOS_base();
int mload_set_gecko_debug();

#ifdef __cplusplus
  }
#endif


#endif
