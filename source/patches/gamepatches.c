#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include "dolpatcher.h"
#include "wip.h"

/** Anti 002 fix for IOS 249 rev > 12 thanks to WiiPower **/
bool Anti_002_fix( u8 * Address, int Size )
{
    u8 SearchPattern[12] =  { 0x2C, 0x00, 0x00, 0x00, 0x48, 0x00, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00 };
    u8 PatchData[12] =      { 0x2C, 0x00, 0x00, 0x00, 0x40, 0x82, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00 };
    return PatchDOL( Address, Size, ( const u8 * ) &SearchPattern, sizeof( SearchPattern ), ( const u8 * ) &PatchData, sizeof( PatchData ) );
}

/** Thanks to WiiPower **/
bool NSMBPatch( u8 * Address, int Size )
{
    if ( IOS_GetVersion() == 222 || IOS_GetVersion() == 223 ) return false; // Don't use this when using Hermes, it'll use the BCA fix instead...

    if ( memcmp( "SMNE", ( char * )0x80000000, 4 ) == 0 )
    {
        u8 SearchPattern[32] =  { 0x94, 0x21, 0xFF, 0xD0, 0x7C, 0x08, 0x02, 0xA6, 0x90, 0x01, 0x00, 0x34, 0x39, 0x61, 0x00, 0x30, 0x48, 0x12, 0xD7, 0x89, 0x7C, 0x7B, 0x1B, 0x78, 0x7C, 0x9C, 0x23, 0x78, 0x7C, 0xBD, 0x2B, 0x78 };
        u8 PatchData[32] =      { 0x4E, 0x80, 0x00, 0x20, 0x7C, 0x08, 0x02, 0xA6, 0x90, 0x01, 0x00, 0x34, 0x39, 0x61, 0x00, 0x30, 0x48, 0x12, 0xD7, 0x89, 0x7C, 0x7B, 0x1B, 0x78, 0x7C, 0x9C, 0x23, 0x78, 0x7C, 0xBD, 0x2B, 0x78 };
        return PatchDOL( Address, Size, ( const u8 * ) &SearchPattern, sizeof( SearchPattern ), ( const u8 * ) &PatchData, sizeof( PatchData ) );

    }
    else if ( memcmp( "SMN", ( char * )0x80000000, 3 ) == 0 )
    {
        u8 SearchPattern[4] =   { 0x7A, 0x6B, 0x6F, 0x6A };
        u8 PatchData[32] =      { 0x4E, 0x80, 0x00, 0x20, 0x7C, 0x08, 0x02, 0xA6, 0x90, 0x01, 0x00, 0x34, 0x39, 0x61, 0x00, 0x30, 0x48, 0x12, 0xD9, 0x39, 0x7C, 0x7B, 0x1B, 0x78, 0x7C, 0x9C, 0x23, 0x78, 0x7C, 0xBD, 0x2B, 0x78 };
        return PatchDOL( Address, Size, ( const u8 * ) &SearchPattern, sizeof( SearchPattern ), ( const u8 * ) &PatchData, sizeof( PatchData ) );
    }
    return false;
}

bool PoPPatch()
{
    if ( memcmp( "SPX", ( char * )0x80000000, 3 ) == 0 || memcmp( "RPW", ( char * )0x80000000, 3 ) == 0 )
    {
        WIP_Code * CodeList = malloc( 5 * sizeof( WIP_Code ) );
        CodeList[0].offset = 0x007AAC6A; CodeList[0].srcaddress = 0x7A6B6F6A; CodeList[0].dstaddress = 0x6F6A7A6B;
        CodeList[1].offset = 0x007AAC75; CodeList[1].srcaddress = 0x7C7A6939; CodeList[1].dstaddress = 0x69397C7A;
        CodeList[2].offset = 0x007AAC82; CodeList[2].srcaddress = 0x7376686B; CodeList[2].dstaddress = 0x686B7376;
        CodeList[3].offset = 0x007AAC92; CodeList[3].srcaddress = 0x80717570; CodeList[3].dstaddress = 0x75708071;
        CodeList[4].offset = 0x007AAC9D; CodeList[4].srcaddress = 0x82806F3F; CodeList[4].dstaddress = 0x6F3F8280;

        if ( set_wip_list( CodeList, 5 ) == false )
        {
            free( CodeList );
            CodeList = NULL;
        }
    }
    return false;
}



