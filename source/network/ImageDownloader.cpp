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

#define VALID_IMAGE(x) (!(x.size == 36864 || x.size <= 1024 || x.size == 7386 || x.size <= 1174 || x.size == 4446 || x.data == NULL))

static const char *serverURL3D = "http://wiitdb.com/wiitdb/artwork/cover3D/";
static const char *serverURL2D = "http://wiitdb.com/wiitdb/artwork/cover/";
static const char *serverURLFullHQ = "http://wiitdb.com/wiitdb/artwork/coverfullHQ/";
static const char *serverURLFull = "http://wiitdb.com/wiitdb/artwork/coverfull/";
static const char *serverURLOrigDiscs = "http://wiitdb.com/wiitdb/artwork/disc/";
static const char *serverURLCustomDiscs = "http://wiitdb.com/wiitdb/artwork/disccustom/";

void ImageDownloader::DownloadImages()
{
	int choice = CheckboxWindow(tr( "Cover Download" ), 0, tr( "3D Covers" ), tr( "Flat Covers" ), tr("Full HQ Covers"), tr("Full LQ Covers"), tr( "Original Discarts" ), tr( "Custom Discarts" )); // ask for download choice
	if (choice == 0)
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
		FindMissing(Settings.covers_path, serverURL3D, NULL, tr("Downloading 3D Covers"));

	if(choices & CheckedBox2)
		FindMissing(Settings.covers2d_path, serverURL2D, NULL, tr("Downloading Flat Covers"));

	if(choices & CheckedBox3)
		FindMissing(Settings.coversFull_path, serverURLFullHQ, (choices & CheckedBox4) ? serverURLFull : NULL, tr("Downloading Full HQ Covers"));

	if(choices & CheckedBox4)
		FindMissing(Settings.coversFull_path, serverURLFull, NULL, tr("Downloading Full LQ Covers"));

	if(choices & CheckedBox5)
	{
		const char * downloadURL = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS || !(choices & CheckedBox6)) ? serverURLOrigDiscs : serverURLCustomDiscs;
		const char * backupURL = (choices & CheckedBox6) ? ((Settings.discart == DISCARTS_ORIGINALS_CUSTOMS) ? serverURLCustomDiscs : serverURLOrigDiscs) : NULL;
		const char * progressTitle = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS || !(choices & CheckedBox6)) ? tr("Downloading original Discarts") : tr("Downloading custom Discarts");
		FindMissing(Settings.disc_path, downloadURL, backupURL, progressTitle);
	}

	if(choices & CheckedBox6)
	{
		const char * downloadURL = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS || !(choices & CheckedBox5)) ? serverURLCustomDiscs : serverURLOrigDiscs;
		const char * progressTitle = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS || !(choices & CheckedBox5)) ? tr("Downloading custom Discarts") : tr("Downloading original Discarts");
		FindMissing(Settings.disc_path, downloadURL, NULL, progressTitle);
	}
}

void ImageDownloader::FindMissing(const char *writepath, const char *downloadURL, const char *backupURL, const char *progressTitle)
{
	if (!CreateSubfolder(writepath))
	{
		WindowPrompt(tr( "Error !" ), fmt("%s %s", tr("Can't create directory"), writepath), tr( "OK" ));
		return;
	}

	std::vector<std::string> MissingFilesList;

	GetMissingGameFiles(writepath, ".png", MissingFilesList);
	int size = MissingImages.size();
	MissingImages.resize(size+MissingFilesList.size());

	for(u32 i = 0, n = size; i < MissingFilesList.size(); ++i, ++n)
	{
		MissingImages[n].gameID = MissingFilesList[i];
		MissingImages[n].downloadURL = downloadURL;
		MissingImages[n].backupURL = backupURL;
		MissingImages[n].writepath = writepath;
		MissingImages[n].progressTitle = progressTitle;
	}

	if(!backupURL)
		MissingImagesCount += MissingFilesList.size();
}

