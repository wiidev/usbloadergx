#ifndef STARTUPPROCESS_H_
#define STARTUPPROCESS_H_

#include "libwiigui/gui.h"

class StartUpProcess
{
    public:
        static bool Run();
    private:
        StartUpProcess();
        ~StartUpProcess();
        bool Execute();
        bool USBSpinUp();
        void TextFade(int direction);
        void SetTextf(const char * format, ...);
        void Draw();

        GuiImageData * GXImageData;
        GuiImage * background;
        GuiImage * GXImage;
        GuiText * titleTxt;
        GuiText * messageTxt;
};

#endif
