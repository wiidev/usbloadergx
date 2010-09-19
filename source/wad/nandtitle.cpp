#include "nandtitle.h"
#include "gecko.h"

extern "C"
{
    extern s32 MagicPatches( s32 );
}

static u8 tmd_buf[ MAX_SIGNED_TMD_SIZE ] ATTRIBUTE_ALIGN( 32 );

//based on one from comex's nand formatter
static u64 atoi_hex( const char *s ) {
    u64 ret=0;
    u32 n = strlen( s );

    for( u32 i = 0; i < n; i++) {
	    if(s[i] > 0x39) {
		    ret += (s[i] & ~0x20) - 0x37;
	    } else {
		    ret += (s[i]-0x30);
	    }
	    if (i != (n-1)) ret *= 16;
    }

    return ret;
}

NandTitle::NandTitle()
{
    numTitles = 0;
    currentIndex = 0;
    currentType = 0;
    list = NULL;
    nameList = NULL;
}

NandTitle::~NandTitle()
{
    if ( list )
    {
        free( list );
        list = NULL;
    }

    if ( nameList )
    {
        free( nameList );
        nameList = NULL;
    }
}

s32 NandTitle::Get()
{
    s32 ret;

    if ( list )
    {
        free( list );
        list = NULL;
    }

    if ( nameList )
    {
        free( nameList );
        nameList = NULL;
    }

    ret = ES_GetNumTitles( &numTitles );
    if ( ret < 0 )
        return WII_EINTERNAL;

    list = ( u64* )memalign( 32, numTitles * sizeof( u64 ) );
    if ( !list )
    {
        return -1;
    }

    ret = ES_GetTitles( list, numTitles );
    if ( ret < 0 )
    {
        free( list );
        return WII_EINTERNAL;
    }

    nameList = ( char* )malloc( IMET_MAX_NAME_LEN * numTitles );
    if ( !nameList )
    {
        free( nameList );
        return -2;
    }
    memset( nameList, 0, IMET_MAX_NAME_LEN * numTitles );

    MagicPatches( 1 );
    int language = CONF_GetLanguage();
    ISFS_Initialize();

    wchar_t name[ IMET_MAX_NAME_LEN ];

    for ( u32 i = 0; i < numTitles; i++ )
    {
        bool r = GetName( list[ i ], language, name );
        if ( r )
        {
            wString *wsname = new wString( name );
            memcpy( nameList + ( IMET_MAX_NAME_LEN * i ), ( wsname->toUTF8() ).c_str(), strlen( ( wsname->toUTF8() ).c_str() ) );
            //gprintf("adding: %s\n", (wsname->toUTF8()).c_str() );
            delete wsname;
        }
    }

    ISFS_Deinitialize();
    MagicPatches( 0 );
    return 1;
}

tmd* NandTitle::GetTMD( u64 tid )
{
    //gprintf("GetTMD( %016llx ): ", tid );
    signed_blob *s_tmd = ( signed_blob * )tmd_buf;
    u32 tmd_size;

    if ( ES_GetStoredTMDSize( tid, &tmd_size ) < 0 )
    {
        //gprintf("!size\n");
        return NULL;
    }

    s32 ret = ES_GetStoredTMD( tid, s_tmd, tmd_size );
    if ( ret < 0 )
    {
        //gprintf("!tmd - %04x\n", ret );
        return NULL;
    }

    tmd *t = ( tmd* )SIGNATURE_PAYLOAD( s_tmd );
    //gprintf("ok\n");

    return t;
}

