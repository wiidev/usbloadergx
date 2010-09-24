#ifndef _GUIDISCCOVER_H_
#define _GUIDISCCOVER_H_

#include "gui.h"

class GuiDiskCover: public GuiImage
{
    public:
        GuiDiskCover();
        GuiDiskCover(GuiImageData * img);
        ~GuiDiskCover();
        void SetBeta(f32 beta);
        void SetBetaRotateEffect(f32 beta, u16 Step);
        bool GetBetaRotateEffect();

        void SetSpin(bool Up);
        void Draw();
    private:
        f32 deg_beta;
        f32 eff_beta;
        u16 eff_step;

        //  f32 spin_angle;
        f32 spin_speedup;
        bool spin_up;
};

#endif /* _GUIDISCCOVER_H_ */
