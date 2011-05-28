/* Copyright (C) 2008 Mega Man */
/* Based on Wii GameCube Homebrew Launcher */
/* Copyright (C) 2008 WiiGator */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <ctype.h>
#include <unistd.h>

#include "elf.h"
#include "add_dip_plugin.h"
#include "debug_printf.h"


#if 0 
// disabled
// XXXXXXXXXXXXXXXXXXXXX

#include "dip_plugin_bin.h"
#define round_up(x,n) (-(-(x) & -(n)))

/** Binary is loaded to this address. */
#if IOS36
#define BIN_PATCH_ADDR 0x202080e0
#define EXTENDED_AREA_START 0x20200000
#define BSS_START 0x2020a000
#define BSS_SIZE  0x0002c000
#elif IOS38
#define BIN_PATCH_ADDR 0x20208200
#define EXTENDED_AREA_START 0x20208000
#define BSS_START 0x20209000
#define BSS_SIZE  0x0002c000
#else
 #ifdef ADD_DIP_PLUGIN

#error "Hey! i need  IOS36 or IOS38 defined!"

#else
//  fake data to compile the code
#define BIN_PATCH_ADDR 0x202080e0
 #define EXTENDED_AREA_START 0x20200000
 #define BSS_START 0x2020a000
 #define BSS_SIZE  0x0002c000

#endif
#endif 
/** Start of ELF area which is extneded. */


/** Header for Wii ARM binaries. */

typedef struct
{
    /** Size of this header. */
    uint32_t headerSize;
    /** Offset to ELF file. */
    uint32_t offset;
    /** Size of ELF file. */
    uint32_t size;
    /** Padded with zeroes. */
    uint32_t resevered;
}

arm_binary_header_t;

typedef struct
{
    uint32_t cmd;
    uint32_t function_orig;
    uint32_t function_patch;
}

jmp_table_patch_entry_t;

/**
 * Copy program sections of ELF file to new ELF file;
 * @param buffer Pointer to ELF file.
 */
static int copy_sections(uint8_t *buffer, uint8_t *out)
{
    Elf32_Ehdr_t *file_header;
    int pos = 0;
    int i;
    uint32_t outPos = 0;

    /* 0x1000 should be enough to copy ELF header. */
    memcpy(out, buffer, 0x1000);

    /* Use output (copied) elf header. */
    file_header = (Elf32_Ehdr_t *) & out[pos];
    pos += sizeof(Elf32_Ehdr_t);

    if (file_header->magic != ELFMAGIC)
    {
        debug_printf("Magic 0x%08x is wrong.\n", file_header->magic);
        return -2;
    }

    outPos = pos + file_header->phnum * sizeof(Elf32_Phdr_t);
    //debug_printf("data start = 0x%02x\n", outPos);

    for (i = 0; i < file_header->phnum; i++)
    {
        Elf32_Phdr_t *program_header;
        program_header = (Elf32_Phdr_t *) & out[pos];
        pos += sizeof(Elf32_Phdr_t);

        if ( (program_header->type == PT_LOAD)
                && (program_header->memsz != 0) )
        {
            unsigned char *src;
            unsigned char *dst;

            // Copy to physical address which can be accessed by loader.
            src = buffer + program_header->offset;

            if (program_header->offset != sizeof(Elf32_Ehdr_t))
            {
                program_header->offset = outPos;
            }

            else
            {
                /* Don't change offset for first section. */
                outPos = program_header->offset;
            }

            dst = out + outPos;

            if (program_header->vaddr == EXTENDED_AREA_START)
            {
                uint32_t origFileSize;
                printf("Extended Area finded!!!!\n");
                origFileSize = program_header->filesz;
                program_header->filesz = program_header->memsz = BIN_PATCH_ADDR - EXTENDED_AREA_START + dip_plugin_bin_size;
                memset(dst, 0, program_header->filesz);

                if (origFileSize != 0)
                {
                    memcpy(dst, src, origFileSize);
                }
            }

            if (program_header->vaddr == BSS_START)
            {
                printf("BSS Start finded!!!!\n");
                //debug_printf("Extending BSS.\n");
                program_header->memsz += 0x1000; //BSS_SIZE;
            }

            /*debug_printf("VAddr: 0x%08x PAddr: 0x%08x Offset 0x%08x File Size 0x%08x Mem Size 0x%08x\n",
            program_header->vaddr,
            program_header->paddr,
            program_header->offset,
            program_header->filesz,
            program_header->memsz);
            */
            if (program_header->filesz != 0)
            {
                if (program_header->vaddr == EXTENDED_AREA_START)
                {
                    //debug_printf("Adding dip plugin with binary data in existing ELF section.\n");
                    memcpy(dst + BIN_PATCH_ADDR - EXTENDED_AREA_START, dip_plugin_bin, dip_plugin_bin_size);
                }

                else
                {
                    memcpy(dst, src, program_header->filesz);
                }

                outPos += program_header->filesz;
            }
        }
    }

    return 0;
}

/**
 * Calculate required memory space.
 * @param buffer Pointer to ELF file.
 */
static uint32_t calculate_new_size(uint8_t *buffer)
{
    Elf32_Ehdr_t *file_header;
    int pos = 0;
    int i;
    uint32_t maxSize = 0;

    file_header = (Elf32_Ehdr_t *) & buffer[pos];
    pos += sizeof(Elf32_Ehdr_t);

    if (file_header->magic != ELFMAGIC)
    {
        debug_printf("Magic 0x%08x is wrong.\n", file_header->magic);
        return 0;
    }

    for (i = 0; i < file_header->phnum; i++)
    {
        Elf32_Phdr_t *program_header;

        program_header = (Elf32_Phdr_t *) & buffer[pos];
        pos += sizeof(Elf32_Phdr_t);

        if ( (program_header->type == PT_LOAD)
                && (program_header->memsz != 0) )
        {
            unsigned char *src;

            /*debug_printf("VAddr: 0x%08x PAddr: 0x%08x Offset 0x%08x File Size 0x%08x Mem Size 0x%08x\n",
                                   program_header->vaddr,
             program_header->paddr,
             program_header->offset,
             program_header->filesz,
             program_header->memsz);
                                 */
            src = buffer + program_header->offset;

            if (program_header->filesz != 0)
            {
                uint32_t size;

                size = program_header->offset + program_header->filesz;

                if (size > maxSize)
                {
                    maxSize = size;
                }
            }
        }
    }

    /* Real calculation after getting all information .*/
    return maxSize + BIN_PATCH_ADDR - EXTENDED_AREA_START + dip_plugin_bin_size + sizeof(Elf32_Phdr_t);
}

int add_dip_plugin(uint8_t **buffer)
{
    uint8_t *inElf = NULL;
    uint8_t *outElf = NULL;
    uint32_t outElfSize;
    uint32_t content_size;

    inElf = *buffer;

    debug_printf("Adding DIP plugin\n");

    outElfSize = calculate_new_size(inElf);

    if (outElfSize <= 0)
    {
        debug_printf("add dip plugin Patching failed\n");
        return -32;
    }

    content_size = round_up(outElfSize, 0x40);
    outElf = malloc(content_size);

    if (outElf == NULL)
    {
        debug_printf("Out of memory\n");
        return -33;
    }


    /* Set default to 0. */
    memset(outElf, 0, content_size);

    debug_printf("\n");

    if (copy_sections(inElf, outElf) < 0)
    {
        debug_printf("Failed to patch ELF.\n");
        return -39;
    }


    *buffer = outElf;
    free(inElf);
    return outElfSize;
}

// XXXXXXXXXXXXXXXXXXXX
#endif