bool NandTitle::GetName( u64 tid, int language, wchar_t* name )
{
    if ( TITLE_UPPER( tid ) != 0x10001 && TITLE_UPPER( tid ) != 0x10002 && TITLE_UPPER( tid ) != 0x10004 )
        return false;
    //gprintf("GetName( %016llx ): ", tid );
    char app[ ISFS_MAXPATH ];
    IMET *imet = ( IMET* )memalign( 32, sizeof( IMET ) );

    tmd* titleTmd = GetTMD( tid );
    if ( !titleTmd )
    {
        //gprintf("no TMD\n");
        free( imet );
        return false;
    }

    u16 i;
    bool ok = false;
    for ( i = 0; i < titleTmd->num_contents; i++ )
    {
        if ( !titleTmd->contents[ i ].index )
        {
            ok = true;
            break;
        }
    }
    if ( !ok )
    {
        free( imet );
        return false;
    }

    snprintf( app, sizeof( app ), "/title/%08x/%08x/content/%08x.app", TITLE_UPPER( tid ), TITLE_LOWER( tid ), titleTmd->contents[ i ].cid );
    //gprintf("%s\n", app );

    if ( language > CONF_LANG_KOREAN )
        language = CONF_LANG_ENGLISH;

    s32 fd = ISFS_Open( app, ISFS_OPEN_READ );
    if ( fd < 0 )
    {
        //gprintf("fd: %d\n", fd );
        free( imet );
        return false;
    }

    if ( ISFS_Seek( fd, IMET_OFFSET, SEEK_SET ) != IMET_OFFSET )
    {
        ISFS_Close( fd );
        free( imet );
        return false;
    }

    if ( ISFS_Read( fd, imet, sizeof( IMET ) ) != sizeof( IMET ) )
    {
        ISFS_Close( fd );
        free( imet );
        return false;
    }

    ISFS_Close( fd );

    if ( imet->sig != IMET_SIGNATURE )
    {
        free( imet );
        return false;
    }

    if ( imet->name_japanese[ language * IMET_MAX_NAME_LEN ] == 0 )
    {
        // channel name is not available in system language
        if ( imet->name_english[ 0 ] != 0 )
        {
            language = CONF_LANG_ENGLISH;
        }
        else
        {
            // channel name is also not available on english, get ascii name
            for ( int i = 0; i < 4; i++ )
            {
                name[ i ] = ( TITLE_LOWER( tid ) >> ( 24 - i * 8 ) ) & 0xFF;
            }
            name[ 4 ] = 0;
            free( imet );
            return true;
        }
    }

    // retrieve channel name in system language or on english
    for ( int i = 0; i < IMET_MAX_NAME_LEN; i++ )
    {
        name[ i ] = imet->name_japanese[ i + ( language * IMET_MAX_NAME_LEN ) ];
    }

    free( imet );

    return true;
}

bool NandTitle::Exists( u64 tid )
{
    char app[ ISFS_MAXPATH ];
    tmd* titleTmd = GetTMD( tid );
    if ( !titleTmd )
        return false;

    u16 i;
    bool ok = false;
    for ( i = 0; i < titleTmd->num_contents; i++ )
    {
        if ( !titleTmd->contents[ i ].index )
        {
            ok = true;
            break;
        }
    }
    if ( !ok )
        return false;


    snprintf( app, sizeof( app ), "/title/%08x/%08x/content/%08x.app", TITLE_UPPER( tid ), TITLE_LOWER( tid ), titleTmd->contents[ i ].cid );
    s32 fd = ISFS_Open( app, ISFS_OPEN_READ );
    if ( fd >= 0 )
        ISFS_Close( fd );

    //gprintf(" fd: %d\n", fd );
    return fd >= 0 || fd == -102; //102 means it exists, but we dont have permission to open it

}

bool NandTitle::ExistsFromIndex( u32 i )
{
    if ( i > numTitles || i < 0 )
        return false;

    return Exists( list[ i ] );
}

u64 NandTitle::At( u32 i )
{
    if ( i > numTitles || i < 0 )
        return 0;

    return list[ i ];
}

int NandTitle::IndexOf( u64 tid )
{
    for ( u32 i = 0; i < numTitles; i++ )
    {
        if ( list[ i ] == tid )
            return i;
    }

    return WII_EINSTALL;
}

char* NandTitle::NameOf( u64 tid )
{
    for ( u32 i = 0; i < numTitles; i++ )
    {
        if ( list[ i ] == tid )
        {
            if ( !nameList[ IMET_MAX_NAME_LEN * i ] )
                return NULL;

            return nameList + ( IMET_MAX_NAME_LEN * i );
        }

    }
    return NULL;

}

