/****************************************************************************
 * HomebrewXML Class
 * for USB Loader GX
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xml/xml.h"

#include "HomebrewXML.h"

#define ENTRIE_SIZE     8192

int HomebrewXML::LoadHomebrewXMLData( const char* filename )
{
    mxml_node_t *nodedataHB = NULL;
    mxml_node_t *nodetreeHB = NULL;

    /* Load XML file */
    FILE *filexml;
    filexml = fopen( filename, "rb" );
    if ( !filexml )
        return -1;

    nodetreeHB = mxmlLoadFile( NULL, filexml, MXML_OPAQUE_CALLBACK );
    fclose( filexml );

    if ( nodetreeHB == NULL )
        return -2;

    nodedataHB = mxmlFindElement( nodetreeHB, nodetreeHB, "app", NULL, NULL, MXML_DESCEND );
    if ( nodedataHB == NULL )
        return -5;

    char * Entrie = new char[ENTRIE_SIZE];

    GetTextFromNode( nodedataHB, nodedataHB, ( char* ) "name", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE );
    Name = Entrie;

    GetTextFromNode( nodedataHB, nodedataHB, ( char* ) "coder", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE );
    Coder = Entrie;

    GetTextFromNode( nodedataHB, nodedataHB, ( char* ) "version", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE );
    Version = Entrie;

    GetTextFromNode( nodedataHB, nodedataHB, ( char* ) "short_description", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE );
    ShortDescription = Entrie;

    GetTextFromNode( nodedataHB, nodedataHB, ( char* ) "long_description", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE );
    LongDescription = Entrie;

    GetTextFromNode( nodedataHB, nodedataHB, ( char* ) "release_date", NULL, NULL, MXML_DESCEND, Entrie, ENTRIE_SIZE );

    int len = ( strlen( Entrie ) - 6 ); //length of the date string without the 200000 at the end
    if ( len == 8 )
        snprintf( Entrie, ENTRIE_SIZE, "%c%c/%c%c/%c%c%c%c", Entrie[4], Entrie[5], Entrie[6], Entrie[7], Entrie[0], Entrie[1], Entrie[2], Entrie[3] );
    else if ( len == 6 )
        snprintf( Entrie, ENTRIE_SIZE, "%c%c/%c%c%c%c", Entrie[4], Entrie[5], Entrie[0], Entrie[1], Entrie[2], Entrie[3] );
    else
        snprintf( Entrie, ENTRIE_SIZE, "%s", Entrie );

    Releasedate = Entrie;

    free( nodedataHB );
    free( nodetreeHB );

    delete [] Entrie;

    return 1;
}
