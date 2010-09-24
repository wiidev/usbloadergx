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
#include "dip_plugin.h"
#include <malloc.h>

#include "gecko.h"

static const char mload_fs[] ATTRIBUTE_ALIGN( 32 ) = "/dev/mload";
//static u32 patch_datas[8] ATTRIBUTE_ALIGN(32);

static s32 mload_fd = -1;
static s32 hid = -1;
//static data_elf my_data_elf;
//static int thread_id = -1;
//void *external_ehcmodule= NULL;
//int size_external_ehcmodule=0;

/*--------------------------------------------------------------------------------------------------------------*/

// to init/test if the device is running

int mload_init()
{
    int n;

    if (hid < 0) hid = iosCreateHeap(0x800);

    if (hid < 0)
    {
        if (mload_fd >= 0) IOS_Close(mload_fd);

        mload_fd = -1;

        return hid;
    }

    if (mload_fd >= 0)
    {
        return 0;
    }

    for (n = 0; n < 20; n++) // try 5 seconds
    {
        mload_fd = IOS_Open(mload_fs, 0);

        if (mload_fd >= 0) break;

        usleep(250 * 1000);
    }

    if (mload_fd < 0)
    {

        if (hid >= 0)
        {
            iosDestroyHeap(hid);
            hid = -1;
        }
    }

    return mload_fd;
}

/*--------------------------------------------------------------------------------------------------------------*/

// to close the device (remember call it when rebooting the IOS!)

int mload_close()
{
    int ret;

    if (hid >= 0)
    {
        iosDestroyHeap(hid);
        hid = -1;
    }

    if (mload_fd < 0) return -1;

    ret = IOS_Close(mload_fd);

    mload_fd = -1;

    return ret;
}

/*--------------------------------------------------------------------------------------------------------------*/

// to get the thread id of mload

int mload_get_thread_id()
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_MLOAD_THREAD_ID, ":");

    return ret;

}

/*--------------------------------------------------------------------------------------------------------------*/

// get the base and the size of the memory readable/writable to load modules

int mload_get_load_base(u32 *starlet_base, int *size)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_GET_LOAD_BASE, ":ii", starlet_base, size);

    return ret;

}

/*--------------------------------------------------------------------------------------------------------------*/

// load and run a module from starlet (it need to allocate MEM2 to send the elf file)
// the module must be a elf made with stripios

int mload_module(void *addr, int len)
{
    int ret;
    void *buf = NULL;

    if (mload_init() < 0) return -1;

    if (hid >= 0)
    {
        iosDestroyHeap(hid);
        hid = -1;
    }

    hid = iosCreateHeap(len + 0x800);

    if (hid < 0) return hid;

    buf = iosAlloc(hid, len);

    if (!buf)
    {
        ret = -1;
        goto out;
    }

    memcpy(buf, addr, len);

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_LOAD_MODULE, ":d", buf, len);

    if (ret < 0) goto out;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_RUN_MODULE, ":");

    if (ret < 0)
    {
        ret = -666;
        goto out;
    }

    out: if (hid >= 0)
    {
        iosDestroyHeap(hid);
        hid = -1;
    }

    return ret;

}

/*--------------------------------------------------------------------------------------------------------------*/

// load a module from the PPC
// the module must be a elf made with stripios

int mload_elf(void *my_elf, data_elf *data_elf)
{
    int n, m;
    int p;
    u8 *adr;
    u32 elf = (u32) my_elf;

    if (elf & 3) return -1; // aligned to 4 please!

    elfheader *head = (void *) elf;
    elfphentry *entries;

    if (head->ident0 != 0x7F454C46) return -1;
    if (head->ident1 != 0x01020161) return -1;
    if (head->ident2 != 0x01000000) return -1;

    p = head->phoff;

    data_elf->start = (void *) head->entry;

    for (n = 0; n < head->phnum; n++)
    {
        entries = (void *) (elf + p);
        p += sizeof(elfphentry);

        if (entries->type == 4)
        {
            adr = (void *) (elf + entries->offset);

            if (getbe32( 0 ) != 0) return -2; // bad info (sure)

            for (m = 4; m < entries->memsz; m += 8)
            {
                switch (getbe32( m ))
                {
                    case 0x9:
                        data_elf->start = (void *) getbe32( m + 4 );
                        break;
                    case 0x7D:
                        data_elf->prio = getbe32( m + 4 );
                        break;
                    case 0x7E:
                        data_elf->size_stack = getbe32( m + 4 );
                        break;
                    case 0x7F:
                        data_elf->stack = (void *) (getbe32( m + 4 ));
                        break;

                }

            }

        }
        else if (entries->type == 1 && entries->memsz != 0 && entries->vaddr != 0)
        {

            if (mload_memset((void *) entries->vaddr, 0, entries->memsz) < 0) return -1;
            if (mload_seek(entries->vaddr, SEEK_SET) < 0) return -1;
            if (mload_write((void *) (elf + entries->offset), entries->filesz) < 0) return -1;

        }
    }

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------*/

// run one thread (you can use to load modules or binary files)

int mload_run_thread(void *starlet_addr, void *starlet_top_stack, int stack_size, int priority)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_RUN_THREAD, "iiii:", starlet_addr, starlet_top_stack, stack_size,
            priority);

    return ret;
}

/*--------------------------------------------------------------------------------------------------------------*/

// stops one starlet thread

int mload_stop_thread(int id)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_STOP_THREAD, "i:", id);

    return ret;

}

/*--------------------------------------------------------------------------------------------------------------*/

