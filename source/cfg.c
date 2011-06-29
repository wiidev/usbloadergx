#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <ogcsys.h>

static char *trimcopy(char *src, char *dest, int size)
{
	int i = 0;
	while (*src == ' ') src++;

	while (*src != 0 && *src != '\n' && *src != '\r' && i < size-1)
	{
		dest[i] = *src;
		i++;
		src++;
	}
	dest[i] = 0;
	i--;

	while(i > 0 && dest[i] == ' ')
	{
		dest[i] = 0;
		i--;
	}

	return dest;
}

static char *cfg_parseline(char *line)
{
	char tmp[300], name[200];
	snprintf(tmp, sizeof(tmp), line);
	char *eq = strchr(tmp, '=');
	if (!eq)
		return NULL;

	*eq = 0;

	trimcopy(tmp, name, sizeof(name));

	if(strcmp(name, "update_path") == 0)
		return eq+1;

	return NULL;
}

bool cfg_parsefile(char *fname, int size)
{
	FILE *f = fopen(fname, "r");
	if (!f)
		return false;

	char line[300];

	while (fgets(line, sizeof(line), f))
	{
		if (line[0] == '#') continue;

		char * value = cfg_parseline(line);
		if(value)
		{
			trimcopy(value, fname, size);
			break;
		}
	}
	fclose(f);
	return true;
}

