/****************************************************************************
 * HomebrewXML Class
 * for USB Loader GX
 ***************************************************************************/
#include <gctypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FileOperations/fileops.h"
#include "xml/xml.h"

#include "HomebrewXML.h"

#define ENTRIE_SIZE     8192

/* qparam filename Filepath of the XML file */
int HomebrewXML::LoadHomebrewXMLData(const char* filename)
{
    Name.clear();
    Coder.clear();
    Version.clear();
    ShortDescription.clear();
    LongDescription.clear();
    Releasedate.clear();

    /* Load XML file */
    u8 * xmlbuffer = NULL;
    u64 size = 0;
    LoadFileToMem(filename, &xmlbuffer, &size);

    if(!xmlbuffer)
        return -1;

    mxml_node_t * nodetree = mxmlLoadString(NULL, (const char *) xmlbuffer, MXML_OPAQUE_CALLBACK);

    if (!nodetree)
        return -2;

    mxml_node_t * node = mxmlFindElement(nodetree, nodetree, "app", NULL, NULL, MXML_DESCEND_FIRST);
    if (!node)
        return -5;

    char * Entrie = new char[ENTRIE_SIZE];

    Entrie[0] = '\0';
    GetTextFromNode(node, nodetree, (char*) "name", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE);
    Name = Entrie;

    Entrie[0] = '\0';
    GetTextFromNode(node, nodetree, (char*) "coder", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE);
    Coder = Entrie;

    Entrie[0] = '\0';
    GetTextFromNode(node, nodetree, (char*) "version", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE);
    Version = Entrie;

    Entrie[0] = '\0';
    GetTextFromNode(node, nodetree, (char*) "short_description", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE);
    ShortDescription = Entrie;

    Entrie[0] = '\0';
    GetTextFromNode(node, nodetree, (char*) "long_description", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE);
    LongDescription = Entrie;

    Entrie[0] = '\0';
    GetTextFromNode(node, nodetree, (char*) "release_date", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE);

    int len = (strlen(Entrie) - 6); //length of the date string without the 200000 at the end
    if (len == 8)
        snprintf(Entrie, ENTRIE_SIZE, "%c%c/%c%c/%c%c%c%c", Entrie[4], Entrie[5], Entrie[6], Entrie[7], Entrie[0],
                Entrie[1], Entrie[2], Entrie[3]);
    else if (len == 6)
        snprintf(Entrie, ENTRIE_SIZE, "%c%c/%c%c%c%c", Entrie[4], Entrie[5], Entrie[0], Entrie[1], Entrie[2], Entrie[3]);
    else snprintf(Entrie, ENTRIE_SIZE, "%s", Entrie);

    Releasedate = Entrie;

    delete[] Entrie;

    mxmlDelete(node);
    mxmlDelete(nodetree);
    free(xmlbuffer);

    return 1;
}

/* Get name */
const char * HomebrewXML::GetName() const
{
    return Name.c_str();
}

/* Set Name */
void HomebrewXML::SetName(char * newName)
{
    Name = newName;
}

/* Get coder */
const char * HomebrewXML::GetCoder() const
{
    return Coder.c_str();
}

/* Get version */
const char * HomebrewXML::GetVersion() const
{
    return Version.c_str();
}

/* Get releasedate */
const char * HomebrewXML::GetReleasedate() const
{
    return Releasedate.c_str();
}

/* Get shortdescription */
const char * HomebrewXML::GetShortDescription() const
{
    return ShortDescription.c_str();
}

/* Get longdescription */
const char * HomebrewXML::GetLongDescription() const
{
    return LongDescription.c_str();
}