// continue one stopped starlet thread

int mload_continue_thread(int id)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_CONTINUE_THREAD, "i:", id);

    return ret;

}
/*--------------------------------------------------------------------------------------------------------------*/

// fix starlet address to read/write (uses SEEK_SET, etc as mode)

int mload_seek(int offset, int mode)
{
    if (mload_init() < 0) return -1;

    return IOS_Seek(mload_fd, offset, mode);
}

/*--------------------------------------------------------------------------------------------------------------*/

// read bytes from starlet (it update the offset)

int mload_read(void* buf, u32 size)
{
    if (mload_init() < 0) return -1;

    return IOS_Read(mload_fd, buf, size);
}

/*--------------------------------------------------------------------------------------------------------------*/

// write bytes from starlet (it update the offset)

int mload_write(const void * buf, u32 size)
{
    if (mload_init() < 0) return -1;

    return IOS_Write(mload_fd, buf, size);
}

/*--------------------------------------------------------------------------------------------------------------*/

// fill a block (similar to memset)

int mload_memset(void *starlet_addr, int set, int len)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_MEMSET, "iii:", starlet_addr, set, len);

    return ret;
}

/*--------------------------------------------------------------------------------------------------------------*/

// get the ehci datas ( ehcmodule.elf uses this address)

void * mload_get_ehci_data()
{
    int ret;

    if (mload_init() < 0) return NULL;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_GET_EHCI_DATA, ":");
    if (ret < 0) return NULL;

    return (void *) ret;
}

/*--------------------------------------------------------------------------------------------------------------*/

// set the dev/es ioctlv in routine

int mload_set_ES_ioctlv_vector(void *starlet_addr)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_SET_ES_IOCTLV, "i:", starlet_addr);

    return ret;
}

int mload_getw(const void * addr, u32 *dat)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_GETW, "i:i", addr, dat);

    return ret;
}

int mload_geth(const void * addr, u16 *dat)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_GETH, "i:h", addr, dat);

    return ret;
}

int mload_getb(const void * addr, u8 *dat)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_GETB, "i:b", addr, dat);

    return ret;
}

int mload_setw(const void * addr, u32 dat)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_SETW, "ii:", addr, dat);

    return ret;
}

int mload_seth(const void * addr, u16 dat)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_SETH, "ih:", addr, dat);

    return ret;
}

int mload_setb(const void * addr, u8 dat)
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_SETB, "ib:", addr, dat);

    return ret;
}

/*--------------------------------------------------------------------------------------------------------------*/

// to get log buffer
// this function return the size of the log buffer and prepare it to read with mload_read() the datas

int mload_get_log()
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_GET_LOG, ":");

    return ret;

}

/*--------------------------------------------------------------------------------------------------------------*/

// to get IOS base for dev/es  to create the cIOS

int mload_get_IOS_base()
{
    int ret;

    if (mload_init() < 0) return -1;

    ret = IOS_IoctlvFormat(hid, mload_fd, MLOAD_GET_IOS_BASE, ":");

    return ret;

}

/*
 int load_ehc_module()
 {
 int is_ios=0;
 FILE *fp;

 if(!external_ehcmodule)
 {

 fp=fopen("SD:/apps/usbloader_gx/ehcmodule.elf","rb");
 if(fp==NULL)
 fp=fopen("SD:/apps/usbloadergx/ehcmodule.elf","rb");

 if(fp!=NULL)
 {

 fseek(fp, 0, SEEK_END);
 size_external_ehcmodule = ftell(fp);
 external_ehcmodule = memalign(32, size_external_ehcmodule);
 if(!external_ehcmodule)
 {fclose(fp);}
 else
 {
 fseek(fp, 0, SEEK_SET);

 if(fread(external_ehcmodule,1, size_external_ehcmodule ,fp)!=size_external_ehcmodule)
 {free(external_ehcmodule); external_ehcmodule=NULL;}

 fclose(fp);
 }
 }
 }

 if(!external_ehcmodule)
 {
 if(mload_init()<0) return -1;

 if (IOS_GetRevision() == 4) {
 gprintf("Loading ehcmodule v4\n");
 mload_elf((void *) ehcmodule_frag_v4_bin, &my_data_elf);
 } else if (IOS_GetRevision() == 0) { // 0? Strange value for v5 ehcmodule
 gprintf("Loading ehcmodule v5\n");
 mload_elf((void *) ehcmodule_frag_v5_bin, &my_data_elf);
 }
 thread_id = mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
 if(thread_id < 0) return -1;
 }
 else
 {
 if(mload_init()<0) return -1;
 mload_elf((void *) external_ehcmodule, &my_data_elf);
 thread_id = mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
 if(thread_id<0) return -1;
 }
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
 memcpy(ios_36, dip_plugin, 4);      // copy the entry_point
 memcpy((void *) dip_plugin, ios_36, 4*10);  // copy the adresses from the array

 mload_seek(0x1377E000, SEEK_SET);   // copy dip_plugin in the starlet
 mload_write(dip_plugin,size_dip_plugin);

 // enables DIP plugin
 mload_seek(0x20209040, SEEK_SET);
 mload_write(ios_36, 4);

 }
 if(is_ios==38)
 {
 // IOS 38

 memcpy(ios_38, dip_plugin, 4);      // copy the entry_point
 memcpy((void *) dip_plugin, ios_38, 4*10);   // copy the adresses from the array

 mload_seek(0x1377E000, SEEK_SET);   // copy dip_plugin in the starlet
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
 */

