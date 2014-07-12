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
#include "memory/memory.h"
#include "memory/mem2.h"
#include "settings/SettingsEnums.h"

typedef struct _appDOL
{
	u8 *dst;
	int len;
} appDOL;

static appDOL *dolList = NULL;
static int dolCount = 0;
extern GXRModeObj *rmode;

void RegisterDOL(u8 *dst, int len)
{
	if(!dolList)
		dolList = (appDOL *) MEM2_alloc(sizeof(appDOL));

	appDOL *tmp = (appDOL *) MEM2_realloc(dolList, (dolCount+1)*sizeof(appDOL));
	if(!tmp)
	{
		MEM2_free(dolList);
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
		MEM2_free(dolList);
	dolList = NULL;
	dolCount = 0;
}

void gamepatches(u8 videoSelected, u8 videoPatchDol, u8 aspectForce, u8 languageChoice, u8 patchcountrystring,
				 u8 vipatch, u8 sneekVideoPatch, u8 hooktype, u64 returnTo, u8 privateServer)
{
	int i;

	/* If a wip file is loaded for this game this does nothing - Dimok */
	PoPPatch();
	NSMBPatch();

	for(i = 0; i < dolCount; ++i)
	{
		u8 *dst = dolList[i].dst;
		int len = dolList[i].len;

		VideoModePatcher(dst, len, videoSelected, videoPatchDol);

		dogamehooks(hooktype, dst, len);

		if (vipatch)
			vidolpatcher(dst, len);

		if(sneekVideoPatch)
			sneek_video_patch(dst, len);

		/*LANGUAGE PATCH - FISHEARS*/
		langpatcher(dst, len, languageChoice);

		/*Thanks to WiiPower*/
		if (patchcountrystring == 1)
			PatchCountryStrings(dst, len);

		do_wip_code(dst, len);

		Anti_002_fix(dst, len);

		if(returnTo)
			PatchReturnTo(dst, len, (u32) returnTo);

		if(aspectForce < 2)
			PatchAspectRatio(dst, len, aspectForce);

		if(privateServer)
			PrivateServerPatcher(dst, len, privateServer);
			
		DCFlushRange(dst, len);
		ICInvalidateRange(dst, len);
	}

	/* ERROR 002 fix (thanks to WiiPower for sharing this)*/
	*(u32 *)0x80003140 = *(u32 *)0x80003188;

	DCFlushRange((void*) 0x80000000, 0x3f00);

	free_wip();
	ClearDOLList();
}

/** Anti 002 fix for IOS 249 rev > 12 thanks to WiiPower **/
bool Anti_002_fix(u8 * Address, int Size)
{
	u8 SearchPattern[12] = { 0x2C, 0x00, 0x00, 0x00, 0x48, 0x00, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00 };
	u8 PatchData[12] = { 0x2C, 0x00, 0x00, 0x00, 0x40, 0x82, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00 };
	return PatchDOL(Address, Size, (const u8 *) SearchPattern, sizeof(SearchPattern), (const u8 *) PatchData, sizeof(PatchData));
}

/** Patch URLs for private Servers - Thanks to ToadKing/wiilauncher-nossl **/
void PrivateServerPatcher(void *addr, u32 len, u8 privateServer)
{

	// Patch protocol https -> http
	char *cur = (char *)addr;
	const char *end = cur + len - 8;
	do
	{
		if (memcmp(cur, "https://", 8) == 0 && cur[8] != 0)
		{
			int len = strlen(cur);
			memmove(cur + 4, cur + 5, len - 5);
			cur[len - 1] = 0;
			cur += len;
		}
	} while (++cur < end);
	
	// Patch nintendowifi.net -> private server domain
	if(privateServer == PRIVSERV_WIIMMFI )
		domainpatcher(addr, len, "wiimmfi.de");
	
	//else if(privateServer == PRIVSERV_CUSTOM)
		//domainpatcher(dst, len, Settings.CustomPrivateServer);
	
}

void domainpatcher(void *addr, u32 len, const char* domain)
{
	if(strlen("nintendowifi.net") < strlen(domain))
		return;

	char *cur = (char *)addr;
	const char *end = cur + len - 16;
	
	do
	{
		if (memcmp(cur, "nintendowifi.net", 16) == 0)
		{
			int len = strlen(cur);
			u8 i;
			memcpy(cur, domain, strlen(domain));
			memmove(cur + strlen(domain), cur + 16, len - 16);
			for(i = 16 - strlen(domain); i > 0 ; i--)
				cur[len - i ] = 0;
			cur += len;
		}
	} while (++cur < end);
}

bool NSMBPatch()
{
	WIP_Code * CodeList = NULL;

	if (memcmp("SMNE01", (char *) 0x80000000, 6) == 0)
	{
		CodeList = MEM2_alloc(3 * sizeof(WIP_Code));
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
		CodeList = MEM2_alloc(3 * sizeof(WIP_Code));
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
		CodeList = MEM2_alloc(3 * sizeof(WIP_Code));
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
		MEM2_free(CodeList);
		CodeList = NULL;
		return false;
	}


	return CodeList != NULL;
}

bool PoPPatch()
{
	if (memcmp("SPX", (char *) 0x80000000, 3) != 0 && memcmp("RPW", (char *) 0x80000000, 3) != 0)
		return false;

	WIP_Code * CodeList = MEM2_alloc(5 * sizeof(WIP_Code));
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
		MEM2_free(CodeList);
		CodeList = NULL;
		return false;
	}

	return true;
}

