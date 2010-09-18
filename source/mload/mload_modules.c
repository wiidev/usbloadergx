#include "mload_modules.h"
#include "ehcmodule_frag_v4_bin.h"
#include "ehcmodule_frag_v5_bin.h"
#include "gecko.h"

#define ALIGNED(x) __attribute__((aligned(x)))

#define DEBUG_MLOAD

/* Used for Hermes NAND emulation */
int global_mount;
int sd_ok = 0;
int ud_ok = 0;

static u32 ios_36[16] ATTRIBUTE_ALIGN( 32 ) =
{
    0, // DI_EmulateCmd
    0,
    0x2022DDAC, // dvd_read_controlling_data
    0x20201010 + 1, // handle_di_cmd_reentry (thumb)
    0x20200b9c + 1, // ios_shared_alloc_aligned (thumb)
    0x20200b70 + 1, // ios_shared_free (thumb)
    0x20205dc0 + 1, // ios_memcpy (thumb)
    0x20200048 + 1, // ios_fatal_di_error (thumb)
    0x20202b4c + 1, // ios_doReadHashEncryptedState (thumb)
    0x20203934 + 1, // ios_printf (thumb)
};

static u32 ios_38[16] ATTRIBUTE_ALIGN( 32 ) =
{
    0, // DI_EmulateCmd
    0,
    0x2022cdac, // dvd_read_controlling_data
    0x20200d38 + 1, // handle_di_cmd_reentry (thumb)
    0x202008c4 + 1, // ios_shared_alloc_aligned (thumb)
    0x20200898 + 1, // ios_shared_free (thumb)
    0x20205b80 + 1, // ios_memcpy (thumb)
    0x20200048 + 1, // ios_fatal_di_error (thumb)
    0x20202874 + 1, // ios_doReadHashEncryptedState (thumb)
    0x2020365c + 1, // ios_printf (thumb)
};


static u32 ios_37[16] ATTRIBUTE_ALIGN( 32 ) =
{
    0, // DI_EmulateCmd
    0,
    0x2022DD60, // dvd_read_controlling_data
    0x20200F04 + 1, // handle_di_cmd_reentry (thumb)
    0x2020096C + 1, // ios_shared_alloc_aligned (thumb)
    0x2020093C + 1, // ios_shared_free (thumb)
    0x20205E54 + 1, // ios_memcpy (thumb)
    0x20200048 + 1, // ios_fatal_di_error (thumb)
    0x20202A70 + 1, // ios_doReadHashEncryptedState (thumb)
    0x2020387C + 1, // ios_printf (thumb)
};

static u32 ios_57[16] ATTRIBUTE_ALIGN( 32 ) =
{
    0, // DI_EmulateCmd
    0,
    0x2022cd60, // dvd_read_controlling_data
    0x20200f04 + 1, // handle_di_cmd_reentry (thumb)
    0x2020096c + 1, // ios_shared_alloc_aligned (thumb) // no usado
    0x2020093C + 1, // ios_shared_free (thumb) // no usado
    0x20205EF0 + 1, // ios_memcpy (thumb)
    0x20200048 + 1, // ios_fatal_di_error (thumb)
    0x20202944 + 1, // ios_doReadHashEncryptedState (thumb)
    0x20203750 + 1, // ios_printf (thumb)
};

static u32 ios_60[16] ATTRIBUTE_ALIGN( 32 ) =
{
    0, // DI_EmulateCmd
    0,
    0x2022cd60, // dvd_read_controlling_data
    0x20200f04 + 1, // handle_di_cmd_reentry (thumb)
    0x2020096c + 1, // ios_shared_alloc_aligned (thumb) // no usado
    0x2020093C + 1, // ios_shared_free (thumb) // no usado
    0x20205e00 + 1, // ios_memcpy (thumb)
    0x20200048 + 1, // ios_fatal_di_error (thumb)
    0x20202944 + 1, // ios_doReadHashEncryptedState (thumb)
    0x20203750 + 1, // ios_printf (thumb)
};


u32 patch_datas[8] ATTRIBUTE_ALIGN( 32 );

data_elf my_data_elf;

void *external_ehcmodule = NULL;
int size_external_ehcmodule = 0;

static int my_thread_id = 0;

