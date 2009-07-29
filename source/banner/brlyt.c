/*
 *  brlyt.h
 *  Parses brlyt file
 *
 *  by nIxx
 *  http://wiibrew.org/wiki/Wii_Animations#Textures_and_Material_lists_.28.2A.brlyt.29 
 *  
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fat.h>

#include "brlyt.h"
#include "openingbnr.h"

int BRLYT_Initialize(const char *rootpath)
{
//fatInitDefault();
FILE * fp = fopen(rootpath,"rb");

if (fp == NULL) return 0;

brlyt_header brlytheader;
lyt1_header lyt1header;
txl1_header txl1header;

fread((void*)&brlytheader,1,sizeof(brlytheader),fp);
fread((void*)&lyt1header,1,sizeof(lyt1header),fp);
fread((void*)&txl1header,1,sizeof(txl1header),fp);

//printf("Filesize: %i\n",be32((u8*)&brlytheader.file_size));
//printf("Num Textures: %i\n",be16((u8*)&txl1header.num_textures));

txl1_offset **txl1offsets = (txl1_offset**)malloc(sizeof(txl1_offset*)*be16((u8*)&txl1header.num_textures));

if(txl1offsets == NULL)
	{
	fprintf(stderr, "out of memory\n");
	return 0;
	}

int i = 0;
for(i = 0; i < be16((u8*)&txl1header.num_textures); i++)
	{
	txl1offsets[i] = (txl1_offset*)malloc(sizeof(txl1_offset));
	if(txl1offsets[i] == NULL)
		{
		fprintf(stderr, "out of memory\n");
		return 0;
		}
	fread(txl1offsets[i],1,sizeof(*txl1offsets[i]),fp);
	//printf("Offset Filename: %i\n",be32((u8*)&txl1offsets[i]->offset_filename));
	}

tpl_files **tplss = (tpl_files**)malloc(sizeof(tpl_files*)*be16((u8*)&txl1header.num_textures));
if(tplss == NULL)
	{
	fprintf(stderr, "out of memory\n");
	return 0;
	}

for(i = 0; i < be16((u8*)&txl1header.num_textures); i++)
	{
	tplss[i] = (tpl_files*)malloc(sizeof(tpl_files));
	if(tplss[i] == NULL)
		{
		fprintf(stderr, "out of memory\n");
		return 0;
		}

	fseek(fp,0x30+ be32((u8*)&txl1offsets[i]->offset_filename),SEEK_SET);
	fread(tplss[i],1,sizeof(*tplss[i]),fp);
	//printf("%i. Filename: %s\n",i,tplss[i]->tplfilename);
	}

fseek(fp,0x10+be32((u8*)&lyt1header.size_header)+be32((u8*)&txl1header.size_section),SEEK_SET);

mat1_header mat1header;
fread((void*)&mat1header,1,sizeof(mat1header),fp);
//printf("Sig: %s SizeHeader:%i Num materials%i\n",mat1header.sig ,be32((u8*)&mat1header.size_section) ,be16((u8*)&mat1header.num_materials) );

mat1_offset **mat1offsets = (mat1_offset**)malloc(sizeof(mat1_offset*)*be16((u8*)&mat1header.num_materials));
if(mat1offsets == NULL)
	{
	fprintf(stderr, "out of memory\n");
	return 0;
	}

for(i = 0; i < be16((u8*)&mat1header.num_materials); i++)
	{
	mat1offsets[i] = (mat1_offset*)malloc(sizeof(mat1_offset));
	if(mat1offsets[i] == NULL)
		{
		fprintf(stderr, "out of memory\n");
		return 0;
		}

	fread(mat1offsets[i],1,sizeof(*mat1offsets[i]),fp);
	//printf("%i. Material Offset: %X\n",i,be32((u8*)&mat1offsets[i]->offset));
	}

mat1_material **mat1materials = (mat1_material**)malloc(sizeof(mat1_material*)*be16((u8*)&mat1header.num_materials));
if(mat1materials == NULL)
	{
	fprintf(stderr, "out of memory\n");
	return 0;
	}

for(i = 0; i < be16((u8*)&mat1header.num_materials); i++)
	{
	mat1materials[i] = (mat1_material*)malloc(sizeof(mat1_material));
	if(mat1materials[i] == NULL)
		{
		fprintf(stderr, "out of memory\n");
		return 0;
		}
	fseek(fp,0x10+be32((u8*)&lyt1header.size_header)+be32((u8*)&txl1header.size_section)+be32((u8*)&mat1offsets[i]->offset),SEEK_SET);
	fread(mat1materials[i],1,sizeof(*mat1materials[i]),fp);
	//printf("%i. Material Names: %s\n",i,mat1materials[i]->name);
	}

fseek(fp,0x10+be32((u8*)&lyt1header.size_header)+be32((u8*)&txl1header.size_section)+be32((u8*)&mat1header.size_section),SEEK_SET);

pan1_header pan1header;
fread((void*)&pan1header,1,sizeof(pan1header),fp);

pas1_header pas1header;
fread((void*)&pas1header,1,sizeof(pas1header),fp);

pic1_header **pic1header = (pic1_header**)malloc(sizeof(pic1_header*)*be16((u8*)&mat1header.num_materials));
if(pic1header == NULL)
	{
	fprintf(stderr, "out of memory\n");
	return 0;
	}

for(i = 0; i < be16((u8*)&mat1header.num_materials); i++)
	{
	pic1header[i] = (pic1_header*)malloc(sizeof(pic1_header));
	if(pic1header[i] == NULL)
		{
		fprintf(stderr, "out of memory\n");
		return 0;
		}
	fpos_t pos;
	fgetpos(fp,&pos);		

	fread(pic1header[i],1,sizeof(*pic1header[i]),fp);
	fseek(fp,pos+be32((u8*)&pic1header[i]->size_section),SEEK_SET);
//	printf("%i. Pic1 Names: %s\n",i,pic1header[i]->name);
	}

pae1_header pae1header;
fread((void*)&pae1header,1,sizeof(pae1header),fp);

grp1_header grp1header;
fread((void*)&grp1header,1,sizeof(grp1header),fp);

//Close File
fclose(fp);

//free memory
for(i = 0; i < be16((u8*)&txl1header.num_textures); i++)
	free(txl1offsets[i]);

free(txl1offsets);

for(i = 0; i < be16((u8*)&txl1header.num_textures); i++)
	free(tplss[i]);

free(tplss);

for(i = 0; i < be16((u8*)&mat1header.num_materials); i++)
	free(mat1offsets[i]);

free(mat1offsets);

for(i = 0; i < be16((u8*)&mat1header.num_materials); i++)
	free(mat1materials[i]);

free(mat1materials);

for(i = 0; i < be16((u8*)&mat1header.num_materials); i++)
	free(pic1header[i]);

free(pic1header);

return 1;
}

int BRLYT_ReadObjects(BRLYT_object** objs)
{
	return 0;
}

void BRLYT_Finish()
{

}