/** Insert the individual gamepatches above with the patterns and patch data **/
/** Following is only the VideoPatcher **/

// Some missing video modes
static GXRModeObj TVPal528Prog = {
	6,				// viDisplayMode
	640,			// fbWidth
	528,			// efbHeight
	528,			// xfbHeight
	40,				// viXOrigin  // (VI_MAX_WIDTH_PAL - 640)/2,
	23,				// viYOrigin  // game uses 0x17 instead of 0x18 so we don't use (VI_MAX_HEIGHT_PAL - 528)/2
	640,			// viWidth
	528,			// viHeight
	VI_XFBMODE_SF,	// xFBmode
	GX_FALSE,		// field_rendering
	GX_FALSE,		// aa

	// sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},	// pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},	// pix 1
		{6,6},{6,6},{6,6},	// pix 2
		{6,6},{6,6},{6,6}	// pix 3
	},

	// vertical filter[7], 1/64 units, 6 bits each
	{
		0,			// line n-1
		0,			// line n-1
		21,			// line n
		22,			// line n
		21,			// line n
		0,			// line n+1
		0			// line n+1
	}
};

static GXRModeObj TVPal528ProgSoft = {
	6,				// viDisplayMode
	640,			// fbWidth
	528,			// efbHeight
	528,			// xfbHeight
	40,				// viXOrigin
	23,				// viYOrigin
	640,			// viWidth
	528,			// viHeight
	VI_XFBMODE_SF,	// xFBmode
	GX_FALSE,		// field_rendering
	GX_FALSE,		// aa

	// sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},	// pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},	// pix 1
		{6,6},{6,6},{6,6},	// pix 2
		{6,6},{6,6},{6,6}	// pix 3
	},

	// vertical filter[7], 1/64 units, 6 bits each
	{
		8,			// line n-1
		8,			// line n-1
		10,			// line n
		12,			// line n
		10,			// line n
		8,			// line n+1
		8			// line n+1
	}

};

static GXRModeObj TVPal524ProgAa = {
	6,				// viDisplayMode
	640,			// fbWidth
	264,			// efbHeight
	524,			// xfbHeight
	40,				// viXOrigin
	23,				// viYOrigin
	640,			// viWidth
	524,			// viHeight
	VI_XFBMODE_SF,	// xFBmode
	GX_FALSE,		// field_rendering
	GX_TRUE,		// aa

	// sample points arranged in increasing Y order
	{
		{3,2},{9,6},{3,10},	// pix 0, 3 sample points, 1/12 units, 4 bits each
		{3,2},{9,6},{3,10},	// pix 1
		{9,2},{3,6},{9,10},	// pix 2
		{9,2},{3,6},{9,10}	// pix 3
	},

	// vertical filter[7], 1/64 units, 6 bits each
	{
		4,			// line n-1
		8,			// line n-1
		12,			// line n
		16,			// line n
		12,			// line n
		8,			// line n+1
		4			// line n+1
	}

};

