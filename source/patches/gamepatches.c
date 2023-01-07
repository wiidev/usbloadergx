#include <ogc/machine/processor.h>
#include <gccore.h>
#include <malloc.h>
#include <string.h>

#include "usbloader/disc.h"
#include "wip.h"
#include "gecko.h"
#include "patchcode.h"
#include "gamepatches.h"
#include "memory/memory.h"
#include "memory/mem2.h"
#include "settings/SettingsEnums.h"
#include "svnrev.h"
#include "kirbypatch.h"

/* GCC 11 false positives */
#if __GNUC__ > 10
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#pragma GCC diagnostic ignored "-Wstringop-overread"
#endif

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
    if (!dolList)
        dolList = (appDOL *)MEM2_alloc(sizeof(appDOL));

    appDOL *tmp = (appDOL *)MEM2_realloc(dolList, (dolCount + 1) * sizeof(appDOL));
    if (!tmp)
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
    if (dolList)
        MEM2_free(dolList);
    dolList = NULL;
    dolCount = 0;
}

void gamepatches(u8 videoSelected, u8 videoPatchDol, u8 aspectForce, u8 languageChoice, u8 patchcountrystring,
                 u8 vipatch, u8 deflicker, u8 sneekVideoPatch, u8 hooktype, u8 videoWidth, u64 returnTo, u8 privateServer, const char *serverAddr)
{
    int i;
    u8 vfilter_off[7] = {0, 0, 21, 22, 21, 0, 0};
    u8 vfilter_low[7] = {4, 4, 16, 16, 16, 4, 4};
    u8 vfilter_medium[7] = {4, 8, 12, 16, 12, 8, 4};
    u8 vfilter_high[7] = {8, 8, 10, 12, 10, 8, 8};

    // If a wip file is loaded for this game this does nothing - Dimok
    patch_nsmb((u8 *)0x80000000);
    patch_pop((u8 *)0x80000000);
    patch_kirby((u8 *)0x80000000);

    for (i = 0; i < dolCount; ++i)
    {
        u8 *dst = dolList[i].dst;
        int len = dolList[i].len;

        VideoModePatcher(dst, len, videoSelected, videoPatchDol);

        if (hooktype)
            dogamehooks(hooktype, dst, len);

        if (vipatch)
            vidolpatcher(dst, len);

        if (sneekVideoPatch)
            sneek_video_patch(dst, len);

        // LANGUAGE PATCH - FISHEARS
        langpatcher(dst, len, languageChoice);

        // Thanks to WiiPower
        if (patchcountrystring == 1)
            PatchCountryStrings(dst, len);

        do_wip_code(dst, len);

        anti_002_fix(dst, len);

        if (videoWidth == WIDTH_FRAMEBUFFER)
            patch_width(dst, len);

        if (deflicker == DEFLICKER_ON_LOW)
        {
            patch_vfilters(dst, len, vfilter_low);
            patch_vfilters_rogue(dst, len, vfilter_low);
        }
        else if (deflicker == DEFLICKER_ON_MEDIUM)
        {
            patch_vfilters(dst, len, vfilter_medium);
            patch_vfilters_rogue(dst, len, vfilter_medium);
        }
        else if (deflicker == DEFLICKER_ON_HIGH)
        {
            patch_vfilters(dst, len, vfilter_high);
            patch_vfilters_rogue(dst, len, vfilter_high);
        }
        else if (deflicker != DEFLICKER_AUTO)
        {
            patch_vfilters(dst, len, vfilter_off);
            patch_vfilters_rogue(dst, len, vfilter_off);
            // This might break fade and brightness effects
            if (deflicker == DEFLICKER_OFF_EXTENDED)
                deflicker_patch(dst, len);
        }

        if (returnTo)
            PatchReturnTo(dst, len, (u32)returnTo);

        if (aspectForce < 2)
            PatchAspectRatio(dst, len, aspectForce);

        if (privateServer)
            PrivateServerPatcher(dst, len, privateServer, serverAddr);

        if (privateServer == PRIVSERV_WIIMMFI)
        {
            // If we end up here, that means it's a NON-MKWii Wiimmfi patch
            // add the new patches.
            do_new_wiimmfi_nonMKWii(dst, len);
        }

        DCFlushRange(dst, len);
        ICInvalidateRange(dst, len);
    }

    // ERROR 002 fix (thanks to WiiPower for sharing this)
    *(u32 *)0x80003140 = *(u32 *)0x80003188;

    DCFlushRange((void *)0x80000000, 0x3f00);

    free_wip();
    ClearDOLList();
}

/** Anti 002 fix for IOS 249 rev > 12 thanks to WiiPower **/
void anti_002_fix(u8 *addr, u32 len)
{
    u32 SearchPattern[3] = {0x2C000000, 0x48000214, 0x3C608000};
    u8 *addr_start = addr;
    u8 *addr_end = addr + len - sizeof(SearchPattern);
    while (addr_start <= addr_end)
    {
        if (memcmp(addr_start, SearchPattern, sizeof(SearchPattern)) == 0)
        {
            *((u32 *)addr_start + 1) = 0x40820214;
            return;
        }
        addr_start += 4;
    }
}

void patch_width(u8 *addr, u32 len)
{
    u8 SearchPattern[32] = {
        0x40, 0x82, 0x00, 0x08, 0x48, 0x00, 0x00, 0x1C,
        0x28, 0x09, 0x00, 0x03, 0x40, 0x82, 0x00, 0x08,
        0x48, 0x00, 0x00, 0x10, 0x2C, 0x03, 0x00, 0x00,
        0x40, 0x82, 0x00, 0x08, 0x54, 0xA5, 0x0C, 0x3C};
    u8 *addr_start = addr;
    u8 *addr_end = addr + len - sizeof(SearchPattern);
    while (addr_start <= addr_end)
    {
        if (memcmp(addr_start, SearchPattern, sizeof(SearchPattern)) == 0)
        {
            if (addr_start[-0x70] == 0xA0 && addr_start[-0x6E] == 0x00 && addr_start[-0x6D] == 0x0A)
            {
                if (addr_start[-0x44] == 0xA0 && addr_start[-0x42] == 0x00 && addr_start[-0x41] == 0x0E)
                {
                    u8 reg_a = (addr_start[-0x6F] >> 5);
                    u8 reg_b = (addr_start[-0x43] >> 5);

                    // Patch to the framebuffer resolution
                    addr_start[-0x41] = 0x04;

                    // Center the image
                    void *offset = addr_start - 0x70;

                    u32 old_heap_ptr = *(u32 *)0x80003110;
                    *(u32 *)0x80003110 = old_heap_ptr - 0x40;
                    u32 heap_space = old_heap_ptr - 0x40;

                    u32 org_address = (addr_start[-0x70] << 24) | (addr_start[-0x6F] << 16);
                    *(u32 *)(heap_space + 0x00) = org_address | 4;
                    *(u32 *)(heap_space + 0x04) = 0x200002D0 | (reg_b << 21) | (reg_a << 16);
                    *(u32 *)(heap_space + 0x08) = 0x38000002 | (reg_a << 21);
                    *(u32 *)(heap_space + 0x0C) = 0x7C000396 | (reg_a << 21) | (reg_b << 16) | (reg_a << 11);

                    *(u32 *)offset = 0x48000000 + ((heap_space - (u32)offset) & 0x3ffffff);
                    *(u32 *)(heap_space + 0x10) = 0x48000000 + ((((u32)offset + 0x04) - (heap_space + 0x10)) & 0x3ffffff);

                    gprintf("Patched resolution. Branched from 0x%x to 0x%x\n", offset, heap_space);
                    return;
                }
            }
        }
        addr_start += 4;
    }
}

/** Patch GXSetCopyFilter to disable the deflicker filter **/
void deflicker_patch(u8 *addr, u32 len)
{
    u32 SearchPattern[18] = {
        0x3D20CC01, 0x39400061, 0x99498000,
        0x2C050000, 0x38800053, 0x39600000,
        0x90098000, 0x38000054, 0x39800000,
        0x508BC00E, 0x99498000, 0x500CC00E,
        0x90698000, 0x99498000, 0x90E98000,
        0x99498000, 0x91098000, 0x41820040};
    u8 *addr_start = addr;
    u8 *addr_end = addr + len - sizeof(SearchPattern);
    while (addr_start <= addr_end)
    {
        if (memcmp(addr_start, SearchPattern, sizeof(SearchPattern)) == 0)
        {
            *((u32 *)addr_start + 17) = 0x48000040; // Change beq to b
            gprintf("Patched GXSetCopyFilter @ %p\n", addr_start);
            return;
        }
        addr_start += 4;
    }
}

