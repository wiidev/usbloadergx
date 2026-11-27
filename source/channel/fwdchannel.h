/****************************************************************************
 * Per-game forwarder channels: build a fakesigned channel (banner + forwarder
 * DOL that starts USB Loader GX with the game ID) entirely in memory.
 *
 * The forwarder DOL is built from tools/forwarder_dol/.
 * Pure C with no libogc dependency, so it also builds on a PC for tests.
 ***************************************************************************/
#ifndef FWDCHANNEL_H_
#define FWDCHANNEL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FWD_CERTS_SIZE   0xA00
#define FWD_TICKET_SIZE  0x2A4
#define FWD_TMD_HDR_SIZE 0x1E4
#define FWD_TEMPLATE_SIZE (FWD_CERTS_SIZE + FWD_TICKET_SIZE + FWD_TMD_HDR_SIZE)

typedef struct
{
	uint8_t title_id[8];      /* 00010001 + 4 ASCII chars */
	uint8_t title_key[16];
	uint8_t *certs;           /* FWD_CERTS_SIZE */
	uint8_t *ticket;          /* FWD_TICKET_SIZE, fakesigned */
	uint8_t *tmd;             /* tmd_size, fakesigned */
	uint32_t tmd_size;
	uint8_t *content[3];      /* plain: 0 = banner, 1 = nandloader (boot), 2 = forwarder DOL */
	uint32_t content_size[3];
	int banner_fixed;         /* 1 if layout names were lower-cased */
} fwd_channel;

/* Default channel title ID for a game: Wii R... -> T..., S... -> K...; GameCube G... -> Y...,
 * D... -> Z... (4 chars; never G: the Wii Menu hides 00010001 titles that start with G).
 * Returns 0 on success. */
int fwd_default_title_id(const char *game_id, char out4[5]);

/* Checks of a disc/cache banner (0x40 zero bytes + IMET + U8). Returns 0 if fine,
 * otherwise a negative code; fwd_error_text() describes it. */
int fwd_check_banner(const uint8_t *bnr, uint32_t len);

#define FWD_NUM_CONTENTS 3

/* Build the channel. template_blob = certs + ticket + TMD header of the UNEO forwarder,
 * nandloader = boot content of a working channel (the UNEO forwarder's): the Wii Menu needs
 * it, a libogc DOL as boot content gives a black screen on a real Wii (Dolphin hides this),
 * dol = forwarder DOL with the ULGXFWD1 config block, common_key = Wii common key.
 * The banner is copied (and its layout names fixed if needed). Returns 0 on success. */
int fwd_build(fwd_channel *ch, const uint8_t *template_blob, const uint8_t *nandloader, uint32_t nandloader_size,
              const uint8_t *dol, uint32_t dol_size,
              const uint8_t *bnr, uint32_t bnr_size, const char *game_id, const char *title4,
              const char *loader_path, const uint8_t common_key[16]);

/* Encrypt content i with the title key; out must hold align16(size). Returns the encrypted size. */
uint32_t fwd_encrypt_content(const fwd_channel *ch, int i, uint8_t *out);

/* Serialize as a WAD (deterministic: same input, same bytes). Caller frees *out. Returns 0 on success. */
int fwd_make_wad(const fwd_channel *ch, uint8_t **out, uint32_t *out_size);

void fwd_free(fwd_channel *ch);

const char *fwd_error_text(int err);

/* Channel banner for a GameCube game, whose disc only has a BNR1/BNR2 opening.bnr (96x32
 * RGB5A3 picture + texts). Built from USB Loader GX's custom_banner.bnr (as a content 0 .app,
 * i.e. with 0x40 extra bytes in front): the game picture in the icon and banner, the GameCube
 * logo (logo_tpl) and background (bg_tpl), and the texts. Strings are NUL-terminated UTF-16.
 * Caller frees *out. Returns 0 on success. */
int fwd_gc_banner(const uint8_t *template_bnr, uint32_t template_size, const uint8_t *gc_bnr, uint32_t gc_size,
                  const uint16_t *title, const uint16_t *developer, const uint16_t *description,
                  const uint8_t *logo_tpl, uint32_t logo_size, const uint8_t *bg_tpl, uint32_t bg_size,
                  uint8_t **out, uint32_t *out_size);

/* Primitives, exported for tests */
void fwd_sha1(const void *data, uint32_t len, uint8_t out[20]);
void fwd_md5(const void *data, uint32_t len, uint8_t out[16]);
void fwd_aes_cbc_encrypt(const uint8_t key[16], const uint8_t iv[16], const uint8_t *in, uint8_t *out, uint32_t len);
uint8_t *fwd_lz77_compress(const uint8_t *in, uint32_t len, uint32_t *out_len);
uint8_t *fwd_fix_banner(const uint8_t *bnr, uint32_t len, uint32_t *out_len, int *changed);

#ifdef __cplusplus
}
#endif

#endif
