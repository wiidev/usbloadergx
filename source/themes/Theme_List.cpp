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
#include "xml/xml.h"
#include "prompts/PromptWindows.h"

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

	themescount = CountThemes(file.data);
	if (themescount <= 0)
	{
        free(file.data);
        return;
	}

	ParseXML(file.data);

    free(file.data);
}

Theme_List::~Theme_List()
{
    for (int i = 0; i < themescount; i++)
    {
        if(Theme[i].themetitle)
            delete [] Theme[i].themetitle;
        if(Theme[i].author)
            delete [] Theme[i].author;
        if(Theme[i].imagelink)
            delete [] Theme[i].imagelink;
        if(Theme[i].downloadlink)
            delete [] Theme[i].downloadlink;
        Theme[i].themetitle = NULL;
        Theme[i].author = NULL;
        Theme[i].imagelink = NULL;
        Theme[i].downloadlink = NULL;
    }

    if(Theme)
        delete [] Theme;
    Theme = NULL;
}


int Theme_List::CountThemes(const u8 * xmlfile)
{
	char tmp[200];
	u32 cnt = 0;
	u32 stringlength = strlen((const char *) xmlfile);
	memset(tmp, 0, sizeof(tmp));

	while (cnt < stringlength)
	{
		if (stringcompare(xmlfile, "<totalthemes>", cnt) == 0)
		{
			copyhtmlsting((const char *) xmlfile, tmp, ">", cnt);
			break;
		}
		cnt++;
	}
	tmp[cnt+1] = 0;

	return atoi(tmp);
}

bool Theme_List::ParseXML(const u8 * xmlfile)
{
	char element_text[1024];
	memset(element_text, 0, sizeof(element_text));
	mxml_node_t *nodetree=NULL;
	mxml_node_t *nodedata=NULL;
	mxml_node_t *nodeid=NULL;
	mxml_index_t *nodeindex=NULL;

	nodetree = mxmlLoadString(NULL, (const char *) xmlfile, MXML_OPAQUE_CALLBACK);

    if (nodetree == NULL)
	{
        return false;
	}

    nodedata = mxmlFindElement(nodetree, nodetree, "themes", NULL, NULL, MXML_DESCEND);
    if (nodedata == NULL)
	{
        return false;
    }

    nodeindex = mxmlIndexNew(nodedata,"name", NULL);
    nodeid = mxmlIndexReset(nodeindex);

	Theme = new Theme_Info[themescount];
	memset(Theme, 0, sizeof(Theme));

	for (int i = 0; i < themescount; i++)
	{
		nodeid = mxmlIndexFind(nodeindex,"name", NULL);
        if (nodeid != NULL)
        {
			get_nodetext(nodeid, element_text, sizeof(element_text));
            Theme[i].themetitle = new char[strlen(element_text)+2];
            snprintf(Theme[i].themetitle,strlen(element_text)+1, "%s", element_text);

			GetTextFromNode(nodeid, nodedata, (char *) "creator", NULL, NULL, MXML_NO_DESCEND, element_text,sizeof(element_text));
            Theme[i].author = new char[strlen(element_text)+2];
            snprintf(Theme[i].author,strlen(element_text)+1, "%s", element_text);

			GetTextFromNode(nodeid, nodedata, (char *) "thumbpath", NULL, NULL, MXML_NO_DESCEND, element_text,sizeof(element_text));
            Theme[i].imagelink = new char[strlen(element_text)+2];
            snprintf(Theme[i].imagelink,strlen(element_text)+1, "%s", element_text);

			GetTextFromNode(nodeid, nodedata, (char *) "downloadpath", NULL, NULL, MXML_NO_DESCEND, element_text,sizeof(element_text));
            Theme[i].downloadlink = new char[strlen(element_text)+2];
            snprintf(Theme[i].downloadlink,strlen(element_text)+1, "%s", element_text);

			GetTextFromNode(nodeid, nodedata, (char *) "averagerating", NULL, NULL, MXML_NO_DESCEND, element_text,sizeof(element_text));
            Theme[i].rating = atoi(element_text);
		}
	}

    mxmlIndexDelete(nodeindex);
	free(nodetree);
	free(nodedata);
	free(nodeid);
	nodetree=NULL;
	nodedata=NULL;
	nodeid=NULL;
	nodeindex=NULL;

    return true;
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
