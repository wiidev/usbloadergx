#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <network.h>
#include <sys/errno.h>
#include <wiiuse/wpad.h>

#include "patchmii_core.h"
#include "sha1.h"
#include "debug.h"
#include "http.h"
//#include "haxx_certs.h"
#include <fat.h>
//#include "ehc_elf.h"
#include "mload_elf.h"

//#define ADD_EHC

#define ADD_HAXX // active this if you want to play with starlet without installing 50 times an hour
//#define REMOVE_OH0
u32 save_nus_object (u16 index, u8 *buf, u32 size);

/* add module inside the tmd
   take care of alignement.
*/

extern int INPUT_TITLEID_L;

void tmd_add_module(tmd *p_tmd,const u8 *elf, u32 elf_size)
{
        int ncid;
        int i,found=0;
        tmd_content *p_cr = (tmd_content *)TMD_CONTENTS(p_tmd);
        sha1 hash;
        int content_size = (elf_size+31)&~31;
        u8 *buf = memalign(32,content_size);
        int index =  p_tmd->num_contents;
		memset((void *) buf,0,content_size);

        memcpy((void *) buf,elf,elf_size);

		
        ncid = 10;
        while(!found){
                found = 1;
                ncid++;
                for (i=0;i<p_tmd->num_contents;i++) {
                        if(p_cr[i].cid == ncid){ found = 0;break;}
                }
        }
		
        debug_printf("found a free cid: %x\n",ncid);
        p_cr[index].cid = ncid;
        p_cr[index].type = 0x0001; // shared is 0x8001
        p_cr[index].size = content_size;
        p_cr[index].index = index;

        //calc sha
        SHA1(buf, content_size, hash);
        memcpy(p_cr[index].hash, hash, sizeof hash);
        p_tmd->num_contents++;
        save_nus_object(ncid, buf, content_size);
}

int add_custom_modules(tmd *p_tmd)
{
        tmd_content tmp;
        int tmd_dirty=0;
        tmd_content *p_cr = (tmd_content *)TMD_CONTENTS(p_tmd);
#ifdef REMOVE_OH0 // remove oh0 module
        {
                int i;
                debug_printf("removing cid %d\n",p_cr[2].cid);
                memmove(&p_cr[2],&p_cr[3],p_tmd->num_contents*sizeof(tmd_content)-2);
                p_tmd->num_contents--;
                for (i=0;i<p_tmd->num_contents;i++) 
                        p_cr[i].index = i;
                tmd_dirty = 1;
        }
#endif
#ifdef ADD_EHC
/* add ehc module. We need it installed before OH0 and OH1, because IOS loads it in the order of the tmd.
   for some reason, we cant shift all indexes or the IOS_Reload will crashed without saying anything.
*/
        debug_printf("adding EHC module\n");
        tmd_add_module(p_tmd,ehc_elf,ehc_elf_size);
   
		tmp = p_cr[3]; // inverse ehc and oh0 place in tmd
        p_cr[3] = p_cr[p_tmd->num_contents-1];
        p_cr[p_tmd->num_contents-1] = tmp;
        tmd_dirty = 1;
#endif
#ifdef ADD_HAXX
/*add haxx module. We need it installed before OH0 and OH1, because IOS loads it in the order of the tmd.
   for some reason, we cant shift all indexes or the IOS_Reload will crashed without saying anything.
*/
        debug_printf("adding haxx module\n");
        tmd_add_module(p_tmd,mload_elf,mload_elf_size);

		tmp = p_cr[3-(INPUT_TITLEID_L==57)]; // inverse ehc and oh0 place in tmd
        p_cr[3-(INPUT_TITLEID_L==57)] = p_cr[p_tmd->num_contents-1];
        p_cr[p_tmd->num_contents-1] = tmp;

   
        tmd_dirty = 1;
#endif
        return tmd_dirty;
}