/**
    480p Pixel Fix Patch by leseratte
    Fix for a Nintendo Revolution SDK bug found by Extrems affecting early Wii console when using 480p video mode.
    https://shmups.system11.org/viewtopic.php?p=1361158#p1361158
    https://github.com/ExtremsCorner/libogc-rice/commit/941d687e271fada68c359bbed98bed1fbb454448
**/
void PatchFix480p()
{
    u8 prefix[2] = {0x4b, 0xff};

    ///         Patch offset: ----------VVVVVVVV
    u32 Pattern_MKW[8] = {0x38000065, 0x9b810019, 0x38810018, 0x386000e0, 0x98010018, 0x38a00002};
    u32 patches_MKW[2] = {0x38600003, 0x98610019};
    /// Used by: MKWii, Wii Play, Need for Speed Nitro, Wii Sports, ...

    ///         Patch offset: -----------------------------------------------VVVVVVVV
    u32 Pattern_NSMB[8] = {0x38000065, 0x9801001c, 0x3881001c, 0x386000e0, 0x9b81001d, 0x38a00002};
    u32 patches_NSMB[2] = {0x38a00003, 0x98a1001d};
    /// Used by: New Super Mario Bros, ...

    /*
     * Code block that is being patched (in MKW):
     *
     * 4bffe30d: bl WaitMicroTime
     * 38000065: li r0, 0x65
     * 9b810019: stb r28, 25(r1) // store the wrong value (1)
     * 38810018: addi r4, r1, 0x18
     * 386000e0: li r3, 0xe0
     * 98010018: stb r0, 24(r1)
     * 38a00002: li r5, 2
     * 4bffe73d: bl __VISendI2CData
     *
     * r28 is a register that is set to 1 at the beginning of the function.
     * However, its contents are used elsewhere as well, so we can't just modify this one function.
     *
     * The following code first searches for one of the patterns above, then replaces the
     * "stb r28, 25(r1)" instruction that stores the wrong value on the stack with a branch instead
     * That branch branches to the injected custom code ("li r3, 3; stb r3, 25(r1)") that stores the
     * correct value (3) instead. At the end of the injected code will be another branch that branches
     * back to the instruction after the one that has been replaced (so, to "addi r4, r1, 0x18").
     * r3 can safely be used as a temporary register because its contents will be replaced immediately
     * afterwards anyways.
     *
     */

    void *offset = NULL;
    void *addr = (void *)0x80000000;
    u32 len = 0x900000;

    void *patch_ptr = 0;
    void *a = addr;

    while ((char *)a < ((char *)addr + len))
    {
        if (memcmp(a, &Pattern_MKW, 6 * 4) == 0)
        {
            // Found pattern?
            if (memcmp(a - 4, &prefix, 2) == 0)
            {
                if (memcmp(a + 8 * 4, &prefix, 2) == 0)
                {
                    offset = a + 4;
                    hexdump(a, 30);
                    patch_ptr = &patches_MKW;
                    break;
                }
            }
        }
        else if (memcmp(a, &Pattern_NSMB, 6 * 4) == 0)
        {
            // Found pattern?
            if (memcmp(a - 4, &prefix, 2) == 0)
            {
                if (memcmp(a + 8 * 4, &prefix, 2) == 0)
                {
                    offset = a + 16;
                    hexdump(a, 30);
                    patch_ptr = &patches_NSMB;
                    break;
                }
            }
        }
        a += 4;
    }

    if (offset == 0)
    {
        // offset is still 0, we didn't find the pattern, return
        gprintf("Didn't find offset for 480p patch!\n");
        return;
    }

    // If we are here, we found the offset. Lets grab some space
    // from the heap for our patch
    u32 old_heap_ptr = *(u32 *)0x80003110;
    *((u32 *)0x80003110) = (old_heap_ptr - 0x20);
    u32 heap_space = old_heap_ptr - 0x20;

    gprintf("Found offset for 480p patch - create branch from 0x%x to heap (0x%x)\n", offset, heap_space);
    hexdump(offset, 30);

    memcpy((void *)heap_space, patch_ptr, 8);

    *((u32 *)offset) = 0x48000000 + (((u32)(heap_space) - ((u32)(offset))) & 0x3ffffff);
    *((u32 *)((u32)heap_space + 8)) = 0x48000000 + (((u32)((u32)offset + 4) - ((u32)(heap_space + 8))) & 0x3ffffff);
    return;
}

/** Patch URLs for private Servers - Thanks to ToadKing/wiilauncher-nossl **/
void PrivateServerPatcher(void *addr, u32 len, u8 privateServer, const char *serverAddr)
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
    if (privateServer == PRIVSERV_WIIMMFI)
        domainpatcher(addr, len, "wiimmfi.de");
    else if (privateServer == PRIVSERV_ALTWFC)
        domainpatcher(addr, len, "zwei.moe");
    else if (privateServer == PRIVSERV_CUSTOM && strlen(serverAddr) > 3)
        domainpatcher(addr, len, serverAddr);
}

static inline int GetOpcode(unsigned int *instructionAddr)
{
    return ((*instructionAddr >> 26) & 0x3f);
}

static inline int GetImmediateDataVal(unsigned int *instructionAddr)
{
    return (*instructionAddr & 0xffff);
}

static inline int GetLoadTargetReg(unsigned int *instructionAddr)
{
    return (int)((*instructionAddr >> 21) & 0x1f);
}

static inline int GetComparisonTargetReg(unsigned int *instructionAddr)
{
    return (int)((*instructionAddr >> 16) & 0x1f);
}

s8 do_new_wiimmfi_nonMKWii(void *addr, u32 len)
{
    // As of February 2021, Wiimmfi requires a special Wiimmfi patcher
    // update which does a bit more than just patch the server addresses.
    // This function is being called by apploader.c, right before
    // jumping to the entry point (only for non-MKWii games on Wiimmfi),
    // and applies all the necessary security fixes to the game.

    // This function has been implemented by Leseratte. Please don't
    // try to modify it without speaking to the Wiimmfi team because
    // doing so could have unintended side effects.

    // This function MUST not run for MKWii, that would break stuff.

    int hasGT2Error = 0;
    char gt2locator[] = {0x38, 0x61, 0x00, 0x08, 0x38, 0xA0, 0x00, 0x14};
    // Opcode list for p2p:
    unsigned char opCodeChainP2P_v1[22] = {32, 32, 21, 21, 21, 21, 20, 20, 31, 40, 21, 20, 20, 31, 31, 10, 20, 36, 21, 44, 36, 16};
    unsigned char opCodeChainP2P_v2[22] = {32, 32, 21, 21, 20, 21, 20, 21, 31, 40, 21, 20, 20, 31, 31, 10, 20, 36, 21, 44, 36, 16};

    // Opcode list for MASTER:
    unsigned char opCodeChainMASTER_v1[22] = {21, 21, 21, 21, 40, 20, 20, 20, 20, 31, 31, 14, 31, 20, 21, 44, 21, 36, 36, 18, 11, 16};
    unsigned char opCodeChainMASTER_v2[22] = {21, 21, 21, 21, 40, 20, 20, 20, 20, 31, 31, 14, 31, 20, 21, 36, 21, 44, 36, 18, 11, 16};

    int MASTERopcodeChainOffset = 0;

    char *cur = addr;
    const char *end = addr + len;

    // Check if the game needs the new patch.
    do
    {
        if (memcmp(cur, "<GT2> RECV-0x%02x <- [--------:-----] [pid=%u]", 0x2e) == 0)
        {
            hasGT2Error++;
        }
    } while (++cur < end);

    cur = addr;

    if (hasGT2Error > 1)
        return 1; // error, this either doesn't exist, or exists once. Can't exist multiple times.

    int successful_patch_p2p = 0;
    int successful_patch_master = 0;

    do
    {
        // Patch the User-Agent so Wiimmfi knows this game has been patched.
        // This also identifies patcher (G=USB-Loader GX) and patch version (=3), please
        // do not change this without talking to Leseratte first.
        if (memcmp(cur, "User-Agent\x00\x00RVL SDK/", 20) == 0)
        {
            if (hasGT2Error)
                memcpy(cur + 12, "G-3-1\x00", 6);
            else
                memcpy(cur + 12, "G-3-0\x00", 6);
        }

        if (hasGT2Error)
        {
            if (memcmp(cur, &gt2locator, 8) == 0)
            {
                int found_opcode_chain_P2P_v1 = 1;
                int found_opcode_chain_P2P_v2 = 1;

                for (int i = 0; i < 22; i++)
                {
                    int offset = (i * 4) + 12;
                    if (opCodeChainP2P_v1[i] != (unsigned char)(GetOpcode((unsigned int *)(cur + offset))))
                        found_opcode_chain_P2P_v1 = 0;

                    if (opCodeChainP2P_v2[i] != (unsigned char)(GetOpcode((unsigned int *)(cur + offset))))
                        found_opcode_chain_P2P_v2 = 0;
                }
                int found_opcode_chain_MASTER;
                for (int dynamic = 0; dynamic < 40; dynamic += 4)
                {
                    found_opcode_chain_MASTER = 1;
                    int offset = 0;
                    for (int i = 0; i < 22; i++)
                    {
                        offset = (i * 4) + 12 + dynamic;
                        if (
                            (opCodeChainMASTER_v1[i] != (unsigned char)(GetOpcode((unsigned int *)(cur + offset)))) &&
                            (opCodeChainMASTER_v2[i] != (unsigned char)(GetOpcode((unsigned int *)(cur + offset))))
                        )
                        {
                            found_opcode_chain_MASTER = 0;
                        }
                    }

                    if (found_opcode_chain_MASTER)
                    {
                        //gprintf("found master opcode chain\n");
                        // We found the opcode chain, let's take a note of the offset.
                        MASTERopcodeChainOffset = (int)(cur + 12 + dynamic);
                        break;
                    }
                }
                if (found_opcode_chain_P2P_v1 || found_opcode_chain_P2P_v2)
                {
                    // Match found
                    // Now compare all the immediate values:
                    if (
                        GetImmediateDataVal((unsigned int *)(cur + 0x0c)) == 0x0c &&
                        GetImmediateDataVal((unsigned int *)(cur + 0x10)) == 0x18 &&
                        GetImmediateDataVal((unsigned int *)(cur + 0x30)) == 0x12 &&
                        GetImmediateDataVal((unsigned int *)(cur + 0x48)) == 0x5a &&
                        GetImmediateDataVal((unsigned int *)(cur + 0x50)) == 0x0c &&
                        GetImmediateDataVal((unsigned int *)(cur + 0x58)) == 0x12 &&
                        GetImmediateDataVal((unsigned int *)(cur + 0x5c)) == 0x18 &&
                        GetImmediateDataVal((unsigned int *)(cur + 0x60)) == 0x18
                    )
                    {
                        //gprintf("Patching P2P...\n");
                        int loadedDataReg = GetLoadTargetReg((unsigned int *)(cur + 0x14));
                        int comparisonDataReg = GetComparisonTargetReg((unsigned int *)(cur + 0x48));

                        if (found_opcode_chain_P2P_v1)
                        {
                            *(int *)(cur + 0x14) = (0x88010011 | (comparisonDataReg << 21));
                            *(int *)(cur + 0x18) = (0x28000080 | (comparisonDataReg << 16));
                            *(int *)(cur + 0x24) = 0x41810064;
                            *(int *)(cur + 0x28) = 0x60000000;
                            *(int *)(cur + 0x2c) = 0x60000000;
                            *(int *)(cur + 0x34) = (0x3C005A00 | (comparisonDataReg << 21));
                            *(int *)(cur + 0x48) = (0x7C000000 | (comparisonDataReg << 16) | (loadedDataReg << 11));
                            successful_patch_p2p++;
                        }
                        if (found_opcode_chain_P2P_v2)
                        {
                            loadedDataReg = 12;

                            *(int *)(cur + 0x14) = (0x88010011 | (comparisonDataReg << 21));
                            *(int *)(cur + 0x18) = (0x28000080 | (comparisonDataReg << 16));
                            *(int *)(cur + 0x1c) = 0x41810070;
                            *(int *)(cur + 0x24) = *(int *)(cur + 0x28);
                            *(int *)(cur + 0x28) = (0x8001000c | (loadedDataReg << 21));
                            *(int *)(cur + 0x2c) = (0x3C005A00 | (comparisonDataReg << 21));
                            *(int *)(cur + 0x34) = (0x7c000000 | (comparisonDataReg << 16) | (loadedDataReg << 11));
                            *(int *)(cur + 0x48) = 0x60000000;
                            successful_patch_p2p++;
                        }
                    }
                }
                else if (found_opcode_chain_MASTER)
                {
                    if (
                        GetImmediateDataVal((unsigned int *)(MASTERopcodeChainOffset + 0x10)) == 0x12 &&
                        GetImmediateDataVal((unsigned int *)(MASTERopcodeChainOffset + 0x2c)) == 0x04 &&

                        GetImmediateDataVal((unsigned int *)(MASTERopcodeChainOffset + 0x48)) == 0x18 &&
                        GetImmediateDataVal((unsigned int *)(MASTERopcodeChainOffset + 0x50)) == 0x00 &&
                        GetImmediateDataVal((unsigned int *)(MASTERopcodeChainOffset + 0x54)) == 0x18
                    )
                    {
                        int master_patch_version = 0;

                        // Check which version we have:
                        if (
                            GetImmediateDataVal((unsigned int *)(MASTERopcodeChainOffset + 0x3c)) == 0x12 &&
                            GetImmediateDataVal((unsigned int *)(MASTERopcodeChainOffset + 0x44)) == 0x0c
                        )
                            master_patch_version = 1;

                        else if (
                            GetImmediateDataVal((unsigned int *)(MASTERopcodeChainOffset + 0x3c)) == 0x0c &&
                            GetImmediateDataVal((unsigned int *)(MASTERopcodeChainOffset + 0x44)) == 0x12
                        )
                            master_patch_version = 2;

                        if (master_patch_version == 2)
                        {
                            // Different opcode order...
                            *(int *)(MASTERopcodeChainOffset + 0x3c) = *(int *)(MASTERopcodeChainOffset + 0x44);
                        }

                        if (master_patch_version != 0)
                        {
                            int rY = GetComparisonTargetReg((unsigned int *)MASTERopcodeChainOffset);
                            int rX = GetLoadTargetReg((unsigned int *)MASTERopcodeChainOffset);

                            *(int *)(MASTERopcodeChainOffset + 0x00) = 0x38000004 | (rX << 21);
                            *(int *)(MASTERopcodeChainOffset + 0x04) = 0x7c00042c | (rY << 21) | (3 << 16) | (rX << 11);
                            *(int *)(MASTERopcodeChainOffset + 0x14) = 0x9000000c | (rY << 21) | (1 << 16);
                            *(int *)(MASTERopcodeChainOffset + 0x18) = 0x88000011 | (rY << 21) | (1 << 16);
                            *(int *)(MASTERopcodeChainOffset + 0x28) = 0x28000080 | (rY << 16);
                            *(int *)(MASTERopcodeChainOffset + 0x38) = 0x60000000;
                            *(int *)(MASTERopcodeChainOffset + 0x44) = 0x41810014;
                            successful_patch_master++;
                        }
                    }
                }
            }
        }
    } while (++cur < end);

    if (hasGT2Error)
    {
        if (successful_patch_master == 0 || successful_patch_p2p == 0)
            return 2;
    }

    return 0;
}

