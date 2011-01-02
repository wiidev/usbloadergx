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

Theme_List::Theme_List(const char * url)
{
    if (!IsNetworkInit())
        return;

    struct block file = downloadfile(url);

    if (!file.data)
        return;

    ParseXML(file.data);

    free(file.data);
}

Theme_List::~Theme_List()
{
}

bool Theme_List::ParseXML(const u8 * xmlfile)
{
    char element_text[1024];
    memset(element_text, 0, sizeof(element_text));
    mxml_node_t *nodetree = NULL;
    mxml_node_t *nodedata = NULL;
    mxml_node_t *nodeid = NULL;
    mxml_index_t *nodeindex = NULL;

    nodetree = mxmlLoadString(NULL, (const char *) xmlfile, MXML_OPAQUE_CALLBACK);

    if (nodetree == NULL)
        return false;

    nodedata = mxmlFindElement(nodetree, nodetree, "themes", NULL, NULL, MXML_DESCEND);
    if (nodedata == NULL)
        return false;

    nodeindex = mxmlIndexNew(nodedata, "name", NULL);
    nodeid = mxmlIndexReset(nodeindex);

    while((nodeid = mxmlIndexFind(nodeindex, "name", NULL)) != NULL)
    {
        int i = ThemesList.size();
        ThemesList.resize(i+1);

        element_text[0] = '\0';
        get_nodetext(nodeid, element_text, sizeof(element_text));
        ThemesList[i].themetitle = element_text;

        element_text[0] = '\0';
        GetTextFromNode(nodeid, nodedata, (char *) "creator", NULL, NULL, MXML_NO_DESCEND, element_text, sizeof(element_text));
        ThemesList[i].author = element_text;

        element_text[0] = '\0';
        GetTextFromNode(nodeid, nodedata, (char *) "thumbpath", NULL, NULL, MXML_NO_DESCEND, element_text, sizeof(element_text));
        ThemesList[i].imagelink = element_text;

        element_text[0] = '\0';
        GetTextFromNode(nodeid, nodedata, (char *) "downloadpath", NULL, NULL, MXML_NO_DESCEND, element_text, sizeof(element_text));
        ThemesList[i].downloadlink = element_text;

        element_text[0] = '\0';
        GetTextFromNode(nodeid, nodedata, (char *) "averagerating", NULL, NULL, MXML_NO_DESCEND, element_text, sizeof(element_text));
        ThemesList[i].rating = atoi(element_text);
    }

    mxmlIndexDelete(nodeindex);
    mxmlDelete(nodedata);
    mxmlDelete(nodetree);

    return true;
}

const char * Theme_List::GetThemeTitle(int ind) const
{
    if (ind < 0 || ind >= (int) ThemesList.size())
        return NULL;

    else return ThemesList[ind].themetitle.c_str();
}

const char * Theme_List::GetThemeAuthor(int ind) const
{
    if (ind < 0 || ind >= (int) ThemesList.size())
        return NULL;

    return ThemesList[ind].author.c_str();
}

const char * Theme_List::GetImageLink(int ind) const
{
    if (ind < 0 || ind >= (int) ThemesList.size())
        return NULL;

    return ThemesList[ind].imagelink.c_str();
}

const char * Theme_List::GetDownloadLink(int ind) const
{
    if (ind < 0 || ind >= (int) ThemesList.size())
        return NULL;

    return ThemesList[ind].downloadlink.c_str();
}
