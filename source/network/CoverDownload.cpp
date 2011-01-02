#include <gccore.h>
#include <string.h>
#include "network/networkops.h"
#include "network/http.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "FileOperations/fileops.h"
#include "settings/CSettings.h"
#include "language/gettext.h"
#include "usbloader/GetMissingGameFiles.hpp"
#include "utils/StringTools.h"

#define VALID_IMAGE(x) (!(x.size == 36864 || x.size <= 1024 || x.size == 7386 || x.size <= 1174 || x.size == 4446 || x.data == NULL))

const char * serverURL3D = "http://wiitdb.com/wiitdb/artwork/cover3D/";
const char * serverURL2D = "http://wiitdb.com/wiitdb/artwork/cover/";
const char * serverURLOrigDiscs = "http://wiitdb.com/wiitdb/artwork/disc/";
const char * serverURLCustomDiscs = "http://wiitdb.com/wiitdb/artwork/disccustom/";

static bool AbortRequested = false;

static void AbortCallback(void)
{
    AbortRequested = true;
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

    char downloadURL[512];
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

        //Creates URL depending from which Country the game is
        switch (MissingFilesList[i][3])
        {
            case 'J':
                sprintf(downloadURL, "%sJA/%s.png", url, MissingFilesList[i].c_str());
                break;
            case 'W':
                sprintf(downloadURL, "%sZH/%s.png", url, MissingFilesList[i].c_str());
                break;
            case 'K':
                sprintf(downloadURL, "%sKO/%s.png", url, MissingFilesList[i].c_str());
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
                sprintf(downloadURL, "%s%s/%s.png", url, Settings.db_language, MissingFilesList[i].c_str());
                break;
            case 'E':
                sprintf(downloadURL, "%sUS/%s.png", url, MissingFilesList[i].c_str());
                break;
        }

        struct block file = downloadfile(downloadURL);
        if(!VALID_IMAGE(file))
        {
            if(file.data)
                free(file.data);

            snprintf(downloadURL, sizeof(downloadURL), "%sEN/%s.png", url, MissingFilesList[i].c_str());
            file = downloadfile(downloadURL);
            if(!VALID_IMAGE(file))
            {
                if(file.data)
                    free(file.data);
                continue;
            }
        }

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

void CoverDownload()
{
    int choice = WindowPrompt(tr( "Cover Download" ), 0, tr( "3D Covers" ), tr( "Flat Covers" ), tr( "Disc Images" ), tr( "Back" )); // ask for download choice
    if (choice == 0)
        return;

    const char * writepath = choice == 1 ? Settings.covers_path : choice == 2 ? Settings.covers2d_path : Settings.disc_path;
    const char * downloadURL = choice == 1 ? serverURL3D : choice == 2 ? serverURL2D : NULL;
    const char * progressTitle = choice != 3 ? tr("Downloading covers") : NULL;
    if(choice == 3)
    {
        downloadURL = (Settings.discart == 0 || Settings.discart == 2) ? serverURLOrigDiscs : serverURLCustomDiscs;
        progressTitle = (Settings.discart == 0 || Settings.discart == 2) ? tr("Downloading original Discarts") : tr("Downloading custom Discarts");
    }

    std::vector<std::string> MissingFilesList;
    GetMissingGameFiles(writepath, ".png", MissingFilesList);

    if(MissingFilesList.size() == 0)
    {
        WindowPrompt(tr( "No file missing!" ), 0, tr("OK"));
        return;
    }

    if (!IsNetworkInit() && !NetworkInitPrompt())
        return;

    const char * PromptText = choice == 3 ? tr("Download Discart image?") : tr("Download Boxart image?");

    char tempCnt[80];
    snprintf(tempCnt, sizeof(tempCnt), "%i %s", MissingFilesList.size(), tr( "Missing files" ));

    if (WindowPrompt(PromptText, tempCnt, tr( "Yes" ), tr( "No" )) == 0)
        return;

    AbortRequested = false;

    int FileSkipped = CoverDownloadWithProgress(downloadURL, progressTitle, writepath, MissingFilesList);

    if(choice == 3 && FileSkipped > 0 && Settings.discart > 1)
    {
        if(downloadURL == serverURLOrigDiscs)
        {
            progressTitle = tr("Trying custom Discarts");
            downloadURL = serverURLCustomDiscs;
        }
        else
        {
            progressTitle = tr("Trying original Discarts");
            downloadURL = serverURLOrigDiscs;
        }

        GetMissingGameFiles(writepath, ".png", MissingFilesList);
        FileSkipped = CoverDownloadWithProgress(downloadURL, progressTitle, writepath, MissingFilesList);
    }

    if (FileSkipped == 0)
    {
        WindowPrompt(tr("Download finished"), tr("All images downloaded successfully."), tr( "OK" ));
    }
    else
    {
        sprintf(tempCnt, "%i %s", FileSkipped, tr( "files not found on the server!" ));
        WindowPrompt(tr( "Download finished" ), tempCnt, tr( "OK" ));
    }
}