s8 do_new_wiimmfi()
{
    // As of November 2018, Wiimmfi requires a special Wiimmfi patcher
    // update which does a bit more than just patch the server addresses.
    // This function is being called by GameBooter.cpp, right before
    // jumping to the entry point (only for Mario Kart Wii & Wiimmfi),
    // and applies all the necessary new patches to the game.
    // This includes support for the new patcher update plus
    // support for StaticR.rel patching.

    // This function has been implemented by Leseratte. Please don't
    // try to modify it without speaking to the Wiimmfi team because
    // doing so could have unintended side effects.

    // Updated in 2021 to add the 51420 error fix.

    // check region:
    char region = *((char *)(0x80000003));
    char *patched;
    void *patch1_offset, *patch2_offset, *patch3_offset;
    void *errorfix_offset;

    // define some offsets and variables depending on the region:
    switch (region)
    {
        case 'P':
            patched = (char *)0x80276054;
            patch1_offset = (void *)0x800ee3a0;
            patch2_offset = (void *)0x801d4efc;
            patch3_offset = (void *)0x801A72E0;
            errorfix_offset = (void *)0x80658ce4;
            break;
        case 'E':
            patched = (char *)0x80271d14;
            patch1_offset = (void *)0x800ee300;
            patch2_offset = (void *)0x801d4e5c;
            patch3_offset = (void *)0x801A7240;
            errorfix_offset = (void *)0x8065485c;
            break;
        case 'J':
            patched = (char *)0x802759f4;
            patch1_offset = (void *)0x800ee2c0;
            patch2_offset = (void *)0x801d4e1c;
            patch3_offset = (void *)0x801A7200;
            errorfix_offset = (void *)0x80658360;
            break;
        case 'K':
            patched = (char *)0x80263E34;
            patch1_offset = (void *)0x800ee418;
            patch2_offset = (void *)0x801d5258;
            patch3_offset = (void *)0x801A763c;
            errorfix_offset = (void *)0x80646ffc;
            break;
        default:
            return -1;
    }

    if (*patched != '*')
        return -2; // ISO already patched

    // This RAM address is set (no asterisk) by all officially
    // updated patchers, so if it is modified, the image is already
    // patched with a new patcher and we don't need to patch anything.

    // For statistics and easier debugging in case of problems, Wiimmfi
    // wants to know what patcher a game has been patched with, thus,
    // let the game know the exact USB-Loader version.
    char *fmt = "USB-Loader GX v3.0 R%-21s";
    char patcher[42] = {0};
    snprintf((char *)&patcher, 42, fmt, GetRev());
    strncpy(patched, (char *)&patcher, 42);

    // Do the plain old patching with the string search
    PrivateServerPatcher((void *)0x80004000, 0x385200, PRIVSERV_WIIMMFI, NULL);

    // Replace some URLs for Wiimmfi's new update system
    char newURL1[] = "http://ca.nas.wiimmfi.de/ca";
    char newURL2[] = "http://naswii.wiimmfi.de/ac";
    char newURL3P[] = "https://main.nas.wiimmfi.de/pp";
    char newURL3E[] = "https://main.nas.wiimmfi.de/pe";
    char newURL3J[] = "https://main.nas.wiimmfi.de/pj";
    char newURL3K[] = "https://main.nas.wiimmfi.de/pk";

    // Write the URLs to the proper place and do some other patching.
    switch (region)
    {
        case 'P':
            memcpy((void *)0x8027A400, newURL1, sizeof(newURL1));
            memcpy((void *)0x8027A400 + 0x28, newURL2, sizeof(newURL2));
            memcpy((void *)0x8027A400 + 0x4C, newURL3P, sizeof(newURL3P));
            *(u32 *)0x802a146c = 0x733a2f2f;
            *(u32 *)0x800ecaac = 0x3bc00000;
            break;
        case 'E':
            memcpy((void *)0x802760C0, newURL1, sizeof(newURL1));
            memcpy((void *)0x802760C0 + 0x28, newURL2, sizeof(newURL2));
            memcpy((void *)0x802760C0 + 0x4C, newURL3E, sizeof(newURL3E));
            *(u32 *)0x8029D12C = 0x733a2f2f;
            *(u32 *)0x800ECA0C = 0x3bc00000;
            break;
        case 'J':
            memcpy((void *)0x80279DA0, newURL1, sizeof(newURL1));
            memcpy((void *)0x80279DA0 + 0x28, newURL2, sizeof(newURL2));
            memcpy((void *)0x80279DA0 + 0x4C, newURL3J, sizeof(newURL3J));
            *(u32 *)0x802A0E0C = 0x733a2f2f;
            *(u32 *)0x800EC9CC = 0x3bc00000;
            break;
        case 'K':
            memcpy((void *)0x802682B0, newURL1, sizeof(newURL1));
            memcpy((void *)0x802682B0 + 0x28, newURL2, sizeof(newURL2));
            memcpy((void *)0x802682B0 + 0x4C, newURL3K, sizeof(newURL3K));
            *(u32 *)0x8028F474 = 0x733a2f2f;
            *(u32 *)0x800ECB24 = 0x3bc00000;
            break;
    }

    // Make some space on heap (0x500) for our custom code.
    u32 old_heap_ptr = *(u32 *)0x80003110;
    *((u32 *)0x80003110) = (old_heap_ptr - 0x500);
    u32 heap_space = old_heap_ptr - 0x500;
    memset((void *)old_heap_ptr - 0x500, 0xed, 0x500);

    // Binary blobs with Wiimmfi patches. Do not modify.
    // Provided by Leseratte on 2018-12-14.
    u32 binary[] = {
        0x37C849A2, 0x8BC32FA4, 0xC9A34B71, 0x1BCB49A2,
        0x2F119304, 0x5F402684, 0x3E4FDA29, 0x50849A21,
        0xB88B3452, 0x627FC9C1, 0xDC24D119, 0x5844350F,
        0xD893444F, 0x19A588DC, 0x16C91184, 0x0C3E237C,
        0x75906CED, 0x6E68A55E, 0x58791842, 0x072237E9,
        0xAB24906F, 0x0A8BDF21, 0x4D11BE42, 0x1AAEDDC8,
        0x1C42F908, 0x280CF2B2, 0x453A1BA4, 0x9A56C869,
        0x786F108E, 0xE8DF05D2, 0x6DB641EB, 0x6DFC84BB,
        0x7E980914, 0x0D7FB324, 0x23442185, 0xA7744966,
        0x53901359, 0xBF2103CC, 0xC24A4EB7, 0x32049A02,
        0xC1683466, 0xCA93689D, 0xD8245106, 0xA84987CF,
        0xEC9B47C9, 0x6FA688FE, 0x0A4D11A6, 0x8B653C7B,
        0x09D27E30, 0x5B936208, 0x5DD336DE, 0xCD092487,
        0xEF2C6D36, 0x1E09DF2D, 0x75B1BE47, 0xE68A7F22,
        0xB0E5F90D, 0xEC49F216, 0xAD1DCC24, 0xE2B5C841,
        0x066F6F63, 0xF4D90926, 0x299F42CD, 0xA3F125D6,
        0x077B093C, 0xB5721268, 0x1BE424D1, 0xEBC30BF0,
        0x77867BED, 0x4F0C9BCA, 0x3E195930, 0xDC32DE2C,
        0x1865D189, 0x70C67E7A, 0x71FA7329, 0x532233D3,
        0x06D2E87B, 0x6CBEBA7F, 0x99F08532, 0x52FA601C,
        0x05F4B82C, 0x4B64839C, 0xB5C65009, 0x1B8396E3,
        0x0A8B2DAF, 0x0DB85BE6, 0x12F1B71D, 0x186F6E4D,
        0x2870DC2E, 0x5960B8E6, 0x8F4D71BD, 0x0614E3C3,
        0x05E8C725, 0x365D8E3D, 0x74351CDE, 0xE1AB3930,
        0xFEDA721B, 0xE53AE4E9, 0xC3B4C9A6, 0xBAE59346,
        0x6D45269D, 0x634E4D1A, 0x2FD99A30, 0x26393449,
        0xE49768D1, 0x81E1D1A1, 0xFCE1A34A, 0x7EB44697,
        0xEB2F8D2D, 0xCECFE5AF, 0x81BD34B6, 0xB1F1696E,
        0x5E6ED2B2, 0xA473A4A0, 0x41664B70, 0xBF40968A,
        0x662F2CCB, 0xC5DF5B8C, 0xB632B772, 0x74EB6F39,
        0xE017DC71, 0xFDA3B890, 0xE3C9713D, 0xCE53E397,
        0xA12BC743, 0x5AD98EA5, 0xBC721C9F, 0x4568395A,
        0x925E72B4, 0x2D7DE4D7, 0x6777C9C7, 0xD6619396,
        0xA502268A, 0x77884D75, 0xF79E9AF0, 0xE6FC3461,
        0xF07468A5, 0xF866D11D, 0xF90CA342, 0xCF9546FF,
        0x87A48D81, 0x06881A51, 0x309C34D1, 0x79B669CE,
        0xFAADD2D7, 0xC8D7A5D1, 0x89214BE5, 0x1B8396EF,
        0x0A8B2DE9, 0x0D985B06, 0x12F1B711, 0x186F6E57,
        0x2850DC0E, 0x5960B8EA, 0x8F4D71AC, 0x0614E3E3,
        0x05E8C729, 0x365D8E39, 0x74351CFE, 0x518E3943,
        0x4A397268, 0x9D58E4B8, 0xD394C9A2, 0x0E069344,
        0xB522268B, 0x636E4D77, 0x2FF99A37, 0xF6DC346D,
        0xE49268B4, 0x2001D1A0, 0x4929A365, 0x7B764691,
        0xFFC68D49, 0x16A81A53, 0x247A34D2, 0xA1D16967,
        0x4B6DD2D5, 0xDDF4A5B7, 0x454A4B70, 0x0FAE96E2,
        0x0A8A2DC7, 0x0D98A47A, 0x06DCB71D, 0x0CCC6E38,
        0x55F25CFB, 0xB08C1E88, 0xDF4259C9, 0x0714E387,
        0xB00D47AF, 0x7B722975, 0x48BE349A, 0x29CC393C,
        0xEA797228, 0x98986471, 0x3778E1A3, 0xD7626D06,
        0x1567268D, 0x668ECD00, 0xD614F5C8, 0x133037CF,
        0x92F26CF2, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000};

    // Fix for error 51420:
    int patchCodeFix51420[] = {
        0x4800000d, 0x00000000,
        0x00000000, 0x7cc803a6,
        0x80860000, 0x7c041800,
        0x4182004c, 0x80a60004,
        0x38a50001, 0x2c050003,
        0x4182003c, 0x90a60004,
        0x90660000, 0x38610010,
        0x3ca08066, 0x38a58418,
        0x3c808066, 0x38848498,
        0x90a10010, 0x90810014,
        0x3ce08066, 0x38e78ce4,
        0x38e7fef0, 0x7ce903a6,
        0x4e800420, 0x3c80801d,
        0x388415f4, 0x7c8803a6,
        0x4e800021, 0x00000000};

    // Prepare patching process...
    int i = 3;
    int idx = 0;
    for (; i < 202; i++)
    {
        if (i == 67 || i == 82)
            idx++;
        binary[i] = binary[i] ^ binary[idx];
        binary[idx] = ((binary[idx] << 1) | ((binary[idx] >> (32 - 1)) & ~(0xfffffffe)));
    }

    // Binary blob needs some changes for regions other than PAL...
    switch (region)
    {
        case 'E':
            binary[29] = binary[67];
            binary[37] = binary[68];
            binary[43] = binary[69];
            binary[185] = 0x61295C74;
            binary[189] = 0x61295D40;
            binary[198] = 0x61086F5C;
            patchCodeFix51420[14] = 0x3ca08065;
            patchCodeFix51420[15] = 0x38a53f90;
            patchCodeFix51420[16] = 0x3c808065;
            patchCodeFix51420[17] = 0x38844010;
            patchCodeFix51420[20] = 0x3ce08065;
            patchCodeFix51420[21] = 0x38e7485c;
            patchCodeFix51420[26] = 0x38841554;
            break;
        case 'J':
            binary[29] = binary[70];
            binary[37] = binary[71];
            binary[43] = binary[72];
            binary[185] = 0x612997CC;
            binary[189] = 0x61299898;
            binary[198] = 0x61086F1C;
            patchCodeFix51420[14] = 0x3ca08065;
            patchCodeFix51420[15] = 0x38a57a84;
            patchCodeFix51420[16] = 0x3c808065;
            patchCodeFix51420[17] = 0x38847b04;
            patchCodeFix51420[20] = 0x3ce08065;
            patchCodeFix51420[21] = 0x38e78350;
            patchCodeFix51420[26] = 0x38841514;
            break;
        case 'K':
            binary[6] = binary[73];
            binary[9] = binary[74];
            binary[11] = binary[75];
            binary[23] = binary[76];
            binary[29] = binary[77];
            binary[33] = binary[78];
            binary[37] = binary[79];
            binary[43] = binary[80];
            binary[63] = binary[81];
            binary[184] = 0x3D208088;
            binary[185] = 0x61298AA4;
            binary[188] = 0x3D208088;
            binary[189] = 0x61298B58;
            binary[198] = 0x61087358;
            patchCodeFix51420[14] = 0x3ca08064;
            patchCodeFix51420[15] = 0x38a56730;
            patchCodeFix51420[16] = 0x3c808064;
            patchCodeFix51420[17] = 0x388467b0;
            patchCodeFix51420[20] = 0x3ce08064;
            patchCodeFix51420[21] = 0x38e76ffc;
            patchCodeFix51420[26] = 0x38841950;
            break;
    }

    // Installing all the patches.
    memcpy((void *)heap_space, (void *)binary, 820);
    u32 code_offset_1 = heap_space + 12;
    u32 code_offset_2 = heap_space + 88;
    u32 code_offset_3 = heap_space + 92;
    u32 code_offset_4 = heap_space + 264;
    u32 code_offset_5 = heap_space + 328;

    *((u32 *)patch1_offset) = 0x48000000 + (((u32)(code_offset_1) - ((u32)(patch1_offset))) & 0x3ffffff);
    *((u32 *)code_offset_2) = 0x48000000 + (((u32)(patch1_offset + 4) - ((u32)(code_offset_2))) & 0x3ffffff);
    *((u32 *)patch2_offset) = 0x48000000 + (((u32)(code_offset_3) - ((u32)(patch2_offset))) & 0x3ffffff);
    *((u32 *)code_offset_4) = 0x48000000 + (((u32)(patch2_offset + 4) - ((u32)(code_offset_4))) & 0x3ffffff);
    *((u32 *)patch3_offset) = 0x48000000 + (((u32)(code_offset_5) - ((u32)(patch3_offset))) & 0x3ffffff);

    // Add the 51420 fix:
    memcpy((void *)heap_space + 0x400, (void *)patchCodeFix51420, 0x78);
    *((u32 *)errorfix_offset) = 0x48000000 + (((u32)(heap_space + 0x400) - ((u32)(errorfix_offset))) & 0x3ffffff);
    *((u32 *)heap_space + 0x400 + 0x74) = 0x48000000 + (((u32)(errorfix_offset + 4) - ((u32)(heap_space + 0x400 + 0x74))) & 0x3ffffff);

    // Patches successfully installed
    // returns 0 when all patching is done and game is ready to be booted.
    return 0;
}

