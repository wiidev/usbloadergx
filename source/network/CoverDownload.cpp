#include <gccore.h>
#include <string.h>
#include "network/networkops.h"
#include "network/http.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "prompts/CheckboxPrompt.hpp"
#include "FileOperations/fileops.h"
#include "settings/CSettings.h"
#include "language/gettext.h"
#include "usbloader/GetMissingGameFiles.hpp"
#include "utils/StringTools.h"
#include "gecko.h"

#define VALID_IMAGE(x) (!(x.size == 36864 || x.size <= 1024 || x.size == 7386 || x.size <= 1174 || x.size == 4446 || x.data == NULL))

const char * serverURLFull = "http://wiitdb.com/wiitdb/artwork/coverfull/";
const char * serverURLFullHQ = "http://wiitdb.com/wiitdb/artwork/coverfullHQ/";
const char * serverURL3D = "http://wiitdb.com/wiitdb/artwork/cover3D/";
const char * serverURL2D = "http://wiitdb.com/wiitdb/artwork/cover/";
const char * serverURLOrigDiscs = "http://wiitdb.com/wiitdb/artwork/disc/";
const char * serverURLCustomDiscs = "http://wiitdb.com/wiitdb/artwork/disccustom/";

static bool AbortRequested = false;

static void AbortCallback(void)
{
    AbortRequested = true;
}

static inline struct block DownloadImage(const char * url, const char * gameID)
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

static int CoverDownloadWithProgress(const char * url, const char * progressTitle, const char * writepath, std::vector<std::string> & MissingFilesList)
{
    if(!url || !writepath)
        return -1;

    if (!CreateSubfolder(writepath))
    {
        WindowPrompt(tr( "Error !" ), fmt("%s %s", tr("Can't create directory"), writepath), tr( "OK" ));
        return -1;
    }

    char progressMsg[255];
    int FilesSkipped = MissingFilesList.size();
    ProgressSetAbortCallback(AbortCallback);

    for(u32 i = 0; i < MissingFilesList.size(); ++i)
    {
        if(AbortRequested)
            break;

        snprintf(progressMsg, sizeof(progressMsg), "http://wiitdb.com : %s.png", MissingFilesList[i].c_str());

        ShowProgress(progressTitle, fmt("%i %s", MissingFilesList.size() - i, tr( "files left" )), progressMsg, i, MissingFilesList.size());

        if(MissingFilesList[i].size() < 4)
            continue;

        struct block file = DownloadImage(url, MissingFilesList[i].c_str());
        if(!file.data)
            continue;

        char imgPath[200];
        snprintf(imgPath, sizeof(imgPath), "%s/%s.png", writepath, MissingFilesList[i].c_str());

        FILE *pfile = fopen(imgPath, "wb");
        if (pfile != NULL)
        {
            fwrite(file.data, 1, file.size, pfile);
            fclose(pfile);
            FilesSkipped--;
        }
        free(file.data);
    }

    //! Reset progress values
    ProgressStop();
    ShowProgress(tr("Downloading file"), " ", 0, MissingFilesList.size(), MissingFilesList.size());
    ProgressSetAbortCallback(NULL);

    return FilesSkipped;
}

static int CoverDownloader(const char * downloadURL, const char *writepath, const char * progressTitle, const char * PromptText, bool skipPrompt)
{
    std::vector<std::string> MissingFilesList;
    GetMissingGameFiles(writepath, ".png", MissingFilesList);

    if(MissingFilesList.size() == 0)
    {
        WindowPrompt(tr( "No file missing!" ), 0, tr("OK"));
        return -1;
    }

    if (!IsNetworkInit() && !NetworkInitPrompt())
    {
        gprintf("No network\n");
        return -1;
    }

    if(!skipPrompt)
    {
        char tempCnt[80];
        snprintf(tempCnt, sizeof(tempCnt), "%i %s", MissingFilesList.size(), tr( "Missing files" ));

        if (WindowPrompt(PromptText, tempCnt, tr( "Yes" ), tr( "No" )) == 0)
            return -1;
    }

    AbortRequested = false;

    gprintf("CoverDownloadWithProgress - downloadURL: %s progressTitle: %s writepath: %s MissingFiles: %i\n", downloadURL, progressTitle, writepath, MissingFilesList.size());

    return CoverDownloadWithProgress(downloadURL, progressTitle, writepath, MissingFilesList);
}

void CoverDownload()
{
    int choice = CheckboxWindow(tr( "Cover Download" ), 0, tr( "3D Covers" ), tr( "Flat Covers" ), tr("Full HQ Covers"), tr("Full LQ Covers"), tr( "Original Discarts" ), tr( "Custom Discarts" )); // ask for download choice
    if (choice == 0)
        return;

    bool skipPrompt = false;
    int FileSkipped = 0;
    int SkippedFull = 0;
    int SkippedDiscArts = 0;

    gprintf("CoverDownload start - choices: %04X\n", choice);

    if(choice & CheckedBox1)
    {
        int ret = CoverDownloader(serverURL3D, Settings.covers_path, tr("Downloading 3D Covers"), tr("Download Boxart image?"), skipPrompt);
		if(ret > 0)
			FileSkipped += ret;
        skipPrompt = true;
    }
    if(choice & CheckedBox2)
    {
        int ret = CoverDownloader(serverURL2D, Settings.covers2d_path, tr("Downloading Flat Covers"), tr("Download Boxart image?"), skipPrompt);
		if(ret > 0)
			FileSkipped += ret;
        skipPrompt = true;
    }
    if(choice & CheckedBox3)
    {
        int ret = CoverDownloader(serverURLFullHQ, Settings.coversFull_path, tr("Downloading Full HQ Covers"), tr("Download Boxart image?"), skipPrompt);
		if(ret > 0)
			SkippedFull = ret;
        skipPrompt = true;
    }
    if(choice & CheckedBox4)
    {
        int ret = CoverDownloader(serverURLFull, Settings.coversFull_path, tr("Downloading Full LQ Covers"), tr("Download Boxart image?"), skipPrompt);
		if(ret > 0)
			SkippedFull = ret;
        skipPrompt = true;
    }
    if(choice & CheckedBox5)
    {
        skipPrompt = true;
        const char * downloadURL = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS) ? serverURLOrigDiscs : serverURLCustomDiscs;
        const char * progressTitle = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS) ? tr("Downloading original Discarts") : tr("Downloading custom Discarts");
        int ret = CoverDownloader(downloadURL, Settings.disc_path, progressTitle, tr("Download Discart image?"), skipPrompt);
		if(ret > 0)
			SkippedDiscArts = ret;
	}
    if(choice & CheckedBox6)
    {
        skipPrompt = true;
        const char * downloadURL = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS) ? serverURLCustomDiscs : serverURLOrigDiscs;
        const char * progressTitle = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS) ? tr("Downloading custom Discarts") : tr("Downloading original Discarts");
        int ret = CoverDownloader(downloadURL, Settings.disc_path, progressTitle, tr("Download Discart image?"), skipPrompt);
		if(ret > 0)
			SkippedDiscArts = ret;
    }

    FileSkipped += SkippedDiscArts+SkippedFull;

    if (FileSkipped == 0)
    {
        WindowPrompt(tr("Download finished"), tr("All images downloaded successfully."), tr( "OK" ));
    }
    else if(FileSkipped > 0)
    {
        char tempCnt[80];
        sprintf(tempCnt, "%i %s", FileSkipped, tr( "files not found on the server!" ));
        WindowPrompt(tr( "Download finished" ), tempCnt, tr( "OK" ));
    }
}
