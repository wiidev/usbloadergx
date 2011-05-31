#include <ogc/machine/processor.h>
#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include "usbloader/disc.h"
#include "dolpatcher.h"
#include "wip.h"
#include "gecko.h"
#include "patchcode.h"
#include "gamepatches.h"
#include "settings/SettingsEnums.h"

typedef struct _appDOL
{
    u8 *dst;
    int len;
} appDOL;

static appDOL *dolList = NULL;
static int dolCount = 0;

static char es_fs[] ATTRIBUTE_ALIGN(32) = "/dev/es";
static s32 es_fd = -1;

void RegisterDOL(u8 *dst, int len)
{
    if(!dolList)
        dolList = (appDOL *) malloc(sizeof(appDOL));

    appDOL *tmp = (appDOL *) realloc(dolList, (dolCount+1)*sizeof(appDOL));
    if(!tmp)
    {
        free(dolList);
        dolCount = 0;
        return;
    }

    dolList = tmp;
    dolList[dolCount].dst = dst;
    dolList[dolCount].len = len;
    dolCount++;
}

void ClearDOLList()
{
    if(dolList)
        free(dolList);
    dolList = NULL;
    dolCount = 0;
}

void gamepatches(u8 videoSelected, u8 languageChoice, u8 patchcountrystring, u8 vipatch, u8 cheat, u8 fix002, u8 blockiosreloadselect, u8 gameIOS, u64 returnTo)
{
    es_fd = IOS_Open(es_fs, 0);
    int i;

    int returnToPatched = PatchNewReturnTo(returnTo);

    for(i = 0; i < dolCount; ++i)
    {
        u8 *dst = dolList[i].dst;
        int len = dolList[i].len;

        VideoModePatcher(dst, len, videoSelected);

        if (cheat)
            dogamehooks(dst, len);

        if (vipatch)
            vidolpatcher(dst, len);

        /*LANGUAGE PATCH - FISHEARS*/
        langpatcher(dst, len, languageChoice);

        /*Thanks to WiiPower*/
        if (patchcountrystring == 1)
            PatchCountryStrings(dst, len);

        do_wip_code(dst, len);

        if (fix002 == 2)
            Anti_002_fix(dst, len);

        if(returnToPatched < 0)
            PatchReturnTo(dst, len, (u32) returnTo);

        DCFlushRange(dst, len);
    }

    if(blockiosreloadselect)
        BlockIOSReload(2, gameIOS);

    if(es_fd >= 0)
        IOS_Close(es_fd);

    /* ERROR 002 fix (thanks to WiiPower for sharing this)*/
    if (fix002 != 0)
        *(u32 *) 0x80003188 = *(u32 *) 0x80003140;

    DCFlushRange((void*) 0x80000000, 0x3f00);
}

/** Anti 002 fix for IOS 249 rev > 12 thanks to WiiPower **/
bool Anti_002_fix(u8 * Address, int Size)
{
    u8 SearchPattern[12] = { 0x2C, 0x00, 0x00, 0x00, 0x48, 0x00, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00 };
    u8 PatchData[12] = { 0x2C, 0x00, 0x00, 0x00, 0x40, 0x82, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00 };
    return PatchDOL(Address, Size, (const u8 *) SearchPattern, sizeof(SearchPattern), (const u8 *) PatchData, sizeof(PatchData));
}

