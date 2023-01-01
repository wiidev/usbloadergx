/****************************************************************************
 * HomebrewXML Class
 * for USB Loader GX
 ***************************************************************************/
#include <gctypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FileOperations/fileops.h"
#include "xml/pugixml.hpp"
#include "gecko.h"

#include "HomebrewXML.h"

#define ENTRIE_SIZE 8192

/* qparam filename Filepath of the XML file */
int HomebrewXML::LoadHomebrewXMLData(const char *filename)
{
	Name.clear();
	Coder.clear();
	Version.clear();
	ShortDescription.clear();
	LongDescription.clear();
	Releasedate.clear();

	pugi::xml_document xmlDoc;
	pugi::xml_parse_result result = xmlDoc.load_file(filename);
	if (!result)
		return 0;

	Name = xmlDoc.child("app").child_value("name");
	Coder = xmlDoc.child("app").child_value("coder");
	Version = xmlDoc.child("app").child_value("version");
	ShortDescription = xmlDoc.child("app").child_value("short_description");
	LongDescription = xmlDoc.child("app").child_value("long_description");

	std::string ReleaseText = xmlDoc.child("app").child_value("release_date");
	if (ReleaseText.length() >= 6)
	{
		if (ReleaseText.find("/") == std::string::npos)
		{
			Releasedate = ReleaseText.substr(4, 2);
			if (ReleaseText.length() >= 8)
			{
				Releasedate.append("/");
				Releasedate.append(ReleaseText.substr(6, 2));
			}
			Releasedate.append("/");
			Releasedate.append(ReleaseText.substr(0, 4));
		}
		else
			Releasedate = ReleaseText;
	}

	for (pugi::xml_node arg : xmlDoc.child("app").child("arguments").children("arg"))
		Arguments.push_back(arg.text().as_string());

	return 1;
}

int HomebrewXML::SaveHomebrewXMLData(const char *filename)
{
	const int max_line_size = 4096;
	char *line = new (std::nothrow) char[max_line_size];
	if (!line)
		return 0;

	FILE *fp = fopen(filename, "wb");
	if (!fp)
	{
		delete[] line;
		return 0;
	}

	snprintf(line, max_line_size, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
	fputs(line, fp);
	snprintf(line, max_line_size, "<app version=\"1\">\n");
	fputs(line, fp);
	snprintf(line, max_line_size, "	<name>%s</name>\n", GetName());
	fputs(line, fp);
	snprintf(line, max_line_size, "	<coder>%s</coder>\n", GetCoder());
	fputs(line, fp);
	snprintf(line, max_line_size, "	<version>%s</version>\n", GetVersion());
	fputs(line, fp);
	snprintf(line, max_line_size, "	<release_date>%s</release_date>\n", GetReleasedate());
	fputs(line, fp);
	if (Arguments.size() > 0)
	{
		snprintf(line, max_line_size, "	<arguments>\n");
		fputs(line, fp);
		for (u8 i = 0; i < Arguments.size(); i++)
		{
			snprintf(line, max_line_size, "		<arg>%s</arg>\n", Arguments[i].c_str());
			fputs(line, fp);
		}
		snprintf(line, max_line_size, "	</arguments>\n");
		fputs(line, fp);
	}
	snprintf(line, max_line_size, "	<ahb_access/>\n");
	fputs(line, fp);
	snprintf(line, max_line_size, "	<short_description>%s</short_description>\n", GetShortDescription());
	fputs(line, fp);
	snprintf(line, max_line_size, "	<long_description>%s</long_description>\n", GetLongDescription());
	fputs(line, fp);
	snprintf(line, max_line_size, "</app>\n");
	fputs(line, fp);

	fclose(fp);
	delete[] line;
	return 1;
}

/* Set argument */
void HomebrewXML::SetArgument(const char *argument)
{
	// Crop value from argument, if present
	char argName[strlen(argument) + 1];
	strcpy(argName, argument);
	char *ptr = strrchr(argName, '=');
	if (ptr)
		*(ptr + 1) = 0;

	// Check if argument already exists and edit it
	bool found = false;
	for (u8 i = 0; i < Arguments.size(); i++)
	{
		size_t pos = Arguments[i].find(argName);
		if (pos != std::string::npos)
		{
			Arguments[i] = argument;
			found = true;
			break;
		}
	}

	// if it doesn't exist, add the new argument.
	if (!found)
		Arguments.push_back(argument);
}

/* Get name */
const char *HomebrewXML::GetName() const
{
	return Name.c_str();
}

/* Set Name */
void HomebrewXML::SetName(char *newName)
{
	Name = newName;
}

/* Get coder */
const char *HomebrewXML::GetCoder() const
{
	return Coder.c_str();
}

/* Get version */
const char *HomebrewXML::GetVersion() const
{
	return Version.c_str();
}

/* Set version */
void HomebrewXML::SetVersion(const char *newVer)
{
	Version = newVer;
}

/* Get releasedate */
const char *HomebrewXML::GetReleasedate() const
{
	return Releasedate.c_str();
}

/* Get shortdescription */
const char *HomebrewXML::GetShortDescription() const
{
	return ShortDescription.c_str();
}

/* Get longdescription */
const char *HomebrewXML::GetLongDescription() const
{
	return LongDescription.c_str();
}
