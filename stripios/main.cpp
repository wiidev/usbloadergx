/*   
	IOS ELF stripper, converts traditional ELF files into the format IOS wants.
    Copyright (C) 2008 neimod.
	Copyright (C) 2009-2010 Hermes

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define ELF_NIDENT 16

typedef struct 
{
        unsigned long		ident0;
		unsigned long		ident1;
		unsigned long		ident2;
		unsigned long		ident3;
        unsigned long		machinetype;
        unsigned long		version;
        unsigned long		entry;
        unsigned long       phoff;
        unsigned long       shoff;
        unsigned long		flags;
        unsigned short      ehsize;
        unsigned short      phentsize;
        unsigned short      phnum;
        unsigned short      shentsize;
        unsigned short      shnum;
        unsigned short      shtrndx;
} elfheader;

typedef struct 
{
       unsigned long      type;
       unsigned long      offset;
       unsigned long      vaddr;
       unsigned long      paddr;
       unsigned long      filesz;
       unsigned long      memsz;
       unsigned long      flags;
       unsigned long      align;
} elfphentry;

typedef struct
{
	unsigned long offset;
	unsigned long size;
} offset_size_pair;

unsigned short getbe16(void* pvoid)
{
	unsigned char* p = (unsigned char*)pvoid;

	return (p[0] << 8) | (p[1] << 0);
}

unsigned long getbe32(void* pvoid)
{
	unsigned char* p = (unsigned char*)pvoid;

	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | (p[3] << 0);
}

void putbe16(void* pvoid, unsigned short val)
{
	unsigned char* p = (unsigned char*)pvoid;

	p[0] = val >> 8;
	p[1] = val >> 0;
}

void putbe32(void* pvoid, unsigned long val)
{
	unsigned char* p = (unsigned char*)pvoid;

	p[0] = val >> 24;
	p[1] = val >> 16;
	p[2] = val >> 8;
	p[3] = val >> 0;
}

int main(int argc, char* argv[])
{
	int result = 0;

unsigned long strip=0;

	fprintf(stdout, "stripios - IOS ELF stripper - by neimod\n");
	if (argc < 3 || argc==4)
	{
		fprintf(stderr,"Usage: %s <in.elf> <out.elf> [strip addr]\n", argv[0]);

		return -1;
	}
	else
	if(argc==5)
	{
	if(!strcmp(argv[3],"strip"))
		{
		 sscanf( argv[4], "%x",&strip );

		printf("strip: %x\n",strip);
		}
	else
		{
		fprintf(stderr,"Usage: %s <in.elf> <out.elf> [strip addr]\n", argv[0]);

		return -1;
		}
	}

	FILE* fin = fopen(argv[1], "rb");
	FILE* fout = fopen(argv[2], "wb");

	if (fin == 0 || fout == 0)
	{
		if (fin == 0)
			fprintf(stderr,"ERROR opening file %s\n", argv[1]);
		if (fout == 0)
			fprintf(stderr,"ERROR opening file %s\n", argv[2]);
		return 1;
	}

	elfheader header;

	if (fread(&header, sizeof(elfheader), 1, fin) != 1)
	{
		fprintf(stderr,"ERROR reading ELF header\n");
		return 1;
	}

	unsigned long elfmagicword = getbe32(&header.ident0);

	if (elfmagicword != 0x7F454C46)
	{
		fprintf(stderr,"ERROR not a valid ELF\n");
		return 1;
	}

	unsigned long phoff = getbe32(&header.phoff);
	unsigned short phnum = getbe16(&header.phnum);
	unsigned long memsz = 0, filesz = 0;
	unsigned long vaddr = 0, paddr = 0;

	putbe32(&header.ident1, 0x01020161);
	putbe32(&header.ident2, 0x01000000);
	putbe32(&header.ident3, 0);
	putbe32(&header.machinetype, 0x20028);
	putbe32(&header.version, 1);
	putbe32(&header.flags, 0x606);
	putbe16(&header.ehsize, 0x34);
	putbe16(&header.phentsize, 0x20);
	putbe16(&header.shentsize, 0);
	putbe16(&header.shnum, 0);
	putbe16(&header.shtrndx, 0);
	putbe32(&header.phoff, 0x34);
	putbe32(&header.shoff, 0);

	putbe16(&header.phnum, phnum + 2);

	elfphentry* origentries = new elfphentry[phnum];

	fseek(fin, phoff, SEEK_SET);
	if (fread(origentries, sizeof(elfphentry), phnum, fin) != phnum)
	{
		fprintf(stderr,"ERROR reading program header\n");
		return 1;
	}


	elfphentry* iosphentry = 0;


	// Find zero-address phentry
	for(int i=0; i<phnum; i++)
	{
		elfphentry* phentry = &origentries[i];

		if (getbe32(&phentry->paddr) == 0)
		{
			iosphentry = phentry;
		}
	}

	if (0 == iosphentry)
	{
		fprintf(stderr,"ERROR IOS table not found in program header\n");
		return 1;
	}
	

	elfphentry* entries = new elfphentry[phnum+2];
	offset_size_pair* offsetsizes = new offset_size_pair[phnum];

	elfphentry* q = entries;
	phoff = 0x34;

	for(int i=0; i<phnum; i++)
	{
		elfphentry phentry;
		elfphentry* p = &origentries[i];

		offsetsizes[i].offset = getbe32(&p->offset);
		offsetsizes[i].size = getbe32(&p->filesz);

		if (p == iosphentry)
		{
			unsigned long startoffset = phoff;
			unsigned long startvaddr = vaddr;
			unsigned long startpaddr = paddr;
			unsigned long totalsize = 0;

			filesz = memsz = (phnum+2) * 0x20;

			// PHDR 
			putbe32(&phentry.type, 6);
			putbe32(&phentry.offset, phoff);
			putbe32(&phentry.vaddr, paddr);
			putbe32(&phentry.paddr, vaddr);
			putbe32(&phentry.filesz, filesz);
			putbe32(&phentry.memsz, memsz);
			putbe32(&phentry.flags, 0x00F00000);
			putbe32(&phentry.align, 0x4);

			*q++ = phentry;	

			phoff += memsz;
			paddr += memsz;
			vaddr += memsz;
			totalsize += memsz;

			filesz = memsz = getbe32(&p->memsz);

			

			// NOTE 
			putbe32(&phentry.type, 4);
			putbe32(&phentry.offset, phoff);
			putbe32(&phentry.vaddr, vaddr);
			putbe32(&phentry.paddr, paddr);
			putbe32(&phentry.filesz, filesz);
			putbe32(&phentry.memsz, memsz);
			putbe32(&phentry.flags, 0x00F00000);
			putbe32(&phentry.align, 0x4);



			*q++ = phentry;	

			phoff += memsz;
			paddr += memsz;
			vaddr += memsz;
			totalsize += memsz;

			filesz = memsz = totalsize;

			// LOAD
			putbe32(&phentry.type, 1);
			putbe32(&phentry.offset, startoffset);
			putbe32(&phentry.vaddr, startvaddr);
			putbe32(&phentry.paddr, startpaddr);
			putbe32(&phentry.filesz, totalsize);
			putbe32(&phentry.memsz, totalsize);
			putbe32(&phentry.flags, 0x00F00000);
			putbe32(&phentry.align, 0x4000);

			*q++ = phentry;
		}
		else
		{

			filesz = getbe32(&p->filesz);
			memsz = getbe32(&p->memsz);
			//printf("flags %x\n",getbe32(&p->flags));
			if(strip==getbe32(&p->vaddr) && strip!=0) // strip zeroes
				{
				filesz=1;
				putbe32(&p->filesz,filesz);
				}
		  

			putbe32(&phentry.type, getbe32(&p->type));
			putbe32(&phentry.offset, phoff);
			putbe32(&phentry.vaddr, getbe32(&p->vaddr));
			putbe32(&phentry.paddr, getbe32(&p->paddr));
			putbe32(&phentry.filesz, filesz);
			putbe32(&phentry.memsz, memsz);
			putbe32(&phentry.flags, (getbe32(&p->flags) | 0xf00000));
			putbe32(&phentry.align, 0x4);

			*q++ = phentry;

			phoff += filesz;
		}
	}
	
	if (fwrite(&header, sizeof(elfheader), 1, fout) != 1)
	{
		fprintf(stderr,"ERROR writing ELF header\n");
		return 1;
	}

	if (fwrite(entries, sizeof(elfphentry), phnum+2, fout) != (phnum+2))
	{
		fprintf(stderr,"ERROR writing ELF program header\n");
		return 1;
	}

	for(int i=0; i<phnum; i++)
	{
		elfphentry *p = &origentries[i];

		unsigned long offset = getbe32(&p->offset);
		unsigned long filesz = getbe32(&p->filesz);

		if (filesz)
		{
			fseek(fin, offset, SEEK_SET);
            
			fprintf(stdout,"Writing segment 0x%08X to 0x%08X (%d bytes) - %x %s\n", getbe32(&p->vaddr), ftell(fout), filesz,getbe32(&p->memsz),
				(strip==getbe32(&p->vaddr)  && strip!=0 )? "- Stripped" : "");

			unsigned char* data = new unsigned char[filesz];

			
			if (fread(data, filesz, 1, fin) != 1 || fwrite(data, filesz, 1, fout) != 1)
			{
				fprintf(stderr,"ERROR writing segment\n");
				delete[] data;
				return 1;
			}

			delete[] data;
		}
		else
		{
			fprintf(stdout,"Skipping segment 0x%08X\n", getbe32(&p->vaddr));
		}
	}

cleanup:
	if (offsetsizes)
		delete[] offsetsizes;
	if (entries)
		delete[] entries;
	if (origentries)
		delete[] origentries;
	if (fout)
		fclose(fout);

	if (fin)
		fclose(fin);

	return result;
}

