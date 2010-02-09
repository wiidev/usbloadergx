/****************************************************************************
 * USB Loader GX Team
 * gui_banner.h
 *
 * Shows TPL Banner images
 ***************************************************************************/

#ifndef _GUIBANNER_H_
#define _GUIBANNER_H_

#include "libwiigui/gui.h"

class GuiBanner  : public GuiImage {
public:
    //!Constructor
    //!\param tplfilepath Path of the tpl file
    GuiBanner(const char *tplfilepath);
    //!Constructor
    //!\param mem Memory of the loaded tpl
    //!\param len Filesize of the tpl
    //!\param w Width of the tpl
    //!\param h Height of the tpl
    GuiBanner(void *mem, u32 len, int w, int h);
    //!Destructor
    ~GuiBanner();
    void Draw();
protected:
    void * memory;
    bool filecheck;
    u32 tplfilesize;
    GXTexObj texObj;
};

#endif /* _GUIBANNER_H_ */
