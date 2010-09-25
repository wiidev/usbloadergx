#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <ogcsys.h>

#include "language/gettext.h"
#include "FileOperations/fileops.h"
#include "cfg.h"
#define isspace2(c) (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')


u8 ocarinaChoice = 0;
u8 videoChoice = 0;
u8 faveChoice = no;
u8 languageChoice = 0;
u8 viChoice = 0;
u8 iosChoice = 0;
u8 parentalcontrolChoice = 0;
u8 fix002 = 0;
u8 reloadblock = 0;
u8 countrystrings = 0;
u8 alternatedol = 0;
u32 alternatedoloffset = 0;
u8 xflip = 0;
u8 sort = 0;
u8 fave = 0;
u8 qboot = 0;
u8 wsprompt = 0;
u8 keyset = 0;
u8 listDisplay = 0;
u8 partition = -1;
char alternatedname[40];
u8 returnToLoaderGV = 1; //global variable used for returnToLoaderShit.  defaults to "yes, patch return to loader"

struct ID_Title
{
    char id[6];
    char * title;
};

struct ID_Control
{
    char id[6];
    u8 block;
};
// renamed titles
int num_title = 0; //number of titles
struct ID_Title *cfg_title = NULL;

int num_control = 0;
struct ID_Control *cfg_control = NULL;

char *cfg_get_title(u8 *id)
{
    if (!id) return NULL;

    int i;
    for (i = 0; i < num_title; i++)
    {
        if (strncmp((char*) id, cfg_title[i].id, 6) == 0)
        {
            return cfg_title[i].title;
        }
    }
    return NULL;
}

char *get_title(struct discHdr *header)
{
    if (!header) return NULL;

    char *title = cfg_get_title(header->id);
    if (title) return title;
    return header->title;
}

void title_set(char *id, char *title)
{
    if (!id || !title) return;

    if (!cfg_title) cfg_title = (struct ID_Title *) malloc(sizeof(struct ID_Title));

    char *idt = cfg_get_title((u8*) id);
    if (idt)
    {
        // replace
        free(idt);
        idt = strdup(title);
    }
    else
    {
        struct ID_Title * tmpStruct = (struct ID_Title *) realloc(cfg_title, (num_title + 1) * sizeof(struct ID_Title));
        if (!tmpStruct)
        {
            // error
            CFG_Cleanup();
            num_title = 0;
            return;
        }

        cfg_title = tmpStruct;

        // add
        strncpy(cfg_title[num_title].id, id, 6);
        cfg_title[num_title].title = strdup(title);
        num_title++;
    }
}

void titles_default()
{
    int i;
    for (i = 0; i < num_title; i++)
    {
        memset(cfg_title[i].id, 0, 6);
        free(cfg_title[i].title);
        cfg_title[i].title = NULL;
    }
}

u8 cfg_get_block(u8 *id)
{
    int i;
    for (i = 0; i < num_control; i++)
    {
        if (memcmp(id, cfg_control[i].id, 6) == 0)
        {
            return cfg_control[i].block;
        }
    }
    return 0;
}

u8 get_block(struct discHdr *header)
{
    return cfg_get_block(header->id);
}

s8 get_pegi_block(struct discHdr *header)
{
    switch (get_block(header))
    {
        case 1:
            return 7;
        case 2:
            return 12;
        case 3:
            return 16;
        case 4:
            return 18;
        default:
            return -1;
    }
}

// trim leading and trailing whitespace
// copy at max n or at max size-1
char* trim_n_copy(char *dest, char *src, int n, int size)
{
    int len;
    // trim leading white space
    while (isspace2( *src ))
    {
        src++;
        n--;
    }
    len = strlen(src);
    if (len > n) len = n;
    // trim trailing white space
    while (len > 0 && isspace2( src[len-1] ))
        len--;
    if (len >= size) len = size - 1;
    strlcpy(dest, src, len + 1);
    //printf("trim_copy: '%s' %d\n", dest, len); //sleep(1);
    return dest;
}

char* trimcopy(char *dest, char *src, int size)
{
    int len;
    while (*src == ' ')
        src++;
    len = strlen(src);
    // trim trailing " \r\n"
    while (len > 0 && strchr(" \r\n", src[len - 1]))
        len--;
    if (len >= size) len = size - 1;
    strlcpy(dest, src, len + 1);
    return dest;
}


// split line to part1 delimiter part2
bool trimsplit(char *line, char *part1, char *part2, char delim, int size)
{
    char *eq = strchr(line, delim);
    if (!eq) return false;
    trim_n_copy(part1, line, eq - line, size);
    trimcopy(part2, eq + 1, size);
    return true;
}
void cfg_parseline(char *line, void(*set_func)(char*, char*))
{
    // split name = value
    char tmp[300], name[200], val[200];
    strlcpy(tmp, line, sizeof(tmp));
    char *eq = strchr(tmp, '=');
    if (!eq) return;
    *eq = 0;
    trimcopy(name, tmp, sizeof(name));
    trimcopy(val, eq + 1, sizeof(val));
    //printf("CFG: %s = %s\n", name, val);
    set_func(name, val);
}

void cfg_parsetitleline(char *line, void(*set_func)(char*, char*, u8))
{
    // split name = value
    char tmp[200], name[200], val[200];
    int block = 0;
    strlcpy(tmp, line, sizeof(tmp));
    char *eq = strchr(tmp, '=');
    if (!eq) return;
    *eq = 0;
    trimcopy(name, tmp, sizeof(name));

    char *blockpos = strrchr(eq + 1, '=');

    if (!blockpos)
        trimcopy(val, eq + 1, sizeof(val));

    else
    {
        *blockpos = 0;
        trimcopy(val, eq + 1, sizeof(val));
        if (sscanf(blockpos + 1, "%d", &block) != 1)
        {
            block = 0;
        }
    }
    set_func(name, val, block);
}

bool cfg_parsefile(char *fname, void(*set_func)(char*, char*))
{
    FILE *f;
    char line[300];

    //printf("opening(%s)\n", fname);
    f = fopen(fname, "r");
    if (!f)
    {
        //printf("error opening(%s)\n", fname);
        return false;
    }
    while (fgets(line, sizeof(line), f))
    {
        // lines starting with # are comments
        if (line[0] == '#') continue;
        cfg_parseline(line, set_func);
    }
    fclose(f);
    return true;
}

bool cfg_parsetitlefile(char *fname, void(*set_func)(char*, char*, u8))
{
    FILE *f;
    char line[200];

    //printf("opening(%s)\n", fname);
    f = fopen(fname, "r");
    if (!f)
    {
        //printf("error opening(%s)\n", fname);
        return false;
    }

    while (fgets(line, sizeof(line), f))
    {
        // lines starting with # are comments
        if (line[0] == '#') continue;
        cfg_parsetitleline(line, set_func);
    }
    fclose(f);
    return true;
}

void CFG_Cleanup(void)
{
    int i = 0;
    for (i = 0; i < num_title; i++)
    {
        if (cfg_title[i].title) free(cfg_title[i].title);
        cfg_title[i].title = NULL;
    }
    if (cfg_title)
    {
        free(cfg_title);
        cfg_title = NULL;
    }
    num_title = 0;
}

