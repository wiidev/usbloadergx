/*
 * Copyright (c) 2001 William L. Pitts
 * Modifications (c) 2004 Felix Domke
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are freely
 * permitted provided that the above copyright notice and this
 * paragraph and the following disclaimer are duplicated in all
 * such forms.
 *
 * This software is provided "AS IS" and without any express or
 * implied warranties, including, without limitation, the implied
 * warranties of merchantability and fitness for a particular
 * purpose.
 */

#include <stdio.h>
#include <string.h>

#include <gccore.h>

#include "elf_abi.h"

/* ======================================================================
 * Determine if a valid ELF image exists at the given memory location.
 * First looks at the ELF header magic field, the makes sure that it is
 * executable and makes sure that it is for a PowerPC.
 * ====================================================================== */
s32 valid_elf_image (void *addr)
{
        Elf32_Ehdr *ehdr; /* Elf header structure pointer */

        ehdr = (Elf32_Ehdr *) addr;

        if (!IS_ELF (*ehdr))
                return 0;

        if (ehdr->e_type != ET_EXEC)
                return -1;

        if (ehdr->e_machine != EM_PPC)
                return -1;

        return 1;
}


/* ======================================================================
 * A very simple elf loader, assumes the image is valid, returns the
 * entry point address.
 * ====================================================================== */
u32 load_elf_image (void *addr)
{
        Elf32_Ehdr *ehdr;
        Elf32_Shdr *shdr;
        u8 *strtab = 0;
        u8 *image;
        int i;

        ehdr = (Elf32_Ehdr *) addr;
        /* Find the section header string table for output info */
        shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff +
                               (ehdr->e_shstrndx * sizeof (Elf32_Shdr)));

        if (shdr->sh_type == SHT_STRTAB)
                strtab = (u8 *) (addr + shdr->sh_offset);

        /* Load each appropriate section */
        for (i = 0; i < ehdr->e_shnum; ++i) {
                shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff +
                                       (i * sizeof (Elf32_Shdr)));

                if (!(shdr->sh_flags & SHF_ALLOC)
                   || shdr->sh_addr == 0 || shdr->sh_size == 0) {
                        continue;
                }

                shdr->sh_addr &= 0x3FFFFFFF;
                shdr->sh_addr |= 0x80000000;

                if (strtab) {
                        /*printf ("%sing section %s @ 0x%08x (0x%08x bytes)\n",
                                (shdr->sh_type == SHT_NOBITS) ?
                                        "clear" : "load",
                                &strtab[shdr->sh_name],
                                (u32) shdr->sh_addr,
                                (u32) shdr->sh_size);*/
                }

                if (shdr->sh_type == SHT_NOBITS) {
                        memset ((void *) shdr->sh_addr, 0, shdr->sh_size);
                } else {
                        image = (u8 *) addr + shdr->sh_offset;
                        memcpy ((void *) shdr->sh_addr,
                                (const void *) image,
                                shdr->sh_size);
                }
                DCFlushRangeNoSync ((void *) shdr->sh_addr, shdr->sh_size);
        }

        return (ehdr->e_entry & 0x3FFFFFFF) | 0x80000000;
}