/** Insert the individual gamepatches above with the patterns and patch data **/
/** Following is only the VideoPatcher **/

#if 0 /** Isn't used right now **/

static GXRModeObj* vmodes[] =
{
    &TVNtsc240Ds,
    &TVNtsc240DsAa,
    &TVNtsc240Int,
    &TVNtsc240IntAa,
    &TVNtsc480IntDf,
    &TVNtsc480IntAa,
    &TVNtsc480Prog,
    &TVMpal480IntDf,
    &TVPal264Ds,
    &TVPal264DsAa,
    &TVPal264Int,
    &TVPal264IntAa,
    &TVPal524IntAa,
    &TVPal528Int,
    &TVPal528IntDf,
    &TVPal574IntDfScale,
    &TVEurgb60Hz240Ds,
    &TVEurgb60Hz240DsAa,
    &TVEurgb60Hz240Int,
    &TVEurgb60Hz240IntAa,
    &TVEurgb60Hz480Int,
    &TVEurgb60Hz480IntDf,
    &TVEurgb60Hz480IntAa,
    &TVEurgb60Hz480Prog,
    &TVEurgb60Hz480ProgSoft,
    &TVEurgb60Hz480ProgAa
};

#endif

static GXRModeObj* PAL2NTSC[] =
{
    &TVMpal480IntDf,        &TVNtsc480IntDf,
    &TVPal264Ds,            &TVNtsc240Ds,
    &TVPal264DsAa,          &TVNtsc240DsAa,
    &TVPal264Int,           &TVNtsc240Int,
    &TVPal264IntAa,         &TVNtsc240IntAa,
    &TVPal524IntAa,         &TVNtsc480IntAa,
    &TVPal528Int,           &TVNtsc480IntAa,
    &TVPal528IntDf,         &TVNtsc480IntDf,
    &TVPal574IntDfScale,    &TVNtsc480IntDf,
    &TVEurgb60Hz240Ds,      &TVNtsc240Ds,
    &TVEurgb60Hz240DsAa,    &TVNtsc240DsAa,
    &TVEurgb60Hz240Int,     &TVNtsc240Int,
    &TVEurgb60Hz240IntAa,   &TVNtsc240IntAa,
    &TVEurgb60Hz480Int,     &TVNtsc480IntAa,
    &TVEurgb60Hz480IntDf,   &TVNtsc480IntDf,
    &TVEurgb60Hz480IntAa,   &TVNtsc480IntAa,
    &TVEurgb60Hz480Prog,    &TVNtsc480Prog,
    &TVEurgb60Hz480ProgSoft, &TVNtsc480Prog,
    &TVEurgb60Hz480ProgAa,  &TVNtsc480Prog,
    0, 0
};

static GXRModeObj* NTSC2PAL[] =
{
    &TVNtsc240Ds,           &TVPal264Ds,
    &TVNtsc240DsAa,         &TVPal264DsAa,
    &TVNtsc240Int,          &TVPal264Int,
    &TVNtsc240IntAa,        &TVPal264IntAa,
    &TVNtsc480IntDf,        &TVPal528IntDf,
    &TVNtsc480IntAa,        &TVPal524IntAa,
    &TVNtsc480Prog,         &TVPal528IntDf,
    0, 0
};

static GXRModeObj* NTSC2PAL60[] =
{
    &TVNtsc240Ds,           &TVEurgb60Hz240Ds,
    &TVNtsc240DsAa,         &TVEurgb60Hz240DsAa,
    &TVNtsc240Int,          &TVEurgb60Hz240Int,
    &TVNtsc240IntAa,        &TVEurgb60Hz240IntAa,
    &TVNtsc480IntDf,        &TVEurgb60Hz480IntDf,
    &TVNtsc480IntAa,        &TVEurgb60Hz480IntAa,
    &TVNtsc480Prog,         &TVEurgb60Hz480Prog,
    0, 0
};