static GXRModeObj* vmodes[] = {
	&TVNtsc240Ds,
	&TVNtsc240DsAa,
	&TVNtsc240Int,
	&TVNtsc240IntAa,
	&TVNtsc480Int,
	&TVNtsc480IntAa,
	&TVNtsc480IntDf,
	&TVNtsc480Prog,
	&TVNtsc480ProgSoft,
	&TVNtsc480ProgAa,
	&TVMpal480IntDf,
	&TVPal264Ds,
	&TVPal264DsAa,
	&TVPal264Int,
	&TVPal264IntAa,
	&TVPal524ProgAa,
	&TVPal524IntAa,
	&TVPal528Int,
	&TVPal528IntDf,
	&TVPal528Prog,
	&TVPal528ProgSoft,
	&TVPal576IntDfScale,
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

static const char * vmodes_name[] = {
	"TVNtsc240Ds",
	"TVNtsc240DsAa",
	"TVNtsc240Int",
	"TVNtsc240IntAa",
	"TVNtsc480Int",
	"TVNtsc480IntAa",
	"TVNtsc480IntDf",
	"TVNtsc480Prog",
	"TVNtsc480ProgSoft",
	"TVNtsc480ProgAa",
	"TVMpal480IntDf",
	"TVPal264Ds",
	"TVPal264DsAa",
	"TVPal264Int",
	"TVPal264IntAa",
	"TVPal524ProgAa",
	"TVPal524IntAa",
	"TVPal528Int",
	"TVPal528IntDf",
	"TVPal528Prog",
	"TVPal528ProgSoft",
	"TVPal576IntDfScale",
	"TVEurgb60Hz240Ds",
	"TVEurgb60Hz240DsAa",
	"TVEurgb60Hz240Int",
	"TVEurgb60Hz240IntAa",
	"TVEurgb60Hz480Int",
	"TVEurgb60Hz480IntDf",
	"TVEurgb60Hz480IntAa",
	"TVEurgb60Hz480Prog",
	"TVEurgb60Hz480ProgSoft",
	"TVEurgb60Hz480ProgAa"
};

static GXRModeObj* PAL2NTSC[] = { 
	&TVMpal480IntDf, &TVNtsc480IntDf,
	&TVPal264Ds, &TVNtsc240Ds,
	&TVPal264DsAa, &TVNtsc240DsAa,
	&TVPal264Int, &TVNtsc240Int,
	&TVPal264IntAa, &TVNtsc240IntAa,
	&TVPal524IntAa, &TVNtsc480IntAa,
	&TVPal528Int, &TVNtsc480Int,
	&TVPal528IntDf, &TVNtsc480IntDf,
	&TVPal528Prog, &TVNtsc480Prog, 
	&TVPal576IntDfScale, &TVNtsc480IntDf,
	&TVEurgb60Hz240Ds, &TVNtsc240Ds,
	&TVEurgb60Hz240DsAa, &TVNtsc240DsAa,
	&TVEurgb60Hz240Int, &TVNtsc240Int,
	&TVEurgb60Hz240IntAa, &TVNtsc240IntAa,
	&TVEurgb60Hz480Int, &TVNtsc480Int,
	&TVEurgb60Hz480IntDf,  &TVNtsc480IntDf,
	&TVEurgb60Hz480IntAa, &TVNtsc480IntAa,
	&TVEurgb60Hz480Prog, &TVNtsc480Prog,
	&TVEurgb60Hz480ProgSoft, &TVNtsc480Prog,
	&TVEurgb60Hz480ProgAa, &TVNtsc480Prog,
	0, 0
};

static GXRModeObj* NTSC2PAL[] = { 
	&TVNtsc240Ds, &TVPal264Ds,
	&TVNtsc240DsAa, &TVPal264DsAa,
	&TVNtsc240Int, &TVPal264Int,
	&TVNtsc240IntAa, &TVPal264IntAa,
	&TVNtsc480Int, &TVPal528Int,
	&TVNtsc480IntDf, &TVPal528IntDf,
	&TVNtsc480IntAa, &TVPal524IntAa,
	&TVNtsc480Prog, &TVPal528Prog,
	0, 0
};

static GXRModeObj* NTSC2PAL60[] = { 
	&TVNtsc240Ds, &TVEurgb60Hz240Ds,
	&TVNtsc240DsAa, &TVEurgb60Hz240DsAa,
	&TVNtsc240Int, &TVEurgb60Hz240Int,
	&TVNtsc240IntAa, &TVEurgb60Hz240IntAa,
	&TVNtsc480Int, &TVEurgb60Hz480Int,
	&TVNtsc480IntDf, &TVEurgb60Hz480IntDf,
	&TVNtsc480IntAa, &TVEurgb60Hz480IntAa,
	&TVNtsc480Prog, &TVEurgb60Hz480Prog,
	0, 0
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
	u32 i, j;

	while (Size >= sizeof(GXRModeObj))
	{
		for (i = 0; Table[i]; i += 2)
		{
			if (compare_videomodes(Table[i], (GXRModeObj*) Addr))
			{
				u8 current_vmode = 0;
				u8 target_vmode = 0;
				for(j = 0; j < sizeof(vmodes)/sizeof(vmodes[0]); j++)
				{
					if(compare_videomodes(Table[i], vmodes[j]))
					{
						current_vmode = j;
						break;
					}
				}
				for(j = 0; j < sizeof(vmodes)/sizeof(vmodes[0]); j++)
				{
					if(compare_videomodes(Table[i+1], vmodes[j]))
					{
						target_vmode = j;
						break;
					}
				}

				gprintf("Video mode found in dol: %s, replaced by: %s \n", vmodes_name[current_vmode], vmodes_name[target_vmode]);
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

static bool Search_and_patch_Video_To(void *Address, u32 Size, GXRModeObj* Table[], GXRModeObj* rmode, bool patchAll)
{
	u8 *Addr = (u8 *)Address;
	bool found = 0;
	u32 i;
	
	u8 target_vmode = 0;
	for(i = 0; i < sizeof(vmodes)/sizeof(vmodes[0]); i++)
	{
		if(compare_videomodes(Table[i], rmode))
		{
			target_vmode = i;
			break;
		}
	}

	while(Size >= sizeof(GXRModeObj))
	{
		// Video mode pattern found
		if( (((GXRModeObj*)Addr)->fbWidth == 0x0280 && ((GXRModeObj*)Addr)->viWidth == 0x02c4) || // TVEurgb60Hz480Prog
		    (((GXRModeObj*)Addr)->fbWidth == 0x0280 && ((GXRModeObj*)Addr)->viWidth == 0x0280) )  // All other video modes
		{
			// display found video mode patterns
			GXRModeObj* vidmode = (GXRModeObj*)Addr;
			gprintf("Video pattern found \t%08x %04x %04x %04x %04x %04x %04x %04x %08x %04x %04x ",
			vidmode->viTVMode, vidmode->fbWidth, vidmode->efbHeight, vidmode->xfbHeight, vidmode->viXOrigin, vidmode->viYOrigin, 
			vidmode->viWidth, vidmode->viHeight, vidmode->xfbMode, vidmode->field_rendering, vidmode->aa);
			gprintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x ",
			vidmode->sample_pattern[0][0],  vidmode->sample_pattern[1][0],  vidmode->sample_pattern[2][0],  vidmode->sample_pattern[3][0], vidmode->sample_pattern[4][0],
			vidmode->sample_pattern[5][0],  vidmode->sample_pattern[6][0],  vidmode->sample_pattern[7][0],  vidmode->sample_pattern[8][0], vidmode->sample_pattern[9][0],
			vidmode->sample_pattern[10][0], vidmode->sample_pattern[11][0], vidmode->sample_pattern[0][1],  vidmode->sample_pattern[1][1], vidmode->sample_pattern[2][1],
			vidmode->sample_pattern[3][1],  vidmode->sample_pattern[4][1],  vidmode->sample_pattern[5][1],  vidmode->sample_pattern[6][1], vidmode->sample_pattern[7][1],
			vidmode->sample_pattern[8][1],  vidmode->sample_pattern[9][1],  vidmode->sample_pattern[10][1], vidmode->sample_pattern[11][1]);
			gprintf("%02x%02x%02x%02x%02x%02x%02x \n",
			vidmode->vfilter[0], vidmode->vfilter[1] , vidmode->vfilter[2], vidmode->vfilter[3] , vidmode->vfilter[4],vidmode->vfilter[5], vidmode->vfilter[6]);

			found = 0;
			for(i = 0; i < sizeof(vmodes)/sizeof(vmodes[0]); i++)
			{
				if(compare_videomodes(Table[i], (GXRModeObj*)Addr))
				{
					found = 1;
					gprintf("Video mode found in dol: %s, replaced by: %s \n", vmodes_name[i], vmodes_name[target_vmode]);
					patch_videomode((GXRModeObj*)Addr, rmode);
					Addr += (sizeof(GXRModeObj)-4);
					Size -= (sizeof(GXRModeObj)-4);
					break;
				}
				
			}
			if(patchAll && !found)
			{
				gprintf("Video mode found in dol: Unknown, replaced by: %s \n", vmodes_name[target_vmode]);
				patch_videomode((GXRModeObj*)Addr, rmode);
				Addr += (sizeof(GXRModeObj)-4);
				Size -= (sizeof(GXRModeObj)-4);
			}
		}
		Addr += 4;
		Size -= 4;
	}

	return found;
}

void VideoModePatcher(u8 * dst, int len, u8 videoSelected, u8 VideoPatchDol)
{
	GXRModeObj** table = NULL;
	if (videoSelected == VIDEO_MODE_PATCH) // patch enum'd in cfg.h
	{
		switch (CONF_GetVideo())
		{
			case CONF_VIDEO_PAL:
				table = CONF_GetEuRGB60() > 0 ? NTSC2PAL60 : NTSC2PAL;
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
	else if(VideoPatchDol == VIDEO_PATCH_DOL_REGION ) //&& rmode != NULL)
	{
		switch(rmode->viTVMode >> 2)
		{
			case VI_PAL:
			case VI_MPAL:
				table = NTSC2PAL;
				break;
			case VI_EURGB60:
				table = NTSC2PAL60;
				break;
			default:
				table = PAL2NTSC;
		}
		Search_and_patch_Video_Modes(dst, len, table);
	}
	else if (VideoPatchDol == VIDEO_PATCH_DOL_ON && rmode != NULL)
	{
		Search_and_patch_Video_To(dst, len, vmodes, rmode, false);
	}
	else if (VideoPatchDol == VIDEO_PATCH_DOL_ALL && rmode != NULL)
	{
		Search_and_patch_Video_To(dst, len, vmodes, rmode, true);
	}
}

void sneek_video_patch(void *addr, u32 len)
{
	u8 *addr_start = addr;
	u8 *addr_end = addr+len;

	while(addr_start < addr_end)
	{
		if(*(vu32*)(addr_start) == 0x3C608000)
		{
			if( ((*(vu32*)(addr_start+4) & 0xFC1FFFFF ) == 0x800300CC) && ((*(vu32*)(addr_start+8) >> 24) == 0x54 ) )
			{
				*(vu32*)(addr_start+4) = 0x5400F0BE | ((*(vu32*)(addr_start+4) & 0x3E00000) >> 5);
			}
		}
		addr_start += 4;
	}
}

//giantpune's magic super patch to return to channels

static u32 ad[ 4 ] = { 0, 0, 0, 0 };//these variables are global on the off chance the different parts needed
static u8 found = 0;			//to find in the dol are found in different sections of the dol
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
			jump[ 1 ] = 0xA0; //3CA00001					//lis r5,1
			jump[ 5 ] = 0xA5; //60A50001					//ori r5,r5,1
			jump[ 9 ] = 0xC0; //3CC0AF1B					//lis r6,(u16)(tid >> 16)
			jump[ 13 ] = 0xC6;//60C6F516					//ori r6,r6,(u16)(tid)
		}

		void* addr = (u32*)ad[ 3 ];

		//write new stuff to in a unused part of the main.dol
		memcpy( addr, jump, sizeof( jump ) );

		//ES_GetTicketViews()
		u32 newval = ( ad[ 3 ] - ad[ 0 ] );
		newval &= 0x03FFFFFC;
		newval |= 0x48000001;
		addr = (u32*)ad[ 0 ];
		memcpy( addr, &newval, sizeof( u32 ) );					//bl ad[ 3 ]
		memcpy( addr + 4, &nop, sizeof( u32 ) );				//nop
		//gprintf("\t%08x -> %08x\n", addr, newval );

		//ES_GetTicketViews() again
		newval = ( ad[ 3 ] - ad[ 1 ] );
		newval &= 0x03FFFFFC;
		newval |= 0x48000001;
		addr = (u32*)ad[ 1 ];
		memcpy( addr, &newval, sizeof( u32 ) );					//bl ad[ 3 ]
		memcpy( addr + 4, &nop, sizeof( u32 ) );				//nop
		//gprintf("\t%08x -> %08x\n", addr, newval );

		//ES_LaunchTitle()
		newval = ( ad[ 3 ] - ad[ 2 ] );
		newval &= 0x03FFFFFC;
		newval |= 0x48000001;
		addr = (u32*)ad[ 2 ];
		memcpy( addr, &newval, sizeof( u32 ) );					//bl ad[ 3 ]
		memcpy( addr + 4, &nop, sizeof( u32 ) );				//nop
		//gprintf("\t%08x -> %08x\n", addr, newval );

		returnToPatched = 1;
	}

	if(returnToPatched)
		gprintf("Return to %08X patched with old method.\n", (u32) id);

	return returnToPatched;
}

int PatchNewReturnTo(int es_fd, u64 title)
{
	if(es_fd < 0 || title == 0)
		return -1;

	//! this is here for test purpose only and needs be moved later
	static u64 sm_title_id  ATTRIBUTE_ALIGN(32);
	ioctlv *vector = (ioctlv *) memalign(32, sizeof(ioctlv));
	if(!vector)
		return -1;

	sm_title_id = title;
	vector[0].data = &sm_title_id;
	vector[0].len = sizeof(sm_title_id);

	int result = -1;

	if(es_fd >= 0)
		result = IOS_Ioctlv(es_fd, 0xA1, 1, 0, vector);

	if(result >= 0)
		gprintf("Return to %08X patched with d2x method.\n", (u32) title);

	free(vector);

	return result;
}

int BlockIOSReload(int es_fd, u8 gameIOS)
{
	if(es_fd < 0)
		return -1;

	static int mode ATTRIBUTE_ALIGN(32);
	static int ios ATTRIBUTE_ALIGN(32);
	ioctlv *vector = (ioctlv *) memalign(32, sizeof(ioctlv) * 2);
	if(!vector)
		return -1;

	int inlen = 2;
	mode = 2;
	ios = gameIOS; // ios to be reloaded in place of the requested one
	vector[0].data = &mode;
	vector[0].len = 4;
	vector[1].data = &ios;
	vector[1].len = 4;

	int result = -1;

	if(es_fd >= 0)
		result = IOS_Ioctlv(es_fd, 0xA0, inlen, 0, vector);

	if(result >= 0)
		gprintf("Block IOS Reload patched with d2x method to IOS%i; result: %i\n", gameIOS, result);

	free(vector);

	return result;
}


void PatchAspectRatio(void *addr, u32 len, u8 aspect)
{
	if(aspect > 1)
		return;

	static const u32 aspect_searchpattern1[5] = {
		0x9421FFF0, 0x7C0802A6, 0x38800001, 0x90010014, 0x38610008
	};

	static const u32 aspect_searchpattern2[15] = {
		0x2C030000, 0x40820010, 0x38000000, 0x98010008, 0x48000018,
		0x88010008, 0x28000001, 0x4182000C, 0x38000000, 0x98010008,
		0x80010014, 0x88610008, 0x7C0803A6, 0x38210010, 0x4E800020
	};

	u8 *addr_start = (u8 *) addr;
	u8 *addr_end = addr_start + len - sizeof(aspect_searchpattern1) - 4 - sizeof(aspect_searchpattern2);

	while(addr_start < addr_end)
	{
		if(   (memcmp(addr_start, aspect_searchpattern1, sizeof(aspect_searchpattern1)) == 0)
		   && (memcmp(addr_start + 4 + sizeof(aspect_searchpattern1), aspect_searchpattern2, sizeof(aspect_searchpattern2)) == 0))
		{
			*((u32 *)(addr_start+0x44)) = (0x38600000 | aspect);
			gprintf("Aspect ratio patched to: %s\n", aspect ? "16:9" : "4:3");
			break;
		}
		addr_start += 4;
	}
}
