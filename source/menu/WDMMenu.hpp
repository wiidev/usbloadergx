#ifndef WDMMENU_HPP_
#define WDMMENU_HPP_

#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "usbloader/disc.h"
#include "usbloader/WDMFile.hpp"

class WDMMenu : public GuiWindow
{
	public:
        WDMMenu(const struct discHdr * header);
        ~WDMMenu();
        int GetChoice();
        static int Show(const struct discHdr * header);
        static u32 GetAlternateDolOffset() { return AlternateDolOffset; }
        static u32 GetDolParameter() { return AlternateDolParameter; }
    private:
		void CheckGameFiles(const struct discHdr * header);

		static u32 AlternateDolOffset;
		static u32 AlternateDolParameter;

        WDMFile * wdmFile;
        vector<pair<int, int> > DOLOffsetList;
        GuiImageData * btnOutline;

        GuiTrigger * trigA;
        GuiTrigger * trigB;

        OptionList * Options;

        GuiText * backBtnTxt;
        GuiImage * backBtnImg;
        GuiButton * backBtn;

        GuiText * defaultBtnTxt;
        GuiImage * defaultBtnImg;
        GuiButton * defaultBtn;

        GuiCustomOptionBrowser * optionBrowser;
};

#endif