void domainpatcher(void *addr, u32 len, const char *domain)
{
    if (strlen("nintendowifi.net") < strlen(domain))
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
            for (i = 16 - strlen(domain); i > 0; i--)
                cur[len - i] = 0;
            cur += len;
        }
    } while (++cur < end);
}

bool patch_nsmb(u8 *gameid)
{
    WIP_Code *CodeList = NULL;

    if (memcmp(gameid, "SMNE01", 6) == 0)
    {
        CodeList = MEM2_alloc(3 * sizeof(WIP_Code));
        if (!CodeList)
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
    else if (memcmp(gameid, "SMNP01", 6) == 0)
    {
        CodeList = MEM2_alloc(3 * sizeof(WIP_Code));
        if (!CodeList)
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
    else if (memcmp(gameid, "SMNJ01", 6) == 0)
    {
        CodeList = MEM2_alloc(3 * sizeof(WIP_Code));
        if (!CodeList)
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
    if (CodeList)
        gprintf("Patched New Super Mario Bros\n");
    return CodeList != NULL;
}

bool patch_pop(u8 *gameid)
{
    if (memcmp(gameid, "SPX", 3) != 0 && memcmp(gameid, "RPW", 3) != 0)
        return false;

    WIP_Code *CodeList = MEM2_alloc(5 * sizeof(WIP_Code));
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
    if (CodeList)
        gprintf("Patched Prince of Persia\n");
    return CodeList != NULL;
}

void patch_error_codes(u8 *gameid)
{
    // Thanks to Seeky for the MKWii gecko codes
    // Thanks to InvoxiPlayGames for the gecko codes for the 23400 fix.
    // Reimplemented by Leseratte without the need for a code handler.
    u32 *patch_addr = 0;
    u32 *patched = 0;

    // Patch error 23400 for CoD (Black Ops, Reflex, MW3) and Rock Band 3 / The Beatles
    if (memcmp(gameid, "SC7", 3) == 0)
    {
        gprintf("Patching error 23400 for %s\n", gameid);
        *(u32 *)0x8023c954 = 0x41414141;
    }
    else if (memcmp(gameid, "RJA", 3) == 0)
    {
        gprintf("Patching error 23400 for %s\n", gameid);
        *(u32 *)0x801b838c = 0x41414141;
    }
    else if (memcmp(gameid, "SM8", 3) == 0)
    {
        gprintf("Patching error 23400 for %s\n", gameid);
        *(u32 *)0x80238c74 = 0x41414141;
    }
    else if (memcmp(gameid, "SZB", 3) == 0)
    {
        gprintf("Patching error 23400 for %s\n", gameid);
        *(u32 *)0x808e3b20 = 0x41414141;
    }
    else if (memcmp(gameid, "R9J", 3) == 0)
    {
        gprintf("Patching error 23400 for %s\n", gameid);
        *(u32 *)0x808d6934 = 0x41414141;
    }

    // Patch RCE vulnerability in MKWii.
    else if (memcmp(gameid, "RMC", 3) == 0)
    {
        switch (gameid[3])
        {
            case 'P':
                patched = (u32 *)0x80276054;
                patch_addr = (u32 *)0x8089a194;
                break;
            case 'E':
                patched = (u32 *)0x80271d14;
                patch_addr = (u32 *)0x80895ac4;
                break;
            case 'J':
                patched = (u32 *)0x802759f4;
                patch_addr = (u32 *)0x808992f4;
                break;
            case 'K':
                patched = (u32 *)0x80263E34;
                patch_addr = (u32 *)0x808885cc;
                break;
            default:
                gprintf("NOT patching RCE vulnerability due to invalid game ID: %s\n", gameid);
                return;
        }

        if (*patched != '*')
            gprintf("Game is already Wiimmfi-patched, don't apply the RCE fix\n");
        else
        {
            gprintf("Patching RCE vulnerability for %s\n", gameid);

            for (int i = 0; i < 7; i++)
                *patch_addr++ = 0xff;
        }
    }
}

/** Insert the individual gamepatches above with the patterns and patch data **/
/** Following is only the VideoPatcher **/

// viYOrigin is calculated as (576 - 528)/2 in libogc 2.0.0 for the following render modes.
// But we need to use (574 - 528)/2 so that the render modes match the Revolution SDK.

static GXRModeObj TVPal528Prog_RVL = {
    6,             // viDisplayMode
    640,           // fbWidth
    528,           // efbHeight
    528,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    23,            // viYOrigin (574 - 528)/2
    640,           // viWidth
    528,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_FALSE,      // aa

    // sample points arranged in increasing Y order
    {
        {6, 6}, {6, 6}, {6, 6}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {6, 6}, {6, 6}, {6, 6}, // pix 1
        {6, 6}, {6, 6}, {6, 6}, // pix 2
        {6, 6}, {6, 6}, {6, 6}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        0,  // line n-1
        0,  // line n-1
        21, // line n
        22, // line n
        21, // line n
        0,  // line n+1
        0   // line n+1
    }
};

static GXRModeObj TVPal528ProgSoft_RVL = {
    6,             // viDisplayMode
    640,           // fbWidth
    528,           // efbHeight
    528,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    23,            // viYOrigin (574 - 528)/2
    640,           // viWidth
    528,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_FALSE,      // aa

    // sample points arranged in increasing Y order
    {
        {6, 6}, {6, 6}, {6, 6}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {6, 6}, {6, 6}, {6, 6}, // pix 1
        {6, 6}, {6, 6}, {6, 6}, // pix 2
        {6, 6}, {6, 6}, {6, 6}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        8,  // line n-1
        8,  // line n-1
        10, // line n
        12, // line n
        10, // line n
        8,  // line n+1
        8   // line n+1
    }
};

static GXRModeObj TVPal524ProgAa_RVL = {
    6,             // viDisplayMode
    640,           // fbWidth
    264,           // efbHeight
    524,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    23,            // viYOrigin (574 - 528)/2
    640,           // viWidth
    524,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_TRUE,       // aa

    // sample points arranged in increasing Y order
    {
        {3, 2}, {9, 6}, {3, 10}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {3, 2}, {9, 6}, {3, 10}, // pix 1
        {9, 2}, {3, 6}, {9, 10}, // pix 2
        {9, 2}, {3, 6}, {9, 10}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        4,  // line n-1
        8,  // line n-1
        12, // line n
        16, // line n
        12, // line n
        8,  // line n+1
        4   // line n+1
    }
};

static GXRModeObj TVPal528Int_RVL = {
    4,             // viDisplayMode
    640,           // fbWidth
    528,           // efbHeight
    528,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    23,            // viYOrigin (574 - 528)/2
    640,           // viWidth
    528,           // viHeight
    VI_XFBMODE_DF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_FALSE,      // aa

    // sample points arranged in increasing Y order
    {
        {6, 6}, {6, 6}, {6, 6}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {6, 6}, {6, 6}, {6, 6}, // pix 1
        {6, 6}, {6, 6}, {6, 6}, // pix 2
        {6, 6}, {6, 6}, {6, 6}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        0,  // line n-1
        0,  // line n-1
        21, // line n
        22, // line n
        21, // line n
        0,  // line n+1
        0   // line n+1
    }
};

static GXRModeObj TVPal528IntDf_RVL = {
    4,             // viDisplayMode
    640,           // fbWidth
    528,           // efbHeight
    528,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    23,            // viYOrigin (574 - 528)/2
    640,           // viWidth
    528,           // viHeight
    VI_XFBMODE_DF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_FALSE,      // aa

    // sample points arranged in increasing Y order
    {
        {6, 6}, {6, 6}, {6, 6}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {6, 6}, {6, 6}, {6, 6}, // pix 1
        {6, 6}, {6, 6}, {6, 6}, // pix 2
        {6, 6}, {6, 6}, {6, 6}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        8,  // line n-1
        8,  // line n-1
        10, // line n
        12, // line n
        10, // line n
        8,  // line n+1
        8   // line n+1
    }
};

static GXRModeObj TVEurgb60Hz480Prog_RVL = {
    22,            // viDisplayMode
    640,           // fbWidth
    480,           // efbHeight
    480,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    0,             // viYOrigin (480 - 480)/2
    640,           // viWidth
    480,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_FALSE,      // aa

    // sample points arranged in increasing Y order
    {
        {6, 6}, {6, 6}, {6, 6}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {6, 6}, {6, 6}, {6, 6}, // pix 1
        {6, 6}, {6, 6}, {6, 6}, // pix 2
        {6, 6}, {6, 6}, {6, 6}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        0,  // line n-1
        0,  // line n-1
        21, // line n
        22, // line n
        21, // line n
        0,  // line n+1
        0   // line n+1
    }
};

static GXRModeObj TVEurgb60Hz480ProgSoft_RVL = {
    22,            // viDisplayMode
    640,           // fbWidth
    480,           // efbHeight
    480,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    0,             // viYOrigin (480 - 480)/2
    640,           // viWidth
    480,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_FALSE,      // aa

    // sample points arranged in increasing Y order
    {
        {6, 6}, {6, 6}, {6, 6}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {6, 6}, {6, 6}, {6, 6}, // pix 1
        {6, 6}, {6, 6}, {6, 6}, // pix 2
        {6, 6}, {6, 6}, {6, 6}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        8,  // line n-1
        8,  // line n-1
        10, // line n
        12, // line n
        10, // line n
        8,  // line n+1
        8   // line n+1
    }
};

static GXRModeObj TVEurgb60Hz480ProgAa_RVL = {
    22,            // viDisplayMode
    640,           // fbWidth
    242,           // efbHeight
    480,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    0,             // viYOrigin (480 - 480)/2
    640,           // viWidth
    480,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_TRUE,       // aa

    // sample points arranged in increasing Y order
    {
        {3, 2}, {9, 6}, {3, 10}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {3, 2}, {9, 6}, {3, 10}, // pix 1
        {9, 2}, {3, 6}, {9, 10}, // pix 2
        {9, 2}, {3, 6}, {9, 10}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        4,  // line n-1
        8,  // line n-1
        12, // line n
        16, // line n
        12, // line n
        8,  // line n+1
        4   // line n+1
    }
};

static GXRModeObj TVPal524IntAa_RVL = {
    4,             // viDisplayMode
    640,           // fbWidth
    264,           // efbHeight
    524,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    23,            // viYOrigin (574 - 528)/2 //574 instead of 576
    640,           // viWidth
    524,           // viHeight
    VI_XFBMODE_DF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_TRUE,       // aa

    // sample points arranged in increasing Y order
    {
        {3, 2}, {9, 6}, {3, 10}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {3, 2}, {9, 6}, {3, 10}, // pix 1
        {9, 2}, {3, 6}, {9, 10}, // pix 2
        {9, 2}, {3, 6}, {9, 10}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        4,  // line n-1
        8,  // line n-1
        12, // line n
        16, // line n
        12, // line n
        8,  // line n+1
        4   // line n+1
    }
};

static GXRModeObj TVPal264Int_RVL = {
    4,             // viDisplayMode
    640,           // fbWidth
    264,           // efbHeight
    264,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    23,            // viYOrigin (574 - 528)/2
    640,           // viWidth
    528,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_TRUE,       // field_rendering
    GX_FALSE,      // aa

    // sample points arranged in increasing Y order
    {
        {6, 6}, {6, 6}, {6, 6}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {6, 6}, {6, 6}, {6, 6}, // pix 1
        {6, 6}, {6, 6}, {6, 6}, // pix 2
        {6, 6}, {6, 6}, {6, 6}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        0,  // line n-1
        0,  // line n-1
        21, // line n
        22, // line n
        21, // line n
        0,  // line n+1
        0   // line n+1
    }
};

static GXRModeObj TVPal264IntAa_RVL = {
    4,             // viDisplayMode
    640,           // fbWidth
    264,           // efbHeight
    264,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    23,            // viYOrigin (574 - 528)/2
    640,           // viWidth
    528,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_TRUE,       // field_rendering
    GX_TRUE,       // aa

    // sample points arranged in increasing Y order
    {
        {3, 2}, {9, 6}, {3, 10}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {3, 2}, {9, 6}, {3, 10}, // pix 1
        {9, 2}, {3, 6}, {9, 10}, // pix 2
        {9, 2}, {3, 6}, {9, 10}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        0,  // line n-1
        0,  // line n-1
        21, // line n
        22, // line n
        21, // line n
        0,  // line n+1
        0   // line n+1
    }
};

static GXRModeObj TVPal264Ds_RVL = {
    5,             // viDisplayMode
    640,           // fbWidth
    264,           // efbHeight
    264,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    11,            // viYOrigin (572/2 - 528/2)/2
    640,           // viWidth
    528,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_FALSE,      // aa

    // sample points arranged in increasing Y order
    {
        {6, 6}, {6, 6}, {6, 6}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {6, 6}, {6, 6}, {6, 6}, // pix 1
        {6, 6}, {6, 6}, {6, 6}, // pix 2
        {6, 6}, {6, 6}, {6, 6}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        0,  // line n-1
        0,  // line n-1
        21, // line n
        22, // line n
        21, // line n
        0,  // line n+1
        0   // line n+1
    }
};

static GXRModeObj TVPal264DsAa_RVL = {
    5,             // viDisplayMode
    640,           // fbWidth
    264,           // efbHeight
    264,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    11,            // viYOrigin (572/2 - 528/2)/2
    640,           // viWidth
    528,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_TRUE,       // aa

    // sample points arranged in increasing Y order
    {
        {3, 2}, {9, 6}, {3, 10}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {3, 2}, {9, 6}, {3, 10}, // pix 1
        {9, 2}, {3, 6}, {9, 10}, // pix 2
        {9, 2}, {3, 6}, {9, 10}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        0,  // line n-1
        0,  // line n-1
        21, // line n
        22, // line n
        21, // line n
        0,  // line n+1
        0   // line n+1
    }
};

static GXRModeObj TVMpal240Int_RVL = {
    8,             // viDisplayMode
    640,           // fbWidth
    240,           // efbHeight
    240,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    0,             // viYOrigin (480 - 480)/2
    640,           // viWidth
    480,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_TRUE,       // field_rendering
    GX_FALSE,      // aa

    // sample points arranged in increasing Y order
    {
        {6, 6}, {6, 6}, {6, 6}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {6, 6}, {6, 6}, {6, 6}, // pix 1
        {6, 6}, {6, 6}, {6, 6}, // pix 2
        {6, 6}, {6, 6}, {6, 6}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        0,  // line n-1
        0,  // line n-1
        21, // line n
        22, // line n
        21, // line n
        0,  // line n+1
        0   // line n+1
    }
};

static GXRModeObj TVMpal240IntAa_RVL = {
    8,             // viDisplayMode
    640,           // fbWidth
    240,           // efbHeight
    240,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    0,             // viYOrigin (480 - 480)/2
    640,           // viWidth
    480,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_TRUE,       // field_rendering
    GX_TRUE,       // aa

    // sample points arranged in increasing Y order
    {
        {3, 2}, {9, 6}, {3, 10}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {3, 2}, {9, 6}, {3, 10}, // pix 1
        {9, 2}, {3, 6}, {9, 10}, // pix 2
        {9, 2}, {3, 6}, {9, 10}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        0,  // line n-1
        0,  // line n-1
        21, // line n
        22, // line n
        21, // line n
        0,  // line n+1
        0   // line n+1
    }
};

static GXRModeObj TVMpal480Int_RVL = {
    8,             // viDisplayMode
    640,           // fbWidth
    480,           // efbHeight
    480,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    0,             // viYOrigin (480 - 480)/2
    640,           // viWidth
    480,           // viHeight
    VI_XFBMODE_DF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_FALSE,      // aa

    // sample points arranged in increasing Y order
    {
        {6, 6}, {6, 6}, {6, 6}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {6, 6}, {6, 6}, {6, 6}, // pix 1
        {6, 6}, {6, 6}, {6, 6}, // pix 2
        {6, 6}, {6, 6}, {6, 6}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        0,  // line n-1
        0,  // line n-1
        21, // line n
        22, // line n
        21, // line n
        0,  // line n+1
        0   // line n+1
    }
};

static GXRModeObj TVMpal480ProgSoft_RVL = {
    10,            // viDisplayMode
    640,           // fbWidth
    480,           // efbHeight
    480,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    0,             // viYOrigin (480 - 480)/2
    640,           // viWidth
    480,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_FALSE,      // aa

    // sample points arranged in increasing Y order
    {
        {6, 6}, {6, 6}, {6, 6}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {6, 6}, {6, 6}, {6, 6}, // pix 1
        {6, 6}, {6, 6}, {6, 6}, // pix 2
        {6, 6}, {6, 6}, {6, 6}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        8,  // line n-1
        8,  // line n-1
        10, // line n
        12, // line n
        10, // line n
        8,  // line n+1
        8   // line n+1
    }
};

static GXRModeObj TVMpal480ProgAa_RVL = {
    10,            // viDisplayMode
    640,           // fbWidth
    242,           // efbHeight
    480,           // xfbHeight
    40,            // viXOrigin (720 - 640)/2
    0,             // viYOrigin (480 - 480)/2
    640,           // viWidth
    480,           // viHeight
    VI_XFBMODE_SF, // xFBmode
    GX_FALSE,      // field_rendering
    GX_TRUE,       // aa

    // sample points arranged in increasing Y order
    {
        {3, 2}, {9, 6}, {3, 10}, // pix 0, 3 sample points, 1/12 units, 4 bits each
        {3, 2}, {9, 6}, {3, 10}, // pix 1
        {9, 2}, {3, 6}, {9, 10}, // pix 2
        {9, 2}, {3, 6}, {9, 10}  // pix 3
    },

    // vertical filter[7], 1/64 units, 6 bits each
    {
        4,  // line n-1
        8,  // line n-1
        12, // line n
        16, // line n
        12, // line n
        8,  // line n+1
        4   // line n+1
    }
};

static GXRModeObj *vmodes[] = {
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
    &TVMpal240Int_RVL,
    &TVMpal240IntAa_RVL,
    &TVMpal240Ds,
    &TVMpal240DsAa,
    &TVMpal480Int_RVL,
    &TVMpal480IntAa,
    &TVMpal480IntDf,
    &TVMpal480Prog,
    &TVMpal480ProgSoft_RVL,
    &TVMpal480ProgAa_RVL,
    &TVPal264Ds_RVL,
    &TVPal264DsAa_RVL,
    &TVPal264Int_RVL,
    &TVPal264IntAa_RVL,
    &TVPal524IntAa_RVL,
    &TVPal524ProgAa_RVL,
    &TVPal528Int_RVL,
    &TVPal528IntDf_RVL,
    &TVPal528Prog_RVL,
    &TVPal528ProgSoft_RVL,
    &TVPal576IntDfScale,
    &TVEurgb60Hz240Ds,
    &TVEurgb60Hz240DsAa,
    &TVEurgb60Hz240Int,
    &TVEurgb60Hz240IntAa,
    &TVEurgb60Hz480Int,
    &TVEurgb60Hz480IntDf,
    &TVEurgb60Hz480IntAa,
    &TVEurgb60Hz480Prog_RVL,
    &TVEurgb60Hz480ProgSoft_RVL,
    &TVEurgb60Hz480ProgAa_RVL};

static const char *vmodes_name[] = {
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
    "TVMpal240Int",
    "TVMpal240IntAa",
    "TVMpal240Ds",
    "TVMpal240DsAa",
    "TVMpal480Int",
    "TVMpal480IntAa",
    "TVMpal480IntDf",
    "TVMpal480Prog",
    "TVMpal480ProgSoft",
    "TVMpal480ProgAa",
    "TVPal264Ds",
    "TVPal264DsAa",
    "TVPal264Int",
    "TVPal264IntAa",
    "TVPal524IntAa",
    "TVPal524ProgAa",
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
    "TVEurgb60Hz480ProgAa"};

static GXRModeObj *PAL2NTSC[] = {
    &TVMpal240Int_RVL, &TVNtsc240Int,
    &TVMpal240IntAa_RVL, &TVNtsc240IntAa,
    &TVMpal240Ds, &TVNtsc240Ds,
    &TVMpal240DsAa, &TVNtsc240DsAa,
    &TVMpal480Int_RVL, &TVNtsc480Int,
    &TVMpal480IntAa, &TVNtsc480IntAa,
    &TVMpal480IntDf, &TVNtsc480IntDf,
    &TVMpal480Prog, &TVNtsc480Prog,
    &TVMpal480ProgSoft_RVL, &TVNtsc480ProgSoft,
    &TVMpal480ProgAa_RVL, &TVNtsc480ProgAa,
    &TVPal264Ds_RVL, &TVNtsc240Ds,
    &TVPal264DsAa_RVL, &TVNtsc240DsAa,
    &TVPal264Int_RVL, &TVNtsc240Int,
    &TVPal264IntAa_RVL, &TVNtsc240IntAa,
    &TVPal524IntAa_RVL, &TVNtsc480IntAa,
    &TVPal524ProgAa_RVL, &TVNtsc480ProgAa,
    &TVPal528Int_RVL, &TVNtsc480Int,
    &TVPal528IntDf_RVL, &TVNtsc480IntDf,
    &TVPal528Prog_RVL, &TVNtsc480Prog,
    &TVPal528ProgSoft_RVL, &TVNtsc480ProgSoft,
    &TVPal576IntDfScale, &TVNtsc480IntDf,
    &TVEurgb60Hz240Ds, &TVNtsc240Ds,
    &TVEurgb60Hz240DsAa, &TVNtsc240DsAa,
    &TVEurgb60Hz240Int, &TVNtsc240Int,
    &TVEurgb60Hz240IntAa, &TVNtsc240IntAa,
    &TVEurgb60Hz480Int, &TVNtsc480Int,
    &TVEurgb60Hz480IntDf, &TVNtsc480IntDf,
    &TVEurgb60Hz480IntAa, &TVNtsc480IntAa,
    &TVEurgb60Hz480Prog_RVL, &TVNtsc480Prog,
    &TVEurgb60Hz480ProgSoft_RVL, &TVNtsc480ProgSoft,
    &TVEurgb60Hz480ProgAa_RVL, &TVNtsc480ProgAa,
    0, 0};

static GXRModeObj *NTSC2PAL[] = {
    &TVNtsc240Ds, &TVPal264Ds_RVL,
    &TVNtsc240DsAa, &TVPal264DsAa_RVL,
    &TVNtsc240Int, &TVPal264Int_RVL,
    &TVNtsc240IntAa, &TVPal264IntAa_RVL,
    &TVNtsc480Int, &TVPal528Int_RVL,
    &TVNtsc480IntAa, &TVPal524IntAa_RVL,
    &TVNtsc480IntDf, &TVPal528IntDf_RVL,
    &TVNtsc480Prog, &TVPal528Prog_RVL,
    &TVNtsc480ProgSoft, &TVPal528ProgSoft_RVL,
    &TVNtsc480ProgAa, &TVPal524ProgAa_RVL,
    0, 0};

static GXRModeObj *NTSC2PAL60[] = {
    &TVNtsc240Ds, &TVEurgb60Hz240Ds,
    &TVNtsc240DsAa, &TVEurgb60Hz240DsAa,
    &TVNtsc240Int, &TVEurgb60Hz240Int,
    &TVNtsc240IntAa, &TVEurgb60Hz240IntAa,
    &TVNtsc480Int, &TVEurgb60Hz480Int,
    &TVNtsc480IntAa, &TVEurgb60Hz480IntAa,
    &TVNtsc480IntDf, &TVEurgb60Hz480IntDf,
    &TVNtsc480Prog, &TVEurgb60Hz480Prog_RVL,
    &TVNtsc480ProgSoft, &TVEurgb60Hz480ProgSoft_RVL,
    &TVNtsc480ProgAa, &TVEurgb60Hz480ProgAa_RVL,
    0, 0};

static u8 PATTERN[12][2] = {
    {6, 6}, {6, 6}, {6, 6},
    {6, 6}, {6, 6}, {6, 6},
    {6, 6}, {6, 6}, {6, 6},
    {6, 6}, {6, 6}, {6, 6}
};

static u8 PATTERN_AA[12][2] = {
    {3, 2}, {9, 6}, {3, 10},
    {3, 2}, {9, 6}, {3, 10},
    {9, 2}, {3, 6}, {9, 10},
    {9, 2}, {3, 6}, {9, 10}
};

static bool compare_videomodes(GXRModeObj *mode1, GXRModeObj *mode2)
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

static void patch_videomode(GXRModeObj *mode1, GXRModeObj *mode2)
{
    mode1->viTVMode = mode2->viTVMode;
    if (mode1->viWidth == 640 || mode1->viWidth == 708)
    {
        mode1->fbWidth = mode2->fbWidth;
        mode1->efbHeight = mode2->efbHeight;
        mode1->xfbHeight = mode2->xfbHeight;
        mode1->viXOrigin = mode2->viXOrigin;
        mode1->viYOrigin = mode2->viYOrigin;
        mode1->viWidth = mode2->viWidth;
        mode1->viHeight = mode2->viHeight;
    }
    else
        gprintf("Skipped patching dimensions %d x %d\n", mode1->viWidth, mode1->viHeight);

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

static bool Search_and_patch_Video_Modes(u8 *Address, u32 Size, GXRModeObj *Table[])
{
    u8 *Addr = (u8 *)Address;
    bool found = 0;
    u32 i, j;

    while (Size >= sizeof(GXRModeObj))
    {
        for (i = 0; Table[i]; i += 2)
        {
            if (compare_videomodes(Table[i], (GXRModeObj *)Addr))
            {
                u8 current_vmode = 0;
                u8 target_vmode = 0;
                for (j = 0; j < sizeof(vmodes) / sizeof(vmodes[0]); j++)
                {
                    if (compare_videomodes(Table[i], vmodes[j]))
                    {
                        current_vmode = j;
                        break;
                    }
                }
                for (j = 0; j < sizeof(vmodes) / sizeof(vmodes[0]); j++)
                {
                    if (compare_videomodes(Table[i + 1], vmodes[j]))
                    {
                        target_vmode = j;
                        break;
                    }
                }

                gprintf("Video mode found in dol: %s, replaced by: %s \n", vmodes_name[current_vmode], vmodes_name[target_vmode]);
                found = 1;
                patch_videomode((GXRModeObj *)Addr, Table[i + 1]);
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

// Patch known and unknown vfilters within GXRModeObj structures
void patch_vfilters(u8 *addr, u32 len, u8 *vfilter)
{
    u8 *addr_start = addr;
    while (len >= sizeof(GXRModeObj))
    {
        GXRModeObj *vidmode = (GXRModeObj *)addr_start;
        if ((memcmp(vidmode->sample_pattern, PATTERN, 24) == 0 || memcmp(vidmode->sample_pattern, PATTERN_AA, 24) == 0) &&
            (vidmode->fbWidth == 640 || vidmode->fbWidth == 608 || vidmode->fbWidth == 512) &&
            (vidmode->field_rendering == 0 || vidmode->field_rendering == 1) &&
            (vidmode->aa == 0 || vidmode->aa == 1))
        {
            gprintf("Replaced vfilter %02x%02x%02x%02x%02x%02x%02x @ %p (GXRModeObj)\n",
                    vidmode->vfilter[0], vidmode->vfilter[1], vidmode->vfilter[2], vidmode->vfilter[3],
                    vidmode->vfilter[4], vidmode->vfilter[5], vidmode->vfilter[6], addr_start);
            memcpy(vidmode->vfilter, vfilter, 7);
            addr_start += (sizeof(GXRModeObj) - 4);
            len -= (sizeof(GXRModeObj) - 4);
        }
        addr_start += 4;
        len -= 4;
    }
}

// Patch rogue vfilters found in some games
void patch_vfilters_rogue(u8 *addr, u32 len, u8 *vfilter)
{
    u8 known_vfilters[7][7] = {
        {8, 8, 10, 12, 10, 8, 8},
        {4, 8, 12, 16, 12, 8, 4},
        {7, 7, 12, 12, 12, 7, 7},
        {5, 5, 15, 14, 15, 5, 5},
        {4, 4, 15, 18, 15, 4, 4},
        {4, 4, 16, 16, 16, 4, 4},
        {2, 2, 17, 22, 17, 2, 2}
    };
    u8 *addr_start = addr;
    u8 *addr_end = addr + len - 8;
    while (addr_start <= addr_end)
    {
        u8 known_vfilter[7];
        for (int i = 0; i < 7; i++)
        {
            for (int x = 0; x < 7; x++)
                known_vfilter[x] = known_vfilters[i][x];
            if (!addr_start[7] && memcmp(addr_start, known_vfilter, 7) == 0)
            {
                gprintf("Replaced vfilter %02x%02x%02x%02x%02x%02x%02x @ %p\n", addr_start[0], addr_start[1],
                        addr_start[2], addr_start[3], addr_start[4], addr_start[5], addr_start[6], addr_start);
                memcpy(addr_start, vfilter, 7);
                addr_start += 7;
                break;
            }
        }
        addr_start += 1;
    }
}

static bool Search_and_patch_Video_To(void *Address, u32 Size, GXRModeObj *Table[], GXRModeObj *rmode, bool patchAll)
{
    u8 *Addr = (u8 *)Address;
    bool found = 0;
    u32 i;
    u8 target_vmode = 0;
    for (i = 0; i < sizeof(vmodes) / sizeof(vmodes[0]); i++)
    {
        if (compare_videomodes(Table[i], rmode))
        {
            target_vmode = i;
            break;
        }
    }

    while (Size >= sizeof(GXRModeObj))
    {
        if ((memcmp(((GXRModeObj *)Addr)->sample_pattern, PATTERN, 24) == 0 || memcmp(((GXRModeObj *)Addr)->sample_pattern, PATTERN_AA, 24) == 0) &&
            (((GXRModeObj *)Addr)->fbWidth == 640 || ((GXRModeObj *)Addr)->fbWidth == 608 || ((GXRModeObj *)Addr)->fbWidth == 512) &&
            (((GXRModeObj *)Addr)->field_rendering == 0 || ((GXRModeObj *)Addr)->field_rendering == 1) &&
            (((GXRModeObj *)Addr)->aa == 0 || ((GXRModeObj *)Addr)->aa == 1)
        )
        {
            // display found video mode patterns
            GXRModeObj *vidmode = (GXRModeObj *)Addr;
            gprintf("GXRModeObj \t%08x %04x %04x %04x %04x %04x %04x %04x %08x %04x %04x "
                    "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x "
                    "%02x%02x%02x%02x%02x%02x%02x \n",
                    vidmode->viTVMode, vidmode->fbWidth, vidmode->efbHeight, vidmode->xfbHeight, vidmode->viXOrigin, vidmode->viYOrigin,
                    vidmode->viWidth, vidmode->viHeight, vidmode->xfbMode, vidmode->field_rendering, vidmode->aa,
                    vidmode->sample_pattern[0][0], vidmode->sample_pattern[1][0], vidmode->sample_pattern[2][0], vidmode->sample_pattern[3][0], vidmode->sample_pattern[4][0],
                    vidmode->sample_pattern[5][0], vidmode->sample_pattern[6][0], vidmode->sample_pattern[7][0], vidmode->sample_pattern[8][0], vidmode->sample_pattern[9][0],
                    vidmode->sample_pattern[10][0], vidmode->sample_pattern[11][0], vidmode->sample_pattern[0][1], vidmode->sample_pattern[1][1], vidmode->sample_pattern[2][1],
                    vidmode->sample_pattern[3][1], vidmode->sample_pattern[4][1], vidmode->sample_pattern[5][1], vidmode->sample_pattern[6][1], vidmode->sample_pattern[7][1],
                    vidmode->sample_pattern[8][1], vidmode->sample_pattern[9][1], vidmode->sample_pattern[10][1], vidmode->sample_pattern[11][1],
                    vidmode->vfilter[0], vidmode->vfilter[1], vidmode->vfilter[2], vidmode->vfilter[3], vidmode->vfilter[4], vidmode->vfilter[5], vidmode->vfilter[6]);

            found = 0;
            for (i = 0; i < sizeof(vmodes) / sizeof(vmodes[0]); i++)
            {
                if (compare_videomodes(Table[i], (GXRModeObj *)Addr))
                {
                    found = 1;
                    gprintf("Video mode found in dol: %s, replaced by: %s \n", vmodes_name[i], vmodes_name[target_vmode]);
                    patch_videomode((GXRModeObj *)Addr, rmode);
                    Addr += (sizeof(GXRModeObj) - 4);
                    Size -= (sizeof(GXRModeObj) - 4);
                    break;
                }
            }
            if (patchAll && !found)
            {
                gprintf("Video mode found in dol: Unknown, replaced by: %s \n", vmodes_name[target_vmode]);
                patch_videomode((GXRModeObj *)Addr, rmode);
                Addr += (sizeof(GXRModeObj) - 4);
                Size -= (sizeof(GXRModeObj) - 4);
            }
        }
        Addr += 4;
        Size -= 4;
    }

    return found;
}

void VideoModePatcher(u8 *dst, int len, u8 videoSelected, u8 VideoPatchDol)
{
    GXRModeObj **table = NULL;
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
    else if (VideoPatchDol == VIDEO_PATCH_DOL_REGION) //&& rmode != NULL)
    {
        switch (rmode->viTVMode >> 2)
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
    u8 *addr_end = addr + len;

    while (addr_start < addr_end)
    {
        if (*(vu32 *)(addr_start) == 0x3C608000)
        {
            if (((*(vu32 *)(addr_start + 4) & 0xFC1FFFFF) == 0x800300CC) && ((*(vu32 *)(addr_start + 8) >> 24) == 0x54))
            {
                *(vu32 *)(addr_start + 4) = 0x5400F0BE | ((*(vu32 *)(addr_start + 4) & 0x3E00000) >> 5);
            }
        }
        addr_start += 4;
    }
}

// giantpune's magic super patch to return to channels

static u32 ad[4] = {0, 0, 0, 0}; // these variables are global on the off chance the different parts needed
static u8 found = 0;             // to find in the dol are found in different sections of the dol
static u8 returnToPatched = 0;

bool PatchReturnTo(void *Address, int Size, u32 id)
{
    if (!id || returnToPatched)
        return 0;
    //gprintf("PatchReturnTo( %p, %08x, %08x )\n", Address, Size, id );

    // new __OSLoadMenu() (SM2.0 and higher)
    u8 SearchPattern[12] = {0x38, 0x80, 0x00, 0x02, 0x38, 0x60, 0x00, 0x01, 0x38, 0xa0, 0x00, 0x00}; // li r4,2
    // li r3,1
    // li r5,0
    // old _OSLoadMenu() (used in launch games)
    u8 SearchPatternB[12] = {0x38, 0xC0, 0x00, 0x02, 0x38, 0xA0, 0x00, 0x01, 0x38, 0xE0, 0x00, 0x00}; // li r6,2
    // li r5,1
    // li r7,0
    // identifier for the safe place
    u8 SearchPattern2[12] = {0x4D, 0x65, 0x74, 0x72, 0x6F, 0x77, 0x65, 0x72, 0x6B, 0x73, 0x20, 0x54}; // "Metrowerks T"

    u8 oldSDK = 0;
    found = 0;

    void *Addr = Address;
    void *Addr_end = Address + Size;

    while (Addr <= Addr_end - 12)
    {
        // find a safe place for the patch to hang out
        if (!ad[3] && memcmp(Addr, SearchPattern2, 12) == 0)
        {
            ad[3] = (u32)Addr + 0x30;
        }
        // find __OSLaunchMenu() and remember some addresses in it
        else if (memcmp(Addr, SearchPattern, 12) == 0)
        {
            ad[found++] = (u32)Addr;
        }
        else if (ad[0] && memcmp(Addr, SearchPattern, 8) == 0) // after the first match is found, only search the first 8 bytes for the other 2
        {
            if (!ad[1])
                ad[found++] = (u32)Addr;
            else if (!ad[2])
                ad[found++] = (u32)Addr;
            if (found >= 3)
                break;
        }
        Addr += 4;
    }
    // check for the older-ass version of the SDK
    if (found < 3 && ad[3])
    {
        Addr = Address;
        ad[0] = 0;
        ad[1] = 0;
        ad[2] = 0;
        found = 0;
        oldSDK = 1;

        while (Addr <= Addr_end - 12)
        {
            // find __OSLaunchMenu() and remember some addresses in it
            if (memcmp(Addr, SearchPatternB, 12) == 0)
            {
                ad[found++] = (u32)Addr;
            }
            else if (ad[0] && memcmp(Addr, SearchPatternB, 8) == 0) // after the first match is found, only search the first 8 bytes for the other 2
            {
                if (!ad[1])
                    ad[found++] = (u32)Addr;
                else if (!ad[2])
                    ad[found++] = (u32)Addr;
                if (found >= 3)
                    break;
            }
            Addr += 4;
        }
    }

    // if the function is found
    if (found == 3 && ad[3])
    {
        //gprintf("patch __OSLaunchMenu( 0x00010001, 0x%08x )\n", id);
        u32 nop = 0x60000000;

        // the magic that writes the TID to the registers
        u8 jump[20] = {
            0x3C, 0x60, 0x00, 0x01,                     // lis r3,1
            0x60, 0x63, 0x00, 0x01,                     // ori r3,r3,1
            0x3C, 0x80, (u8)(id >> 24), (u8)(id >> 16), // lis r4,(u16)(tid >> 16)
            0x60, 0x84, (u8)(id >> 8), (u8)id,          // ori r4,r4,(u16)(tid)
            0x4E, 0x80, 0x00, 0x20};                    // blr

        if (oldSDK)
        {
            jump[1] = 0xA0;  // 3CA00001 // lis r5,1
            jump[5] = 0xA5;  // 60A50001 // ori r5,r5,1
            jump[9] = 0xC0;  // 3CC0AF1B // lis r6,(u16)(tid >> 16)
            jump[13] = 0xC6; // 60C6F516 // ori r6,r6,(u16)(tid)
        }

        void *addr = (u32 *)ad[3];

        // write new stuff to in a unused part of the main.dol
        memcpy(addr, jump, sizeof(jump));

        // ES_GetTicketViews()
        u32 newval = (ad[3] - ad[0]);
        newval &= 0x03FFFFFC;
        newval |= 0x48000001;
        addr = (u32 *)ad[0];
        memcpy(addr, &newval, sizeof(u32));  // bl ad[ 3 ]
        memcpy(addr + 4, &nop, sizeof(u32)); // nop
        // gprintf("\t%08x -> %08x\n", addr, newval );

        // ES_GetTicketViews() again
        newval = (ad[3] - ad[1]);
        newval &= 0x03FFFFFC;
        newval |= 0x48000001;
        addr = (u32 *)ad[1];
        memcpy(addr, &newval, sizeof(u32));  // bl ad[ 3 ]
        memcpy(addr + 4, &nop, sizeof(u32)); // nop
        // gprintf("\t%08x -> %08x\n", addr, newval );

        // ES_LaunchTitle()
        newval = (ad[3] - ad[2]);
        newval &= 0x03FFFFFC;
        newval |= 0x48000001;
        addr = (u32 *)ad[2];
        memcpy(addr, &newval, sizeof(u32));  // bl ad[ 3 ]
        memcpy(addr + 4, &nop, sizeof(u32)); // nop
        // gprintf("\t%08x -> %08x\n", addr, newval );

        returnToPatched = 1;
    }

    if (returnToPatched)
        gprintf("Return to %08X patched with old method.\n", (u32)id);

    return returnToPatched;
}

int PatchNewReturnTo(int es_fd, u64 title)
{
    if (es_fd < 0 || title == 0)
        return -1;

    //! this is here for test purpose only and needs be moved later
    static u64 sm_title_id ATTRIBUTE_ALIGN(32);
    ioctlv *vector = (ioctlv *)memalign(32, sizeof(ioctlv));
    if (!vector)
        return -1;

    sm_title_id = title;
    vector[0].data = &sm_title_id;
    vector[0].len = sizeof(sm_title_id);

    int result = -1;

    if (es_fd >= 0)
        result = IOS_Ioctlv(es_fd, 0xA1, 1, 0, vector);

    if (result >= 0)
        gprintf("Return to %08X patched with d2x method.\n", (u32)title);

    free(vector);

    return result;
}

int BlockIOSReload(int es_fd, u32 gameIOS)
{
    if (es_fd < 0)
        return -1;

    static int mode ATTRIBUTE_ALIGN(32);
    static int ios ATTRIBUTE_ALIGN(32);
    ioctlv *vector = (ioctlv *)memalign(32, sizeof(ioctlv) * 2);
    if (!vector)
        return -1;

    int inlen = 2;
    mode = 2;
    ios = gameIOS; // ios to be reloaded in place of the requested one
    vector[0].data = &mode;
    vector[0].len = 4;
    vector[1].data = &ios;
    vector[1].len = 4;

    int result = -1;

    if (es_fd >= 0)
        result = IOS_Ioctlv(es_fd, 0xA0, inlen, 0, vector);

    if (result >= 0)
        gprintf("Block IOS Reload patched with d2x method to IOS%i; result: %i\n", gameIOS, result);

    free(vector);

    return result;
}

void PatchAspectRatio(void *addr, u32 len, u8 aspect)
{
    if (aspect > 1)
        return;

    static const u32 aspect_searchpattern1[5] = {
        0x9421FFF0, 0x7C0802A6, 0x38800001, 0x90010014, 0x38610008};

    static const u32 aspect_searchpattern2[15] = {
        0x2C030000, 0x40820010, 0x38000000, 0x98010008, 0x48000018,
        0x88010008, 0x28000001, 0x4182000C, 0x38000000, 0x98010008,
        0x80010014, 0x88610008, 0x7C0803A6, 0x38210010, 0x4E800020};

    u8 *addr_start = (u8 *)addr;
    u8 *addr_end = addr_start + len - sizeof(aspect_searchpattern1) - 4 - sizeof(aspect_searchpattern2);

    while (addr_start < addr_end)
    {
        if ((memcmp(addr_start, aspect_searchpattern1, sizeof(aspect_searchpattern1)) == 0) &&
            (memcmp(addr_start + 4 + sizeof(aspect_searchpattern1), aspect_searchpattern2, sizeof(aspect_searchpattern2)) == 0))
        {
            *((u32 *)(addr_start + 0x44)) = (0x38600000 | aspect);
            gprintf("Aspect ratio patched to: %s\n", aspect ? "16:9" : "4:3");
            break;
        }
        addr_start += 4;
    }
}
