// GX -> Friivolution contract test: the REAL adapter builder against the
// REAL Friivolution consumer (bootcfg-body.inc, extracted from the
// Friivolution tree the same way its own run-usb2.sh does). No independent
// models: adapter output bytes go straight into BootCfg_ValidateSized.
//
// Build & run: sh run.sh [FRIIV_SRC]
//   FRIIV_SRC = Friivolution checkout (default: ../../../../friivolution).
//   Without it only adapter-local structural checks run and the script
//   reports the platform boundary instead of failing.
#include <cassert>
#include <cstdio>
#include <cstring>
#include "../FriivAdapter.h"

#ifdef HAVE_FRIIV_CONSUMER
#include "../launcher-include/FriivConfig.h"
#include "../launcher-include/bootcfg.h"
#include "bootcfg-body.inc"

static void ExpectAccepted(const FRIIV_CFG *built)
{
	FRIIV_CFG out;
	// Exact-size and oversized reads accept; short reads refuse.
	assert(BootCfg_ValidateSized(built, sizeof(*built), &out));
	assert(BootCfg_ValidateSized(built, sizeof(*built) + 16, &out));
	assert(!BootCfg_ValidateSized(built, sizeof(*built) - 1, &out));
	assert(!BootCfg_ValidateSized(built, 0, &out));
	assert(BootCfg_Validate(built, &out));
	assert(out.Magic == FRIIV_CFG_MAGIC && out.Version == FRIIV_CFG_VERSION);
}
#else
struct FRIIV_CFG;
static void ExpectAccepted(const FRIIV_CFG *) {}
#endif

int main()
{
	// Both sides compile against a 384-byte v1 layout.
	assert(sizeof(FRIIV_CFG) == 384);

	char rel[512];
	bool usb = false;
	FRIIV_CFG cfg;

	// Normal SD WBFS backup.
	assert(FriivTranslateGamePath("sd:/wbfs/SMG2 [SB4E01]/SB4E01.wbfs", false, rel, sizeof(rel), &usb) == FRIIV_OK);
	assert(!usb && !strcmp(rel, "/wbfs/SMG2 [SB4E01]/SB4E01.wbfs"));
	{
		char id[4]; memcpy(id, "SB4E", 4);
		assert(FriivBuildConfig(rel, usb, id, "", &cfg) == FRIIV_OK);
		assert(cfg.Flags == FRIIV_CFG_AUTO_BOOT);
		ExpectAccepted(&cfg);
	}

	// USB backup sets the USB source bit; spaces and brackets survive.
	assert(FriivTranslateGamePath("usb1:/riivolution/backups/game.iso", false, rel, sizeof(rel), &usb) == FRIIV_OK);
	assert(usb && !strcmp(rel, "/riivolution/backups/game.iso"));
	assert(FriivBuildConfig(rel, usb, NULL, "smg2-mod.xml", &cfg) == FRIIV_OK);
	assert(cfg.Flags == (FRIIV_CFG_AUTO_BOOT | FRIIV_CFG_GAME_USB));
	assert(cfg.GameID == 0 && !strcmp(cfg.ModXml, "smg2-mod.xml"));
	ExpectAccepted(&cfg);

	// Physical disc: empty path, no USB bit.
	assert(FriivTranslateGamePath(NULL, true, rel, sizeof(rel), &usb) == FRIIV_OK);
	assert(!usb && !rel[0]);
	assert(FriivBuildConfig(rel, usb, NULL, "", &cfg) == FRIIV_OK);
	ExpectAccepted(&cfg);

	// Split first parts pass; companion parts are refused, never misbooted.
	assert(FriivTranslateGamePath("usb1:/wbfs/g.wbfs", false, rel, sizeof(rel), &usb) == FRIIV_OK);
	assert(FriivTranslateGamePath("usb1:/wbfs/g.wbf1", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_PATH);
	assert(FriivTranslateGamePath("usb1:/wbfs/g.wbf2", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_PATH);
	assert(FriivTranslateGamePath("sd:/wbfs/g.iso", false, rel, sizeof(rel), &usb) == FRIIV_OK);
	assert(FriivTranslateGamePath("sd:/wbfs/g.iso1", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_PATH);
	assert(FriivTranslateGamePath("sd:/wbfs/g.part0.iso", false, rel, sizeof(rel), &usb) == FRIIV_OK);
	assert(FriivTranslateGamePath("sd:/wbfs/g.part1.iso", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_PATH);
	assert(FriivTranslateGamePath("sd:/wbfs/g.ciso", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_PATH);

	// Devices: SD and first USB only; everything else is an explicit refusal.
	assert(FriivTranslateGamePath("usb2:/wbfs/g.wbfs", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_DEVICE);
	assert(FriivTranslateGamePath("usb8:/wbfs/g.wbfs", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_DEVICE);
	assert(FriivTranslateGamePath("/wbfs/g.wbfs", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_PATH);
	assert(FriivTranslateGamePath("wbfs/g.wbfs", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_PATH);
	assert(FriivTranslateGamePath(NULL, false, rel, sizeof(rel), &usb) == FRIIV_ERR_NO_PATH);

	// Malformed and hostile paths are refused, never truncated into boot.
	assert(FriivTranslateGamePath("usb1:wbfs/g.wbfs", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_PATH);
	assert(FriivTranslateGamePath("usb1:/wbfs/../g.wbfs", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_PATH);
	assert(FriivTranslateGamePath("usb1:/..", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_PATH);
	assert(FriivTranslateGamePath("usb1:/wbfs\\g.wbfs", false, rel, sizeof(rel), &usb) == FRIIV_ERR_BAD_PATH);
	{
		char longp[600];
		strcpy(longp, "usb1:/wbfs/");
		memset(longp + 11, 'a', sizeof(longp) - 20);
		strcpy(longp + sizeof(longp) - 9, ".wbfs");
		assert(FriivTranslateGamePath(longp, false, rel, sizeof(rel), &usb) == FRIIV_ERR_PATH_TOO_LONG);
	}

	// Mod names: bare filenames only (incl. a real fixture name; matching on
	// target is case-insensitive in ParseXMLs, the bytes pass through here).
	assert(FriivBuildConfig("/wbfs/g.wbfs", false, NULL, "", &cfg) == FRIIV_OK);
	assert(FriivBuildConfig("/wbfs/g.wbfs", false, NULL, "mod.xml", &cfg) == FRIIV_OK);
	assert(FriivBuildConfig("/wbfs/Spectral [SMN01]/SMN01.wbfs", false, NULL, "Spectral_USA.xml", &cfg) == FRIIV_OK);
	ExpectAccepted(&cfg);
	assert(FriivBuildConfig("/wbfs/g.wbfs", false, NULL, "../mod.xml", &cfg) == FRIIV_ERR_BAD_MOD);
	assert(FriivBuildConfig("/wbfs/g.wbfs", false, NULL, "sub/mod.xml", &cfg) == FRIIV_ERR_BAD_MOD);
	assert(FriivBuildConfig("/wbfs/g.wbfs", false, NULL, "..", &cfg) == FRIIV_ERR_BAD_MOD);
	assert(FriivBuildConfig("wbfs/g.wbfs", false, NULL, "", &cfg) == FRIIV_ERR_BAD_PATH);

	// Result texts are always presentable.
	assert(FriivResultText(FRIIV_OK) && FriivResultText(FRIIV_ERR_BAD_DEVICE));

	puts("friiv-contract: ok");
	return 0;
}
