/***************************************************************************
 * Copyright (C) 2011
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
 ***************************************************************************/
#include "Wiinnertag.h"
#include "FileOperations/fileops.h"
#include "settings/CSettings.h"
#include "network/networkops.h"
#include "utils/StringTools.h"
#include "gecko.h"

Wiinnertag::Wiinnertag(const string &filepath)
{
    ReadXML(filepath);
}

bool Wiinnertag::ReadXML(const string &filepath)
{
    u8 *buffer = NULL;
    u64 filesize = 0;

    LoadFileToMem(filepath.c_str(), &buffer, &filesize);

    if(!buffer)
        return false;

    mxml_node_t *xmlfile = mxmlLoadString(NULL, (const char *) buffer, MXML_OPAQUE_CALLBACK);
    if(!xmlfile)
    {
        free(buffer);
        return false;
    }

    mxml_node_t *node = mxmlFindElement(xmlfile, xmlfile, "Tag", NULL, NULL, MXML_DESCEND_FIRST);

    while(node != NULL)
    {
        const char * URL = mxmlElementGetAttr(node, "URL");
        const char * Key = mxmlElementGetAttr(node, "Key");

        if(URL && Key)
        {
            int size = tagList.size();
            tagList.resize(size+1);
            tagList[size].first = URL;
            tagList[size].second = Key;
        }

        node = mxmlFindElement(node, xmlfile, "Tag", NULL, NULL, MXML_DESCEND);
    }

    mxmlDelete(xmlfile);
    free(buffer);

    return true;
}

bool Wiinnertag::Send(const char *gameID)
{
    if(!IsNetworkInit())
        return false;

    char sendURL[1024];

    for(u32 i = 0; i < tagList.size(); ++i)
    {
        strcpy(sendURL, tagList[i].first.c_str());

        replaceString(sendURL, "{ID6}", gameID);
        replaceString(sendURL, "{KEY}", tagList[i].second.c_str());

	    download_request(sendURL);
	    CloseConnection();
    }

    return true;
}

bool Wiinnertag::TagGame(const char *gameID)
{
    Wiinnertag Tag(Settings.WiinnertagPath);
    return Tag.Send(gameID);
}

static const char * XMLSaveCallback(mxml_node_t *node, int where)
{
	const char *name = node->value.element.name;

	if (where == MXML_WS_BEFORE_OPEN)
	{
		if(!strcmp(name, "Tag"))
			return "\n";
	}
	return (NULL);
}

bool Wiinnertag::CreateExample(const string &filepath)
{
    FILE * f = fopen(filepath.c_str(), "wb");
    if(!f)
        return false;

    mxml_node_t *xmlfile = mxmlNewXML("1.0");
    mxmlSetWrapMargin(0);

    mxml_node_t	*node = mxmlNewElement(xmlfile, "Tag");
    mxmlElementSetAttr(node, "URL", "http://www.wiinnertag.com/wiinnertag_scripts/update_sign.php?key={KEY}&game_id={ID6}");
    mxmlElementSetAttr(node, "Key", "1234567890");

    mxmlSaveFile(xmlfile, f, XMLSaveCallback);
    fclose(f);

    mxmlDelete(xmlfile);

    return true;
}
