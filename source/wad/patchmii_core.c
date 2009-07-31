/*  patchmii_core -- low-level functions to handle the downloading, patching
    and installation of updates on the Wii

    Copyright (C) 2008 bushing / hackmii.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 2.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

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

#include "patchmii_core.h"
#include "sha1.h"

#define ALIGN(a,b) ((((a)+(b)-1)/(b))*(b))

int http_status = 0;
int tmd_dirty = 0, tik_dirty = 0, temp_ios_slot = 0;

// yeah, yeah, I know.
signed_blob *s_tmd = NULL, *s_tik = NULL, *s_certs = NULL;
//static u8 tmdbuf[MAX_SIGNED_TMD_SIZE] ATTRIBUTE_ALIGN(0x20);
//static u8 tikbuf[STD_SIGNED_TIK_SIZE] ATTRIBUTE_ALIGN(0x20);


void zero_sig(signed_blob *sig) {
    u8 *sig_ptr = (u8 *)sig;
    memset(sig_ptr + 4, 0, SIGNATURE_SIZE(sig)-4);
}

void brute_tmd(tmd *p_tmd) {
    u16 fill;
    for (fill=0; fill<65535; fill++) {
        p_tmd->fill3=fill;
        sha1 hash;
        //    debug_printf("SHA1(%p, %x, %p)\n", p_tmd, TMD_SIZE(p_tmd), hash);
        SHA1((u8 *)p_tmd, TMD_SIZE(p_tmd), hash);;

        if (hash[0]==0) {
            //      debug_printf("setting fill3 to %04hx\n", fill);
            return;
        }
    }
    printf("Unable to fix tmd :(\n");
    exit(4);
}

void brute_tik(tik *p_tik) {
    u16 fill;
    for (fill=0; fill<65535; fill++) {
        p_tik->padding=fill;
        sha1 hash;
        //    debug_printf("SHA1(%p, %x, %p)\n", p_tmd, TMD_SIZE(p_tmd), hash);
        SHA1((u8 *)p_tik, sizeof(tik), hash);

        if (hash[0]==0) return;
    }
    printf("Unable to fix tik :(\n");
    exit(5);
}

void forge_tmd(signed_blob *s_tmd) {
//  debug_printf("forging tmd sig");
    zero_sig(s_tmd);
    brute_tmd(SIGNATURE_PAYLOAD(s_tmd));
}

void forge_tik(signed_blob *s_tik) {
//  debug_printf("forging tik sig");
    zero_sig(s_tik);
    brute_tik(SIGNATURE_PAYLOAD(s_tik));
}