int load_ehc_module()
{
    int is_ios = 0;

#if 0

    FILE *fp;

// WARNING!!!: load external module suspended
    if ( sd_ok && !external_ehcmodule )
    {

        fp = fopen( "sd:/apps/usbloader_gx/ehcmodule.elf", "rb" );
        if ( fp == NULL )
            fp = fopen( "sd:/apps/usbloadergx/ehcmodule.elf", "rb" );

        if ( fp != 0 )
        {
            fseek( fp, 0, SEEK_END );
            size_external_ehcmodule = ftell( fp );
            external_ehcmodule = memalign( 32, size_external_ehcmodule );
            if ( !external_ehcmodule )
                {fclose( fp );}
            else
            {
                fseek( fp, 0, SEEK_SET );

                if ( fread( external_ehcmodule, 1, size_external_ehcmodule , fp ) != size_external_ehcmodule )
                    {free( external_ehcmodule ); external_ehcmodule = NULL;}

                fclose( fp );
            }
        }
    }
#endif

    /*
        if(mload_init()<0) return -1;
        mload_elf((void *) logmodule, &my_data_elf);
        my_thread_id= mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
        if(my_thread_id<0) return -1;
        */

    if ( !external_ehcmodule )
    {
#ifdef DEBUG_MLOAD
        gprintf( "before mload_init\n" );
#endif
        if ( mload_init() < 0 ) return -1;
#ifdef DEBUG_MLOAD
        gprintf( "after mload_init\n" );
#endif
        if ( IOS_GetRevision() == 4 )
        {
#ifdef DEBUG_MLOAD
            gprintf( "Loading ehcmodule v4\n" );
#endif
            mload_elf( ( void * ) ehcmodule_frag_v4_bin, &my_data_elf );
        }
        else if ( IOS_GetRevision() == 65535 )
        {
#ifdef DEBUG_MLOAD
            gprintf( "Loading ehcmodule v5\n" );
#endif
            mload_elf( ( void * ) ehcmodule_frag_v5_bin, &my_data_elf );
        }
        else
        {
            return -2;
        }
//      mload_elf((void *) ehcmodule, &my_data_elf);
#ifdef DEBUG_MLOAD
        gprintf( "before mload_run_thread\n" );
#endif
        my_thread_id = mload_run_thread( my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio );
        if ( my_thread_id < 0 ) return -1;
        //if(mload_module(ehcmodule, size_ehcmodule)<0) return -1;
    }
    else
    {
        //if(mload_module(external_ehcmodule, size_external_ehcmodule)<0) return -1;
        if ( mload_init() < 0 ) return -1;
        mload_elf( ( void * ) external_ehcmodule, &my_data_elf );
        my_thread_id = mload_run_thread( my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio );
        if ( my_thread_id < 0 ) return -1;
    }
    usleep( 350*1000 );


    // Test for IOS

#if 0
    mload_seek( 0x20207c84, SEEK_SET );
    mload_read( patch_datas, 32 );
    if ( patch_datas[0] == 0x6e657665 )
    {
        is_ios = 38;
    }
    else
    {
        is_ios = 36;
    }

#endif
    is_ios = mload_get_IOS_base();

    switch ( is_ios )
    {

        case 36:

            memcpy( ios_36, dip_plugin, 4 );    // copy the entry_point
            memcpy( dip_plugin, ios_36, 4*10 ); // copy the adresses from the array

            mload_seek( 0x1377E000, SEEK_SET ); // copy dip_plugin in the starlet
            mload_write( dip_plugin, size_dip_plugin );

            // enables DIP plugin
            mload_seek( 0x20209040, SEEK_SET );
            mload_write( ios_36, 4 );
            break;

        case 37:

            memcpy( ios_37, dip_plugin, 4 );    // copy the entry_point
            memcpy( dip_plugin, ios_37, 4*10 ); // copy the adresses from the array

            mload_seek( 0x1377E000, SEEK_SET ); // copy dip_plugin in the starlet
            mload_write( dip_plugin, size_dip_plugin );

            // enables DIP plugin
            mload_seek( 0x20209030, SEEK_SET );
            mload_write( ios_37, 4 );
            break;

        case 38:

            memcpy( ios_38, dip_plugin, 4 );    // copy the entry_point
            memcpy( dip_plugin, ios_38, 4*10 ); // copy the adresses from the array

            mload_seek( 0x1377E000, SEEK_SET ); // copy dip_plugin in the starlet
            mload_write( dip_plugin, size_dip_plugin );

            // enables DIP plugin
            mload_seek( 0x20208030, SEEK_SET );
            mload_write( ios_38, 4 );
            break;

        case 57:

            memcpy( ios_57, dip_plugin, 4 );    // copy the entry_point
            memcpy( dip_plugin, ios_57, 4*10 ); // copy the adresses from the array

            mload_seek( 0x1377E000, SEEK_SET ); // copy dip_plugin in the starlet
            mload_write( dip_plugin, size_dip_plugin );

            // enables DIP plugin
            mload_seek( 0x20208030, SEEK_SET );
            mload_write( ios_57, 4 );
            break;

        case 60:

            memcpy( ios_60, dip_plugin, 4 );    // copy the entry_point
            memcpy( dip_plugin, ios_60, 4*10 ); // copy the adresses from the array

            mload_seek( 0x1377E000, SEEK_SET ); // copy dip_plugin in the starlet
            mload_write( dip_plugin, size_dip_plugin );

            // enables DIP plugin
            mload_seek( 0x20208030, SEEK_SET );
            mload_write( ios_60, 4 );
            break;

    }

    mload_close();

    return 0;
}

