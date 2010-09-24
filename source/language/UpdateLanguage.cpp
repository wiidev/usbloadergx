/****************************************************************************
 * languagefile updater
 * for USB Loader GX    *giantpune*
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>

#include "UpdateLanguage.h"
#include "listfiles.h"
#include "menu.h"
#include "network/networkops.h"
#include "network/http.h"

int updateLanguageFiles()
{
    char languageFiles[50][MAXLANGUAGEFILES];

    //get all the files in the language path
    int countfiles = GetAllDirFiles(Settings.languagefiles_path);

    //give up now if we didn't find any
    if (!countfiles) return -2;

    //now from the files we got, get only the .lang files
    for (int cnt = 0; cnt < countfiles; cnt++)
    {
        char filename[64];
        strlcpy(filename, GetFileName(cnt), sizeof(filename));
        if (strcasestr(filename, ".lang"))
        {
            strcpy(languageFiles[cnt], filename);
        }
    }

    subfoldercreate(Settings.languagefiles_path);

    //we assume that the network will already be init by another function
    // ( that has gui eletents in it because this one doesn't)
    int done = 0, j = 0;
    if (IsNetworkInit())
    {
        //build the URL, save path, and download each file and save it
        while (j < countfiles)
        {
            char savepath[150];
            char codeurl[200];
            snprintf(codeurl, sizeof(codeurl), "http://usbloader-gui.googlecode.com/svn/trunk/Languages/%s",
                    languageFiles[j]);
            snprintf(savepath, sizeof(savepath), "%s%s", Settings.languagefiles_path, languageFiles[j]);

            struct block file = downloadfile(codeurl);

            if (file.data != NULL)
            {
                FILE * pfile;
                pfile = fopen(savepath, "wb");
                if (pfile != NULL)
                {
                    fwrite(file.data, 1, file.size, pfile);
                    fclose(pfile);
                    free(file.data);
                    done++;
                }
            }

            j++;
        }

    }
    //if there was no network init
    else return -1;

    // return the number of files we updated
    return done;
}

