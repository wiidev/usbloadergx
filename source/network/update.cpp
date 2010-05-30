 /***************************************************************************
 * Copyright (C) 2009
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * update.cpp
 *
 * Update operations
 * for Wii-Xplorer 2009
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ogcsys.h>

#include "http.h"
#include "networkops.h"
#include "HTML_Stream.h"

/****************************************************************************
 * Checking if an Update is available
 ***************************************************************************/
int CheckForBetaUpdate()
{
    int revnumber = 0;

    HTML_Stream HTML("http://code.google.com/p/usbloader-gui/downloads/list");

    const char * HTML_Pos = NULL;

    do
    {
        HTML_Pos = HTML.FindStringEnd("href='");
        char * tmpLink = HTML.CopyString("'\"");
        if(tmpLink)
        {
            char *fileext = strrchr(tmpLink, '.');
            if(fileext)
            {
                if(strcasecmp(fileext, ".dol") == 0)
                {
                    char revtxt[80];
                    char *filename = strrchr(tmpLink, '/')+2;
                    u8 n = 0;
                    for (n = 0; n < strlen(filename)-2; n++)
                        revtxt[n] = filename[n];
                    revtxt[n] = 0;
                    int fileRev = atoi(revtxt);

                    if(fileRev > revnumber)
                    {
                        revnumber = fileRev;
                    }
                }
            }
            free(tmpLink);
        }
    }
    while(HTML_Pos != NULL);

    return revnumber;
}
