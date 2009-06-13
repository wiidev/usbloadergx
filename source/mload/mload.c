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

#include "mload.h"
#include "ehcmodule.h"
#include "dip_plugin.h"

static const char mload_fs[] ATTRIBUTE_ALIGN(32) = "/dev/mload";
static u32 patch_datas[8] ATTRIBUTE_ALIGN(32);
static s32 mload_fd = -1;


/*--------------------------------------------------------------------------------------------------------------*/

// to init/test if the device is running

int mload_init()
{
int n;

	if(mload_fd>=0) return 0;

	for(n=0;n<10;n++) // try 2.5 seconds
	{
		mload_fd=IOS_Open(mload_fs, 0);

		if(mload_fd>=0) break;

		usleep(250*1000);
	}

return mload_fd;
}

/*--------------------------------------------------------------------------------------------------------------*/

// to close the device (remember call it when rebooting the IOS!)

int mload_close()
{
int ret;

	if(mload_fd<0) return -1;

	ret=IOS_Close(mload_fd);

	mload_fd=-1;

return ret;
}

/*--------------------------------------------------------------------------------------------------------------*/

// to get the thread id of mload

int mload_get_thread_id()
{
int ret;
s32 hid = -1;


	if(mload_init()<0) return -1;

	hid = iosCreateHeap(0x800);

	if(hid<0) return hid;

	ret= IOS_IoctlvFormat(hid, mload_fd, MLOAD_MLOAD_THREAD_ID, ":");


	iosDestroyHeap(hid);

return ret;

}

/*--------------------------------------------------------------------------------------------------------------*/

// get the base and the size of the memory readable/writable to load modules

int mload_get_load_base(u32 *starlet_base, int *size)
{
int ret;
s32 hid = -1;


	if(mload_init()<0) return -1;

	hid = iosCreateHeap(0x800);

	if(hid<0) return hid;

	ret= IOS_IoctlvFormat(hid, mload_fd, MLOAD_GET_LOAD_BASE, ":ii",starlet_base, size);


	iosDestroyHeap(hid);

return ret;

}

/*--------------------------------------------------------------------------------------------------------------*/

// load and run a module from starlet (it need to allocate MEM2 to send the elf file)
// the module must be a elf made with stripios

int mload_module(void *addr, int len)
{
int ret;
void *buf=NULL;
s32 hid = -1;

	if(mload_init()<0) return -1;

	hid = iosCreateHeap(len+0x800);

	if(hid<0) return hid;

	buf= iosAlloc(hid, len);

	if(!buf) {ret= -1;goto out;}


	memcpy(buf, addr,len);

	ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_LOAD_MODULE, ":d", buf, len);

	if(ret<0) goto out;

	ret=IOS_IoctlvFormat(hid, mload_fd, MLOAD_RUN_MODULE, ":");

	if(ret<0) {ret= -666;goto out;}

out:

	iosDestroyHeap(hid);

return ret;

}

/*--------------------------------------------------------------------------------------------------------------*/

// load a module from the PPC
// the module must be a elf made with stripios

int mload_elf(void *my_elf, data_elf *data_elf)
{
int n,m;
int p;
u8 *adr;
u32 elf=(u32) my_elf;

if(elf & 3) return -1; // aligned to 4 please!

elfheader *head=(void *) elf;
elfphentry *entries;

if(head->ident0!=0x7F454C46) return -1;
if(head->ident1!=0x01020161) return -1;
if(head->ident2!=0x01000000) return -1;

p=head->phoff;

data_elf->start=(void *)  head->entry;

for(n=0; n<head->phnum; n++)
	{
	entries=(void *) (elf+p);
	p+=sizeof(elfphentry);

	if(entries->type == 4)
		{
		adr=(void *) (elf + entries->offset);

        if(getbe32(0)!=0) return -2; // bad info (sure)

		for(m=4; m < entries->memsz; m+=8)
			{
			switch(getbe32(m))
				{
				case 0x9:
					data_elf->start= (void *) getbe32(m+4);
					break;
				case 0x7D:
					data_elf->prio= getbe32(m+4);
					break;
				case 0x7E:
					data_elf->size_stack= getbe32(m+4);
					break;
				case 0x7F:
					data_elf->stack= (void *) (getbe32(m+4));
					break;

				}

			}

		}
    else
	if(entries->type == 1  && entries->memsz != 0 && entries->vaddr!=0)
		{

		if(mload_memset((void *) entries->vaddr, 0, entries->memsz)<0) return -1;
		if(mload_seek(entries->vaddr, SEEK_SET)<0) return -1;
	    if(mload_write((void *) (elf + entries->offset), entries->filesz)<0) return -1;

		}
	}

return 0;
}

/*--------------------------------------------------------------------------------------------------------------*/

// run one thread (you can use to load modules or binary files)

int mload_run_thread(void *starlet_addr, void *starlet_top_stack, int stack_size, int priority)
{
int ret;
s32 hid = -1;


	if(mload_init()<0) return -1;

	hid = iosCreateHeap(0x800);

	if(hid<0) return hid;

	ret= IOS_IoctlvFormat(hid, mload_fd, MLOAD_RUN_THREAD, "iiii:", starlet_addr,starlet_top_stack, stack_size, priority);


	iosDestroyHeap(hid);

return ret;
}

/*--------------------------------------------------------------------------------------------------------------*/

// stops one starlet thread

