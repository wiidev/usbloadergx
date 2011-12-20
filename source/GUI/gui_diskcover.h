#ifndef _GUIDISCCOVER_H_
#define _GUIDISCCOVER_H_

#include "gui.h"

class GuiDiskCover: public GuiImage
{
	public:
		GuiDiskCover();
		GuiDiskCover(GuiImageData * img);
		virtual ~GuiDiskCover();
		void SetBeta(f32 beta);
		void SetBetaRotateEffect(f32 beta, u16 Step);
		bool GetBetaRotateEffect();

		void SetSpin(bool Up) { spin_up = Up; }
		void SetState(int s, int c = -1);
		void Draw();
	private:
		f32 deg_beta;
		f32 eff_beta;
		u16 eff_step;

		//  f32 spin_angle;
		f32 spin_speedup;
		int PosZ;
		int Distance;
		f32 OldDegBeta;
		bool spin_up;
};

#endif /* _GUIDISCCOVER_H_ */