bool NSMBPatch()
{
    WIP_Code * CodeList = NULL;

    if (memcmp("SMNE01", (char *) 0x80000000, 6) == 0)
    {
        CodeList = malloc(3 * sizeof(WIP_Code));
        if(!CodeList)
            return false;

        CodeList[0].offset = 0x001AB610;
        CodeList[0].srcaddress = 0x9421FFD0;
        CodeList[0].dstaddress = 0x4E800020;
        CodeList[1].offset = 0x001CED53;
        CodeList[1].srcaddress = 0xDA000000;
        CodeList[1].dstaddress = 0x71000000;
        CodeList[2].offset = 0x001CED6B;
        CodeList[2].srcaddress = 0xDA000000;
        CodeList[2].dstaddress = 0x71000000;

    }
    else if (memcmp("SMNP01", (char *) 0x80000000, 6) == 0)
    {
        CodeList = malloc(3 * sizeof(WIP_Code));
        if(!CodeList)
            return false;

        CodeList[0].offset = 0x001AB750;
        CodeList[0].srcaddress = 0x9421FFD0;
        CodeList[0].dstaddress = 0x4E800020;
        CodeList[1].offset = 0x001CEE90;
        CodeList[1].srcaddress = 0x38A000DA;
        CodeList[1].dstaddress = 0x38A00071;
        CodeList[2].offset = 0x001CEEA8;
        CodeList[2].srcaddress = 0x388000DA;
        CodeList[2].dstaddress = 0x38800071;
    }
    else if (memcmp("SMNJ01", (char *) 0x80000000, 6) == 0)
    {
        CodeList = malloc(3 * sizeof(WIP_Code));
        if(!CodeList)
            return false;

        CodeList[0].offset = 0x001AB420;
        CodeList[0].srcaddress = 0x9421FFD0;
        CodeList[0].dstaddress = 0x4E800020;
        CodeList[1].offset = 0x001CEB63;
        CodeList[1].srcaddress = 0xDA000000;
        CodeList[1].dstaddress = 0x71000000;
        CodeList[2].offset = 0x001CEB7B;
        CodeList[2].srcaddress = 0xDA000000;
        CodeList[2].dstaddress = 0x71000000;
    }

    if (CodeList && set_wip_list(CodeList, 3) == false)
    {
        free(CodeList);
        CodeList = NULL;
        return false;
    }


    return CodeList != NULL;
}

