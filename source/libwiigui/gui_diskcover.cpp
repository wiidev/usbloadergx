#include "gui_diskcover.h"

GuiDiskCover::GuiDiskCover()
{
    deg_beta = 0.0;
    eff_step = 0;
    //  spin_angle = 0;
    spin_speedup = 1.0;
    spin_up = false;
}
GuiDiskCover::GuiDiskCover(GuiImageData *Disk) :
    GuiImage(Disk)
{
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

void GuiDiskCover::SetSpin(bool Up)
{
    spin_up = Up;
}

void Menu_DrawDiskCover(f32 xpos, f32 ypos, f32 zpos, u16 width, u16 height, u16 distance, u8 data[], f32 deg_alpha,
        f32 deg_beta, f32 scaleX, f32 scaleY, u8 alpha, bool shadow);
void Menu_DrawDiskCoverShadow(f32 xpos, f32 ypos, f32 zpos, u16 width, u16 height, u16 distance, u8 data[],
        f32 deg_alpha, f32 deg_beta, f32 scaleX, f32 scaleY, u8 alpha, bool shadow);

void GuiDiskCover::Draw()
{
    LOCK( this );
    if (!image || !this->IsVisible()) return;
    float currScale = this->GetScale();
    //  Menu_DrawDiskCoverShadow(this->GetLeft(), this->GetTop(), 190, width, height, 40, image, imageangle, deg_beta, widescreen ? currScale*0.8 : currScale, currScale, this->GetAlpha(), true);
    Menu_DrawDiskCover(this->GetLeft(), this->GetTop(), 50, width, height, 55, image, imageangle, deg_beta,
            widescreen ? currScale * 0.8 : currScale, currScale, 64, true);
    Menu_DrawDiskCover(this->GetLeft(), this->GetTop(), 50, width, height, 55, image, imageangle, deg_beta,
            widescreen ? currScale * 0.8 : currScale, currScale, this->GetAlpha(), false);

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