static bool compare_videomodes( GXRModeObj* mode1, GXRModeObj* mode2 )
{
    if ( mode1->viTVMode != mode2->viTVMode || mode1->fbWidth != mode2->fbWidth ||   mode1->efbHeight != mode2->efbHeight || mode1->xfbHeight != mode2->xfbHeight ||
            mode1->viXOrigin != mode2->viXOrigin || mode1->viYOrigin != mode2->viYOrigin || mode1->viWidth != mode2->viWidth || mode1->viHeight != mode2->viHeight ||
            mode1->xfbMode != mode2->xfbMode || mode1->field_rendering != mode2->field_rendering || mode1->aa != mode2->aa || mode1->sample_pattern[0][0] != mode2->sample_pattern[0][0] ||
            mode1->sample_pattern[1][0] != mode2->sample_pattern[1][0] ||   mode1->sample_pattern[2][0] != mode2->sample_pattern[2][0] ||
            mode1->sample_pattern[3][0] != mode2->sample_pattern[3][0] ||   mode1->sample_pattern[4][0] != mode2->sample_pattern[4][0] ||
            mode1->sample_pattern[5][0] != mode2->sample_pattern[5][0] ||   mode1->sample_pattern[6][0] != mode2->sample_pattern[6][0] ||
            mode1->sample_pattern[7][0] != mode2->sample_pattern[7][0] ||   mode1->sample_pattern[8][0] != mode2->sample_pattern[8][0] ||
            mode1->sample_pattern[9][0] != mode2->sample_pattern[9][0] ||   mode1->sample_pattern[10][0] != mode2->sample_pattern[10][0] ||
            mode1->sample_pattern[11][0] != mode2->sample_pattern[11][0] || mode1->sample_pattern[0][1] != mode2->sample_pattern[0][1] ||
            mode1->sample_pattern[1][1] != mode2->sample_pattern[1][1] ||   mode1->sample_pattern[2][1] != mode2->sample_pattern[2][1] ||
            mode1->sample_pattern[3][1] != mode2->sample_pattern[3][1] ||   mode1->sample_pattern[4][1] != mode2->sample_pattern[4][1] ||
            mode1->sample_pattern[5][1] != mode2->sample_pattern[5][1] ||   mode1->sample_pattern[6][1] != mode2->sample_pattern[6][1] ||
            mode1->sample_pattern[7][1] != mode2->sample_pattern[7][1] ||   mode1->sample_pattern[8][1] != mode2->sample_pattern[8][1] ||
            mode1->sample_pattern[9][1] != mode2->sample_pattern[9][1] ||   mode1->sample_pattern[10][1] != mode2->sample_pattern[10][1] ||
            mode1->sample_pattern[11][1] != mode2->sample_pattern[11][1] || mode1->vfilter[0] != mode2->vfilter[0] ||
            mode1->vfilter[1] != mode2->vfilter[1] ||   mode1->vfilter[2] != mode2->vfilter[2] || mode1->vfilter[3] != mode2->vfilter[3] || mode1->vfilter[4] != mode2->vfilter[4] ||
            mode1->vfilter[5] != mode2->vfilter[5] ||   mode1->vfilter[6] != mode2->vfilter[6] )
    {
        return false;
    }
    else
    {
        return true;
    }
}


