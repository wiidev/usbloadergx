#pragma once
#include "RiivolutionMods.hpp"
#include "settings/menus/SettingsMenu.hpp"
#include "settings/CGameSettings.h"
#include <vector>
#include <string>
#include "GUI/OptionList.hpp"
#include "GUI/gui_optionbrowser.h"
#include "themes/CTheme.h"
#include "GUI/gui.h"

class RiivolutionModsSM : public SettingsMenu
{
public:
    RiivolutionModsSM(std::vector<RiivolutionModXml>& mods, bool wide);
    RiivolutionModsSM(RiivolutionModXml& mod, bool allowSelect, bool wide); // For single-mod shortcut
    virtual ~RiivolutionModsSM();

    bool Show();
    bool wantsSelectAnother() const;

    std::vector<RiivolutionModXml>& modList;
    int currentModIdx;

protected:
    void SetOptionNames();
    void SetOptionValues();
    int ShowModPicker();

    RiivolutionModXml* currentMod;
    bool selectAnother;
    bool isWide;

    OptionList GuiOptions;

    GuiText* saveBtnTxt;
    GuiImage* saveBtnImg;
    GuiButton* saveBtn;
    GuiText* selectModBtnTxt;
    GuiImage* selectModBtnImg;
    GuiButton* selectModBtn;
    GuiTrigger* trigA;
};