#define IOCTL_FAT_MOUNTSD   0xF0
#define IOCTL_FAT_UMOUNTSD  0xF1
#define IOCTL_FAT_MOUNTUSB  0xF2
#define IOCTL_FAT_UMOUNTUSB 0xF3

#define IOCTL_FFS_MODE      0x80

int load_fatffs_module( u8 *discid )
{
    static char fs[] ATTRIBUTE_ALIGN( 32 ) = "fat";
    s32 hid = -1, fd = -1;
    static char file_name[256]  ALIGNED( 0x20 ) = "SD:";
    int n;
    char *p;
    s32 ret;

    p = &file_name[0];

    if ( mload_init() < 0 ) return -1;

    mload_elf( ( void * ) fatffs_module_bin, &my_data_elf );
    my_thread_id = mload_run_thread( my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio );
    if ( my_thread_id < 0 ) return -1;

    global_mount &= ~0xc;

    if ( discid )
    {
        sd_ok = ud_ok = 1;

        /* Function get_fat_name not implemented, this should return the path to the save file */
        // p=get_fat_name(discid);
        p = NULL;

        sd_ok = ud_ok = 0;

        if ( !p ) return -1;

        global_mount &= ~0xf;

        // change 'ud:' by 'usb:'
        if ( p[0] == 'U' ) {global_mount |= 2; file_name[0] = 'U'; file_name[1] = 'S'; file_name[2] = 'B'; memcpy( file_name + 3, ( void * )p + 2, 253 );}
        else {global_mount |= 1; memcpy( file_name, ( void * ) p, 256 );}

        // copy filename to dip_plugin filename area
        mload_seek( *( ( u32 * ) ( dip_plugin + 14*4 ) ), SEEK_SET ); // offset 14 (filename Address - 256 bytes)
        mload_write( file_name, sizeof( file_name ) );
        mload_close();


    }
    else
    {
        if ( ( global_mount & 3 ) == 0 ) return 0;
        if ( global_mount & 1 ) p[0] = 's';
        if ( global_mount & 2 ) p[0] = 'u';
    }
    usleep( 350*1000 );

    /* Create heap */
    if ( hid < 0 )
    {
        hid = iosCreateHeap( 0x100 );
        if ( hid < 0 )
            return -1;
    }

    /* Open USB device */
    fd = IOS_Open( fs, 0 );

    if ( fd < 0 )
    {
        if ( hid >= 0 )
        {
            iosDestroyHeap( hid );
            hid = -1;
        }
        return -1;
    }

    n = 30; // try 20 times
    while ( n > 0 )
    {
        if ( ( global_mount & 10 ) == 2 ) {ret = IOS_IoctlvFormat( hid, fd, IOCTL_FAT_MOUNTUSB, ":" ); if ( ret == 0 ) global_mount |= 8;}
        else {ret = IOS_IoctlvFormat( hid, fd, IOCTL_FAT_MOUNTSD, ":" ); if ( ret == 0 ) {global_mount |= 4;}}

        if ( ( global_mount & 7 ) == 3 && ret == 0 )
            {ret = IOS_IoctlvFormat( hid, fd, IOCTL_FAT_MOUNTSD, ":" ); if ( ret == 0 ) global_mount |= 4;}

        if ( ( global_mount & 3 ) == ( ( global_mount >> 2 ) & 3 ) && ( global_mount & 3 ) ) {ret = 0; break;}
        else ret = -1;

        //ret=IOS_IoctlvFormat(hid, fd, IOCTL_FAT_MOUNTSD, ":");
        //if(ret==0) break;
        usleep( 500*1000 );
        n--;
    }

    if ( fd >= 0 )
    {
        IOS_Close( fd );
        fd = -1;
    }

    if ( hid >= 0 )
    {
        iosDestroyHeap( hid );
        hid = -1;
    }

    if ( n == 0 ) return -1;

    return 0;
}