bool PoPPatch()
{
    if (memcmp("SPX", (char *) 0x80000000, 3) != 0 && memcmp("RPW", (char *) 0x80000000, 3) != 0)
        return false;

    WIP_Code * CodeList = malloc(5 * sizeof(WIP_Code));
    CodeList[0].offset = 0x007AAC6A;
    CodeList[0].srcaddress = 0x7A6B6F6A;
    CodeList[0].dstaddress = 0x6F6A7A6B;
    CodeList[1].offset = 0x007AAC75;
    CodeList[1].srcaddress = 0x7C7A6939;
    CodeList[1].dstaddress = 0x69397C7A;
    CodeList[2].offset = 0x007AAC82;
    CodeList[2].srcaddress = 0x7376686B;
    CodeList[2].dstaddress = 0x686B7376;
    CodeList[3].offset = 0x007AAC92;
    CodeList[3].srcaddress = 0x80717570;
    CodeList[3].dstaddress = 0x75708071;
    CodeList[4].offset = 0x007AAC9D;
    CodeList[4].srcaddress = 0x82806F3F;
    CodeList[4].dstaddress = 0x6F3F8280;

    if (set_wip_list(CodeList, 5) == false)
    {
        free(CodeList);
        CodeList = NULL;
        return false;
    }

    return true;
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

static GXRModeObj* PAL2NTSC[] = { &TVMpal480IntDf, &TVNtsc480IntDf, &TVPal264Ds, &TVNtsc240Ds, &TVPal264DsAa,
                                  &TVNtsc240DsAa, &TVPal264Int, &TVNtsc240Int, &TVPal264IntAa, &TVNtsc240IntAa, &TVPal524IntAa, &TVNtsc480IntAa,
                                  &TVPal528Int, &TVNtsc480IntAa, &TVPal528IntDf, &TVNtsc480IntDf, &TVPal574IntDfScale, &TVNtsc480IntDf,
                                  &TVEurgb60Hz240Ds, &TVNtsc240Ds, &TVEurgb60Hz240DsAa, &TVNtsc240DsAa, &TVEurgb60Hz240Int, &TVNtsc240Int,
                                  &TVEurgb60Hz240IntAa, &TVNtsc240IntAa, &TVEurgb60Hz480Int, &TVNtsc480IntAa, &TVEurgb60Hz480IntDf,
                                  &TVNtsc480IntDf, &TVEurgb60Hz480IntAa, &TVNtsc480IntAa, &TVEurgb60Hz480Prog, &TVNtsc480Prog,
                                  &TVEurgb60Hz480ProgSoft, &TVNtsc480Prog, &TVEurgb60Hz480ProgAa, &TVNtsc480Prog, 0, 0
                                };

static GXRModeObj* NTSC2PAL[] = { &TVNtsc240Ds, &TVPal264Ds, &TVNtsc240DsAa, &TVPal264DsAa, &TVNtsc240Int,
                                  &TVPal264Int, &TVNtsc240IntAa, &TVPal264IntAa, &TVNtsc480IntDf, &TVPal528IntDf, &TVNtsc480IntAa,
                                  &TVPal524IntAa, &TVNtsc480Prog, &TVPal528IntDf, 0, 0
                                };

static GXRModeObj* NTSC2PAL60[] = { &TVNtsc240Ds, &TVEurgb60Hz240Ds, &TVNtsc240DsAa, &TVEurgb60Hz240DsAa,
                                    &TVNtsc240Int, &TVEurgb60Hz240Int, &TVNtsc240IntAa, &TVEurgb60Hz240IntAa, &TVNtsc480IntDf,
                                    &TVEurgb60Hz480IntDf, &TVNtsc480IntAa, &TVEurgb60Hz480IntAa, &TVNtsc480Prog, &TVEurgb60Hz480Prog, 0, 0
                                  };

static bool compare_videomodes(GXRModeObj* mode1, GXRModeObj* mode2)
{
    if (mode1->viTVMode != mode2->viTVMode || mode1->fbWidth != mode2->fbWidth || mode1->efbHeight != mode2->efbHeight
            || mode1->xfbHeight != mode2->xfbHeight || mode1->viXOrigin != mode2->viXOrigin || mode1->viYOrigin
            != mode2->viYOrigin || mode1->viWidth != mode2->viWidth || mode1->viHeight != mode2->viHeight
            || mode1->xfbMode != mode2->xfbMode || mode1->field_rendering != mode2->field_rendering || mode1->aa
            != mode2->aa || mode1->sample_pattern[0][0] != mode2->sample_pattern[0][0] || mode1->sample_pattern[1][0]
            != mode2->sample_pattern[1][0] || mode1->sample_pattern[2][0] != mode2->sample_pattern[2][0]
            || mode1->sample_pattern[3][0] != mode2->sample_pattern[3][0] || mode1->sample_pattern[4][0]
            != mode2->sample_pattern[4][0] || mode1->sample_pattern[5][0] != mode2->sample_pattern[5][0]
            || mode1->sample_pattern[6][0] != mode2->sample_pattern[6][0] || mode1->sample_pattern[7][0]
            != mode2->sample_pattern[7][0] || mode1->sample_pattern[8][0] != mode2->sample_pattern[8][0]
            || mode1->sample_pattern[9][0] != mode2->sample_pattern[9][0] || mode1->sample_pattern[10][0]
            != mode2->sample_pattern[10][0] || mode1->sample_pattern[11][0] != mode2->sample_pattern[11][0]
            || mode1->sample_pattern[0][1] != mode2->sample_pattern[0][1] || mode1->sample_pattern[1][1]
            != mode2->sample_pattern[1][1] || mode1->sample_pattern[2][1] != mode2->sample_pattern[2][1]
            || mode1->sample_pattern[3][1] != mode2->sample_pattern[3][1] || mode1->sample_pattern[4][1]
            != mode2->sample_pattern[4][1] || mode1->sample_pattern[5][1] != mode2->sample_pattern[5][1]
            || mode1->sample_pattern[6][1] != mode2->sample_pattern[6][1] || mode1->sample_pattern[7][1]
            != mode2->sample_pattern[7][1] || mode1->sample_pattern[8][1] != mode2->sample_pattern[8][1]
            || mode1->sample_pattern[9][1] != mode2->sample_pattern[9][1] || mode1->sample_pattern[10][1]
            != mode2->sample_pattern[10][1] || mode1->sample_pattern[11][1] != mode2->sample_pattern[11][1]
            || mode1->vfilter[0] != mode2->vfilter[0] || mode1->vfilter[1] != mode2->vfilter[1] || mode1->vfilter[2]
            != mode2->vfilter[2] || mode1->vfilter[3] != mode2->vfilter[3] || mode1->vfilter[4] != mode2->vfilter[4]
            || mode1->vfilter[5] != mode2->vfilter[5] || mode1->vfilter[6] != mode2->vfilter[6])
    {
        return false;
    }
    else
    {
        return true;
    }
}

static void patch_videomode(GXRModeObj* mode1, GXRModeObj* mode2)
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

static bool Search_and_patch_Video_Modes(u8 * Address, u32 Size, GXRModeObj* Table[])
{
    u8 *Addr = (u8 *) Address;
    bool found = 0;
    u32 i;

    while (Size >= sizeof(GXRModeObj))
    {
        for (i = 0; Table[i]; i += 2)
        {
            if (compare_videomodes(Table[i], (GXRModeObj*) Addr))
            {
                found = 1;
                patch_videomode((GXRModeObj*) Addr, Table[i + 1]);
                Addr += (sizeof(GXRModeObj) - 4);
                Size -= (sizeof(GXRModeObj) - 4);
                break;
            }
        }

        Addr += 4;
        Size -= 4;
    }

    return found;
}

void VideoModePatcher(u8 * dst, int len, u8 videoSelected)
{
    GXRModeObj** table = NULL;
    if (videoSelected == VIDEO_MODE_PATCH) // patch enum'd in cfg.h

    {
        switch (CONF_GetVideo())
        {
        case CONF_VIDEO_PAL:
            if (CONF_GetEuRGB60() > 0)
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
        Search_and_patch_Video_Modes(dst, len, table);
    }
}

//giantpune's magic super patch to return to channels

static u32 ad[ 4 ] = { 0, 0, 0, 0 };//these variables are global on the off chance the different parts needed
static u8 found = 0;		    //to find in the dol are found in different sections of the dol
static u8 returnToPatched = 0;

bool PatchReturnTo( void *Address, int Size, u32 id )
{
    if( !id || returnToPatched )
        return 0;
    //gprintf("PatchReturnTo( %p, %08x, %08x )\n", Address, Size, id );

    //new __OSLoadMenu() (SM2.0 and higher)
    u8 SearchPattern[ 12 ] = 	{ 0x38, 0x80, 0x00, 0x02, 0x38, 0x60, 0x00, 0x01, 0x38, 0xa0, 0x00, 0x00 }; //li r4,2
    //li r3,1
    //li r5,0
    //old _OSLoadMenu() (used in launch games)
    u8 SearchPatternB[ 12 ] = 	{ 0x38, 0xC0, 0x00, 0x02, 0x38, 0xA0, 0x00, 0x01, 0x38, 0xE0, 0x00, 0x00 }; //li r6,2
    //li r5,1
    //li r7,0
    //identifier for the safe place
    u8 SearchPattern2[ 12 ] = 	{ 0x4D, 0x65, 0x74, 0x72, 0x6F, 0x77, 0x65, 0x72, 0x6B, 0x73, 0x20, 0x54 }; //"Metrowerks T"

    u8 oldSDK = 0;
    found = 0;

    void *Addr = Address;
    void *Addr_end = Address+Size;

    while (Addr <= Addr_end - 12 )
    {
        //find a safe place or the patch to hang out
        if ( ! ad[ 3 ] && memcmp( Addr, SearchPattern2, 12 ) == 0 )
        {
            ad[ 3 ] = (u32)Addr + 0x30;
        }
        //find __OSLaunchMenu() and remember some addresses in it
        else if ( memcmp( Addr, SearchPattern, 12 )==0 )
        {
            ad[ found++ ] = (u32)Addr;
        }
        else if ( ad[ 0 ] && memcmp( Addr, SearchPattern, 8 )==0 ) //after the first match is found, only search the first 8 bytes for the other 2
        {
            if( !ad[ 1 ] ) ad[ found++ ] = (u32)Addr;
            else if( !ad[ 2 ] ) ad[ found++ ] = (u32)Addr;
            if( found >= 3 )break;
        }
        Addr += 4;
    }
    //check for the older-ass version of the SDK
    if( found < 3 && ad[ 3 ] )
    {
        Addr = Address;
        ad[ 0 ] = 0;
        ad[ 1 ] = 0;
        ad[ 2 ] = 0;
        found = 0;
        oldSDK = 1;

        while (Addr <= Addr_end - 12 )
        {
            //find __OSLaunchMenu() and remember some addresses in it
            if ( memcmp( Addr, SearchPatternB, 12 )==0 )
            {
                ad[ found++ ] = (u32)Addr;
            }
            else if ( ad[ 0 ] && memcmp( Addr, SearchPatternB, 8 ) == 0 ) //after the first match is found, only search the first 8 bytes for the other 2
            {
                if( !ad[ 1 ] ) ad[ found++ ] = (u32)Addr;
                else if( !ad[ 2 ] ) ad[ found++ ] = (u32)Addr;
                if( found >= 3 )break;
            }
            Addr += 4;
        }
    }

    //if the function is found
    if( found == 3 && ad[ 3 ] )
    {
        //gprintf("patch __OSLaunchMenu( 0x00010001, 0x%08x )\n", id);
        u32 nop = 0x60000000;

        //the magic that writes the TID to the registers
        u8 jump[ 20 ] = { 0x3C, 0x60, 0x00, 0x01,				//lis r3,1
                          0x60, 0x63, 0x00, 0x01,				//ori r3,r3,1
                          0x3C, 0x80, (u8)( id >> 24 ), (u8)( id >> 16 ),	//lis r4,(u16)(tid >> 16)
                          0x60, 0x84, (u8)( id >> 8 ), (u8)id,			//ori r4,r4,(u16)(tid)
                          0x4E, 0x80, 0x00, 0x20
                        };				//blr

        if( oldSDK )
        {
            jump[ 1 ] = 0xA0; //3CA00001				    //lis r5,1
            jump[ 5 ] = 0xA5; //60A50001				    //ori r5,r5,1
            jump[ 9 ] = 0xC0; //3CC0AF1B				    //lis r6,(u16)(tid >> 16)
            jump[ 13 ] = 0xC6;//60C6F516				    //ori r6,r6,(u16)(tid)
        }

        void* addr = (u32*)ad[ 3 ];

        //write new stuff to in a unused part of the main.dol
        memcpy( addr, jump, sizeof( jump ) );

        //ES_GetTicketViews()
        u32 newval = ( ad[ 3 ] - ad[ 0 ] );
        newval &= 0x03FFFFFC;
        newval |= 0x48000001;
        addr = (u32*)ad[ 0 ];
        memcpy( addr, &newval, sizeof( u32 ) );				    //bl ad[ 3 ]
        memcpy( addr + 4, &nop, sizeof( u32 ) );			    //nop
        //gprintf("\t%08x -> %08x\n", addr, newval );

        //ES_GetTicketViews() again
        newval = ( ad[ 3 ] - ad[ 1 ] );
        newval &= 0x03FFFFFC;
        newval |= 0x48000001;
        addr = (u32*)ad[ 1 ];
        memcpy( addr, &newval, sizeof( u32 ) );				    //bl ad[ 3 ]
        memcpy( addr + 4, &nop, sizeof( u32 ) );			    //nop
        //gprintf("\t%08x -> %08x\n", addr, newval );

        //ES_LaunchTitle()
        newval = ( ad[ 3 ] - ad[ 2 ] );
        newval &= 0x03FFFFFC;
        newval |= 0x48000001;
        addr = (u32*)ad[ 2 ];
        memcpy( addr, &newval, sizeof( u32 ) );				    //bl ad[ 3 ]
        memcpy( addr + 4, &nop, sizeof( u32 ) );			    //nop
        //gprintf("\t%08x -> %08x\n", addr, newval );

        returnToPatched = 1;
    }

    return returnToPatched;
}

int PatchNewReturnTo(u64 title)
{
    if(title == 0) return -1;

    //! this is here for test purpose only and needs be moved later
    static u64 sm_title_id  ATTRIBUTE_ALIGN(32);
    STACK_ALIGN(ioctlv, vector, 1, 32);

    sm_title_id = title;
    vector[0].data = &sm_title_id;
    vector[0].len = sizeof(sm_title_id);

    int result = -1;

    if(es_fd >= 0)
        result = IOS_Ioctlv(es_fd, 0xA1, 1, 0, vector);

    return result;
}

bool BlockIOSReload(u8 blockiosreloadselect, u8 gameIOS)
{
    if(blockiosreloadselect == 0)
        return false;

    static int mode ATTRIBUTE_ALIGN(32);
    static int ios ATTRIBUTE_ALIGN(32);
    STACK_ALIGN(ioctlv, vector, 2, 32);

    mode = blockiosreloadselect;
    vector[0].data = &mode;
    vector[0].len = 4;

    int inlen = 1;
    if (mode == 2) {
        inlen = 2;
        ios = gameIOS; // ios to be reloaded in place of the requested one
        vector[1].data = &ios;
        vector[1].len = 4;
    }

    int result = -1;

    if(es_fd >= 0)
        result = IOS_Ioctlv(es_fd, 0xA0, inlen, 0, vector);

    return (result >= 0);
}