char* NandTitle::NameFromIndex( u32 i )
{
    if ( i > numTitles || i < 0 )
        return NULL;

    if ( !nameList[ IMET_MAX_NAME_LEN * i ] )
        return NULL;

    return nameList + ( IMET_MAX_NAME_LEN * i );
}

u16 NandTitle::VersionOf( u64 tid )
{
    for ( u32 i = 0; i < numTitles; i++ )
    {
        if ( list[ i ] == tid )
        {
            tmd* Tmd = GetTMD( tid );
            if ( !Tmd )
                break;

            return Tmd->title_version;
        }
    }
    return 0;

}

u16 NandTitle::VersionFromIndex( u32 i )
{
    if ( i > numTitles || i < 0 )
        return 0;

    tmd* Tmd = GetTMD( list[ i ] );
    if ( !Tmd )
        return 0;

    return Tmd->title_version;
}

u32 NandTitle::CountType( u32 type )
{
    u32 ret = 0;
    for ( u32 i = 0; i < numTitles; i++ )
    {
        if ( TITLE_UPPER( list[ i ] ) == type )
        {
            ret++;
        }
    }
    return ret;
}

u32 NandTitle::SetType( u32 upper )
{
    currentType = upper;
    currentIndex = 0;

    return CountType( upper );
}

u64 NandTitle::Next()
{
    u64 ret = 0;
    //gprintf("Next( %08x, %u )\n", currentType, currentIndex );
    u32 i;
    for ( i = currentIndex; i < numTitles; i++ )
    {
        if ( currentType )
        {
            if ( currentType == TITLE_UPPER( list[ i ] ) )
            {
                ret = list[ i ];
                break;
            }
        }
        else
        {
            ret = list[ i ];
            break;
        }
    }
    currentIndex = i + 1;

    return ret;
}

void NandTitle::ResetCounter()
{
    currentIndex = 0;
}

void NandTitle::AsciiTID( u64 tid, char* out )
{
    //gprintf("AsciiTID( %016llx ): ");
    out[ 0 ] = ascii( TITLE_3( tid ) );
    out[ 1 ] = ascii( TITLE_2( tid ) );
    out[ 2 ] = ascii( TITLE_1( tid ) );
    out[ 3 ] = ascii( ( u8 )( tid ) );
    out[ 4 ] = 0;
    //gprintf("%s\n", out );
}

void NandTitle::AsciiFromIndex( u32 i, char* out )
{
    if ( i > numTitles || i < 0 )
    {
        out[ 0 ] = 0;
        return;
    }

    AsciiTID( list[ i ], out );
}

s32 NandTitle::GetTicketViews( u64 tid, tikview **outbuf, u32 *outlen )
{
    tikview *views = NULL;

    u32 nb_views;
    s32 ret;

    /* Get number of ticket views */
    ret = ES_GetNumTicketViews( tid, &nb_views );
    if ( ret < 0 )
        return ret;

    /* Allocate memory */
    views = ( tikview * )memalign( 32, sizeof( tikview ) * nb_views );
    if ( !views )
        return -1;

    /* Get ticket views */
    ret = ES_GetTicketViews( tid, views, nb_views );
    if ( ret < 0 )
        goto err;

    /* Set values */
    *outbuf = views;
    *outlen = nb_views;

    return 0;

err:
    /* Free memory */
    if ( views )
        free( views );

    return ret;
}

int NandTitle::FindU64( const char *s )
{
    u64 tid = atoi_hex( s );
    return IndexOf( tid );
}

int NandTitle::FindU32( const char *s )
{
    u64 tid = atoi_hex( s );
    for ( u32 i = 0; i < numTitles; i++ )
    {
	if ( TITLE_LOWER( list[ i ] ) == TITLE_LOWER( tid ) )
	    return i;
    }
    return WII_EINSTALL;
}
