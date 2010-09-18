#include <ogcsys.h>
#include <malloc.h>
#include <string.h>

#include "usbloader/sdhc.h"
#include "usbloader/usbstorage2.h"
#include "usbloader/wdvd.h"
#include "wbfs_rw.h"

/* Constants */
#define MAX_NB_SECTORS  32

u32 sector_size = 512;
rw_sector_callback_t readCallback  = NULL;
rw_sector_callback_t writeCallback = NULL;

void SetSectorSize( u32 size )
{
    sector_size = size;
}

s32 __ReadDVD( void *fp, u32 lba, u32 len, void *iobuf )
{
    void *buffer = NULL;

    u64 offset;
    u32 mod, size;
    s32 ret;

    /* Calculate offset */
    offset = ( ( u64 )lba ) << 2;

    /* Calcualte sizes */
    mod  = len % 32;
    size = len - mod;

    /* Read aligned data */
    if ( size )
    {
        ret = WDVD_UnencryptedRead( iobuf, size, offset );
        if ( ret < 0 )
            goto out;
    }

    /* Read non-aligned data */
    if ( mod )
    {
        /* Allocate memory */
        buffer = memalign( 32, 0x20 );
        if ( !buffer )
            return -1;

        /* Read data */
        ret = WDVD_UnencryptedRead( buffer, 0x20, offset + size );
        if ( ret < 0 )
            goto out;

        /* Copy data */
        void *ptr = ( ( u8 * ) iobuf ) + size;
        memcpy( ptr, buffer, mod );
    }

    /* Success */
    ret = 0;

out:
    /* Free memory */
    if ( buffer )
        free( buffer );

    return ret;
}

s32 __ReadUSB( void *fp, u32 lba, u32 count, void *iobuf )
{
    u32 cnt = 0;
    s32 ret;

    /* Do reads */
    while ( cnt < count )
    {
        void *ptr     = ( ( u8 * )iobuf ) + ( cnt * sector_size );
        u32   sectors = ( count - cnt );

        /* Read sectors is too big */
        if ( sectors > MAX_NB_SECTORS )
            sectors = MAX_NB_SECTORS;

        /* USB read */
        ret = USBStorage2_ReadSectors( lba + cnt, sectors, ptr );
        if ( ret < 0 )
            return ret;

        /* Increment counter */
        cnt += sectors;
    }

    return 0;
}

s32 __WriteUSB( void *fp, u32 lba, u32 count, void *iobuf )
{
    u32 cnt = 0;
    s32 ret;

    /* Do writes */
    while ( cnt < count )
    {
        void *ptr     = ( ( u8 * )iobuf ) + ( cnt * sector_size );
        u32   sectors = ( count - cnt );

        /* Write sectors is too big */
        if ( sectors > MAX_NB_SECTORS )
            sectors = MAX_NB_SECTORS;

        /* USB write */
        ret = USBStorage2_WriteSectors( lba + cnt, sectors, ptr );
        if ( ret < 0 )
            return ret;

        /* Increment counter */
        cnt += sectors;
    }

    return 0;
}

s32 __ReadSDHC( void *fp, u32 lba, u32 count, void *iobuf )
{
    u32 cnt = 0;
    s32 ret;

    /* Do reads */
    while ( cnt < count )
    {
        void *ptr     = ( ( u8 * )iobuf ) + ( cnt * sector_size );
        u32   sectors = ( count - cnt );

        /* Read sectors is too big */
        if ( sectors > MAX_NB_SECTORS )
            sectors = MAX_NB_SECTORS;

        /* SDHC read */
        ret = SDHC_ReadSectors( lba + cnt, sectors, ptr );
        if ( !ret )
            return -1;

        /* Increment counter */
        cnt += sectors;
    }

    return 0;
}

s32 __WriteSDHC( void *fp, u32 lba, u32 count, void *iobuf )
{
    u32 cnt = 0;
    s32 ret;

    /* Do writes */
    while ( cnt < count )
    {
        void *ptr     = ( ( u8 * )iobuf ) + ( cnt * sector_size );
        u32   sectors = ( count - cnt );

        /* Write sectors is too big */
        if ( sectors > MAX_NB_SECTORS )
            sectors = MAX_NB_SECTORS;

        /* SDHC write */
        ret = SDHC_WriteSectors( lba + cnt, sectors, ptr );
        if ( !ret )
            return -1;

        /* Increment counter */
        cnt += sectors;
    }

    return 0;
}
