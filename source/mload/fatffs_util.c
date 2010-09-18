/*
    From Custom IOS Module (FAT)

    Copyright (C) 2009 Waninkoko.
    Copyright (C) 2010 Hermes.

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


#include "fatffs_util.h"

#include <string.h>

#include <sys/dir.h>
#include <sys/stat.h>
#include <stdio.h>
#include <malloc.h>

extern u32 nand_mode;

s32 FAT_DeleteDir( const char *dirpath )
{
    DIR_ITER *dir;

    s32 ret;

    /* Open directory */
    dir = diropen( dirpath );
    if ( !dir )
        return -1;

    /* Read entries */
    for ( ;; )
    {
        char   filename[256], newpath[256];
        struct stat filestat;

        /* Read entry */
        if ( dirnext( dir, filename, &filestat ) )
            break;

        /* Non valid entry */
        if ( filename[0] == '.' )
            continue;

        /* Generate entry path */
        strcpy( newpath, dirpath );
        strcat( newpath, "/" );
        strcat( newpath, filename );

        /* Delete directory contents */
        if ( filestat.st_mode & S_IFDIR )
            FAT_DeleteDir( newpath );

        /* Delete object */
        ret = remove( newpath );

        /* Error */
        if ( ret != 0 )
            break;
    }

    /* Close directory */
    dirclose( dir );

    return 0;
}

static int global_error = 0;

static char temp_read_buffer[16384] ATTRIBUTE_ALIGN( 32 );

s32 _FFS_to_FAT_Copy( const char *ffsdirpath, const char *fatdirpath )
{
    int n;
    u32 blocks, ionodes;
    int pos = 0;
    char *list;
    s32 ret;

    u32 ionodes_temp;

    if ( ISFS_GetUsage( ffsdirpath, &blocks, &ionodes ) ) {global_error = -1; return -1;}


    list = memalign( 32, ionodes * 13 );

    if ( !list ) {global_error = -2; return -2;}

    if ( ISFS_ReadDir( ffsdirpath, list , &ionodes ) ) {free( list ); global_error = -3; return -3;}

    if ( ionodes ) mkdir( fatdirpath, S_IRWXO | S_IRWXG | S_IRWXU );


    /* Read entries */
    for ( n = 0; n < ionodes; n++ )
    {
        char  * filename;
        char newffspath[256], newfatpath[256];

        /* Read entry */
        filename = &list[pos];
        pos += strlen( &list[pos] ) + 1;

        /* Non valid entry */
        if ( filename[0] == '.' )
            continue;

        /* Generate entry path */
        strcpy( newffspath, ffsdirpath );
        strcat( newffspath, "/" );
        strcat( newffspath, filename );

        strcpy( newfatpath, fatdirpath );
        strcat( newfatpath, "/" );
        strcat( newfatpath, filename );

        ret = ISFS_ReadDir( newffspath, NULL, &ionodes_temp );
        if ( ret == 0 ) // it is a directory
        {

            _FFS_to_FAT_Copy( newffspath, newfatpath );

            if ( global_error ) {free( list ); return global_error;}
        }

        else // copy the file
        {
            int fd;
            FILE *fp;
            fd = ISFS_Open( newffspath, ISFS_OPEN_READ );
            if ( fd < 0 )
            {
                global_error = -4;
                free( list ); return global_error;
            }
            else
            {
                int len;
                fp = fopen( newfatpath, "w" );
                if ( !fd ) {ISFS_Close( fd ); global_error = -5; free( list ); return global_error;}

                len = ISFS_Seek( fd, 0, 2 );
                //if(len<0) {ISFS_Close(fd);global_error=-6;free(list);return global_error;}
                ISFS_Seek( fd, 0, 0 );

                while ( len > 0 )
                {
                    ret = len; if ( len > 16384 ) ret = 16384;
                    if ( ISFS_Read( fd, temp_read_buffer, ret ) != ret )
                    {
                        global_error = -7;
                        break;
                    }
                    if ( fwrite( temp_read_buffer, 1, ret, fp ) != ret )
                    {
                        global_error = -8;
                        break;
                    }
                    len -= ret;
                }

                fclose( fp );
                ISFS_Close( fd );

                if ( global_error ) {free( list ); return global_error;}
            }
        }

    }

    free( list );
    return 0;
}

s32 FFS_to_FAT_Copy( const char *ffsdirpath, const char *fatdirpath )
{
    u32 blocks, ionodes;
    int ret;

    char create_dir[256];

    ISFS_Initialize();

    ret = ISFS_GetUsage( ffsdirpath, &blocks, &ionodes );

    if ( ret == 0 )
    {
        int n = 0;

        // creating the path directory

        strcpy( create_dir, fatdirpath );

        while ( create_dir[n] != 0 && create_dir[n] != '/' ) n++;

        if ( create_dir[n] == '/' ) n++;

        while ( create_dir[n] != 0 )
        {
            if ( create_dir[n] == '/' )
            {
                create_dir[n] = 0;
                mkdir( create_dir, S_IRWXO | S_IRWXG | S_IRWXU );
                create_dir[n] = '/';
            }
            n++;
        }
        global_error = 0;
        // copy files
        _FFS_to_FAT_Copy( ffsdirpath, fatdirpath );

        ret = global_error = 0;
    }
    else ret = -101;

    ISFS_Deinitialize();


    return ret;

}

static char temp_cad[512];

void create_FAT_FFS_Directory( struct discHdr *header )
{

    char device[2][4] =
    {
        "sd:",
        "ud:"
    };


    if ( !header ) return;

    sprintf( ( char * ) temp_cad, "%s/nand%c", &device[( nand_mode & 2 )!=0][0], ( nand_mode & 0xc ) ? 49 + ( ( nand_mode >> 2 ) & 3 ) : '\0' );

    sprintf( ( char * ) temp_cad + 32, "%2.2x%2.2x%2.2x%2.2x", header->id[0], header->id[1], header->id[2], header->id[3] );


    sprintf( ( char * ) temp_cad + 64, "%s/title/00010000/%s", temp_cad, temp_cad + 32 );
    sprintf( ( char * ) temp_cad + 128, "%s/title/00010004/%s", temp_cad, temp_cad + 32 );
    sprintf( ( char * ) temp_cad + 256, "/title/00010000/%s", temp_cad + 32 );
    sprintf( ( char * ) temp_cad + 384, "/title/00010004/%s", temp_cad + 32 );


}

int test_FAT_game( char * directory )
{
    DIR_ITER * dir = NULL;

    dir = diropen( directory );

    if ( dir ) {dirclose( dir ); return 1;}

    return 0;
}


char *get_FAT_directory1( void )
{
    return temp_cad + 64;
}

char *get_FAT_directory2( void )
{
    return temp_cad + 128;
}

char *get_FFS_directory1( void )
{
    return temp_cad + 256;
}

char *get_FFS_directory2( void )
{
    return temp_cad + 384;
}