int enable_ffs( int mode )
{
    static char fs[] ATTRIBUTE_ALIGN( 32 ) = "fat";
    s32 hid = -1, fd = -1;
    s32 ret;

    /* Create heap */
    if ( hid < 0 )
    {
        hid = iosCreateHeap( 0x100 );
        if ( hid < 0 )
            return -1;
    }

    /* Open USB device */
    fd = IOS_Open( fs, 0 );

    if ( fd < 0 )
    {
        if ( hid >= 0 )
        {
            iosDestroyHeap( hid );
            hid = -1;
        }
        return -1;
    }


    ret = IOS_IoctlvFormat( hid, fd, IOCTL_FFS_MODE, "i:", mode );


    if ( fd >= 0 )
    {
        IOS_Close( fd );
        fd = -1;
    }

    if ( hid >= 0 )
    {
        iosDestroyHeap( hid );
        hid = -1;
    }



    return ret;
}

void enable_ES_ioctlv_vector( void )
{
    patch_datas[0] = *( ( u32 * ) ( dip_plugin + 16 * 4 ) );
    mload_set_ES_ioctlv_vector( ( void * ) patch_datas[0] );
}

void Set_DIP_BCA_Datas( u8 *bca_data )
{
    // write in dip_plugin bca data area
    mload_seek( *( ( u32 * ) ( dip_plugin + 15*4 ) ), SEEK_SET ); // offset 15 (bca_data area)
    mload_write( bca_data, 64 );
    mload_close();
}

u8 *search_for_ehcmodule_cfg( u8 *p, int size )
{
    int n;

    for ( n = 0; n < size; n++ )
    {
        if ( !memcmp( ( void * ) &p[n], "EHC_CFG", 8 ) && p[n+8] == 0x12 && p[n+9] == 0x34 && p[n+10] == 0x00 && p[n+11] == 0x01 )
        {
            return &p[n];
        }
    }

    return NULL;
}

void test_and_patch_for_port1()
{
    // test for port 1

    u8 * ehc_data = NULL;

    if ( IOS_GetRevision() == 4 )
    {
        ehc_data = search_for_ehcmodule_cfg( ( void * ) ehcmodule_frag_v4_bin, ehcmodule_frag_v4_bin_size );
    }
    else if ( IOS_GetRevision() == 65535 )
    {
        ehc_data = search_for_ehcmodule_cfg( ( void * ) ehcmodule_frag_v5_bin, ehcmodule_frag_v5_bin_size );
    }

    if ( ehc_data )
    {
        ehc_data += 12;
        use_port1 = ehc_data[0];

    }


    if ( use_port1 )
        // release port 0 and use port 1
    {
        u32 dat = 0;
        u32 addr;

        // get EHCI base registers
        mload_getw( ( void * ) 0x0D040000, &addr );

        addr &= 0xff;
        addr += 0x0D040000;


        mload_getw( ( void * ) ( addr + 0x44 ), &dat );
        if ( ( dat & 0x2001 ) == 1 ) mload_setw( ( void * ) ( addr + 0x44 ), 0x2000 );
        mload_getw( ( void * ) ( addr + 0x48 ), &dat );
        if ( ( dat & 0x2000 ) == 0x2000 ) mload_setw( ( void * ) ( addr + 0x48 ), 0x1001 );
    }
}
//////////////////////////////////