static void patch_videomode( GXRModeObj* mode1, GXRModeObj* mode2 )
{
    mode1->viTVMode = mode2->viTVMode;
    mode1->fbWidth = mode2->fbWidth;
    mode1->efbHeight = mode2->efbHeight;
    mode1->xfbHeight = mode2->xfbHeight;
    mode1->viXOrigin = mode2->viXOrigin;
    mode1->viYOrigin = mode2->viYOrigin;
    mode1->viWidth = mode2->viWidth;
    mode1->viHeight = mode2->viHeight;
    mode1->xfbMode = mode2->xfbMode;
    mode1->field_rendering = mode2->field_rendering;
    mode1->aa = mode2->aa;
    mode1->sample_pattern[0][0] = mode2->sample_pattern[0][0];
    mode1->sample_pattern[1][0] = mode2->sample_pattern[1][0];
    mode1->sample_pattern[2][0] = mode2->sample_pattern[2][0];
    mode1->sample_pattern[3][0] = mode2->sample_pattern[3][0];
    mode1->sample_pattern[4][0] = mode2->sample_pattern[4][0];
    mode1->sample_pattern[5][0] = mode2->sample_pattern[5][0];
    mode1->sample_pattern[6][0] = mode2->sample_pattern[6][0];
    mode1->sample_pattern[7][0] = mode2->sample_pattern[7][0];
    mode1->sample_pattern[8][0] = mode2->sample_pattern[8][0];
    mode1->sample_pattern[9][0] = mode2->sample_pattern[9][0];
    mode1->sample_pattern[10][0] = mode2->sample_pattern[10][0];
    mode1->sample_pattern[11][0] = mode2->sample_pattern[11][0];
    mode1->sample_pattern[0][1] = mode2->sample_pattern[0][1];
    mode1->sample_pattern[1][1] = mode2->sample_pattern[1][1];
    mode1->sample_pattern[2][1] = mode2->sample_pattern[2][1];
    mode1->sample_pattern[3][1] = mode2->sample_pattern[3][1];
    mode1->sample_pattern[4][1] = mode2->sample_pattern[4][1];
    mode1->sample_pattern[5][1] = mode2->sample_pattern[5][1];
    mode1->sample_pattern[6][1] = mode2->sample_pattern[6][1];
    mode1->sample_pattern[7][1] = mode2->sample_pattern[7][1];
    mode1->sample_pattern[8][1] = mode2->sample_pattern[8][1];
    mode1->sample_pattern[9][1] = mode2->sample_pattern[9][1];
    mode1->sample_pattern[10][1] = mode2->sample_pattern[10][1];
    mode1->sample_pattern[11][1] = mode2->sample_pattern[11][1];
    mode1->vfilter[0] = mode2->vfilter[0];
    mode1->vfilter[1] = mode2->vfilter[1];
    mode1->vfilter[2] = mode2->vfilter[2];
    mode1->vfilter[3] = mode2->vfilter[3];
    mode1->vfilter[4] = mode2->vfilter[4];
    mode1->vfilter[5] = mode2->vfilter[5];
    mode1->vfilter[6] = mode2->vfilter[6];
}

static bool Search_and_patch_Video_Modes( u8 * Address, u32 Size, GXRModeObj* Table[] )
{
    u8 *Addr = ( u8 * )Address;
    bool found = 0;
    u32 i;

    while ( Size >= sizeof( GXRModeObj ) )
    {

        for ( i = 0; Table[i]; i += 2 )
        {

            if ( compare_videomodes( Table[i], ( GXRModeObj* )Addr ) )

            {
                found = 1;
                patch_videomode( ( GXRModeObj* )Addr, Table[i+1] );
                Addr += ( sizeof( GXRModeObj ) - 4 );
                Size -= ( sizeof( GXRModeObj ) - 4 );
                break;
            }
        }

        Addr += 4;
        Size -= 4;
    }


    return found;
}

void VideoModePatcher( u8 * dst, int len, u8 videoSelected )
{
    GXRModeObj** table = NULL;
    if ( videoSelected == 5 ) // patch

    {
        switch ( CONF_GetVideo() )
        {
            case CONF_VIDEO_PAL:
                if ( CONF_GetEuRGB60() > 0 )
                {
                    table = NTSC2PAL60;
                }
                else
                {
                    table = NTSC2PAL;
                }
                break;

            case CONF_VIDEO_MPAL:



                table = NTSC2PAL;
                break;


            default:
                table = PAL2NTSC;
                break;
        }
        Search_and_patch_Video_Modes( dst, len, table );
    }
}
