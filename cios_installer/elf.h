/* Copyright (C) 2008 Mega Man */
#ifndef __ELF_H_
#define	__ELF_H_

#include "stdint.h"

/* File header */
typedef struct Elf32_Ehdr
{
	uint32_t	magic;			/* Magic number */
	unsigned char	info[12];	/* other info */
	uint16_t	type;			/* Object file type */
	uint16_t	machine;		/* Architecture */
	uint32_t	version;		/* Object file version */
	uint32_t	entry;			/* Entry point virtual address */
	uint32_t	phoff;			/* Program header table file offset */
	uint32_t	shoff;			/* Section header table file offset */
	uint32_t	flags;			/* Processor-specific flags */
	uint16_t	ehsize;			/* ELF header size in bytes */
	uint16_t	phentsize;		/* Program header table entry size */
	uint16_t	phnum;			/* Program header table entry count */
	uint16_t	shentsize;		/* Section header table entry size */
	uint16_t	shnum;			/* Section header table entry count */
	uint16_t	shstrndx;		/* Section header string table index */
} Elf32_Ehdr_t;

/* Conglomeration of the identification bytes, for easy testing as a word.  */
#if 0
/** ELF magic for little endian. */
#define	ELFMAGIC	0x464c457f
#else
/** ELF magic for big endian. */
#define	ELFMAGIC	0x7f454c46
#endif

/* Program segment header.  */
typedef struct Elf32_Phdr
{
	uint32_t	type;		/* type */
	uint32_t	offset;		/* file offset */
	uint32_t	vaddr;		/* virtual address */
	uint32_t	paddr;		/* physical address */
	uint32_t	filesz;		/* size in file */
	uint32_t	memsz;		/* size in memory */
	uint32_t	flags;		/* flags */
	uint32_t	align;		/* alignment */
} Elf32_Phdr_t;

/* Possible values for segment type.  */
#define PT_LOAD		1		/* Loadable program segment */

/* Possible values for segment flags.  */
#define PF_X		1		/* executable */
#define PF_W		2		/* writable */
#define PF_R		4		/* readable */

#endif	/* __ELF_H_ */
