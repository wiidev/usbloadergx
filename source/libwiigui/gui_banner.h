#ifndef _GUIBANNER_H_
#define _GUIBANNER_H_

#include "gui.h"

class GuiBanner  : public GuiImage
{
public:
	GuiBanner();
	GuiBanner(const char *tplfilepath);
	GuiBanner(void *mem,u32 len,u16 w, u16 h);
	~GuiBanner();
	
	void Draw();
private:
	f32 deg_beta;
	const char *filepath;
	const void *memory;
	bool filecheck;
};

#endif /* _GUIDISCCOVER_H_ */
