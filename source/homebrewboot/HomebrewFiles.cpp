/****************************************************************************
 * HomebrewFiles Class
 * for USB Loader GX
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>

#include "HomebrewFiles.h"

HomebrewFiles::HomebrewFiles( const char * path )
{
    filecount = 0;

    FileInfo = ( FileInfos * ) malloc( sizeof( FileInfos ) );
    if ( !FileInfo )
    {
        return;
    }

    memset( &FileInfo[filecount], 0, sizeof( FileInfos ) );

    this->LoadPath( path );
    this->SortList();
}

HomebrewFiles::~HomebrewFiles()
{
    if ( FileInfo )
    {
        free( FileInfo );
        FileInfo = NULL;
    }
}

bool HomebrewFiles::LoadPath( const char * folderpath )
{
    struct stat st;
    DIR_ITER *dir = NULL;
    char filename[1024];

    dir = diropen( folderpath );
    if ( dir == NULL )
    {
        return false;
    }

    while ( dirnext( dir, filename, &st ) == 0 )
    {
        if ( ( st.st_mode & S_IFDIR ) != 0 )
        {
            if ( strcmp( filename, "." ) != 0 && strcmp( filename, ".." ) != 0 )
            {
                char currentname[200];
                snprintf( currentname, sizeof( currentname ), "%s%s/", folderpath, filename );
                this->LoadPath( currentname );
            }
        }
        else
        {
            char temp[5];
            for ( int i = 0; i < 5; i++ )
            {
                temp[i] = filename[strlen( filename )-4+i];
            }

            if ( ( strncasecmp( temp, ".dol", 4 ) == 0 || strncasecmp( temp, ".elf", 4 ) == 0 )
                    && filecount < MAXHOMEBREWS && filename[0] != '.' )
            {

                FileInfo = ( FileInfos * ) realloc( FileInfo, ( filecount + 1 ) * sizeof( FileInfos ) );

                if ( !FileInfo )
                {
                    free( FileInfo );
                    FileInfo = NULL;
                    filecount = 0;
                    dirclose( dir );
                    return false;
                }

                memset( &( FileInfo[filecount] ), 0, sizeof( FileInfo ) );

                strlcpy( FileInfo[filecount].FilePath, folderpath, sizeof( FileInfo[filecount].FilePath ) );
                strlcpy( FileInfo[filecount].FileName, filename, sizeof( FileInfo[filecount].FileName ) );
                FileInfo[filecount].FileSize = st.st_size;
                filecount++;
            }
        }
    }
    dirclose( dir );

    return true;
}

char * HomebrewFiles::GetFilename( int ind )
{
    if ( ind > filecount )
        return NULL;
    else
        return FileInfo[ind].FileName;
}

char * HomebrewFiles::GetFilepath( int ind )
{
    if ( ind > filecount )
        return NULL;
    else
        return FileInfo[ind].FilePath;
}

unsigned int HomebrewFiles::GetFilesize( int ind )
{
    if ( ind > filecount || !filecount || !FileInfo )
        return NULL;
    else
        return FileInfo[ind].FileSize;
}

int HomebrewFiles::GetFilecount()
{
    return filecount;
}

static int ListCompare( const void *a, const void *b )
{
    FileInfos *ab = ( FileInfos* ) a;
    FileInfos *bb = ( FileInfos* ) b;

    return stricmp( ( char * ) ab->FilePath, ( char * ) bb->FilePath );
}
void HomebrewFiles::SortList()
{
    qsort( FileInfo, filecount, sizeof( FileInfos ), ListCompare );
}
