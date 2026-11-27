/****************************************************************************
 * Per-game forwarder channels, see fwdchannel.h.
 * Formats: wiibrew.org/wiki/WAD_files, /Ticket, /Title_metadata, /IMET, /U8_archive.
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fwdchannel.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define ALIGN(n, a) (((n) + (a) - 1) / (a) * (a))

enum
{
	FWD_OK = 0,
	FWD_ERR_NOMEM = -1,
	FWD_ERR_BANNER_HDR = -2,
	FWD_ERR_BANNER_MD5 = -3,
	FWD_ERR_BANNER_U8 = -4,
	FWD_ERR_BANNER_PARTS = -5,
	FWD_ERR_BANNER_LAYOUT = -6,
	FWD_ERR_DOL_CFG = -7,
	FWD_ERR_GAME_ID = -8,
	FWD_ERR_TITLE_ID = -9,
	FWD_ERR_FAKESIGN = -10,
	FWD_ERR_PATH = -11,
	FWD_ERR_NANDLOADER = -12,
	FWD_ERR_GC_BNR = -13,
};

const char *fwd_error_text(int err)
{
	switch (err)
	{
	case FWD_OK: return "OK";
	case FWD_ERR_NOMEM: return "Out of memory";
	case FWD_ERR_BANNER_HDR: return "Banner has no IMET header at 0x40";
	case FWD_ERR_BANNER_MD5: return "Banner IMET checksum is wrong";
	case FWD_ERR_BANNER_U8: return "Banner archive is damaged";
	case FWD_ERR_BANNER_PARTS: return "Banner has no icon.bin, banner.bin or sound.bin";
	case FWD_ERR_BANNER_LAYOUT: return "Banner has no icon.brlyt or banner.brlyt";
	case FWD_ERR_DOL_CFG: return "Forwarder DOL has no ULGXFWD1 block";
	case FWD_ERR_GAME_ID: return "Game ID must be 6 letters or digits";
	case FWD_ERR_TITLE_ID: return "No channel ID rule for this game";
	case FWD_ERR_FAKESIGN: return "Fakesigning failed";
	case FWD_ERR_PATH: return "Loader path is too long";
	case FWD_ERR_NANDLOADER: return "No nandloader (install the UNEO forwarder channel)";
	case FWD_ERR_GC_BNR: return "The game has no GameCube opening.bnr";
	}
	return "Unknown error";
}

static void put16(u8 *p, u16 v) { p[0] = v >> 8; p[1] = v; }
static void put32(u8 *p, u32 v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }
static u32 get32(const u8 *p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3]; }

/* ---------------------------------------------------------------- SHA-1 */

static u32 rol(u32 x, int n) { return x << n | x >> (32 - n); }

static void sha1_block(u32 h[5], const u8 *p)
{
	u32 w[80], a, b, c, d, e, f, k, t;
	int i;
	for (i = 0; i < 16; i++)
		w[i] = get32(p + i * 4);
	for (; i < 80; i++)
		w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
	a = h[0]; b = h[1]; c = h[2]; d = h[3]; e = h[4];
	for (i = 0; i < 80; i++)
	{
		if (i < 20) { f = (b & c) | (~b & d); k = 0x5A827999; }
		else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
		else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
		else { f = b ^ c ^ d; k = 0xCA62C1D6; }
		t = rol(a, 5) + f + e + k + w[i];
		e = d; d = c; c = rol(b, 30); b = a; a = t;
	}
	h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
}

void fwd_sha1(const void *data, u32 len, u8 out[20])
{
	u32 h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
	const u8 *p = data;
	u8 tail[128];
	u32 i, rest = len % 64, tail_len;
	for (i = 0; i + 64 <= len; i += 64)
		sha1_block(h, p + i);
	memset(tail, 0, sizeof(tail));
	memcpy(tail, p + i, rest);
	tail[rest] = 0x80;
	tail_len = rest + 9 <= 64 ? 64 : 128;
	put32(tail + tail_len - 8, (u32)((uint64_t)len >> 29));
	put32(tail + tail_len - 4, len << 3);
	sha1_block(h, tail);
	if (tail_len == 128)
		sha1_block(h, tail + 64);
	for (i = 0; i < 5; i++)
		put32(out + i * 4, h[i]);
}

/* ------------------------------------------------------------------ MD5 */

