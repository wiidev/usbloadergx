/****************************************************************************
 * Install a channel built by fwdchannel.c into the real NAND through ES.
 * Needs an IOS that accepts fakesigned tickets/TMDs (d2x cIOS does).
 ***************************************************************************/
#include <gccore.h>
#include <ogc/es.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "fwdinstall.h"

#define ALIGN32(x) (((x) + 31) & ~31)

static u8 *aligned_copy(const u8 *src, u32 size)
{
	u8 *p = (u8 *)memalign(32, ALIGN32(size));
	if (p)
		memcpy(p, src, size);
	return p;
}

s32 fwd_install(const fwd_channel *ch, fwd_progress_cb progress, int *failed_step)
{
	u64 tid = 0;
	u32 i, total = 0, done_all = 0;
	s32 ret = -1;
	int step = FWD_STEP_MEMORY;
	u8 *certs = aligned_copy(ch->certs, FWD_CERTS_SIZE);
	u8 *tik = aligned_copy(ch->ticket, FWD_TICKET_SIZE);
	u8 *tmd = aligned_copy(ch->tmd, ch->tmd_size);
	u32 max = 0;
	u8 *enc;

	for (i = 0; i < 8; i++)
		tid = tid << 8 | ch->title_id[i];
	for (i = 0; i < FWD_NUM_CONTENTS; i++)
	{
		total += (ch->content_size[i] + 15) & ~15;
		if (ch->content_size[i] > max)
			max = ch->content_size[i];
	}
	enc = (u8 *)memalign(32, ALIGN32(max + 16));
	if (!certs || !tik || !tmd || !enc)
		goto out;

	step = FWD_STEP_TICKET;
	ret = ES_AddTicket((signed_blob *)tik, FWD_TICKET_SIZE, (signed_blob *)certs, FWD_CERTS_SIZE, NULL, 0);
	if (ret < 0)
		goto out;
	step = FWD_STEP_TITLE;
	ret = ES_AddTitleStart((signed_blob *)tmd, ch->tmd_size, (signed_blob *)certs, FWD_CERTS_SIZE, NULL, 0);
	if (ret < 0)
		goto out;
	for (i = 0; i < FWD_NUM_CONTENTS; i++)
	{
		u32 len = fwd_encrypt_content(ch, i, enc), done;
		s32 cfd;
		step = FWD_STEP_CONTENT;
		cfd = ES_AddContentStart(tid, i);
		if (cfd < 0)
		{
			ret = cfd;
			goto cancel;
		}
		for (done = 0; done < len;)
		{
			u32 chunk = len - done > 0x8000 ? 0x8000 : len - done;
			ret = ES_AddContentData(cfd, enc + done, chunk);
			if (ret < 0)
			{
				ES_AddContentFinish(cfd);
				goto cancel;
			}
			done += chunk;
			done_all += chunk;
			if (progress)
				progress(done_all, total);
		}
		ret = ES_AddContentFinish(cfd);
		if (ret < 0)
			goto cancel;
	}
	step = FWD_STEP_FINISH;
	ret = ES_AddTitleFinish();
	if (ret >= 0)
	{
		step = FWD_STEP_DONE;
		goto out;
	}
cancel:
	ES_AddTitleCancel();
out:
	if (failed_step)
		*failed_step = step;
	free(certs);
	free(tik);
	free(tmd);
	free(enc);
	return ret < 0 ? ret : 0;
}

int fwd_title_installed(const u8 title_id[8])
{
	u64 tid = 0, *titles;
	u32 count = 0, i;
	int found = 0;
	for (i = 0; i < 8; i++)
		tid = tid << 8 | title_id[i];
	if (ES_GetNumTitles(&count) < 0 || count == 0)
		return 0;
	titles = (u64 *)memalign(32, ALIGN32(count * sizeof(u64)));
	if (!titles)
		return 0;
	if (ES_GetTitles(titles, count) >= 0)
		for (i = 0; i < count; i++)
			if (titles[i] == tid)
				found = 1;
	free(titles);
	return found;
}

/* Our shortcut channel: the last content is the forwarder DOL with the ULGXFWD1 block
 * (3 contents with a nandloader, or 2 in the first version). Guards Remove against a
 * different title with the same ID and lets the game list hide shortcuts. */
