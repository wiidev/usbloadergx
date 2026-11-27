/* Host test: build a channel WAD with source/channel/fwdchannel.c and write it out,
 * e.g. to compare byte for byte with an independent implementation.
 * usage: fwdtest template.bin nandloader.app forwarder.dol banner.bnr GAMEID TITLE /loader/path keys.bin out.wad */
#include <stdio.h>
#include <stdlib.h>
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