static const u32 md5_k[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};
static const u8 md5_r[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
	5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
	4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
	6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

static u32 le32(const u8 *p) { return p[0] | p[1] << 8 | p[2] << 16 | (u32)p[3] << 24; }

static void md5_block(u32 h[4], const u8 *p)
{
	u32 w[16], a = h[0], b = h[1], c = h[2], d = h[3], f, g, t;
	int i;
	for (i = 0; i < 16; i++)
		w[i] = le32(p + i * 4);
	for (i = 0; i < 64; i++)
	{
		if (i < 16) { f = (b & c) | (~b & d); g = i; }
		else if (i < 32) { f = (d & b) | (~d & c); g = (5 * i + 1) % 16; }
		else if (i < 48) { f = b ^ c ^ d; g = (3 * i + 5) % 16; }
		else { f = c ^ (b | ~d); g = (7 * i) % 16; }
		t = d; d = c; c = b;
		b = b + rol(a + f + md5_k[i] + w[g], md5_r[i]);
		a = t;
	}
	h[0] += a; h[1] += b; h[2] += c; h[3] += d;
}

void fwd_md5(const void *data, u32 len, u8 out[16])
{
	u32 h[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
	const u8 *p = data;
	u8 tail[128];
	u32 i, j, rest = len % 64, tail_len;
	for (i = 0; i + 64 <= len; i += 64)
		md5_block(h, p + i);
	memset(tail, 0, sizeof(tail));
	memcpy(tail, p + i, rest);
	tail[rest] = 0x80;
	tail_len = rest + 9 <= 64 ? 64 : 128;
	for (j = 0; j < 4; j++)
		tail[tail_len - 8 + j] = (u8)((len << 3) >> (8 * j));
	tail[tail_len - 4] = (u8)(len >> 29);
	md5_block(h, tail);
	if (tail_len == 128)
		md5_block(h, tail + 64);
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			out[i * 4 + j] = (u8)(h[i] >> (8 * j));
}

/* -------------------------------------------------------------- AES-128 */

static u8 aes_sbox[256];
static int aes_ready;

static u8 xtime(u8 x) { return (u8)(x << 1) ^ ((x & 0x80) ? 0x1b : 0); }

static void aes_init(void)
{
	/* S-box from the multiplicative inverse in GF(2^8) and the affine map */
	u8 p = 1, q = 1, x;
	if (aes_ready)
		return;
	do
	{
		p = p ^ (u8)(p << 1) ^ ((p & 0x80) ? 0x1b : 0); /* p *= 3 */
		q ^= q << 1; q ^= q << 2; q ^= q << 4;           /* q /= 3 */
		if (q & 0x80) q ^= 0x09;
		x = q ^ (u8)(q << 1 | q >> 7) ^ (u8)(q << 2 | q >> 6) ^ (u8)(q << 3 | q >> 5) ^ (u8)(q << 4 | q >> 4);
		aes_sbox[p] = x ^ 0x63;
	} while (p != 1);
	aes_sbox[0] = 0x63;
	aes_ready = 1;
}

static void aes_expand(const u8 key[16], u8 rk[176])
{
	u8 rcon = 1, t[4];
	int i;
	memcpy(rk, key, 16);
	for (i = 16; i < 176; i += 4)
	{
		memcpy(t, rk + i - 4, 4);
		if (i % 16 == 0)
		{
			u8 u = t[0];
			t[0] = aes_sbox[t[1]] ^ rcon; t[1] = aes_sbox[t[2]]; t[2] = aes_sbox[t[3]]; t[3] = aes_sbox[u];
			rcon = xtime(rcon);
		}
		rk[i] = rk[i - 16] ^ t[0]; rk[i + 1] = rk[i - 15] ^ t[1];
		rk[i + 2] = rk[i - 14] ^ t[2]; rk[i + 3] = rk[i - 13] ^ t[3];
	}
}

static void aes_block(const u8 rk[176], u8 s[16])
{
	int r, i, c;
	u8 t[16];
	for (i = 0; i < 16; i++)
		s[i] ^= rk[i];
	for (r = 1; r <= 10; r++)
	{
		for (i = 0; i < 16; i++) /* SubBytes + ShiftRows (column-major state) */
			t[i] = aes_sbox[s[(i + 4 * (i % 4)) % 16]];
		if (r != 10)
			for (c = 0; c < 4; c++) /* MixColumns */
			{
				u8 *col = t + 4 * c, a0 = col[0], a1 = col[1], a2 = col[2], a3 = col[3], all = a0 ^ a1 ^ a2 ^ a3;
				col[0] ^= all ^ xtime(a0 ^ a1);
				col[1] ^= all ^ xtime(a1 ^ a2);
				col[2] ^= all ^ xtime(a2 ^ a3);
				col[3] ^= all ^ xtime(a3 ^ a0);
			}
		for (i = 0; i < 16; i++)
			s[i] = t[i] ^ rk[16 * r + i];
	}
}

void fwd_aes_cbc_encrypt(const u8 key[16], const u8 iv[16], const u8 *in, u8 *out, u32 len)
{
	u8 rk[176], chain[16];
	u32 i, j;
	aes_init();
	aes_expand(key, rk);
	memcpy(chain, iv, 16);
	for (i = 0; i + 16 <= len; i += 16)
	{
		for (j = 0; j < 16; j++)
			chain[j] ^= in[i + j];
		aes_block(rk, chain);
		memcpy(out + i, chain, 16);
	}
}

/* ------------------------------------------------------------ LZ77 0x10 */

static u8 *lz77_decompress(const u8 *in, u32 len, u32 *out_len)
{
	u32 size, i = 4, o = 0;
	u8 *out;
	if (len >= 4 && memcmp(in, "LZ77", 4) == 0) { in += 4; len -= 4; }
	if (len < 4 || in[0] != 0x10)
		return NULL;
	size = in[1] | in[2] << 8 | in[3] << 16;
	out = malloc(size ? size : 1);
	if (!out)
		return NULL;
	while (o < size)
	{
		u8 flags;
		int bit;
		if (i >= len) goto bad;
		flags = in[i++];
		for (bit = 0; bit < 8 && o < size; bit++)
		{
			if (flags & (0x80 >> bit))
			{
				u32 n, disp;
				if (i + 1 >= len) goto bad;
				n = (in[i] >> 4) + 3;
				disp = ((in[i] & 0xF) << 8 | in[i + 1]) + 1;
				i += 2;
				if (disp > o) goto bad;
				while (n-- && o < size) { out[o] = out[o - disp]; o++; }
			}
			else
			{
				if (i >= len) goto bad;
				out[o++] = in[i++];
			}
		}
	}
	*out_len = size;
	return out;
bad:
	free(out);
	return NULL;
}

/* Greedy and deterministic, so the same banner always gives the same bytes:
 * candidates are earlier positions with the same 3 bytes, newest first, at most 64
 * of them, within 0x1000 bytes. */
#define LZ_HASH_BITS 15
uint8_t *fwd_lz77_compress(const uint8_t *data, uint32_t n, uint32_t *out_len)
{
	u32 cap = 8 + n + n / 8 + 16, o, i = 0;
	u8 *out = malloc(cap);
	int32_t *head = malloc(sizeof(int32_t) << LZ_HASH_BITS);
	int32_t *prev = malloc(sizeof(int32_t) * (n ? n : 1));
	if (!out || !head || !prev)
	{
		free(out); free(head); free(prev);
		return NULL;
	}
	memset(head, 0xff, sizeof(int32_t) << LZ_HASH_BITS);
	memcpy(out, "LZ77", 4);
	out[4] = 0x10; out[5] = n; out[6] = n >> 8; out[7] = n >> 16;
	o = 8;
#define KEY(p) ((u32)data[p] << 16 | (u32)data[(p) + 1] << 8 | data[(p) + 2])
#define HASH(k) (((k) * 2654435761u) >> (32 - LZ_HASH_BITS))
	while (i < n)
	{
		u32 flag_pos = o++, bit;
		u8 flags = 0;
		for (bit = 0; bit < 8 && i < n; bit++)
		{
			u32 best_len = 0, best_disp = 0, step, k;
			if (i + 3 <= n)
			{
				u32 key = KEY(i), seen = 0;
				int32_t j = head[HASH(key)];
				while (j >= 0 && seen < 64)
				{
					u32 disp = i - (u32)j, len;
					if (disp > 0x1000)
						break;
					if (KEY(j) == key)
					{
						seen++;
						len = 3;
						while (len < 18 && i + len < n && data[j + len] == data[i + len])
							len++;
						if (len > best_len)
						{
							best_len = len;
							best_disp = disp;
							if (len == 18)
								break;
						}
					}
					j = prev[j];
				}
			}
			step = best_len >= 3 ? best_len : 1;
			for (k = i; k < i + step && k + 2 < n; k++)
			{
				u32 h = HASH(KEY(k));
				prev[k] = head[h];
				head[h] = (int32_t)k;
			}
			if (best_len >= 3)
			{
				u32 d = best_disp - 1;
				flags |= 0x80 >> bit;
				out[o++] = (u8)((best_len - 3) << 4 | d >> 8);
				out[o++] = (u8)d;
				i += best_len;
			}
			else
				out[o++] = data[i++];
		}
		out[flag_pos] = flags;
	}
#undef KEY
#undef HASH
	free(head);
	free(prev);
	*out_len = o;
	return out;
}

/* ------------------------------------------------------------------- U8 */

typedef struct
{
	char path[128];
	const u8 *data;
	u32 size;
	u8 *owned; /* replaced data, freed with the list */
} u8_entry;

typedef struct
{
	u8_entry *e;
	int count;
} u8_list;

static void u8_list_free(u8_list *l)
{
	int i;
	for (i = 0; i < l->count; i++)
		free(l->e[i].owned);
	free(l->e);
	l->e = NULL;
	l->count = 0;
}

static int u8_parse(const u8 *arc, u32 len, u8_list *out)
{
	u32 root, total, strings, i;
	struct { char name[64]; u32 end; } stack[16];
	int depth = 0;
	out->e = NULL;
	out->count = 0;
	if (len < 0x20 || get32(arc) != 0x55AA382D)
		return -1;
	root = get32(arc + 4);
	if (root + 12 > len)
		return -1;
	total = get32(arc + root + 8);
	strings = root + total * 12;
	if (total == 0 || strings > len)
		return -1;
	out->e = calloc(total, sizeof(u8_entry));
	if (!out->e)
		return -1;
	for (i = 1; i < total; i++)
	{
		const u8 *node = arc + root + i * 12;
		u32 name_off = get32(node) & 0xFFFFFF, off = get32(node + 4), size = get32(node + 8);
		const char *name = (const char *)arc + strings + name_off;
		int d;
		if (strings + name_off >= len)
			goto bad;
		while (depth && i >= stack[depth - 1].end)
			depth--;
		if (node[0] == 1)
		{
			if (depth >= 16 || strlen(name) >= 64)
				goto bad;
			strcpy(stack[depth].name, name);
			stack[depth].end = size;
			depth++;
		}
		else
		{
			u8_entry *e = &out->e[out->count++];
			if ((uint64_t)off + size > len)
				goto bad;
			e->path[0] = 0;
			for (d = 0; d < depth; d++)
			{
				if (strlen(e->path) + strlen(stack[d].name) + 2 >= sizeof(e->path))
					goto bad;
				strcat(e->path, stack[d].name);
				strcat(e->path, "/");
			}
			if (strlen(e->path) + strlen(name) >= sizeof(e->path))
				goto bad;
			strcat(e->path, name);
			e->data = arc + off;
			e->size = size;
		}
	}
	return 0;
bad:
	u8_list_free(out);
	return -1;
}

static const u8_entry *u8_find(const u8_list *l, const char *path)
{
	int i;
	for (i = 0; i < l->count; i++)
		if (strcmp(l->e[i].path, path) == 0)
			return &l->e[i];
	return NULL;
}

static int ci_cmp(const char *a, const char *b)
{
	for (;; a++, b++)
	{
		int ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a, cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;
		if (ca != cb || !ca)
			return ca - cb;
	}
}

typedef struct
{
	u8 *nodes; u32 node_count, node_cap;
	char *names; u32 names_len, names_cap;
	const u8_list *files;
	u32 *file_order; u32 file_count; /* files in node order, for the data area */
} u8_builder;

static int ub_name(u8_builder *b, const char *name, u32 *off)
{
	u32 n = strlen(name) + 1;
	if (b->names_len + n > b->names_cap)
	{
		u32 cap = (b->names_len + n) * 2;
		char *p = realloc(b->names, cap);
		if (!p) return -1;
		b->names = p; b->names_cap = cap;
	}
	*off = b->names_len;
	memcpy(b->names + b->names_len, name, n);
	b->names_len += n;
	return 0;
}

static int ub_node(u8_builder *b, u32 *idx)
{
	if (b->node_count == b->node_cap)
	{
		u32 cap = b->node_cap ? b->node_cap * 2 : 64;
		u8 *p = realloc(b->nodes, cap * 12);
		if (!p) return -1;
		b->nodes = p; b->node_cap = cap;
	}
	*idx = b->node_count++;
	memset(b->nodes + *idx * 12, 0, 12);
	return 0;
}

/* Children of directory "prefix" (with trailing '/', or "" for the root), sorted case-insensitively */
static int ub_dir(u8_builder *b, const char *prefix, u32 parent)
{
	u32 plen = strlen(prefix), count = 0, i, k;
	const char **names = malloc(sizeof(char *) * (b->files->count + 1));
	u32 *lens = malloc(sizeof(u32) * (b->files->count + 1));
	if (!names || !lens) { free(names); free(lens); return -1; }
	for (i = 0; i < (u32)b->files->count; i++)
	{
		const char *p = b->files->e[i].path, *slash;
		u32 len;
		if (strncmp(p, prefix, plen) != 0)
			continue;
		p += plen;
		slash = strchr(p, '/');
		len = slash ? (u32)(slash - p) : strlen(p);
		for (k = 0; k < count; k++)
			if (lens[k] == len && strncmp(names[k], p, len) == 0)
				break;
		if (k == count) { names[count] = p; lens[count] = len; count++; }
	}
	/* insertion sort keeps the first-seen order for equal keys, like Python's sorted() */
	for (i = 1; i < count; i++)
	{
		const char *n = names[i];
		u32 l = lens[i];
		char a[128], c[128];
		memcpy(a, n, l); a[l] = 0;
		for (k = i; k > 0; k--)
		{
			memcpy(c, names[k - 1], lens[k - 1]); c[lens[k - 1]] = 0;
			if (ci_cmp(c, a) <= 0) break;
			names[k] = names[k - 1]; lens[k] = lens[k - 1];
		}
		names[k] = n; lens[k] = l;
	}
	for (i = 0; i < count; i++)
	{
		char name[128], sub[256];
		u32 idx, off;
		int is_file = -1;
		memcpy(name, names[i], lens[i]); name[lens[i]] = 0;
		snprintf(sub, sizeof(sub), "%s%s", prefix, name);
		for (k = 0; k < (u32)b->files->count; k++)
			if (strcmp(b->files->e[k].path, sub) == 0) { is_file = k; break; }
		if (ub_name(b, name, &off) < 0 || ub_node(b, &idx) < 0) goto fail;
		if (is_file >= 0)
		{
			u8 *nd = b->nodes + idx * 12;
			put32(nd, off);
			put32(nd + 8, b->files->e[is_file].size); /* data offset filled in later */
			b->file_order[b->file_count++] = is_file;
		}
		else
		{
			u8 *nd;
			snprintf(sub, sizeof(sub), "%s%s/", prefix, name);
			nd = b->nodes + idx * 12;
			put32(nd, 0x01000000 | off);
			put32(nd + 4, parent);
			if (ub_dir(b, sub, idx) < 0) goto fail;
			put32(b->nodes + idx * 12 + 8, b->node_count);
		}
	}
	free(names); free(lens);
	return 0;
fail:
	free(names); free(lens);
	return -1;
}

static u8 *u8_build(const u8_list *files, u32 *out_len)
{
	u8_builder b;
	u32 root, header_size, data_off, total, i, n, pos;
	u8 *out = NULL;
	memset(&b, 0, sizeof(b));
	b.files = files;
	b.file_order = malloc(sizeof(u32) * (files->count + 1));
	if (!b.file_order || ub_name(&b, "", &root) < 0 || ub_node(&b, &root) < 0)
		goto done;
	put32(b.nodes, 0x01000000);
	if (ub_dir(&b, "", 0) < 0)
		goto done;
	put32(b.nodes + 8, b.node_count);
	header_size = b.node_count * 12 + b.names_len;
	data_off = ALIGN(0x20 + header_size, 0x20);
	total = data_off;
	for (i = 0; i < b.file_count; i++)
		total += ALIGN(files->e[b.file_order[i]].size, 0x20);
	out = calloc(1, total ? total : 1);
	if (!out)
		goto done;
	put32(out, 0x55AA382D); put32(out + 4, 0x20); put32(out + 8, header_size); put32(out + 12, data_off);
	/* data offsets: files in node order */
	pos = data_off;
	n = 0;
	for (i = 0; i < b.node_count; i++)
	{
		u8 *nd = b.nodes + i * 12;
		if (nd[0] == 0 && i != 0)
		{
			const u8_entry *e = &files->e[b.file_order[n++]];
			put32(nd + 4, pos);
			memcpy(out + pos, e->data, e->size);
			pos += ALIGN(e->size, 0x20);
		}
	}
	memcpy(out + 0x20, b.nodes, b.node_count * 12);
	memcpy(out + 0x20 + b.node_count * 12, b.names, b.names_len);
	*out_len = total;
done:
	free(b.nodes); free(b.names); free(b.file_order);
	return out;
}

/* --------------------------------------------------------------- banner */

#define IMET_OFFSET 0x40
#define IMET_HASHED 0x600
#define IMET_MD5 0x5F0

static int imet_md5_ok(const u8 *bnr)
{
	u8 hdr[IMET_HASHED], md5[16];
	memcpy(hdr, bnr, IMET_HASHED);
	memset(hdr + IMET_MD5, 0, 16);
	fwd_md5(hdr, IMET_HASHED, md5);
	return memcmp(md5, bnr + IMET_MD5, 16) == 0;
}

/* icon.bin / banner.bin: IMD5 header, then LZ77 or plain U8. Returns a malloc'd U8. */
static u8 *banner_part(const u8 *p, u32 len, u32 *out_len)
{
	u8 *out;
	if (len >= 0x20 && memcmp(p, "IMD5", 4) == 0) { p += 0x20; len -= 0x20; }
	if ((len >= 4 && memcmp(p, "LZ77", 4) == 0) || (len >= 1 && p[0] == 0x10))
		return lz77_decompress(p, len, out_len);
	out = malloc(len ? len : 1);
	if (out) { memcpy(out, p, len); *out_len = len; }
	return out;
}

/* arc/blyt/Icon.brlyt -> arc/blyt/icon.brlyt, arc/anim/Banner_Start.brlan -> arc/anim/banner_Start.brlan */
static int layout_rename(char *path)
{
	static const char *dirs[] = {"arc/blyt/", "arc/anim/"};
	static const char *wants[] = {"icon", "banner"};
	int d, w;
	for (d = 0; d < 2; d++)
	{
		char *name;
		size_t dl = strlen(dirs[d]);
		if (strncmp(path, dirs[d], dl) != 0 || strchr(path + dl, '/'))
			continue;
		name = path + dl;
		for (w = 0; w < 2; w++)
		{
			size_t wl = strlen(wants[w]);
			size_t k;
			int lower_match = strlen(name) >= wl, exact = strncmp(name, wants[w], wl) == 0;
			for (k = 0; lower_match && k < wl; k++)
			{
				char c = name[k];
				if (c >= 'A' && c <= 'Z') c += 32;
				if (c != wants[w][k]) lower_match = 0;
			}
			if (lower_match && !exact)
			{
				memcpy(name, wants[w], wl);
				return 1;
			}
		}
	}
	return 0;
}

static int part_has_layout(const u8 *part, u32 len, const char *layout)
{
	u32 ulen;
	u8 *u = banner_part(part, len, &ulen);
	u8_list l;
	int ok = 0;
	if (!u) return 0;
	if (u8_parse(u, ulen, &l) == 0)
	{
		ok = u8_find(&l, layout) != NULL;
		u8_list_free(&l);
	}
	free(u);
	return ok;
}

int fwd_check_banner(const uint8_t *bnr, uint32_t len)
{
	u8_list outer;
	const u8_entry *icon, *banner, *sound;
	int i, ret = FWD_OK;
	if (len < IMET_HASHED + 0x20 || memcmp(bnr + IMET_OFFSET, "IMET", 4) != 0)
		return FWD_ERR_BANNER_HDR;
	for (i = 0; i < IMET_OFFSET; i++)
		if (bnr[i]) return FWD_ERR_BANNER_HDR;
	if (!imet_md5_ok(bnr))
		return FWD_ERR_BANNER_MD5;
	if (u8_parse(bnr + IMET_HASHED, len - IMET_HASHED, &outer) < 0)
		return FWD_ERR_BANNER_U8;
	icon = u8_find(&outer, "meta/icon.bin");
	banner = u8_find(&outer, "meta/banner.bin");
	sound = u8_find(&outer, "meta/sound.bin");
	if (!icon || !banner || !sound)
		ret = FWD_ERR_BANNER_PARTS;
	else if (!part_has_layout(icon->data, icon->size, "arc/blyt/icon.brlyt")
	         || !part_has_layout(banner->data, banner->size, "arc/blyt/banner.brlyt"))
		ret = FWD_ERR_BANNER_LAYOUT;
	u8_list_free(&outer);
	return ret;
}

uint8_t *fwd_fix_banner(const uint8_t *bnr, uint32_t len, uint32_t *out_len, int *changed)
{
	static const char *parts[2] = {"meta/icon.bin", "meta/banner.bin"};
	static const u32 size_fields[2] = {0x4C, 0x50};
	u8_list outer;
	u8 header[IMET_HASHED];
	u8 *result = NULL;
	int p, any = 0;
	*changed = 0;
	if (len < IMET_HASHED || u8_parse(bnr + IMET_HASHED, len - IMET_HASHED, &outer) < 0)
		return NULL;
	memcpy(header, bnr, IMET_HASHED);
	for (p = 0; p < 2; p++)
	{
		u8_entry *e = NULL;
		u8_list inner;
		u32 ulen, rawlen, zlen, i;
		u8 *u, *raw, *z, *wrapped;
		int renamed = 0;
		for (i = 0; i < (u32)outer.count; i++)
			if (strcmp(outer.e[i].path, parts[p]) == 0) e = &outer.e[i];
		if (!e)
			continue;
		u = banner_part(e->data, e->size, &ulen);
		if (!u || u8_parse(u, ulen, &inner) < 0) { free(u); goto fail; }
		for (i = 0; i < (u32)inner.count; i++)
			renamed |= layout_rename(inner.e[i].path);
		if (!renamed) { u8_list_free(&inner); free(u); continue; }
		raw = u8_build(&inner, &rawlen);
		u8_list_free(&inner);
		free(u);
		if (!raw) goto fail;
		z = fwd_lz77_compress(raw, rawlen, &zlen);
		free(raw);
		if (!z) goto fail;
		wrapped = malloc(zlen + 0x20);
		if (!wrapped) { free(z); goto fail; }
		memcpy(wrapped, "IMD5", 4);
		put32(wrapped + 4, zlen);
		memset(wrapped + 8, 0, 8);
		fwd_md5(z, zlen, wrapped + 16);
		memcpy(wrapped + 0x20, z, zlen);
		free(z);
		free(e->owned);
		e->owned = wrapped;
		e->data = wrapped;
		e->size = zlen + 0x20;
		put32(header + size_fields[p], rawlen);
		any = 1;
	}
	if (!any)
	{
		u8_list_free(&outer);
		result = malloc(len);
		if (result) { memcpy(result, bnr, len); *out_len = len; }
		return result;
	}
	{
		u32 arclen;
		u8 *arc = u8_build(&outer, &arclen);
		if (!arc) goto fail;
		memset(header + IMET_MD5, 0, 16);
		fwd_md5(header, IMET_HASHED, header + IMET_MD5);
		result = malloc(IMET_HASHED + arclen);
		if (result)
		{
			memcpy(result, header, IMET_HASHED);
			memcpy(result + IMET_HASHED, arc, arclen);
			*out_len = IMET_HASHED + arclen;
			*changed = 1;
		}
		free(arc);
	}
fail:
	u8_list_free(&outer);
	return result;
}

/* ------------------------------------------------------ GameCube banner */

#define GC_BNR_IMAGE 0x20
#define GC_IMAGE_W 96
#define GC_IMAGE_H 32

static u16 get16(const u8 *p) { return (u16)(p[0] << 8 | p[1]); }

static size_t u16len(const u16 *s)
{
	size_t n = 0;
	while (s && s[n])
		n++;
	return n;
}

/* One-picture TPL: header and filters of the template, new size, format and pixels */
static u8 *tpl_replace(const u8 *template_tpl, u32 w, u32 h, u32 fmt, const u8 *pixels, u32 pixels_size, u32 *out_len)
{
	u8 *t = malloc(0x40 + pixels_size);
	u32 img;
	if (!t)
		return NULL;
	memcpy(t, template_tpl, 0x40);
	img = get32(t + 0x0C);
	put16(t + img, (u16)h);
	put16(t + img + 2, (u16)w);
	put32(t + img + 4, fmt);
	put32(t + img + 8, 0x40);
	put32(t + 0x10, 0); /* no palette */
	memcpy(t + 0x40, pixels, pixels_size);
	*out_len = 0x40 + pixels_size;
	return t;
}

static void u8_set(u8_list *l, const char *path, u8 *data, u32 size)
{
	int i;
	for (i = 0; i < l->count; i++)
		if (strcmp(l->e[i].path, path) == 0)
		{
			free(l->e[i].owned);
			l->e[i].owned = data;
			l->e[i].data = data;
			l->e[i].size = size;
			return;
		}
	free(data);
}

typedef struct { const char *pane; const u16 *text; } brlyt_text;

/* Set txt1 texts (within their buffers) and hide panes by name */
static u8 *brlyt_edit(const u8 *layout, u32 len, const brlyt_text *texts, int ntexts, const char *const *hide, int nhide)
{
	u8 *L = malloc(len);
	u32 o, n, k;
	if (!L)
		return NULL;
	memcpy(L, layout, len);
	o = get16(L + 0x0C);
	n = get16(L + 0x0E);
	for (k = 0; k < n && o + 8 <= len; k++)
	{
		u32 size = get32(L + o + 4);
		if (!memcmp(L + o, "pan1", 4) || !memcmp(L + o, "pic1", 4) || !memcmp(L + o, "txt1", 4)
		    || !memcmp(L + o, "bnd1", 4) || !memcmp(L + o, "wnd1", 4))
		{
			char name[17];
			int i;
			memcpy(name, L + o + 0x0C, 16);
			name[16] = 0;
			for (i = 0; i < nhide; i++)
				if (!strcmp(name, hide[i]))
					L[o + 8] &= (u8)~1;
			if (!memcmp(L + o, "txt1", 4))
				for (i = 0; i < ntexts; i++)
					if (!strcmp(name, texts[i].pane))
					{
						u32 buf = get16(L + o + 0x4C), stroff = get32(L + o + 0x58), chars, c;
						size_t tl = u16len(texts[i].text);
						chars = buf / 2 - 1;
						if (tl < chars)
							chars = (u32)tl;
						memset(L + o + stroff, 0, buf);
						for (c = 0; c < chars; c++)
							put16(L + o + stroff + c * 2, texts[i].text[c]);
						put16(L + o + 0x4E, (u16)((chars + 1) * 2));
					}
		}
		o += size;
	}
	return L;
}

/* Rebuild an icon.bin/banner.bin (IMD5 + LZ77 U8) from a file list; returns the raw U8 size */
static u8 *part_wrap(const u8_list *files, u32 *wrapped_len, u32 *raw_len)
{
	u32 zlen;
	u8 *raw = u8_build(files, raw_len), *z, *w;
	if (!raw)
		return NULL;
	z = fwd_lz77_compress(raw, *raw_len, &zlen);
	free(raw);
	if (!z)
		return NULL;
	w = malloc(zlen + 0x20);
	if (w)
	{
		memcpy(w, "IMD5", 4);
		put32(w + 4, zlen);
		memset(w + 8, 0, 8);
		fwd_md5(z, zlen, w + 16);
		memcpy(w + 0x20, z, zlen);
		*wrapped_len = zlen + 0x20;
	}
	free(z);
	return w;
}

int fwd_gc_banner(const uint8_t *template_bnr, uint32_t template_size, const uint8_t *gc_bnr, uint32_t gc_size,
                  const uint16_t *title, const uint16_t *developer, const uint16_t *description,
                  const uint8_t *logo_tpl, uint32_t logo_size, const uint8_t *bg_tpl, uint32_t bg_size,
                  uint8_t **out, uint32_t *out_size)
{
	static const u16 s_gamecube[] = {'G', 'a', 'm', 'e', 'C', 'u', 'b', 'e', 0};
	static const u16 s_developer[] = {'D', 'e', 'v', 'e', 'l', 'o', 'p', 'e', 'r', ':', 0};
	static const u16 s_space[] = {' ', 0};
	static const char *const hide[] = {"Line1", "Line2", "T_Released", "T_release_date", "T_versiontext", "T_version"};
	const u8 *tpl, *image = gc_bnr + GC_BNR_IMAGE;
	const u32 image_size = GC_IMAGE_W * GC_IMAGE_H * 2;
	u8 header[IMET_HASHED];
	u8_list outer, icon, banner;
	u8 *uicon = NULL, *ubanner = NULL;
	u32 ulen, i, lang;
	int ret = FWD_ERR_BANNER_U8;
	size_t tl;

	*out = NULL;
	if (template_size < 0x40 + IMET_HASHED || memcmp(template_bnr + 0x80, "IMET", 4) != 0)
		return FWD_ERR_BANNER_HDR;
	if (gc_size < GC_BNR_IMAGE + image_size || (memcmp(gc_bnr, "BNR1", 4) && memcmp(gc_bnr, "BNR2", 4)))
		return FWD_ERR_GC_BNR;
	tpl = template_bnr + 0x40;
	memcpy(header, tpl, IMET_HASHED);
	if (u8_parse(tpl + IMET_HASHED, template_size - 0x40 - IMET_HASHED, &outer) < 0)
		return FWD_ERR_BANNER_U8;
	memset(&icon, 0, sizeof(icon));
	memset(&banner, 0, sizeof(banner));
	{
		const u8_entry *ei = u8_find(&outer, "meta/icon.bin"), *eb = u8_find(&outer, "meta/banner.bin");
		if (!ei || !eb)
			goto done;
		uicon = banner_part(ei->data, ei->size, &ulen);
		if (!uicon || u8_parse(uicon, ulen, &icon) < 0)
			goto done;
		ubanner = banner_part(eb->data, eb->size, &ulen);
		if (!ubanner || u8_parse(ubanner, ulen, &banner) < 0)
			goto done;
	}

	/* icon: game picture + GameCube logo */
	{
		const u8_entry *e = u8_find(&icon, "arc/timg/Iconpng.tpl");
		u8 *t, *logo;
		u32 tl2;
		if (!e) goto done;
		t = tpl_replace(e->data, GC_IMAGE_W, GC_IMAGE_H, 5, image, image_size, &tl2);
		logo = malloc(logo_size);
		if (!t || !logo) { free(t); free(logo); ret = FWD_ERR_NOMEM; goto done; }
		memcpy(logo, logo_tpl, logo_size);
		u8_set(&icon, "arc/timg/Iconpng.tpl", t, tl2);
		u8_set(&icon, "arc/timg/HBLogo.tpl", logo, logo_size);
	}
	/* banner: game picture, GameCube background, texts */
	{
		const u8_entry *e = u8_find(&banner, "arc/timg/HBPic.tpl"), *lay = u8_find(&banner, "arc/blyt/banner.brlyt");
		brlyt_text texts[5];
		u8 *t, *bg, *L;
		u32 tl2;
		if (!e || !lay) goto done;
		texts[0].pane = "T_PF"; texts[0].text = s_gamecube;
		texts[1].pane = "T_Coded_by"; texts[1].text = s_developer;
		texts[2].pane = "T_coder"; texts[2].text = developer;
		texts[3].pane = "T_name"; texts[3].text = title;
		texts[4].pane = "T_short_descript"; texts[4].text = (description && description[0]) ? description : s_space;
		L = brlyt_edit(lay->data, lay->size, texts, 5, hide, 6);
		t = tpl_replace(e->data, GC_IMAGE_W, GC_IMAGE_H, 5, image, image_size, &tl2);
		bg = malloc(bg_size);
		if (!L || !t || !bg) { free(L); free(t); free(bg); ret = FWD_ERR_NOMEM; goto done; }
		memcpy(bg, bg_tpl, bg_size);
		u8_set(&banner, "arc/blyt/banner.brlyt", L, lay->size);
		u8_set(&banner, "arc/timg/HBPic.tpl", t, tl2);
		u8_set(&banner, "arc/timg/bg.tpl", bg, bg_size);
	}
	/* repack icon.bin / banner.bin, IMET sizes and names */
	{
		u32 wl, rl;
		u8 *w = part_wrap(&icon, &wl, &rl);
		if (!w) { ret = FWD_ERR_NOMEM; goto done; }
		u8_set(&outer, "meta/icon.bin", w, wl);
		put32(header + 0x4C, rl);
		w = part_wrap(&banner, &wl, &rl);
		if (!w) { ret = FWD_ERR_NOMEM; goto done; }
		u8_set(&outer, "meta/banner.bin", w, wl);
		put32(header + 0x50, rl);
	}
	tl = u16len(title);
	if (tl > 41)
		tl = 41;
	for (lang = 0; lang < 10; lang++)
	{
		u8 *p = header + IMET_OFFSET + 0x1C + lang * 84;
		memset(p, 0, 84);
		for (i = 0; i < tl; i++)
			put16(p + i * 2, title[i]);
	}
	memset(header + IMET_MD5, 0, 16);
	fwd_md5(header, IMET_HASHED, header + IMET_MD5);
	{
		u32 arclen;
		u8 *arc = u8_build(&outer, &arclen);
		if (!arc) { ret = FWD_ERR_NOMEM; goto done; }
		*out = malloc(IMET_HASHED + arclen);
		if (*out)
		{
			memcpy(*out, header, IMET_HASHED);
			memcpy(*out + IMET_HASHED, arc, arclen);
			*out_size = IMET_HASHED + arclen;
			ret = FWD_OK;
		}
		else
			ret = FWD_ERR_NOMEM;
		free(arc);
	}
done:
	u8_list_free(&icon);
	u8_list_free(&banner);
	u8_list_free(&outer);
	free(uicon);
	free(ubanner);
	return ret;
}

/* ------------------------------------------------------- ticket and TMD */

#define SIG_BODY 0x140
#define TIK_ENC_KEY 0x1BF
#define TIK_TITLE_ID 0x1DC
#define TIK_KEY_INDEX 0x1F1
#define TIK_FAKESIGN 0x1F2
#define TMD_TITLE_ID 0x18C
#define TMD_TITLE_VERSION 0x1DC
#define TMD_NUM_CONTENTS 0x1DE
#define TMD_BOOT_INDEX 0x1E0
#define TMD_FAKESIGN 0x1E2
#define TMD_CONTENTS 0x1E4

static int fakesign(u8 *blob, u32 len, u32 field)
{
	u32 n;
	u8 h[20];
	memset(blob + 4, 0, 0x100);
	for (n = 0; n < 0x10000; n++)
	{
		put16(blob + field, (u16)n);
		fwd_sha1(blob + SIG_BODY, len - SIG_BODY, h);
		if (h[0] == 0)
			return 0;
	}
	return FWD_ERR_FAKESIGN;
}

int fwd_default_title_id(const char *game_id, char out4[5])
{
	char first;
	if (!game_id || strlen(game_id) < 4)
		return FWD_ERR_GAME_ID;
	if (game_id[0] == 'R') first = 'T';
	else if (game_id[0] == 'S') first = 'K';
	else if (game_id[0] == 'G') first = 'Y';
	else if (game_id[0] == 'D') first = 'Z';
	else return FWD_ERR_TITLE_ID;
	out4[0] = first;
	memcpy(out4 + 1, game_id + 1, 3);
	out4[4] = 0;
	return FWD_OK;
}

static int is_alnum_upper(char c) { return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'); }

#define CFG_MAGIC "ULGXFWD1"
#define CFG_PATH 8
#define CFG_PATH_LEN 120
#define CFG_ARG1 128
#define CFG_ARG_LEN 64

int fwd_build(fwd_channel *ch, const uint8_t *tpl, const uint8_t *nandloader, uint32_t nandloader_size,
              const uint8_t *dol, uint32_t dol_size,
              const uint8_t *bnr, uint32_t bnr_size, const char *game_id, const char *title4,
              const char *loader_path, const uint8_t common_key[16])
{
	u8 iv[16], h[20], seed[18 + 8];
	u32 i, cfg = 0, found = 0;
	int ret;
	memset(ch, 0, sizeof(*ch));
	if (!game_id || strlen(game_id) != 6)
		return FWD_ERR_GAME_ID;
	for (i = 0; i < 6; i++)
		if (!is_alnum_upper(game_id[i])) return FWD_ERR_GAME_ID;
	if (!title4 || strlen(title4) != 4)
		return FWD_ERR_TITLE_ID;
	for (i = 0; i < 4; i++)
		if (!is_alnum_upper(title4[i])) return FWD_ERR_TITLE_ID;
	if (!loader_path || loader_path[0] != '/' || strlen(loader_path) >= CFG_PATH_LEN)
		return FWD_ERR_PATH;

	/* banner */
	ch->content[0] = fwd_fix_banner(bnr, bnr_size, &ch->content_size[0], &ch->banner_fixed);
	if (!ch->content[0]) { ret = FWD_ERR_BANNER_U8; goto fail; }
	if ((ret = fwd_check_banner(ch->content[0], ch->content_size[0])) < 0)
		goto fail;

	/* forwarder DOL with the config block */
	for (i = 0; i + 256 <= dol_size; i++)
		if (memcmp(dol + i, CFG_MAGIC, 8) == 0) { cfg = i; found++; }
	if (found != 1) { ret = FWD_ERR_DOL_CFG; goto fail; }
	ch->content[2] = malloc(dol_size);
	if (!ch->content[2]) { ret = FWD_ERR_NOMEM; goto fail; }
	memcpy(ch->content[2], dol, dol_size);
	memset(ch->content[2] + cfg, 0, 256);
	memcpy(ch->content[2] + cfg, CFG_MAGIC, 8);
	memcpy(ch->content[2] + cfg + CFG_PATH, loader_path, strlen(loader_path));
	memcpy(ch->content[2] + cfg + CFG_ARG1, game_id, 6);
	ch->content_size[2] = dol_size;

	/* nandloader, the boot content */
	if (!nandloader || nandloader_size < 0x100) { ret = FWD_ERR_NANDLOADER; goto fail; }
	ch->content[1] = malloc(nandloader_size);
	if (!ch->content[1]) { ret = FWD_ERR_NOMEM; goto fail; }
	memcpy(ch->content[1], nandloader, nandloader_size);
	ch->content_size[1] = nandloader_size;

	/* title ID and a title key derived from it, so rebuilding a channel gives the same WAD */
	put32(ch->title_id, 0x00010001);
	memcpy(ch->title_id + 4, title4, 4);
	memcpy(seed, "wii-repo-forwarder", 18);
	memcpy(seed + 18, ch->title_id, 8);
	fwd_sha1(seed, sizeof(seed), h);
	memcpy(ch->title_key, h, 16);

	ch->certs = malloc(FWD_CERTS_SIZE);
	ch->ticket = malloc(FWD_TICKET_SIZE);
	ch->tmd_size = TMD_CONTENTS + FWD_NUM_CONTENTS * 36;
	ch->tmd = malloc(ch->tmd_size);
	if (!ch->certs || !ch->ticket || !ch->tmd) { ret = FWD_ERR_NOMEM; goto fail; }
	memcpy(ch->certs, tpl, FWD_CERTS_SIZE);

	/* ticket */
	memcpy(ch->ticket, tpl + FWD_CERTS_SIZE, FWD_TICKET_SIZE);
	memcpy(ch->ticket + TIK_TITLE_ID, ch->title_id, 8);
	memset(iv, 0, 16);
	memcpy(iv, ch->title_id, 8);
	fwd_aes_cbc_encrypt(common_key, iv, ch->title_key, ch->ticket + TIK_ENC_KEY, 16);
	ch->ticket[TIK_KEY_INDEX] = 0;
	memset(ch->ticket + 0x1D0, 0, 12); /* ticket ID, console ID */
	put16(ch->ticket + 0x1E6, 0);      /* title version */
	memset(ch->ticket + 0x262, 0, 2);  /* padding; junk in some UNEO builds */
	if ((ret = fakesign(ch->ticket, FWD_TICKET_SIZE, TIK_FAKESIGN)) < 0)
		goto fail;

	/* TMD: header fields (IOS 58, AHB access) from the template */
	memcpy(ch->tmd, tpl + FWD_CERTS_SIZE + FWD_TICKET_SIZE, TMD_CONTENTS);
	memcpy(ch->tmd + TMD_TITLE_ID, ch->title_id, 8);
	put16(ch->tmd + TMD_TITLE_VERSION, 0);
	memset(ch->tmd + 0x19A, 0, 2);     /* zero after the group ID; junk in some UNEO builds */
	put16(ch->tmd + TMD_NUM_CONTENTS, FWD_NUM_CONTENTS);
	put16(ch->tmd + TMD_BOOT_INDEX, 1);
	for (i = 0; i < FWD_NUM_CONTENTS; i++)
	{
		u8 *r = ch->tmd + TMD_CONTENTS + i * 36;
		put32(r, i);
		put16(r + 4, i);
		put16(r + 6, 1);
		put32(r + 8, 0);
		put32(r + 12, ch->content_size[i]);
		fwd_sha1(ch->content[i], ch->content_size[i], r + 16);
	}
	if ((ret = fakesign(ch->tmd, ch->tmd_size, TMD_FAKESIGN)) < 0)
		goto fail;
	return FWD_OK;
fail:
	fwd_free(ch);
	return ret;
}

uint32_t fwd_encrypt_content(const fwd_channel *ch, int i, uint8_t *out)
{
	u8 iv[16];
	u32 size = ch->content_size[i], padded = ALIGN(size, 16);
	memset(iv, 0, 16);
	put16(iv, (u16)i);
	memcpy(out, ch->content[i], size);
	memset(out + size, 0, padded - size);
	fwd_aes_cbc_encrypt(ch->title_key, iv, out, out, padded);
	return padded;
}

int fwd_make_wad(const fwd_channel *ch, uint8_t **out, uint32_t *out_size)
{
	u32 enc[FWD_NUM_CONTENTS], data_len = 0, i;
	for (i = 0; i < FWD_NUM_CONTENTS; i++)
	{
		enc[i] = ALIGN(ch->content_size[i], 16);
		data_len = (i ? ALIGN(data_len, 0x40) : 0) + enc[i];
	}
	u32 total = 0x40 + ALIGN(FWD_CERTS_SIZE, 0x40) + ALIGN(FWD_TICKET_SIZE, 0x40) + ALIGN(ch->tmd_size, 0x40) + ALIGN(data_len, 0x40);
	u8 *w = calloc(1, total), *p;
	if (!w)
		return FWD_ERR_NOMEM;
	put32(w, 0x20);
	memcpy(w + 4, "Is\0\0", 4);
	put32(w + 8, FWD_CERTS_SIZE);
	put32(w + 12, 0);
	put32(w + 16, FWD_TICKET_SIZE);
	put32(w + 20, ch->tmd_size);
	put32(w + 24, data_len);
	put32(w + 28, 0);
	p = w + 0x40;
	memcpy(p, ch->certs, FWD_CERTS_SIZE); p += ALIGN(FWD_CERTS_SIZE, 0x40);
	memcpy(p, ch->ticket, FWD_TICKET_SIZE); p += ALIGN(FWD_TICKET_SIZE, 0x40);
	memcpy(p, ch->tmd, ch->tmd_size); p += ALIGN(ch->tmd_size, 0x40);
	for (i = 0; i < FWD_NUM_CONTENTS; i++)
	{
		fwd_encrypt_content(ch, i, p);
		p += ALIGN(enc[i], 0x40);
	}
	*out = w;
	*out_size = total;
	return FWD_OK;
}

void fwd_free(fwd_channel *ch)
{
	free(ch->certs);
	free(ch->ticket);
	free(ch->tmd);
	free(ch->content[0]);
	free(ch->content[1]);
	free(ch->content[2]);
	memset(ch, 0, sizeof(*ch));
}
