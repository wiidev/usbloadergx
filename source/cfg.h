#ifndef _CFG_H_
#define _CFG_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern char update_path[150];

void cfg_set(char *name, char *val);
bool cfg_parsefile(char * fname, void (*set_func)(char*, char*));

#ifdef __cplusplus
}
#endif

#endif
