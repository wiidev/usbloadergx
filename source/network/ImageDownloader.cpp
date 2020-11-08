/****************************************************************************
 * Copyright (C) 2011 Dimok
 * Copyright (C) 2012 Cyan
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <gccore.h>
#include <string.h>
#include "ImageDownloader.h"
#include "network/networkops.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "prompts/CheckboxPrompt.hpp"
#include "FileOperations/fileops.h"
#include "settings/CSettings.h"
#include "settings/GameTitles.h"
#include "language/gettext.h"
#include "usbloader/GetMissingGameFiles.hpp"
#include "utils/StringTools.h"
#include "gecko.h"

#define VALID_IMAGE(x) (!(x->size == 36864 || x->size <= 1024 || x->size == 7386 || x->size <= 1174 || x->size == 4446 || x->data == NULL))

void ImageDownloader::DownloadImages()
{
	bool showBanner = (Settings.LoaderMode & MODE_GCGAMES);

	int choice = CheckboxWindow(tr( "Cover Download" ), 0, tr( "3D Covers" ), tr( "Flat Covers" ), tr("Full Covers"), tr( "Discarts" ), showBanner ? tr( "Custom Banners" ) : 0, 0, showBanner ? 0x1F : 0xF); // ask for download choice
	if (choice == 0 || choice == CheckedNone)
		return;

	ImageDownloader Downloader;
	Downloader.SetChoices(choice);
	Downloader.Start();
}

void ImageDownloader::Start()
{
	gprintf("CoverDownload start - choices: %04X\n", choices);

	MissingImagesCount = 0;
	FindMissingImages();

	if(MissingImagesCount == 0)
	{
		WindowPrompt(tr( "No file missing!" ), 0, tr("OK"));
		return;
	}

	u32 TotalDownloadCount = MissingImagesCount;

	if (WindowPrompt(tr("Found missing images."), fmt(tr("%i missing files"), TotalDownloadCount), tr( "Yes" ), tr( "No" )) == 0)
		return;

	if (!IsNetworkInit() && !NetworkInitPrompt())
	{
		gprintf("No network\n");
		return;
	}

	ProgressCancelEnable(true);

	DownloadProcess(TotalDownloadCount);

	ProgressCancelEnable(false);

	ProgressStop();

	if(MissingImagesCount == 0)
		WindowPrompt(tr("Download finished"), tr("All images downloaded successfully."), tr( "OK" ));
	else
	{
		int res = WindowPrompt(tr( "Download finished" ), fmt(tr("%i files not found on the server!"), MissingImagesCount), tr("Save List"), tr( "OK" ));
		if(res)
			CreateCSVLog();
	}
}

void ImageDownloader::FindMissingImages()
{
	if(choices & CheckedBox1)
		FindMissing(Settings.covers_path, Settings.URL_Covers3D, NULL, tr("Downloading 3D Covers"), NULL, ".png");

	if(choices & CheckedBox2)
		FindMissing(Settings.covers2d_path, Settings.URL_Covers2D, NULL, tr("Downloading Flat Covers"), NULL, ".png");

	if(choices & CheckedBox3)
	{
		const char * downloadURL = (Settings.coversfull == COVERSFULL_HQ || Settings.coversfull == COVERSFULL_HQ_LQ ) ? Settings.URL_CoversFullHQ : Settings.URL_CoversFull;
		const char * progressTitle = (Settings.coversfull == COVERSFULL_HQ || Settings.coversfull == COVERSFULL_HQ_LQ ) ? tr("Downloading Full HQ Covers") : tr("Downloading Full LQ Covers");
		const char * backupURL = (Settings.coversfull == COVERSFULL_HQ_LQ || Settings.coversfull == COVERSFULL_LQ_HQ) ? ((Settings.coversfull == COVERSFULL_HQ_LQ) ? Settings.URL_CoversFull : Settings.URL_CoversFullHQ) : NULL;
		const char * backupProgressTitle = (Settings.coversfull == COVERSFULL_HQ_LQ || Settings.coversfull == COVERSFULL_LQ_HQ) ? ((Settings.coversfull == COVERSFULL_HQ_LQ) ? tr("Downloading Full LQ Covers") : tr("Downloading Full HQ Covers")) : NULL;
		FindMissing(Settings.coversFull_path, downloadURL, backupURL, progressTitle, backupProgressTitle, ".png");
	}

	if(choices & CheckedBox4)
	{
		const char * downloadURL = (Settings.discart == DISCARTS_ORIGINALS || Settings.discart == DISCARTS_ORIGINALS_CUSTOMS ) ? Settings.URL_Discs : Settings.URL_DiscsCustom;
		const char * progressTitle = (Settings.discart == DISCARTS_ORIGINALS || Settings.discart == DISCARTS_ORIGINALS_CUSTOMS ) ? tr("Downloading original Discarts") : tr("Downloading custom Discarts");
		const char * backupURL = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS || Settings.discart == DISCARTS_CUSTOMS_ORIGINALS) ? ((Settings.discart == DISCARTS_ORIGINALS_CUSTOMS) ? Settings.URL_DiscsCustom : Settings.URL_Discs) : NULL;
		const char * backupProgressTitle = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS || Settings.discart == DISCARTS_CUSTOMS_ORIGINALS) ? ((Settings.discart == DISCARTS_ORIGINALS_CUSTOMS) ? tr("Downloading custom Discarts") : tr("Downloading original Discarts")) : NULL;
		FindMissing(Settings.disc_path, downloadURL, backupURL, progressTitle, backupProgressTitle, ".png");
	}

	if(choices & CheckedBox5)
	{
		FindMissing(Settings.BNRCachePath, Settings.URL_Banners, NULL, tr("Downloading Custom Banners"), NULL, ".bnr");
	}
}

void ImageDownloader::FindMissing(const char *writepath, const char *downloadURL, const char *backupURL, const char *progressTitle, const char *backupProgressTitle, const char *fileExt)
{
	if (!CreateSubfolder(writepath))
	{
		WindowPrompt(tr( "Error !" ), fmt("%s %s", tr("Can't create directory"), writepath), tr( "OK" ));
		return;
	}

	std::vector<std::string> MissingFilesList;

	if((Settings.LoaderMode & MODE_GCGAMES) && strcmp(fileExt, ".bnr") == 0)
	{
		short LoaderModeBackup = Settings.LoaderMode;
		Settings.LoaderMode = MODE_GCGAMES;		// Limit banner download for GameCube Only.
		GetMissingGameFiles(writepath, fileExt, MissingFilesList);
		Settings.LoaderMode = LoaderModeBackup;
	}
	else
	{
		GetMissingGameFiles(writepath, fileExt, MissingFilesList);
	}
	int size = MissingImages.size();
	MissingImages.resize(size+MissingFilesList.size());

	for(u32 i = 0, n = size; i < MissingFilesList.size(); ++i, ++n)
	{
		MissingImages[n].gameID = MissingFilesList[i];
		MissingImages[n].downloadURL = downloadURL;
		MissingImages[n].backupURL = backupURL;
		MissingImages[n].writepath = writepath;
		MissingImages[n].progressTitle = progressTitle;
		MissingImages[n].backupProgressTitle = backupProgressTitle;
		MissingImages[n].fileExt = fileExt;
	}

		MissingImagesCount += MissingFilesList.size();
}

int ImageDownloader::DownloadProcess(int TotalDownloadCount)
{
	char progressMsg[270];

	for(u32 i = 0, pos = 0; i < MissingImages.size(); ++i, ++pos)
	{
		if(ProgressCanceled())
			break;

		if(strcmp(MissingImages[i].fileExt, ".bnr") == 0)
		{
			char *path = strchr(MissingImages[i].downloadURL + ((strncmp(Settings.URL_Banners, "https://", 8) == 0) ? 8 : 7), '/');
			int domainlength = path - Settings.URL_Banners;
			char domain[domainlength + 1];
			strlcpy(domain, Settings.URL_Banners, domainlength + 1);
			snprintf(progressMsg, sizeof(progressMsg), "%s : %s.bnr", domain, MissingImages[i].gameID.c_str());
		}
		else
		{
			char *path = strchr(MissingImages[i].downloadURL + ((strncmp(MissingImages[i].downloadURL, "https://", 8) == 0) ? 8 : 7), '/');
			int domainlength = path - MissingImages[i].downloadURL;
			char domain[domainlength + 1];
			strlcpy(domain, MissingImages[i].downloadURL, domainlength + 1);
			snprintf(progressMsg, sizeof(progressMsg), "%s : %s.png", domain, MissingImages[i].gameID.c_str());
		}

		ShowProgress(MissingImages[i].progressTitle, fmt("%i %s", TotalDownloadCount - pos, tr( "files left" )), progressMsg, pos, TotalDownloadCount);

		if(MissingImages[i].gameID.size() < 3)
			continue;

		struct download file = {};
		DownloadImage(MissingImages[i].downloadURL, MissingImages[i].gameID.c_str(), MissingImages[i].fileExt, &file);
		if(file.size <= 0)
		{
			if(MissingImages[i].backupURL)
			{
				gprintf("Trying backup URL.\n");
				MissingImages[i].downloadURL = MissingImages[i].backupURL;
				MissingImages[i].backupURL = NULL;
				MissingImages[i].progressTitle = MissingImages[i].backupProgressTitle;
				--i;
				--pos;
			}
			continue;
		}

		gprintf(" - OK\n");
		char imgPath[200];
		snprintf(imgPath, sizeof(imgPath), "%s/%s%s", MissingImages[i].writepath, MissingImages[i].gameID.c_str(), MissingImages[i].fileExt);

		FILE *pfile = fopen(imgPath, "wb");
		if (pfile != NULL)
		{
			fwrite(file.data, 1, file.size, pfile);
			fclose(pfile);
			MissingImagesCount--;
		}
		MEM2_free(file.data);
		gprintf("File saved successfully (%s%s)\n", MissingImages[i].gameID.c_str(), MissingImages[i].fileExt);

		//! Remove the image from the vector since it's done
		MissingImages.erase(MissingImages.begin()+i);
		--i;
	}

	return MissingImages.size();
}

void ImageDownloader::DownloadImage(const char *url, const char *gameID, const char *fileExt, struct download *file)
{
	char CheckedRegion[10];
	char downloadURL[512];
	bool PAL = false;

	if(strcmp(fileExt, ".bnr") == 0)
	{
		snprintf(downloadURL, sizeof(downloadURL), "%s%s.bnr", url, gameID);
		gprintf("%s\n", downloadURL);
		downloadfile(downloadURL, file);
		if(file->size > 132 && IsValidBanner(file->data)) // 132 = IMET magic location in the banner with u8 header
			return;

		snprintf(downloadURL, sizeof(downloadURL), "%s%.3s.bnr", url, gameID);
		gprintf(" - Not found. trying ID3: %s\n", downloadURL);
		
		downloadfile(downloadURL, file);
		if(file->size > 132 && IsValidBanner(file->data))
			return;

		gprintf(" - Not found.\n");
		return;
	}

	//Creates URL depending from which Country the game is
	switch (gameID[3])
	{
		case 'J':
			sprintf(downloadURL, "%sJA/%s.png", url, gameID);
			sprintf(CheckedRegion, "JA");
			break;
		case 'W':
			sprintf(downloadURL, "%sZH/%s.png", url, gameID);
			sprintf(CheckedRegion, "ZH");
			break;
		case 'K':
			sprintf(downloadURL, "%sKO/%s.png", url, gameID);
			sprintf(CheckedRegion, "KO");
			break;
		case 'P':
		case 'D':
		case 'F':
		case 'I':
		case 'S':
		case 'H':
		case 'U':
		case 'X':
		case 'Y':
		case 'Z':
			sprintf(downloadURL, "%s%s/%s.png", url, Settings.db_language, gameID);
			sprintf(CheckedRegion, "%s", Settings.db_language);
			PAL = true;
			break;
		case 'E':
			sprintf(downloadURL, "%sUS/%s.png", url, gameID);
			sprintf(CheckedRegion, "US");
			break;
		default:
			strcpy(downloadURL, "");
			strcpy(CheckedRegion, "");
			break;
	}

	gprintf("%s\n", downloadURL);
	downloadfile(downloadURL, file);
	if(VALID_IMAGE(file))
		return;

	if(PAL && strcmp(CheckedRegion, "EN") != 0)
	{
		snprintf(downloadURL, sizeof(downloadURL), "%sEN/%s.png", url, gameID);
		gprintf(" - Not found.\n%s\n", downloadURL);
		downloadfile(downloadURL, file);
		if(VALID_IMAGE(file))
			return;
	}
	else if(strcmp(CheckedRegion, "") == 0)
	{
		const char * lang = Settings.db_language;

		if(strcmp(lang, "EN") == 0 && CONF_GetRegion() == CONF_REGION_US)
			lang = "US";

		snprintf(downloadURL, sizeof(downloadURL), "%s%s/%s.png", url, lang, gameID);
		gprintf(" - Not found.\n%s\n", downloadURL);
		downloadfile(downloadURL, file);
		if(VALID_IMAGE(file))
			return;

		snprintf(downloadURL, sizeof(downloadURL), "%sOTHER/%s.png", url, gameID);
		gprintf(" - Not found.\n%s\n", downloadURL);
		downloadfile(downloadURL, file);
		if(VALID_IMAGE(file))
			return;
		
		if(gameID[3] == 'R') // no english cover found, try russian
		{
			lang = "RU";
			snprintf(downloadURL, sizeof(downloadURL), "%s%s/%s.png", url, lang, gameID);
			gprintf(" - Not found.\n%s\n", downloadURL);
			downloadfile(downloadURL, file);
			if(VALID_IMAGE(file))
				return;
		}
		
		if(gameID[3] == 'V') // no English cover found, try Finnish and Swedish
		{
			lang = "FI";
			snprintf(downloadURL, sizeof(downloadURL), "%s%s/%s.png", url, lang, gameID);
			gprintf(" - Not found.\n%s\n", downloadURL);
			downloadfile(downloadURL, file);
			if(VALID_IMAGE(file))
				return;
			
			lang = "SE";
			snprintf(downloadURL, sizeof(downloadURL), "%s%s/%s.png", url, lang, gameID);
			gprintf(" - Not found.\n%s\n", downloadURL);
			downloadfile(downloadURL, file);
			if(VALID_IMAGE(file))
				return;
		}
	}

	gprintf(" - Not found.\n");
}

void ImageDownloader::CreateCSVLog()
{
	char path[200];
	snprintf(path, sizeof(path), "%s/MissingImages.csv", Settings.update_path);

	FILE *f = fopen(path, "wb");
	if(!f) return;

	const char *ImageType = "3D Cover";

	fprintf(f, "\"ID\",\"Name\",\"ImageType\"\n");

	for (u32 i = 0; i < MissingImages.size(); ++i)
	{
		if(MissingImages[i].downloadURL == Settings.URL_Covers3D)
		{
			ImageType = "3D Cover";
		}
		else if(MissingImages[i].downloadURL == Settings.URL_Covers2D)
		{
			ImageType = "2D Cover";
		}
		else if(MissingImages[i].downloadURL == Settings.URL_CoversFullHQ)
		{
			ImageType = "Full HQ Cover";
		}
		else if(MissingImages[i].downloadURL == Settings.URL_CoversFull)
		{
			ImageType = "Full LQ Cover";
		}
		else if(MissingImages[i].downloadURL == Settings.URL_Discs)
		{
			ImageType = "Original Discart";
		}
		else if(MissingImages[i].downloadURL == Settings.URL_DiscsCustom)
		{
			ImageType = "Custom Discart";
		}
		else if(MissingImages[i].downloadURL == Settings.URL_Banners)
		{
			ImageType = "Custom Banner";
		}

		fprintf(f, "\"%s\",\"%s\",\"%s\"\n", MissingImages[i].gameID.c_str(), GameTitles.GetTitle(MissingImages[i].gameID.c_str()), ImageType);
		gprintf("\"%s\",\"%s\",\"%s\"\n", MissingImages[i].gameID.c_str(), GameTitles.GetTitle(MissingImages[i].gameID.c_str()), ImageType);
	}

	fclose(f);
}

bool ImageDownloader::IsValidBanner(char *banner)
{
	if(!((*(u32*)(banner+64)) == 'IMET'))
	{
		if(!((*(u32*)(banner+128)) == 'IMET')) // with U8Archive header
			return false;
	}
	return true;
}
