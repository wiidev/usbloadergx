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

inline u32 le32(u32 i)
{
    return ((i & 0xFF) << 24) | ((i & 0xFF00) << 8) | ((i & 0xFF0000) >> 8) | ((i & 0xFF000000) >> 24);
}

inline u16 le16(u16 i)
{
    return ((i & 0xFF) << 8) | ((i & 0xFF00) >> 8);
}

static u8 *uncompressLZ77(const u8 *inBuf, u32 inLength, u32 &size)
{
    u8 *buffer = NULL;
    if (inLength <= 0x8 || *((const u32 *) inBuf) != 0x4C5A3737 /*"LZ77"*/|| inBuf[4] != 0x10) return NULL;
    u32 uncSize = le32(((const u32 *) inBuf)[1] << 8);

    const u8 *inBufEnd = inBuf + inLength;
    inBuf += 8;
    buffer = new (std::nothrow) u8[uncSize];
    if (!buffer) return buffer;

    u8 *bufCur = buffer;
    u8 *bufEnd = buffer + uncSize;

    while (bufCur < bufEnd && inBuf < inBufEnd)
    {
        u8 flags = *inBuf;
        ++inBuf;
        for (int i = 0; i < 8 && bufCur < bufEnd && inBuf < inBufEnd; ++i)
        {
            if ((flags & 0x80) != 0)
            {
                const LZ77Info &info = *(const LZ77Info *) inBuf;
                inBuf += sizeof(LZ77Info);
                int length = info.length + 3;
                if (bufCur - info.offset - 1 < buffer || bufCur + length > bufEnd) return buffer;
                memcpy(bufCur, bufCur - info.offset - 1, length);
                bufCur += length;
            }
            else
            {
                *bufCur = *inBuf;
                ++inBuf;
                ++bufCur;
            }
            flags <<= 1;
        }
    }
    size = uncSize;
    return buffer;
}

const u8 *LoadBannerSound(const u8 *discid, u32 *size)
{
    if (!discid) return NULL;

    Disc_SetUSB(NULL);
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
        u32 uncSize = NULL;
        u8 * uncompressed_data = uncompressLZ77(soundChunk, soundChunkSize, uncSize);
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
    u8 *out = new (std::nothrow) u8[soundChunkSize];
    if (out)
    {
        memcpy(out, soundChunk, soundChunkSize);
        if (size) *size = soundChunkSize;
    }
    free(opening_bnr);
    return out;
}
