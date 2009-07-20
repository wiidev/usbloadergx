/****************************************************************************
 * HomebrewXML Class
 * for USB Loader GX
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xml/xml.h"

#include "HomebrewXML.h"

HomebrewXML::HomebrewXML()
{
    strcpy(name,"");
    strcpy(coder,"");
    strcpy(version,"");
    strcpy(releasedate,"");
    strcpy(shortdescription,"");
    strcpy(longdescription,"");
}

HomebrewXML::~HomebrewXML()
{
}

int HomebrewXML::LoadHomebrewXMLData(const char* filename)
{
	mxml_node_t *nodedataHB = NULL;
	mxml_node_t *nodetreeHB = NULL;
	char tmp1[40];

    /* Load XML file */
    FILE *filexml;
    filexml = fopen(filename, "rb");
    if (!filexml) {
        return -1;
    }

    nodetreeHB = mxmlLoadFile(NULL, filexml, MXML_NO_CALLBACK);
    fclose(filexml);

	if (nodetreeHB == NULL) {
		return -2;
    }

    nodedataHB = mxmlFindElement(nodetreeHB, nodetreeHB, "app", NULL, NULL, MXML_DESCEND);
   	if (nodedataHB == NULL) {
	    return -5;
	}

    GetTextFromNode(nodedataHB, nodedataHB, (char*) "name", NULL, NULL, MXML_DESCEND, name,sizeof(name));
    GetTextFromNode(nodedataHB, nodedataHB, (char*) "coder", NULL, NULL, MXML_DESCEND, coder,sizeof(coder));
    GetTextFromNode(nodedataHB, nodedataHB, (char*) "version", NULL, NULL, MXML_DESCEND, version,sizeof(version));
    GetTextFromNode(nodedataHB, nodedataHB, (char*) "release_date", NULL, NULL, MXML_DESCEND, tmp1,sizeof(tmp1));
    GetTextFromNode(nodedataHB, nodedataHB, (char*) "short_description", NULL, NULL, MXML_DESCEND, shortdescription,sizeof(shortdescription));
    GetTextFromNode(nodedataHB, nodedataHB, (char*) "long_description", NULL, NULL, MXML_DESCEND, longdescription,sizeof(longdescription));

    int len = (strlen(tmp1)-6); //length of the date string without the 200000 at the end

    if (len == 8)
        snprintf(releasedate, sizeof(releasedate), "%c%c/%c%c/%c%c%c%c", tmp1[4],tmp1[5],tmp1[6],tmp1[7],tmp1[0],tmp1[1],tmp1[2],tmp1[3]);
    else if (len == 6)
        snprintf(releasedate, sizeof(releasedate), "%c%c/%c%c%c%c", tmp1[4],tmp1[5],tmp1[0],tmp1[1],tmp1[2],tmp1[3]);
    else snprintf(releasedate, sizeof(releasedate), "%s", tmp1);

	free(nodedataHB);
	free(nodetreeHB);

	return 1;
}
