/****************************************************************************
 * Friivolution launch adapter for USB Loader GX. See FriivAdapter.h.
 * Core only: no GX/Wii headers, host-testable.
 ****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include "FriivAdapter.h"

static bool FriivHasNul(const char *s, size_t max)
{
	for (size_t i = 0; i < max; i++)
	{
		if (!s[i])
			return true;
	}
	return false;
}

static bool FriivHasDotDot(const char *path)
{
	for (const char *p = path; *p; p++)
	{
		if (p[0] == '.' && p[1] == '.' &&
		    (p == path || p[-1] == '/') &&
		    (p[2] == '/' || p[2] == 0))
			return true;
	}
	return false;
}

static bool FriivEndsWithCI(const char *s, const char *suffix)
{
	size_t sl = strlen(s), xl = strlen(suffix);
	if (sl < xl)
		return false;
	const char *tail = s + sl - xl;
	for (size_t i = 0; i < xl; i++)
	{
		if (tolower((unsigned char)tail[i]) != suffix[i])
			return false;
	}
	return true;
}

/* True for companion parts that must never be used as the entry point. */
static bool FriivIsCompanionPart(const char *rel)
{
	/* ".wbf1".."wbf9" (GX split.c names parts by patching the last char). */
	size_t n = strlen(rel);
	if (n >= 5 && rel[n - 5] == '.' &&
	    (rel[n - 4] == 'w' || rel[n - 4] == 'W') &&
	    (rel[n - 3] == 'b' || rel[n - 3] == 'B') &&
	    (rel[n - 2] == 'f' || rel[n - 2] == 'F') &&
	    rel[n - 1] >= '1' && rel[n - 1] <= '9')
		return true;
	/* ".iso1".."iso9". */
	if (FriivEndsWithCI(rel, ".iso1") || FriivEndsWithCI(rel, ".iso2") ||
	    FriivEndsWithCI(rel, ".iso3") || FriivEndsWithCI(rel, ".iso4") ||
	    FriivEndsWithCI(rel, ".iso5") || FriivEndsWithCI(rel, ".iso6") ||
	    FriivEndsWithCI(rel, ".iso7") || FriivEndsWithCI(rel, ".iso8") ||
	    FriivEndsWithCI(rel, ".iso9"))
		return true;
	/* ".partN.iso" with N > 0. */
	if (n >= 11 && rel[n - 4] == '.' &&
	    (rel[n - 3] == 'i' || rel[n - 3] == 'I'))
	{
		const char *dot = NULL;
		for (const char *p = rel + n - 5; p > rel; p--)
		{
			if (*p == '.')
			{
				dot = p;
				break;
			}
			if (*p == '/')
				break;
		}
		if (dot && !strncmp(dot, ".part", 5) && dot[5] >= '1' && dot[5] <= '9')
			return true;
	}
	return false;
}

FriivBuildResult FriivTranslateGamePath(const char *fatPath, bool isDisc,
					char *relOut, size_t relLen, bool *usbOut)
{
	if (!relOut || !relLen || !usbOut)
		return FRIIV_ERR_ARGS;
	relOut[0] = 0;
	*usbOut = false;

	if (isDisc)
	{
		if (fatPath && fatPath[0])
			return FRIIV_ERR_ARGS;
		return FRIIV_OK; /* empty path = disc in the drive */
	}

	if (!fatPath || !fatPath[0])
		return FRIIV_ERR_NO_PATH;

	/* Split "usb1:/wbfs/..." into device prefix and device-relative path. */
	const char *colon = strchr(fatPath, ':');
	if (!colon || colon == fatPath)
		return FRIIV_ERR_BAD_PATH;
	size_t devLen = (size_t)(colon - fatPath);
	const char *rel = colon + 1;

	bool usb;
	if (devLen == 2 && !strncmp(fatPath, "sd", 2))
		usb = false;
	else if (devLen == 4 && !strncmp(fatPath, "usb1", 4))
		usb = true;
	else
		return FRIIV_ERR_BAD_DEVICE;

	if (!rel[0] || rel[0] != '/')
		return FRIIV_ERR_BAD_PATH;
	if (strlen(rel) >= FRIIV_CFG_PATH_MAX)
		return FRIIV_ERR_PATH_TOO_LONG;
	if (strchr(rel, '\\') || FriivHasDotDot(rel))
		return FRIIV_ERR_BAD_PATH;
	if (FriivIsCompanionPart(rel))
		return FRIIV_ERR_BAD_PATH;
	if (!FriivEndsWithCI(rel, ".wbfs") && !FriivEndsWithCI(rel, ".iso") &&
	    !FriivEndsWithCI(rel, ".part0.iso"))
		return FRIIV_ERR_BAD_PATH;

	if (strlen(rel) + 1 > relLen)
		return FRIIV_ERR_PATH_TOO_LONG;
	strcpy(relOut, rel);
	*usbOut = usb;
	return FRIIV_OK;
}