int fwd_is_forwarder_channel(const u8 title_id[8])
{
	u64 tid = 0;
	u32 size = 0, i;
	signed_blob *s_tmd;
	char path[64];
	int ok = 0;
	for (i = 0; i < 8; i++)
		tid = tid << 8 | title_id[i];
	if (ES_GetStoredTMDSize(tid, &size) < 0 || size < 0x1E4 || size > 0x4000)
		return 0;
	s_tmd = (signed_blob *)memalign(32, ALIGN32(size));
	if (!s_tmd)
		return 0;
	if (ES_GetStoredTMD(tid, s_tmd, size) >= 0)
	{
		const u8 *t = (const u8 *)s_tmd;
		u16 n = t[0x1DE] << 8 | t[0x1DF], boot = t[0x1E0] << 8 | t[0x1E1];
		if ((n == 2 || n == 3) && boot == 1 && (u32)0x1E4 + n * 36 <= size)
		{
			const u8 *r = t + 0x1E4 + (n - 1) * 36;
			u32 csize = r[12] << 24 | r[13] << 16 | r[14] << 8 | r[15];
			snprintf(path, sizeof(path), "/title/%08x/%08x/content/%02x%02x%02x%02x.app",
			         (unsigned int)(tid >> 32), (unsigned int)tid, r[0], r[1], r[2], r[3]);
			if (csize > 0 && csize < 0x200000)
			{
				s32 fd = ISFS_Open(path, ISFS_OPEN_READ);
				u8 *dol = (u8 *)memalign(32, ALIGN32(csize));
				if (fd >= 0 && dol && ISFS_Read(fd, dol, csize) == (s32)csize)
					for (i = 0; i + 8 <= csize && !ok; i += 4) /* the block is 32-byte aligned */
						ok = memcmp(dol + i, "ULGXFWD1", 8) == 0;
				free(dol);
				if (fd >= 0)
					ISFS_Close(fd);
			}
		}
	}
	free(s_tmd);
	return ok;
}

/* Delete the channel and its tickets. Game saves (00010000-XXXX) are not touched. */
s32 fwd_uninstall(const u8 title_id[8])
{
	u64 tid = 0;
	u32 count = 0, i;
	s32 ret, tret = 0;
	tikview *views;
	for (i = 0; i < 8; i++)
		tid = tid << 8 | title_id[i];
	ret = ES_DeleteTitleContent(tid);
	if (ret >= 0)
		ret = ES_DeleteTitle(tid);
	if (ES_GetNumTicketViews(tid, &count) >= 0 && count > 0)
	{
		views = (tikview *)memalign(32, ALIGN32(sizeof(tikview) * count));
		if (views && ES_GetTicketViews(tid, views, count) >= 0)
			for (i = 0; i < count; i++)
				if ((tret = ES_DeleteTicket(&views[i])) < 0)
					break;
		free(views);
	}
	return ret < 0 ? ret : tret;
}

const char *fwd_step_text(int step)
{
	switch (step)
	{
	case FWD_STEP_MEMORY: return "memory";
	case FWD_STEP_TICKET: return "ticket";
	case FWD_STEP_TITLE: return "TMD";
	case FWD_STEP_CONTENT: return "contents";
	case FWD_STEP_FINISH: return "finish";
	case FWD_STEP_DONE: return "done";
	}
	return "?";
}

/* The boot content (nandloader) of the installed UNEO forwarder channel. */
int fwd_read_nandloader(u8 **out, u32 *size)
{
	const u64 uneo = 0x00010001554E454FULL;
	u32 tmd_size = 0, i;
	signed_blob *s_tmd;
	char path[64];
	s32 fd, ret = -1;
	fstats *st = NULL;
	*out = NULL;
	if (ES_GetStoredTMDSize(uneo, &tmd_size) < 0 || tmd_size < 0x1E4 || tmd_size > 0x4000)
		return -1;
	s_tmd = (signed_blob *)memalign(32, ALIGN32(tmd_size));
	if (!s_tmd)
		return -1;
	if (ES_GetStoredTMD(uneo, s_tmd, tmd_size) >= 0)
	{
		const u8 *t = (const u8 *)s_tmd;
		u16 n = t[0x1DE] << 8 | t[0x1DF], boot = t[0x1E0] << 8 | t[0x1E1];
		/* only the 3-content UNEO has a separate nandloader as boot content */
		if (n == 3 && boot == 1)
			for (i = 0; i < n; i++)
			{
				const u8 *r = t + 0x1E4 + i * 36;
				if ((r[4] << 8 | r[5]) == boot)
				{
					snprintf(path, sizeof(path), "/title/00010001/554e454f/content/%02x%02x%02x%02x.app", r[0], r[1], r[2], r[3]);
					ret = 0;
				}
			}
	}
	free(s_tmd);
	if (ret < 0)
		return -2;
	fd = ISFS_Open(path, ISFS_OPEN_READ);
	if (fd < 0)
		return fd;
	st = (fstats *)memalign(32, ALIGN32(sizeof(fstats)));
	ret = -3;
	if (st && ISFS_GetFileStats(fd, st) >= 0 && st->file_length > 0x100)
	{
		*out = (u8 *)memalign(32, ALIGN32(st->file_length));
		if (*out && ISFS_Read(fd, *out, st->file_length) == (s32)st->file_length)
		{
			*size = st->file_length;
			ret = 0;
		}
		else
		{
			free(*out);
			*out = NULL;
		}
	}
	free(st);
	ISFS_Close(fd);
	return ret;
}
