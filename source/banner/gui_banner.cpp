/****************************************************************************
 * USB Loader GX Team
 * gui_banner.cpp
 *
 * Shows TPL Banner images
 ***************************************************************************/
#include "gui_banner.h"

GuiBanner::GuiBanner( const char *tplfilepath )
{
    memory = NULL;
    tplfilesize = 0;
    width = 0;
    height = 0;

    FILE *tplfp = fopen( tplfilepath, "rb" );

    if ( tplfp != NULL )
    {

        unsigned short heighttemp = 0;
        unsigned short widthtemp = 0;

        fseek( tplfp , 0x14, SEEK_SET );
        fread( ( void* )&heighttemp, 1, 2, tplfp );
        fread( ( void* )&widthtemp, 1, 2, tplfp );
        fseek ( tplfp , 0 , SEEK_END );
        tplfilesize = ftell ( tplfp );
        rewind ( tplfp );
        memory = memalign( 32, tplfilesize );
        if ( !memory )
        {
            fclose( tplfp );
            return;
        }
        fread( memory, 1, tplfilesize, tplfp );
        fclose( tplfp );

        TPLFile tplfile;
        int ret;

        ret = TPL_OpenTPLFromMemory( &tplfile, memory, tplfilesize );
        if ( ret < 0 )
        {
            free( memory );
            memory = NULL;
            return;
        }
        ret = TPL_GetTexture( &tplfile, 0, &texObj );
        if ( ret < 0 )
        {
            free( memory );
            memory = NULL;
            return;
        }
        TPL_CloseTPLFile( &tplfile );

        width = widthtemp;
        height = heighttemp;
        widescreen = 0;
        filecheck = true;

    }
    else
    {
        filecheck = false;
        fclose( tplfp );
    }
}

GuiBanner::GuiBanner( void *mem, u32 len, int w, int h )
{
    if ( !mem || !len )
        return;
    memory = mem;
    tplfilesize = len;
    width = w;
    height = h;

    TPLFile tplfile;

    int ret;

    ret = TPL_OpenTPLFromMemory( &tplfile, memory, tplfilesize );
    if ( ret < 0 )
    {
        free( memory );
        memory = NULL;
        return;
    }
    ret = TPL_GetTexture( &tplfile, 0, &texObj );
    if ( ret < 0 )
    {
        free( memory );
        memory = NULL;
        return;
    }
    TPL_CloseTPLFile( &tplfile );

    filecheck = true;
}

GuiBanner::~GuiBanner()
{
    if ( memory != NULL )
    {
        free( memory );
        memory = NULL;
    }
}

void GuiBanner::Draw()
{
    LOCK( this );
    if ( !filecheck || !this->IsVisible() )
        return;

    float currScale = this->GetScale();

    Menu_DrawTPLImg( this->GetLeft(), this->GetTop(), 0, width, height, &texObj, imageangle, widescreen ? currScale*0.80 : currScale, currScale, this->GetAlpha(), xx1, yy1, xx2, yy2, xx3, yy3, xx4, yy4 );

    this->UpdateEffects();
}