int ImageDownloader::DownloadProcess(int TotalDownloadCount)
{
	char progressMsg[255];

	for(u32 i = 0, pos = 0; i < MissingImages.size(); ++i, ++pos)
	{
		if(ProgressCanceled())
			break;

		snprintf(progressMsg, sizeof(progressMsg), "http://wiitdb.com : %s.png", MissingImages[i].gameID.c_str());

		ShowProgress(MissingImages[i].progressTitle, fmt("%i %s", TotalDownloadCount - pos, tr( "files left" )), progressMsg, pos, TotalDownloadCount);

		if(MissingImages[i].gameID.size() < 4)
			continue;

		struct block file = DownloadImage(MissingImages[i].downloadURL, MissingImages[i].gameID.c_str());
		if(!file.data)
		{
			if(MissingImages[i].backupURL)
			{
				MissingImages.erase(MissingImages.begin()+i);
				--i;
				--pos;
			}
			continue;
		}

		char imgPath[200];
		snprintf(imgPath, sizeof(imgPath), "%s/%s.png", MissingImages[i].writepath, MissingImages[i].gameID.c_str());

		FILE *pfile = fopen(imgPath, "wb");
		if (pfile != NULL)
		{
			fwrite(file.data, 1, file.size, pfile);
			fclose(pfile);
			MissingImagesCount--;
		}
		free(file.data);

		if(MissingImages[i].backupURL)
		{
			//! Find and remove the backup download image.
			//! The backup image is always further in the vector, so let's save cpu time.
			for(u32 n = i+1; n < MissingImages.size(); ++n)
			{
				if(MissingImages[n].downloadURL == MissingImages[i].backupURL &&
				   MissingImages[n].gameID == MissingImages[i].gameID)
				{
					MissingImages.erase(MissingImages.begin()+n);
					break;
				}
			}
		}

		//! Remove the image from the vector since it's done
		MissingImages.erase(MissingImages.begin()+i);
		--i;
	}

	return MissingImages.size();
}

struct block ImageDownloader::DownloadImage(const char * url, const char * gameID)
{
	char CheckedRegion[10];
	char downloadURL[512];
	bool PAL = false;

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

	struct block file = downloadfile(downloadURL);
	if(VALID_IMAGE(file))
		return file;

	free(file.data);
	file.data = NULL;

	if(PAL && strcmp(CheckedRegion, "EN") != 0)
	{
		snprintf(downloadURL, sizeof(downloadURL), "%sEN/%s.png", url, gameID);
		file = downloadfile(downloadURL);
		if(VALID_IMAGE(file))
			return file;
	}
	else if(strcmp(CheckedRegion, "") == 0)
	{
		const char * lang = Settings.db_language;

		if(strcmp(lang, "EN") == 0 && CONF_GetRegion() == CONF_REGION_US)
			lang = "US";

		snprintf(downloadURL, sizeof(downloadURL), "%s%s/%s.png", url, lang, gameID);
		file = downloadfile(downloadURL);
		if(VALID_IMAGE(file))
			return file;

		free(file.data);

		snprintf(downloadURL, sizeof(downloadURL), "%sOTHER/%s.png", url, gameID);
		file = downloadfile(downloadURL);
		if(VALID_IMAGE(file))
			return file;
	}

	free(file.data);

	memset(&file, 0, sizeof(struct block));

	return file;
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
		if(MissingImages[i].downloadURL == serverURL3D)
		{
			ImageType = "3D Cover";
		}
		else if(MissingImages[i].downloadURL == serverURL2D)
		{
			ImageType = "2D Cover";
		}
		else if(MissingImages[i].downloadURL == serverURLFullHQ)
		{
			ImageType = "Full HQ Cover";
		}
		else if(MissingImages[i].downloadURL == serverURLFull)
		{
			ImageType = "Full LQ Cover";
		}
		else if(MissingImages[i].downloadURL == serverURLOrigDiscs)
		{
			ImageType = "Original Discart";
		}
		else if(MissingImages[i].downloadURL == serverURLCustomDiscs)
		{
			ImageType = "Custom Discart";
		}

		fprintf(f, "\"%s\",\"%s\",\"%s\"\n", MissingImages[i].gameID.c_str(), GameTitles.GetTitle(MissingImages[i].gameID.c_str()), ImageType);
	}

	fclose(f);
}
