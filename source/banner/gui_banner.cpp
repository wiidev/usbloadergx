/****************************************************************************
 * USB Loader GX Team
 * gui_banner.cpp
 *
 * Shows TPL Banner images
 ***************************************************************************/
#include "gui_banner.h"

typedef struct
{
	u32 texture_header_offset;
	u32 palette_header_offset;
} TPLTexture;

typedef struct
{
	u16 heigth;
	u16 width;
	//...
	//there is more but we only need these
} TPLTextureHeader;

//only one field tpls
typedef struct
{
    u32 magic;
    u32 ntextures;
    u32 texture_size;
    TPLTexture textures;
} TPLHeader;


GuiBanner::GuiBanner(const char *tplfilepath)
{
    memory = NULL;
    tplfilesize = 0;
    width = 0;
    height = 0;

	FILE *tplfp = fopen(tplfilepath,"rb");

	if(tplfp !=NULL) {

		fseek (tplfp , 0 , SEEK_END);
        tplfilesize = ftell (tplfp);
        rewind (tplfp);
        memory = memalign(32, tplfilesize);
        if(!memory) {
            fclose(tplfp);
            return;
        }
        fread(memory, 1, tplfilesize, tplfp);
		fclose(tplfp);

		const u8 * buffer = (const u8*) memory;
        const TPLHeader *hdr = (TPLHeader *) buffer;
        const TPLTextureHeader *texhdr = (TPLTextureHeader *) &buffer[hdr->textures.texture_header_offset];

        height = texhdr[0].heigth;
        width = texhdr[0].width;

        TPLFile tplfile;
        int ret;

        ret = TPL_OpenTPLFromMemory(&tplfile, memory, tplfilesize);
        if(ret < 0) {
            free(memory);
            memory = NULL;
            return;
        }
        ret = TPL_GetTexture(&tplfile,0,&texObj);
        if(ret < 0) {
            free(memory);
            memory = NULL;
            return;
        }
        TPL_CloseTPLFile(&tplfile);

		widescreen = 0;
		filecheck = true;

    } else {
		filecheck = false;
		fclose(tplfp);
    }
}

GuiBanner::GuiBanner(void *mem, u32 len)
{
    if(!mem || !len)
        return;

    memory = mem;
    tplfilesize = len;

	const u8 * buffer = (const u8*) memory;
    const TPLHeader *hdr = (TPLHeader *) buffer;
    const TPLTextureHeader *texhdr = (TPLTextureHeader *) &buffer[hdr->textures.texture_header_offset];

    height = texhdr[0].heigth;
    width = texhdr[0].width;

    TPLFile tplfile;

    int ret;

    ret = TPL_OpenTPLFromMemory(&tplfile, memory, tplfilesize);
    if(ret < 0) {
        free(memory);
        memory = NULL;
        return;
    }
    ret = TPL_GetTexture(&tplfile,0,&texObj);
    if(ret < 0) {
        free(memory);
        memory = NULL;
        return;
    }
    TPL_CloseTPLFile(&tplfile);

    filecheck = true;
}

GuiBanner::~GuiBanner()
{
    if(memory != NULL) {
        free(memory);
        memory = NULL;
    }
}

void GuiBanner::Draw()
{
	LOCK(this);
	if(!filecheck ||!this->IsVisible())
		return;

	float currScale = this->GetScale();

    Menu_DrawTPLImg(this->GetLeft(), this->GetTop(), 0, width, height, &texObj, imageangle, widescreen ? currScale*0.80 : currScale, currScale, this->GetAlpha(), xx1,yy1,xx2,yy2,xx3,yy3,xx4,yy4);

	this->UpdateEffects();
}
