/****************************************************************************
 * HomebrewXML Class
 * for USB Loader GX
 ***************************************************************************/
#include <gctypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FileOperations/fileops.h"
#include "xml/tinyxml.h"
#include "gecko.h"

#include "HomebrewXML.h"

#define ENTRIE_SIZE	 8192

/* qparam filename Filepath of the XML file */
int HomebrewXML::LoadHomebrewXMLData(const char* filename)
{
	Name.clear();
	Coder.clear();
	Version.clear();
	ShortDescription.clear();
	LongDescription.clear();
	Releasedate.clear();

	TiXmlDocument xmlDoc(filename);
	if(!xmlDoc.LoadFile())
		return false;

	TiXmlElement *appNode =  xmlDoc.FirstChildElement("app");
	if(!appNode)
		return false;

	TiXmlElement *node = NULL;

	node = appNode->FirstChildElement("name");
	if(node && node->FirstChild() && node->FirstChild()->Value())
		Name = node->FirstChild()->Value();

	node = appNode->FirstChildElement("coder");
	if(node && node->FirstChild() && node->FirstChild()->Value())
		Coder = node->FirstChild()->Value();

	node = appNode->FirstChildElement("version");
	if(node && node->FirstChild() && node->FirstChild()->Value())
		Version = node->FirstChild()->Value();

	node = appNode->FirstChildElement("short_description");
	if(node && node->FirstChild() && node->FirstChild()->Value())
		ShortDescription = node->FirstChild()->Value();

	node = appNode->FirstChildElement("long_description");
	if(node && node->FirstChild() && node->FirstChild()->Value())
		LongDescription = node->FirstChild()->Value();

	char ReleaseText[200];
	memset(ReleaseText, 0, sizeof(ReleaseText));

	node = appNode->FirstChildElement("release_date");
	if(node && node->FirstChild() && node->FirstChild()->Value())
		snprintf(ReleaseText, sizeof(ReleaseText), node->FirstChild()->Value());

	int len = (strlen(ReleaseText) - 6); //length of the date string without the 200000 at the end
	if (len == 8)
		snprintf(ReleaseText, sizeof(ReleaseText), "%c%c/%c%c/%c%c%c%c", ReleaseText[4], ReleaseText[5], ReleaseText[6], ReleaseText[7], ReleaseText[0], ReleaseText[1], ReleaseText[2], ReleaseText[3]);
	else if (len == 6)
		snprintf(ReleaseText, sizeof(ReleaseText), "%c%c/%c%c%c%c", ReleaseText[4], ReleaseText[5], ReleaseText[0], ReleaseText[1], ReleaseText[2], ReleaseText[3]);
	else
		snprintf(ReleaseText, sizeof(ReleaseText), "%s", ReleaseText);

	Releasedate = ReleaseText;

	node = appNode->FirstChildElement("arguments");
	if(!node)
		return 1;

	TiXmlElement *argNode = node->FirstChildElement("arg");

	while(argNode)
	{
		if(argNode->FirstChild() && argNode->FirstChild()->Value())
			Arguments.push_back(std::string(argNode->FirstChild()->Value()));

		argNode = argNode->NextSiblingElement();
	}

	return 1;
}

int HomebrewXML::SaveHomebrewXMLData(const char* filename)
{
	const int max_line_size = 4096;
	char *line = new (std::nothrow) char[max_line_size];
	if(!line) return 0;

	FILE *fp = fopen(filename, "wb");
	if(!fp)
	{
		delete [] line;
		return 0;
	}

	snprintf(line, max_line_size,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"); fputs(line, fp);
	snprintf(line, max_line_size,"<app version=\"1\">\n"); fputs(line, fp);
	snprintf(line, max_line_size,"	<name>%s</name>\n", GetName()); fputs(line, fp);
	snprintf(line, max_line_size,"	<coder>%s</coder>\n", GetCoder()); fputs(line, fp);
	snprintf(line, max_line_size,"	<version>%s</version>\n", GetVersion()); fputs(line, fp);
	snprintf(line, max_line_size,"	<release_date>%s</release_date>\n", GetReleasedate()); fputs(line, fp);
	if (Arguments.size() > 0)
	{
		snprintf(line, max_line_size,"	<arguments>\n"); fputs(line, fp);
		for(u8 i = 0; i < Arguments.size(); i++)
		{
			snprintf(line, max_line_size,"		<arg>%s</arg>\n", Arguments[i].c_str()); fputs(line, fp);
		}
		snprintf(line, max_line_size,"	</arguments>\n"); fputs(line, fp);
	}
	snprintf(line, max_line_size,"	<ahb_access/>\n"); fputs(line, fp);
	snprintf(line, max_line_size,"	<short_description>%s</short_description>\n", GetShortDescription()); fputs(line, fp);
	snprintf(line, max_line_size,"	<long_description>%s</long_description>\n", GetLongDescription()); fputs(line, fp);
	snprintf(line, max_line_size,"</app>\n"); fputs(line, fp);
		
	fclose(fp);
	delete [] line;
	return 1;
}

/* Set argument */
void HomebrewXML::SetArgument(const char* argument)
{
	// Crop value from argument, if present
	char argName[strlen(argument)+1];
	strcpy(argName, argument);
	char *ptr = strrchr(argName, '=');
	if(ptr) *(ptr+1) = 0;

	// Check if argument already exists and edit it
	bool found = false;
	for(u8 i=0; i < Arguments.size(); i++)
	{
		size_t pos = Arguments[i].find(argName);
		if(pos != std::string::npos)
		{
			Arguments[i] = argument;
			found = true;
			break;
		}
	}

	// if it doesn't exist, add the new argument.
	if(!found)
		Arguments.push_back(argument);
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

/* Set version */
void HomebrewXML::SetVersion(const char * newVer)
{
	Version = newVer;
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
