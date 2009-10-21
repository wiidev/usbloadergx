/***************************************************************************
 * Copyright (C) 2009
 * by USB Loader GX Team
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
 * Theme_List Class
 * for the USB Loader GX
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gctypes.h>

#include "Theme_List.h"

#define stringcompare(text, cmp, pos) strncasecmp((const char*) &text[pos-strlen(cmp)], (const char*) cmp, strlen((const char*) cmp))

static void copyhtmlsting(const char *from, char *outtext, const char *stopat, u32 &cnt)
{
    u32 cnt2 = 0;

    u32 stringlength = strlen(from);

    while ((stringcompare(from, stopat, cnt+strlen(stopat)) != 0) && (cnt2 < 1024) && (cnt < stringlength))
    {
        outtext[cnt2] = from[cnt];
        cnt2++;
        cnt++;
    }
    outtext[cnt2] = '\0';
}

Theme_List::Theme_List(const char * url)
{
    Theme = NULL;
    themescount = 0;
    sitepages = 0;

    if (!IsNetworkInit())
    {
        themescount = -1;
        return;
    }

    struct block file = downloadfile(url);

    if (!file.data || !file.size)
    {
        themescount = -2;
        return;
    }

    u32 cnt = 0;
    char temp[1024];

    Theme = (Theme_Info *) malloc(sizeof(Theme_Info));
    if (!Theme) {
        free(file.data);
        themescount = -3;
        return;
    }

    memset(&Theme[themescount], 0, sizeof(Theme_Info));

    while (cnt < file.size) {

        if(stringcompare(file.data, "\"themetitle\">", cnt) == 0)
        {
            Theme = (Theme_Info *) realloc(Theme, (themescount+1)*sizeof(Theme_Info));

            if (!Theme)
            {
                for (int i = 0; i < themescount; i++)
                {
                    if(Theme[i].imagelink)
                        delete [] Theme[i].imagelink;
                    if(Theme[i].imagelink)
                        delete [] Theme[i].downloadlink;
                    Theme[i].imagelink = NULL;
                    Theme[i].downloadlink = NULL;
                }
                free(Theme);
                Theme = NULL;
                free(file.data);
                themescount = -4;
                break;
            }

            memset(&(Theme[themescount]), 0, sizeof(Theme_Info));

            copyhtmlsting((const char *) file.data, temp, "</", cnt);

            snprintf(Theme[themescount].themetitle, sizeof(Theme[themescount].themetitle), "%s", temp);

            while (cnt < file.size && stringcompare(file.data, "\"themecreated\">By: ", cnt) != 0)
                cnt++;

            copyhtmlsting((const char *) file.data, temp, " - <", cnt);

            snprintf(Theme[themescount].author, sizeof(Theme[themescount].author), "%s", temp);

            while(cnt < file.size && stringcompare(file.data, "class=\"image\" src=\"", cnt) != 0)
                cnt++;

            copyhtmlsting((const char *) file.data, temp, "\" ", cnt);

            Theme[themescount].imagelink = new char[strlen(temp)+1];

            snprintf(Theme[themescount].imagelink, strlen(temp)+1, "%s", temp);

            if (strncmp(Theme[themescount].imagelink, "http://", strlen("http://")) != 0)
                Theme[themescount].direct[0] = false;
            else
                Theme[themescount].direct[0] = true;

            while(cnt < file.size && stringcompare(file.data, "href=\"getfile.php", cnt+strlen("getfile.php")) != 0)
                cnt++;

            copyhtmlsting((const char *) file.data, temp, "\">", cnt);

            Theme[themescount].downloadlink = new char[strlen(temp)+1];

            snprintf(Theme[themescount].downloadlink, strlen(temp)+1, "%s", temp);

            if (strncmp(Theme[themescount].downloadlink, "http://", strlen("http://")) != 0)
                Theme[themescount].direct[1] = false;
            else
                Theme[themescount].direct[1] = true;

            themescount++;
        }

        if(stringcompare(file.data, "/themes.php?creator=&sort=1&page=", cnt) == 0)
        {
            copyhtmlsting((const char *) file.data, temp, "class", cnt);
            int currentvalue = atoi(temp);

            if(currentvalue > sitepages);
                sitepages = currentvalue;
        }

        cnt++;
    }

    free(file.data);
}

Theme_List::~Theme_List()
{
    for (int i = 0; i < themescount; i++)
    {
        if(Theme[i].imagelink)
            delete [] Theme[i].imagelink;
        if(Theme[i].imagelink)
            delete [] Theme[i].downloadlink;
        Theme[i].imagelink = NULL;
        Theme[i].downloadlink = NULL;
    }

    if(Theme)
        free(Theme);
    Theme = NULL;
}

const char * Theme_List::GetThemeTitle(int ind)
{
    if (ind > themescount || ind < 0 || !Theme || themescount <= 0)
        return NULL;
    else
        return Theme[ind].themetitle;
}

const char * Theme_List::GetThemeAuthor(int ind)
{
    if (ind > themescount || ind < 0 || !Theme || themescount <= 0)
        return NULL;
    else
        return Theme[ind].author;
}

const char * Theme_List::GetImageLink(int ind)
{
    if (ind > themescount || ind < 0 || !Theme || themescount <= 0)
        return NULL;
    else
        return Theme[ind].imagelink;
}

const char * Theme_List::GetDownloadLink(int ind)
{
    if (ind > themescount || ind < 0 || !Theme || themescount <= 0)
        return NULL;
    else
        return Theme[ind].downloadlink;
}

int Theme_List::GetThemeCount()
{
    return themescount;
}

int Theme_List::GetSitepageCount()
{
    return sitepages;
}

bool Theme_List::IsDirectImageLink(int ind)
{
    if (ind > themescount || ind < 0 || !Theme || themescount <= 0)
        return false;
    else
        return Theme[ind].direct[0];
}

bool Theme_List::IsDirectDownloadLink(int ind)
{
    if (ind > themescount || ind < 0 || !Theme || themescount <= 0)
        return false;
    else
        return Theme[ind].direct[1];
}

static int ListCompare(const void *a, const void *b)
{
    Theme_Info *ab = (Theme_Info*) a;
    Theme_Info *bb = (Theme_Info*) b;

    return stricmp((char *) ab->themetitle, (char *) bb->themetitle);
}
void Theme_List::SortList()
{
    qsort(Theme, themescount, sizeof(Theme_Info), ListCompare);
}
