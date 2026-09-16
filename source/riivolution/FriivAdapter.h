/****************************************************************************
 * Friivolution launch adapter for USB Loader GX.
 *
 * This module translates a GX-selected Wii backup into the standalone
 * Friivolution boot contract (FRIIV_CFG, see FriivConfig.h) and hands it
 * over with the same binary-argument mechanism GX uses for Nintendont
 * (AddBootArgument + BootHomebrewFromMem).
 *
 * Deliberately OUT of scope here: Riivolution XML parsing, patch planning,
 * option editing, table rebuilds, runtime serving. Friivolution owns all of
 * that; GX only supplies the game/mod selection and launches the app.
 *
 * The core (Translate/Build/ResultText/ScanModDir) uses no GX or Wii
 * headers so it can be unit-tested on the host. Thin GX glue
 * (settings, prompts, homebrew launch) lives in GameBooter::BootFriivolution.
 ****************************************************************************/
#ifndef FRIIV_ADAPTER_H_
#define FRIIV_ADAPTER_H_

#include <stddef.h>
#include "FriivConfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Maximum mod filenames collected for the in-GX chooser. */
#define FRIIV_MOD_MAX 64
/* Matches FRIIV_CFG_XML_MAX; bare "name.xml", no directory. */
#define FRIIV_MOD_NAME_MAX 64

typedef enum FriivBuildResult
{
	FRIIV_OK = 0,
	FRIIV_ERR_ARGS,      /* null pointer / zero length passed in */
	FRIIV_ERR_TYPE,      /* not a Wii backup or disc GX can hand over */
	FRIIV_ERR_NO_PATH,   /* backup has no FAT path (e.g. WBFS partition) */
	FRIIV_ERR_BAD_PATH,  /* path breaks contract rules (relative, escape, ...) */
	FRIIV_ERR_PATH_TOO_LONG,
	FRIIV_ERR_CISO,      /* compressed ISO: Friivolution reads plain ISO/WBFS */
	FRIIV_ERR_BAD_DEVICE,/* not SD or the first USB device */
	FRIIV_ERR_BAD_MOD    /* mod name breaks contract rules */
} FriivBuildResult;

/*
 * Translate a GX FAT path ("usb1:/wbfs/.../SB4E01.wbfs", "sd:/...") into the
 * device-relative representation Friivolution documents ("/wbfs/...").
 *
 * isDisc selects the physical disc in the drive instead; fatPath must then
 * be NULL or empty and relOut comes back empty with usbOut false.
 *
 * Split images: pass the first part (.wbfs / .iso / .part0.iso), which is
 * what GX lists. A non-first part (.wbf1, .iso1, .part1.iso, ...) is refused
 * rather than booting the wrong data.
 *
 * Only SD and the first USB device are accepted. v1 names no partition, so
 * anything else is refused with FRIIV_ERR_BAD_DEVICE instead of being
 * silently resolved to the wrong volume.
 */
FriivBuildResult FriivTranslateGamePath(const char *fatPath, bool isDisc,
					char *relOut, size_t relLen, bool *usbOut);

/*
 * Fill a FRIIV_CFG for handoff. relPath is "" (disc) or a translated "/..."
 * path; usb sets FRIIV_CFG_GAME_USB. gameId4 is the 4-byte disc ID prefix or
 * NULL for no game check. modXml is "" (every matching mod, as by hand) or a
 * bare filename limited to one XML in /riivolution/.
 */
FriivBuildResult FriivBuildConfig(const char *relPath, bool usb,
					const void *gameId4, const char *modXml,
					FRIIV_CFG *out);

/*
 * Collect bare "*.xml" basenames from one "<dev>:/riivolution" directory
 * into list (deduplicated, sorted, NUL-terminated). Returns entries added
 * from this directory, or -1 when the directory cannot be opened (not an
 * error: the device may simply be absent). *count is updated.
 */
int FriivScanModDir(const char *dirPath, char list[][FRIIV_MOD_NAME_MAX],
					int maxList, int *count);

/*
 * Collect every mod XML basename from the /riivolution folders GX can see
 * (SD plus all USB devices), for the in-GX mod chooser. No XML is parsed
 * here; Friivolution owns matching names to games. Returns total entries.
 */
int FriivCollectModNames(char list[][FRIIV_MOD_NAME_MAX], int maxList);

/* Short human-readable reason for a result, for prompts and logs. */
const char *FriivResultText(FriivBuildResult r);

#ifdef __cplusplus
}
#endif

#endif
