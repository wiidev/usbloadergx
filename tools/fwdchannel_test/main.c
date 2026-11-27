/* Host test: build a channel WAD with source/channel/fwdchannel.c and write it out,
 * e.g. to compare byte for byte with an independent implementation.
 * usage: fwdtest template.bin nandloader.app forwarder.dol banner.bnr GAMEID TITLE /loader/path keys.bin out.wad */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../source/channel/fwdchannel.h"

static unsigned char *slurp(const char *p, unsigned *n)
{
	FILE *f = fopen(p, "rb");
	unsigned char *b;
	long s;
	if (!f) { perror(p); exit(1); }
	fseek(f, 0, SEEK_END); s = ftell(f); rewind(f);
	b = malloc(s);
	if (fread(b, 1, s, f) != (size_t)s) { perror(p); exit(1); }
	fclose(f);
	*n = (unsigned)s;
	return b;
}

/* GameCube texts as make_wad.py reads them: latin-1, whitespace trimmed like Python's str.strip */
static int py_space(unsigned c) { return (c >= 9 && c <= 13) || (c >= 0x1c && c <= 0x20) || c == 0x85 || c == 0xa0; }
static void gc_text(const unsigned char *bnr, unsigned off, unsigned n, unsigned short *out)
{
	unsigned len = 0, a = 0, b, i;
	const unsigned char *p = bnr + 0x1820 + off;
	while (len < n && p[len]) len++;
	while (a < len && py_space(p[a])) a++;
	b = len;
	while (b > a && py_space(p[b - 1])) b--;
	for (i = a; i < b; i++) *out++ = p[i];
	*out = 0;
}

int main(int argc, char **argv)
{
	unsigned tn, nn, dn, bn, kn, wn;
	unsigned char *tpl, *nl, *dol, *bnr, *keys, *wad;
	fwd_channel ch;
	int r;
	FILE *f;
	if (argc != 10) { fprintf(stderr, "usage: template nandloader dol bnr GAMEID TITLE path keys.bin out.wad\n"); return 2; }
	tpl = slurp(argv[1], &tn); nl = slurp(argv[2], &nn); dol = slurp(argv[3], &dn); bnr = slurp(argv[4], &bn); keys = slurp(argv[8], &kn);
	if (tn != FWD_TEMPLATE_SIZE || kn < 0x124) { fprintf(stderr, "bad template or keys\n"); return 1; }
	if (bn > 4 && (!memcmp(bnr, "BNR1", 4) || !memcmp(bnr, "BNR2", 4)))
	{
		/* GameCube: build the channel banner from the loader's template, like make_wad.py */
		static unsigned short title[0x41], dev[0x41], devs[0x21], desc[0x81], shortt[0x21];
		unsigned cn, ln, gn, gbn;
		unsigned char *cb = slurp("../../data/binary/custom_banner.bnr", &cn), *logo = slurp("../../data/binary/fwd_gc_logo.bin", &ln),
		              *bg = slurp("../../data/binary/fwd_gc_bg.bin", &gn), *gb;
		gc_text(bnr, 0x00, 0x20, shortt);
		gc_text(bnr, 0x20, 0x20, devs);
		gc_text(bnr, 0x40, 0x40, title);
		gc_text(bnr, 0x80, 0x40, dev);
		gc_text(bnr, 0xC0, 0x80, desc);
		r = fwd_gc_banner(cb, cn, bnr, bn, title[0] ? title : shortt, dev[0] ? dev : devs, desc, logo, ln, bg, gn, &gb, &gbn);
		if (r) { fprintf(stderr, "fwd_gc_banner: %s\n", fwd_error_text(r)); return 1; }
		bnr = gb;
		bn = gbn;
	}
	r = fwd_build(&ch, tpl, nl, nn, dol, dn, bnr, bn, argv[5], argv[6], argv[7], keys + 0x114);
	if (r) { fprintf(stderr, "fwd_build: %s\n", fwd_error_text(r)); return 1; }
	r = fwd_make_wad(&ch, &wad, &wn);
	if (r) { fprintf(stderr, "fwd_make_wad: %s\n", fwd_error_text(r)); return 1; }
	f = fopen(argv[9], "wb");
	fwrite(wad, 1, wn, f);
	fclose(f);
	printf("%s: %u bytes, banner %s\n", argv[9], wn, ch.banner_fixed ? "fixed" : "as is");
	fwd_free(&ch);
	return 0;
}
