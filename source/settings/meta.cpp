/*
Copyright (c) 2014 - Cyan

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#include "homebrewboot\HomebrewXML.h"
#include "FileOperations\fileops.h"
#include "settings\CSettings.h"
#include "svnrev.h"

int updateMetaXML (void)
{
	HomebrewXML MetaXML;
	char filepath[255];
	snprintf(filepath, sizeof(filepath), "%s/meta.xml", Settings.update_path);
	if(!MetaXML.LoadHomebrewXMLData(filepath))
		return 0;

	char line[50];
	snprintf(line, sizeof(line), "--ios=%d", Settings.LoaderIOS);
	MetaXML.SetArgument(line);
	snprintf(line, sizeof(line), "--usbport=%d", Settings.USBPort);
	MetaXML.SetArgument(line);
	snprintf(line, sizeof(line), "--mountusb=%d", Settings.USBAutoMount);
	MetaXML.SetArgument(line);
	snprintf(line, sizeof(line), "3.0 r%s", GetRev());
	MetaXML.SetVersion(line);

	int ret = MetaXML.SaveHomebrewXMLData(filepath);
	return ret;
}

int editMetaArguments (void)
{
	char metapath[255] = "";
	char metatmppath[255] = "";

	snprintf(metapath, sizeof(metapath), "%s/meta.xml", Settings.update_path);
	snprintf(metatmppath, sizeof(metatmppath), "%s/meta.tmp", Settings.update_path);

	FILE *source = fopen(metapath, "rb");
	if(!source)
	{
		return 0;
	}

	FILE *destination = fopen(metatmppath, "wb");
	if(!destination)
	{
		fclose(source);
		return 0;
	}

	const int max_line_size = 255;
	char *line = new char[max_line_size];
	while (fgets(line, max_line_size, source) != NULL) 
	{
		// delete commented lines
		if( strstr(line, "<!--   // remove this line to enable arguments") != NULL ||
			strstr(line, "// remove this line to enable arguments -->")   != NULL)
		{
			strcpy(line, "		\n");
		}

		// generate argurments
		if(strstr(line, "<arguments>") != NULL)
		{
			fputs(line, destination);
			snprintf(line, max_line_size, "				<arg>--ios=%d</arg>\n", Settings.LoaderIOS);
			fputs(line, destination);
			snprintf(line, max_line_size, "				<arg>--usbport=%d</arg>\n", Settings.USBPort);
			fputs(line, destination);
			snprintf(line, max_line_size, "				<arg>--mountusb=%d</arg>\n", Settings.USBAutoMount);
			fputs(line, destination);
			
			while(strstr(line, "</arguments>") == NULL)
			{
				fgets(line, max_line_size, source); // advance one line
				if(feof(source))
				{
					fclose(source);
					fclose(destination);
					delete [] line;
					return 0;
				}
			}
		}
		fputs(line, destination);
	}

	fclose(source);
	fclose(destination);
	delete [] line;
	
	if(CopyFile(metatmppath, metapath) <0)
		return 0;
	
	RemoveFile(metatmppath);
	
	return 1;
}
