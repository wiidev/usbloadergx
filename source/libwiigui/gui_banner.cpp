#include "gui_banner.h"

GuiBanner::GuiBanner()
{
	deg_beta=0.0;
}

GuiBanner::~GuiBanner()
{

}

GuiBanner::GuiBanner(const char *tplfilepath)
{
	FILE *tplfp = fopen(tplfilepath,"rb");

	if(tplfp !=NULL)
		{
		unsigned short heighttemp = 0;
		unsigned short widthtemp = 0;
		
		fseek(tplfp , 0x14, SEEK_SET);
		fread((void*)&heighttemp,1,2,tplfp);
		fread((void*)&widthtemp,1,2,tplfp);
		fclose(tplfp);

		filepath = tplfilepath;
		width = widthtemp;
		height = heighttemp;
		widescreen = 0;
		filecheck = true;
		}
	else 
		{
		filecheck = false;
		fclose(tplfp);
		}
}

GuiBanner::GuiBanner(void *mem,u32 len,u16 w, u16 h)
{
	if(memory !=NULL)
		{
		memory = mem;
		width = w;
		height = h;
		widescreen = 0;
		filecheck = true;
		free(mem);
		}
	else 
		{
		filecheck = false;
		free(mem);
		}

}
//void Menu_DrawTPL(f32 xpos, f32 ypos, f32 zpos, u16 width, u16 height, u16 distance,const char *filepath,f32 deg_alpha, f32 deg_beta, f32 scaleX, f32 scaleY, u8 alpha, bool shadow);
//void Menu_DrawTPLMem(f32 xpos, f32 ypos, f32 zpos, u16 width, u16 height, u16 distance,const void *memory,u32 len,	f32 deg_alpha, f32 deg_beta, f32 scaleX, f32 scaleY, u8 alpha, bool shadow);

void GuiBanner::Draw()
{
	LOCK(this);
	if(!filecheck ||!this->IsVisible())
		return;
	float currScale = this->GetScale();
    //Menu_DrawTPL(50,70, 20, width, height, 50, filepath, 1, deg_beta, widescreen ? currScale*0.8 : currScale, currScale, 64, true);
	Menu_DrawTPL(xoffset,yoffset, 50, width, height, 50, filepath, 0, deg_beta, widescreen ? currScale*0.8 : currScale, currScale, 64, true);
	
	this->UpdateEffects();
}