int mload_stop_thread(int id)
{
int ret;
s32 hid = -1;


	if(mload_init()<0) return -1;

	hid = iosCreateHeap(0x800);

	if(hid<0) return hid;

	ret= IOS_IoctlvFormat(hid, mload_fd, MLOAD_STOP_THREAD, "i:", id);


	iosDestroyHeap(hid);

return ret;

}

/*--------------------------------------------------------------------------------------------------------------*/

// continue one stopped starlet thread

int mload_continue_thread(int id)
{
int ret;
s32 hid = -1;


	if(mload_init()<0) return -1;

	hid = iosCreateHeap(0x800);

	if(hid<0) return hid;

	ret= IOS_IoctlvFormat(hid, mload_fd, MLOAD_CONTINUE_THREAD, "i:", id);


	iosDestroyHeap(hid);

return ret;

}
/*--------------------------------------------------------------------------------------------------------------*/

// fix starlet address to read/write (uses SEEK_SET, etc as mode)

int mload_seek(int offset, int mode)
{
	if(mload_init()<0) return -1;
	return IOS_Seek(mload_fd, offset, mode);
}

/*--------------------------------------------------------------------------------------------------------------*/

// read bytes from starlet (it update the offset)

int mload_read(void* buf, u32 size)
{
	if(mload_init()<0) return -1;
	return IOS_Read(mload_fd, buf, size);
}

/*--------------------------------------------------------------------------------------------------------------*/

// write bytes from starlet (it update the offset)

int mload_write(const void * buf, u32 size)
{
	if(mload_init()<0) return -1;
	return IOS_Write(mload_fd, buf, size);
}

/*--------------------------------------------------------------------------------------------------------------*/

// fill a block (similar to memset)

int mload_memset(void *starlet_addr, int set, int len)
{
int ret;
s32 hid = -1;


	if(mload_init()<0) return -1;

	hid = iosCreateHeap(0x800);

	if(hid<0) return hid;

	ret= IOS_IoctlvFormat(hid, mload_fd, MLOAD_MEMSET, "iii:", starlet_addr, set, len);


	iosDestroyHeap(hid);

return ret;
}

/*--------------------------------------------------------------------------------------------------------------*/

// get the ehci datas ( ehcmodule.elf uses this address)

void * mload_get_ehci_data()
{
int ret;
s32 hid = -1;


	if(mload_init()<0) return NULL;

	hid = iosCreateHeap(0x800);

	if(hid<0) return NULL;

	ret= IOS_IoctlvFormat(hid, mload_fd, MLOAD_GET_EHCI_DATA, ":");
	if(ret<0) return NULL;

	iosDestroyHeap(hid);

return (void *) ret;
}

/*--------------------------------------------------------------------------------------------------------------*/

// set the dev/es ioctlv in routine

int mload_set_ES_ioctlv_vector(void *starlet_addr)
{
int ret;
s32 hid = -1;


	if(mload_init()<0) return -1;

	hid = iosCreateHeap(0x800);

	if(hid<0) return hid;

	ret= IOS_IoctlvFormat(hid, mload_fd, MLOAD_SET_ES_IOCTLV, "i:", starlet_addr);


	iosDestroyHeap(hid);

return ret;
}

static u32 ios_36[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022DDAC, // dvd_read_controlling_data
	0x20201010+1, // handle_di_cmd_reentry (thumb)
	0x20200b9c+1, // ios_shared_alloc_aligned (thumb)
	0x20200b70+1, // ios_shared_free (thumb)
	0x20205dc0+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202b4c+1, // ios_doReadHashEncryptedState (thumb)
	0x20203934+1, // ios_printf (thumb)
};

static u32 ios_38[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022cdac, // dvd_read_controlling_data
	0x20200d38+1, // handle_di_cmd_reentry (thumb)
	0x202008c4+1, // ios_shared_alloc_aligned (thumb)
	0x20200898+1, // ios_shared_free (thumb)
	0x20205b80+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202874+1, // ios_doReadHashEncryptedState (thumb)
	0x2020365c+1, // ios_printf (thumb)
};


int load_ehc_module() {

    int is_ios=0;

	if(mload_module(ehcmodule, size_ehcmodule)<0) return -1;
	usleep(350*1000);


	// Test for IOS

	mload_seek(0x20207c84, SEEK_SET);
	mload_read(patch_datas, 4);
	if(patch_datas[0]==0x6e657665)
		{
		is_ios=38;
		}
	else
		{
		is_ios=36;
		}

	if(is_ios==36)
		{
		// IOS 36
		memcpy(ios_36, dip_plugin, 4);		// copy the entry_point
		memcpy(dip_plugin, ios_36, 4*10);	// copy the adresses from the array

		mload_seek(0x1377E000, SEEK_SET);	// copy dip_plugin in the starlet
		mload_write(dip_plugin,size_dip_plugin);

		// enables DIP plugin
		mload_seek(0x20209040, SEEK_SET);
		mload_write(ios_36, 4);

		}
	if(is_ios==38)
		{
		// IOS 38

		memcpy(ios_38, dip_plugin, 4);	    // copy the entry_point
		memcpy(dip_plugin, ios_38, 4*10);   // copy the adresses from the array

		mload_seek(0x1377E000, SEEK_SET);	// copy dip_plugin in the starlet
		mload_write(dip_plugin,size_dip_plugin);

		// enables DIP plugin
		mload_seek(0x20208030, SEEK_SET);
		mload_write(ios_38, 4);

		}


	mload_close();

return 0;
}

int patch_cios_data() {

    patch_datas[0]=*((u32 *) (dip_plugin+16*4));
	mload_set_ES_ioctlv_vector((void *) patch_datas[0]);
	return 1;
}
