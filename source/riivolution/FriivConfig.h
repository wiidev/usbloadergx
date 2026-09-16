/* Vendored from Friivolution for USB Loader GX.
 *
 * Upstream: https://git.gay/december/friivolution
 * File: launcher/include/FriivConfig.h at 69c8216d2f170e18d0855c49bc36294ebfe157b8
 * License: same as Friivolution (see below and upstream COPYING/README).
 * The upstream header says: "Copy this file into your loader. It has no
 * dependencies." Only this provenance banner was added; the contract below
 * is byte-identical to upstream. If upstream bumps FRIIV_CFG_VERSION, this
 * copy must be re-vendored and the adapter's static asserts will fail the
 * build until the new layout is handled.
 */

/*
 * FriivConfig.h - boot configuration handed to Friivolution by another loader.
 *
 * Copy this file into your loader. It has no dependencies.
 *
 * Friivolution can be told to boot a game without showing its own menus,
 * the same way Nintendont does (see Nintendont's CommonConfig.h and USB Loader
 * GX's GameBooter.cpp / NIN_Config.h). Delivery is via arguments: pass the
 * raw FRIIV_CFG bytes as the second argument when launching Friivolution's
 * boot.dol/boot.elf. argv[0] is the path of the dol. The struct can contain
 * NUL bytes.
 *
 * Detecting support: read <version> from the meta.xml next to boot.elf;
 * 1.01 and later understand this argument. You can basically just check
 * if it's not 1.00, as that's the only build without this. Older builds
 * (and Riivolution itself) ignore arguments and simply show their own
 * menus.
 *
 * While starting up, Friivolution shows "Autoboot: hold B" and drops to its
 * normal picker if B is held on any controller during that time.
 *
 * Mod options: Friivolution loads every XML in /riivolution/ whose game ID
 * matches, and each option's choice comes from /riivolution/config/XXXX.xml
 * (XXXX = first 4 characters of the game ID), which Friivolution's own menu
 * writes. A loader that wants to set choices itself should write that file,
 * in Riivolution's format:
 *   <riivolution version="2">
 *     <option id="Language" default="1"/>   (0 = disabled, 1..n = choice)
 *   </riivolution>
 *
 * To limit a boot to a single mod, name its XML in ModXml instead.
 */
#ifndef __FRIIV_CONFIG_H__
#define __FRIIV_CONFIG_H__

#define FRIIV_CFG_MAGIC      0x46524956u  /* 'FRIV' */
#define FRIIV_CFG_VERSION    1u
#define FRIIV_CFG_PATH_MAX   255
#define FRIIV_CFG_XML_MAX    64

typedef struct FRIIV_CFG {
	unsigned int  Magic;      /* FRIIV_CFG_MAGIC */
	unsigned int  Version;    /* FRIIV_CFG_VERSION */
	unsigned int  Flags;      /* FRIIV_CFG_* bits below */
	unsigned int  GameID;     /* first 4 bytes of the disc header ('SB4E'); 0 = don't check */

	/* Path of the image below the device root, e.g.
	 *   "/wbfs/Super Mario Galaxy 2 [SB4E01]/SB4E01.wbfs"
	 *   "/riivolution/backups/game.iso"
	 * For split images give the first part (.wbfs / .iso / .part0.iso).
	 * Empty = use the disc in the drive.
	 * Device: SD, or USB when FRIIV_CFG_GAME_USB is set. */
	char          GamePath[FRIIV_CFG_PATH_MAX];

	/* Optional. File name (no directory) of one mod XML in /riivolution/,
	 * e.g. "smg2-mod.xml". Only that mod is loaded for this boot; its
	 * option choices still come from /riivolution/config/XXXX.xml.
	 * Empty = every mod XML for the game, as when started by hand. */
	char          ModXml[FRIIV_CFG_XML_MAX];

	unsigned char Reserved[49]; /* zero; pads the struct to 384 bytes */
} FRIIV_CFG;

enum friivconfigbits
{
	/* Skip the menus and boot GamePath. Without this bit the argument is
	 * ignored, exactly like Nintendont. You may think this is useless,
	 * considering you have to explicitly pass a second argument.
	 * ...yeah, probably. Better safe than sorry, though. */
	FRIIV_CFG_AUTO_BOOT   = (1u << 0),
	/* GamePath is on the USB drive instead of the SD card. */
	FRIIV_CFG_GAME_USB    = (1u << 1),
	/* Boot the image with no Riivolution patches at all (vanilla). Mod XMLs
	 * are not read; ModXml is ignored. This is also probably useless LOL
	 * but it might be good if you are making your own loader and want to rely
	 * on us I guess? */
	FRIIV_CFG_NO_PATCHES  = (1u << 2)
};

#endif
