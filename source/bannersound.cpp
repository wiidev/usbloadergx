#include <stdio.h>
#include <ogcsys.h>
#include <unistd.h>
#include <new>

#include "usbloader/disc.h"
#include "usbloader/wbfs.h"
#include "prompts/PromptWindows.h"
#include "libs/libwbfs/libwbfs.h"
#include "language/gettext.h"
#include "bannersound.h"
#include "utils/uncompress.h"

struct IMD5Header
{
        u32 fcc;
        u32 filesize;
        u8 zeroes[8];
        u8 crypto[16];
}__attribute__( ( packed ) );

struct IMETHeader
{
        u8 zeroes[64];
        u32 fcc;
        u8 unk[8];
        u32 iconSize;
        u32 bannerSize;
        u32 soundSize;
        u32 flag1;
        u8 names[7][84];
        u8 zeroes_2[0x348];
        u8 crypto[16];
}__attribute__( ( packed ) );

struct U8Header
{
        u32 fcc;
        u32 rootNodeOffset;
        u32 headerSize;
        u32 dataOffset;
        u8 zeroes[16];
}__attribute__( ( packed ) );

struct U8Entry
{
        struct
        {
                u32 fileType :8;
                u32 nameOffset :24;
        };
        u32 fileOffset;
        union
        {
                u32 fileLength;
                u32 numEntries;
        };
}__attribute__( ( packed ) );

struct LZ77Info
{
        u16 length :4;
        u16 offset :12;
}__attribute__( ( packed ) );

static char *u8Filename(const U8Entry *fst, int i)
{
    return (char *) (fst + fst[0].numEntries) + fst[i].nameOffset;
}

const u8 *LoadBannerSound(const u8 *discid, u32 *size)
{
    if (!discid) return NULL;

    wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) discid);
    if (!disc)
    {
        // WindowPrompt(tr("Can't find disc"), 0, tr("OK"));
        return NULL;
    }
    wiidisc_t *wdisc = wd_open_disc((int(*)(void *, u32, u32, void *)) wbfs_disc_read, disc);
    if (!wdisc)
    {
        //WindowPrompt(tr("Could not open Disc"), 0, tr("OK"));
        return NULL;
    }
    u8 * opening_bnr = wd_extract_file(wdisc, ALL_PARTITIONS, (char *) "opening.bnr");
    if (!opening_bnr)
    {
        //WindowPrompt(tr("ERROR"), tr("Failed to extract opening.bnr"), tr("OK"));
        return NULL;
    }

    wd_close_disc(wdisc);
    WBFS_CloseDisc(disc);

    const U8Entry *fst;

    const IMETHeader *imetHdr = (IMETHeader *) opening_bnr;
    if (imetHdr->fcc != 0x494D4554 /*"IMET"*/)
    {
        //  WindowPrompt(tr("IMET Header wrong."), 0, tr("OK"));
        free(opening_bnr);
        return NULL;
    }
    const U8Header *bnrArcHdr = (U8Header *) (imetHdr + 1);

    fst = (const U8Entry *) (((const u8 *) bnrArcHdr) + bnrArcHdr->rootNodeOffset);
    u32 i;
    for (i = 1; i < fst[0].numEntries; ++i)
        if (fst[i].fileType == 0 && strcasecmp(u8Filename(fst, i), "sound.bin") == 0) break;
    if (i >= fst[0].numEntries)
    {
        /* Not all games have a sound.bin and this message is annoying **/
        //WindowPrompt(tr("sound.bin not found."), 0, tr("OK"));
        free(opening_bnr);
        return NULL;
    }
    const u8 *sound_bin = ((const u8 *) bnrArcHdr) + fst[i].fileOffset;
    if (((IMD5Header *) sound_bin)->fcc != 0x494D4435 /*"IMD5"*/)
    {
        //  WindowPrompt(tr("IMD5 Header not right."), 0, tr("OK"));
        free(opening_bnr);
        return NULL;
    }
    const u8 *soundChunk = sound_bin + sizeof(IMD5Header);
    ;
    u32 soundChunkSize = fst[i].fileLength - sizeof(IMD5Header);

    if (*((u32*) soundChunk) == 0x4C5A3737 /*"LZ77"*/)
    {
        u32 uncSize = 0;
        u8 * uncompressed_data = uncompressLZ77(soundChunk, soundChunkSize, &uncSize);
        if (!uncompressed_data)
        {
            //  WindowPrompt(tr("Can't decompress LZ77"), 0, tr("OK"));
            free(opening_bnr);
            return NULL;
        }
        if (size) *size = uncSize;
        free(opening_bnr);
        return uncompressed_data;
    }
    u8 *out = (u8 *) malloc(soundChunkSize);
    if (out)
    {
        memcpy(out, soundChunk, soundChunkSize);
        if (size) *size = soundChunkSize;
    }
    free(opening_bnr);
    return out;
}
