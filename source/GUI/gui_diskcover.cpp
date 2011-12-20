#include "gui_diskcover.h"
#include "settings/CSettings.h"

GuiDiskCover::GuiDiskCover()
{
	PosZ = 50;
	Distance = 55;
	OldDegBeta = 0.0;
	deg_beta = 0.0;
	eff_step = 0;
	//  spin_angle = 0;
	spin_speedup = 1.0;
	spin_up = false;
}
GuiDiskCover::GuiDiskCover(GuiImageData *Disk) :
	GuiImage(Disk)
{
	PosZ = 50;
	Distance = 55;
	OldDegBeta = 0.0;
	deg_beta = 0.0;
	eff_step = 0;
	//  spin_angle = 0;
	spin_speedup = 1.0;
	spin_up = false;
}
GuiDiskCover::~GuiDiskCover()
{
}

void GuiDiskCover::SetBeta(f32 beta)
{
	deg_beta = beta;
}
void GuiDiskCover::SetBetaRotateEffect(f32 beta, u16 step)
{
	eff_beta = beta / (f32) step;
	eff_step = step;
}
bool GuiDiskCover::GetBetaRotateEffect()
{
	return eff_step != 0;
}

void GuiDiskCover::SetState(int s, int c)
{
	if(state == STATE_DEFAULT && s == STATE_DISABLED)
	{
		PosZ = 0;
		Distance = 0;
		OldDegBeta = deg_beta;
		deg_beta = 0.0f;
	}
	else if(state == STATE_DISABLED && s == STATE_DEFAULT)
	{
		PosZ = 50;
		Distance = 55;
		deg_beta = OldDegBeta;
	}

	GuiImage::SetState(s, c);
}

void Menu_DrawDiskCover(f32 xpos, f32 ypos, f32 zpos, u16 width, u16 height, u16 distance, u8 data[], f32 deg_alpha,
		f32 deg_beta, f32 scaleX, f32 scaleY, u8 alpha, bool shadow);

void GuiDiskCover::Draw()
{
	LOCK( this );
	if (!image || !this->IsVisible()) return;
	float currScale = this->GetScale();

	Menu_DrawDiskCover(this->GetLeft(), this->GetTop(), PosZ, width, height, Distance, image, imageangle, deg_beta,
			widescreen ? currScale * Settings.WSFactor : currScale, currScale, 64, true);
	Menu_DrawDiskCover(this->GetLeft(), this->GetTop(), PosZ, width, height, Distance, image, imageangle, deg_beta,
			widescreen ? currScale * Settings.WSFactor : currScale, currScale, this->GetAlpha(), false);

	if (eff_step)
	{
		deg_beta += eff_beta;
		eff_step--;
	}
	GuiImage::imageangle += spin_speedup;
	while (GuiImage::imageangle >= 360.0)
		GuiImage::imageangle -= 360.0;

	if (spin_up)
	{
		if (spin_speedup < 11) // speed up
		spin_speedup += 0.20;
	}
	else
	{
		if (spin_speedup > 1) spin_speedup -= 0.05; //slow down
	}

	this->UpdateEffects();
}
