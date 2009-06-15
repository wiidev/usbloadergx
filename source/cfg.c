#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <ogcsys.h>

#include "cfg.h"

char update_path[150];

static char *cfg_name, *cfg_val;

char* strcopy(char *dest, char *src, int size)
{
	strncpy(dest,src,size);
	dest[size-1] = 0;
	return dest;
}

char* trimcopy(char *dest, char *src, int size)
{
	int len;
	while (*src == ' ') src++;
	len = strlen(src);
	// trim trailing " \r\n"
	while (len > 0 && strchr(" \r\n", src[len-1])) len--;
	if (len >= size) len = size-1;
	strncpy(dest, src, len);
	dest[len] = 0;
	return dest;
}

void cfg_parseline(char *line, void (*set_func)(char*, char*))
{
	// split name = value
	char tmp[200], name[100], val[100];
	strcopy(tmp, line, sizeof(tmp));
	char *eq = strchr(tmp, '=');
	if (!eq) return;
	*eq = 0;
	trimcopy(name, tmp, sizeof(name));
	trimcopy(val, eq+1, sizeof(val));
	//printf("CFG: %s = %s\n", name, val);
	set_func(name, val);
}


bool cfg_parsefile(char *fname, void (*set_func)(char*, char*))
{
	FILE *f;
	char line[200];

	//printf("opening(%s)\n", fname);
	f = fopen(fname, "r");
	if (!f) {
		//printf("error opening(%s)\n", fname);
		return false;
	}
	while (fgets(line, sizeof(line), f)) {
		// lines starting with # are comments
		if (line[0] == '#') continue;
		cfg_parseline(line, set_func);
	}
	fclose(f);
	return true;
}

void cfg_set(char *name, char *val)
{
    cfg_name = name;
    cfg_val = val;
	if (strcmp(name, "update_path") == 0) {
		strcopy(update_path, val, sizeof(update_path));
	}
}
