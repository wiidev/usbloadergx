#include <stdlib.h>
#include <string.h>
#include "WDMFile.hpp"

static inline int GetNumber(const char * line)
{
	while(*line == ' ') line++;

	if(line[0] == '0' && (line[1] == 'x' || line[1] == 'X'))
		return strtol(line, 0, 16);
	else
		return strtol(line, 0, 10);
}

WDMFile::WDMFile(const char * path)
{
	FILE * file = fopen(path, "rb");
	if(!file)
		return;

	char line[255];
	int entry_number = 0;
	int counter = 0;
	WDMEntry Entry;

	while (fgets(line, sizeof(line), file))
	{
		if(line[0] == '#' || line[0] == '\0')
			continue;

		entry_number++;

		if(entry_number < 3)
			continue;

		if(counter == 0)
		{
			int strlength = strlen(line);
			while(strlength > 0 && (line[strlength-1] == '\n' || line[strlength-1] == '\r' || line[strlength-1] == ' '))
			{
				line[strlength-1] = '\0';
				strlength--;
			}

			Entry.ReplaceName = line;
		}
		else if(counter == 1)
		{
			int strlength = strlen(line);
			while(strlength > 0 && (line[strlength-1] == '\n' || line[strlength-1] == '\r' || line[strlength-1] == ' '))
			{
				line[strlength-1] = '\0';
				strlength--;
			}

			Entry.DolName = line;
		}
		else if(counter == 2)
		{
			Entry.Parameter = GetNumber(line);
			WDMEntries.push_back(Entry);
		}
		else if(counter == 3)
		{
			//This is actually the place where submenus are described
			//But we skip it because its never used
			counter = 0;
			continue;
		}

		counter++;
	}

	fclose(file);
}