FriivBuildResult FriivBuildConfig(const char *relPath, bool usb,
					const void *gameId4, const char *modXml,
					FRIIV_CFG *out)
{
	if (!relPath || !out)
		return FRIIV_ERR_ARGS;
	if (!modXml)
		modXml = "";

	if (relPath[0])
	{
		if (relPath[0] != '/' || !FriivHasNul(relPath, FRIIV_CFG_PATH_MAX))
			return FRIIV_ERR_BAD_PATH;
		if (strlen(relPath) >= FRIIV_CFG_PATH_MAX)
			return FRIIV_ERR_PATH_TOO_LONG;
		if (strchr(relPath, '\\') || FriivHasDotDot(relPath))
			return FRIIV_ERR_BAD_PATH;
	}
	if (modXml[0])
	{
		if (!FriivHasNul(modXml, FRIIV_CFG_XML_MAX))
			return FRIIV_ERR_BAD_MOD;
		if (strlen(modXml) >= FRIIV_CFG_XML_MAX)
			return FRIIV_ERR_BAD_MOD;
		if (strchr(modXml, '/') || strchr(modXml, '\\') ||
		    !strcmp(modXml, ".") || !strcmp(modXml, ".."))
			return FRIIV_ERR_BAD_MOD;
	}

	memset(out, 0, sizeof(*out));
	out->Magic = FRIIV_CFG_MAGIC;
	out->Version = FRIIV_CFG_VERSION;
	out->Flags = FRIIV_CFG_AUTO_BOOT | (usb ? FRIIV_CFG_GAME_USB : 0);
	if (gameId4)
		memcpy(&out->GameID, gameId4, 4);
	strcpy(out->GamePath, relPath);
	strcpy(out->ModXml, modXml);
	/* Reserved stays zeroed. */
	return FRIIV_OK;
}

int FriivScanModDir(const char *dirPath, char list[][FRIIV_MOD_NAME_MAX],
					int maxList, int *count)
{
	if (!dirPath || !list || maxList <= 0 || !count)
		return -1;
	DIR *d = opendir(dirPath);
	if (!d)
		return -1;

	int added = 0;
	struct dirent *ent;
	while ((ent = readdir(d)) != NULL)
	{
		const char *name = ent->d_name;
		if (!strcmp(name, ".") || !strcmp(name, ".."))
			continue;
		if (!FriivEndsWithCI(name, ".xml"))
			continue;
		if (strlen(name) >= FRIIV_MOD_NAME_MAX)
			continue;
		bool dup = false;
		for (int i = 0; i < *count; i++)
		{
			if (!strcmp(list[i], name))
			{
				dup = true;
				break;
			}
		}
		if (dup || *count >= maxList)
			continue;
		/* Sorted insert for a stable chooser. */
		int pos = *count;
		while (pos > 0 && strcmp(list[pos - 1], name) > 0)
		{
			strcpy(list[pos], list[pos - 1]);
			pos--;
		}
		strcpy(list[pos], name);
		(*count)++;
		added++;
	}
	closedir(d);
	return added;
}

int FriivCollectModNames(char list[][FRIIV_MOD_NAME_MAX], int maxList)
{
	if (!list || maxList <= 0)
		return 0;
	int count = 0;
	char dir[32];
	const char *devs[] = { "sd", "usb1", "usb2", "usb3", "usb4",
			       "usb5", "usb6", "usb7", "usb8" };
	for (size_t i = 0; i < sizeof(devs) / sizeof(devs[0]); i++)
	{
		snprintf(dir, sizeof(dir), "%s:/riivolution", devs[i]);
		FriivScanModDir(dir, list, maxList, &count);
		if (count >= maxList)
			break;
	}
	return count;
}

const char *FriivResultText(FriivBuildResult r)
{
	switch (r)
	{
	case FRIIV_OK:
		return "OK";
	case FRIIV_ERR_ARGS:
		return "Internal error building the Friivolution request.";
	case FRIIV_ERR_TYPE:
		return "Only Wii backups and Wii discs can launch via Friivolution.";
	case FRIIV_ERR_NO_PATH:
		return "This game has no file path (WBFS partitions are not supported; use FAT32).";
	case FRIIV_ERR_BAD_PATH:
		return "The game path cannot be handed to Friivolution (must be a first-part .wbfs/.iso on SD or USB).";
	case FRIIV_ERR_PATH_TOO_LONG:
		return "The game path is too long for the Friivolution request.";
	case FRIIV_ERR_CISO:
		return "Compressed ISO (.ciso) is not supported by Friivolution; use plain ISO or WBFS.";
	case FRIIV_ERR_BAD_DEVICE:
		return "The game must be on the SD card or the first USB device for Friivolution.";
	case FRIIV_ERR_BAD_MOD:
		return "The selected mod filename is invalid (bare \"name.xml\", max 63 chars).";
	default:
		return "Unknown Friivolution error.";
	}
}
