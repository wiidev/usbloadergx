#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>

//#define OH0_REPL
#ifdef OH0_REPL
#include "oh0_elf.h"
#endif
struct ios_module_replacement
{
  char *name;
  u8 *data;
  u32 *size;
};

struct ios_module_replacement ios_repls[]={
#ifdef OH0_REPL
        {"OH0",(u8*)oh0_elf,(u32*)&oh0_elf_size}
#endif
};

void debug_printf(const char *fmt, ...);

int replace_ios_modules(u8 **decrypted_buf,  u32 *content_size)
{
        int i,version;
        u32 len;
        u8*buf = *decrypted_buf;
        char *ios_version_tag = "$IOSVersion:";
        len = *content_size;
        if (len == 64) 
                return 0;
        version =0;
        for (i=0; i<(len-64); i++) 
                if (!strncmp((char *)buf+i, ios_version_tag, 10)) {
                        version = i;
                        break;
                }

        for (i=0;i<sizeof(ios_repls)/sizeof(ios_repls[0]); i++){
                struct ios_module_replacement *repl = &ios_repls[i];
                if(!memcmp(buf+version+13,repl->name,strlen(repl->name)))
                {
                        debug_printf("replaced %s\n",repl->name);
                        free(*decrypted_buf);
                        len = *repl->size;
                        len+=31;
                        len&=~31;
                        *decrypted_buf = malloc(len);
                        memcpy(*decrypted_buf,repl->data,len);
                        *content_size = len;
                        return 1;
                }
        }
        return 0;
}

