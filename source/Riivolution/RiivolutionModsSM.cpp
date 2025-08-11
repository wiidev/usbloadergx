#include <unistd.h>
#include <gccore.h>
#include "settings/CSettings.h"
#include "themes/CTheme.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "RiivolutionModsSM.hpp"
#include "RiivolutionMods.hpp"
#include "themes/Resources.h"
#include "GUI/OptionList.hpp"
#include "GUI/gui_optionbrowser.h"
#include "GUI/gui.h"
#include "sys.h"

static const char* DisabledText = "Disabled";

RiivolutionModsSM::RiivolutionModsSM(std::vector<RiivolutionModXml>& mods, bool wide)
    : SettingsMenu(mods.size() ? mods[0].sectionTitle.c_str() : tr("No Mods Found"), &GuiOptions, MENU_NONE),
      modList(mods),
      currentModIdx(0),
      currentMod(mods.size() ? &mods[0] : nullptr),
      selectAnother(false),
      isWide(wide),
      saveBtnTxt(nullptr),
      saveBtnImg(nullptr),
      saveBtn(nullptr),
      selectModBtnTxt(nullptr),
      selectModBtnImg(nullptr),
      selectModBtn(nullptr),
      trigA(nullptr)
{
    SetOptionNames();
    SetOptionValues();

    optionBrowser = new GuiOptionBrowser(400, 360, &GuiOptions, "bg_options_settings.png", false);
    optionBrowser->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    optionBrowser->SetPosition(30, 50);
    Append(optionBrowser);

    // Save button
    GuiImageData* btnOutline = Resources::GetImageData("button_dialogue_box.png");
    saveBtnImg = new GuiImage(btnOutline);
    saveBtnTxt = new GuiText(tr("Save"), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
    saveBtnTxt->SetMaxWidth(btnOutline->GetWidth() - 30);
    trigA = new GuiTrigger();
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    saveBtn = new GuiButton(saveBtnImg, saveBtnImg, 2, 3, 380, 400, trigA, btnSoundOver, btnSoundClick2, 1);
    saveBtn->SetLabel(saveBtnTxt);
    Append(saveBtn);

    // "Select Another Mod" button
    if (mods.size() > 1) {
        selectModBtnImg = new GuiImage(btnOutline);
        selectModBtnTxt = new GuiText(tr("Select Another Mod"), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
        selectModBtnTxt->SetMaxWidth(btnOutline->GetWidth() - 30);
        selectModBtn = new GuiButton(selectModBtnImg, selectModBtnImg, 2, 3, 30, 400, trigA, btnSoundOver, btnSoundClick2, 1);
        selectModBtn->SetLabel(selectModBtnTxt);
        Append(selectModBtn);
    }

    SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
    SetPosition(0, 0);
}

RiivolutionModsSM::RiivolutionModsSM(RiivolutionModXml& mod, bool allowSelect, bool wide)
    : SettingsMenu(mod.sectionTitle.c_str(), &GuiOptions, MENU_NONE),
      modList(*new std::vector<RiivolutionModXml>{mod}), // Dangerous: only for temporary/single-use
      currentModIdx(0),
      currentMod(&mod),
      selectAnother(false),
      isWide(wide),
      saveBtnTxt(nullptr),
      saveBtnImg(nullptr),
      saveBtn(nullptr),
      selectModBtnTxt(nullptr),
      selectModBtnImg(nullptr),
      selectModBtn(nullptr),
      trigA(nullptr)
{
    SetOptionNames();
    SetOptionValues();

    optionBrowser = new GuiOptionBrowser(400, 360, &GuiOptions, "bg_options_settings.png", false);
    optionBrowser->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    optionBrowser->SetPosition(30, 50);
    Append(optionBrowser);

    // Save button
    GuiImageData* btnOutline = Resources::GetImageData("button_dialogue_box.png");
    saveBtnImg = new GuiImage(btnOutline);
    saveBtnTxt = new GuiText(tr("Save"), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
    saveBtnTxt->SetMaxWidth(btnOutline->GetWidth() - 30);
    trigA = new GuiTrigger();
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    saveBtn = new GuiButton(saveBtnImg, saveBtnImg, 2, 3, 380, 400, trigA, btnSoundOver, btnSoundClick2, 1);
    saveBtn->SetLabel(saveBtnTxt);
    Append(saveBtn);

    // "Select Another Mod" button if allowed
    if (allowSelect) {
        selectModBtnImg = new GuiImage(btnOutline);
        selectModBtnTxt = new GuiText(tr("Select Another Mod"), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
        selectModBtnTxt->SetMaxWidth(btnOutline->GetWidth() - 30);
        selectModBtn = new GuiButton(selectModBtnImg, selectModBtnImg, 2, 3, 30, 400, trigA, btnSoundOver, btnSoundClick2, 1);
        selectModBtn->SetLabel(selectModBtnTxt);
        Append(selectModBtn);
    }

    SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
    SetPosition(0, 0);
}

RiivolutionModsSM::~RiivolutionModsSM()
{
    HaltGui();
    Remove(optionBrowser);
    Remove(saveBtn);
    if (selectModBtn) Remove(selectModBtn);
    delete optionBrowser;
    delete saveBtnTxt;
    delete saveBtnImg;
    delete saveBtn;
    delete trigA;
    if (selectModBtnTxt) delete selectModBtnTxt;
    if (selectModBtnImg) delete selectModBtnImg;
    if (selectModBtn) delete selectModBtn;
    ResumeGui();
}

void RiivolutionModsSM::SetOptionNames()
{
    GuiOptions.ClearList();
    if (!currentMod) return;
    int idx = 0;
    for (size_t i = 0; i < currentMod->options.size(); ++i)
        GuiOptions.SetName(idx++, "%s", currentMod->options[i].name.c_str());
}

void RiivolutionModsSM::SetOptionValues()
{
    if (!currentMod) return;
    int idx = 0;
    for (size_t i = 0; i < currentMod->options.size(); ++i)
    {
        const RiivoOption& opt = currentMod->options[i];
        const std::string& choice = (opt.selected >= 0 && opt.selected < (int)opt.choices.size())
            ? opt.choices[opt.selected].name
            : DisabledText;
        GuiOptions.SetValue(idx++, "%s", choice.c_str());
    }
}

int RiivolutionModsSM::ShowModPicker()
{
    // Show a simple mod picker menu (scrollable if needed)
    OptionList modPickOptions;
    for (size_t i = 0; i < modList.size(); ++i)
        modPickOptions.SetName(i, "%s", modList[i].sectionTitle.c_str());
    for (size_t i = 0; i < modList.size(); ++i)
        modPickOptions.SetValue(i, "%s", "");

    GuiImageData* btnOutline = Resources::GetImageData("button_dialogue_box.png");
    GuiImage* pickBackImg = new GuiImage(btnOutline);
    GuiText* pickBackTxt = new GuiText(tr("Cancel"), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
    pickBackTxt->SetMaxWidth(btnOutline->GetWidth() - 30);
    GuiButton* modPickBackBtn = new GuiButton(pickBackImg, pickBackImg, 2, 3, 30, 400, trigA, btnSoundOver, btnSoundClick2, 1);
    modPickBackBtn->SetLabel(pickBackTxt);

    GuiOptionBrowser* modPickBrowser = new GuiOptionBrowser(400, 360, &modPickOptions, "bg_options_settings.png", false);
    modPickBrowser->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    modPickBrowser->SetPosition(30, 50);

    Append(modPickBrowser);
    Append(modPickBackBtn);

    int ret = -1;
    modPickBrowser->ResetState();
    modPickBackBtn->ResetState();

    while (ret < 0)
    {
        Draw();
        Update(&userInput[0]);
        usleep(1000);

        if (modPickBackBtn->GetState() == STATE_CLICKED) {
            ret = -2; // Cancelled
            modPickBackBtn->ResetState();
        }

        ret = modPickBrowser->GetClickedOption();
    }

    // Remove GUI
    Remove(modPickBrowser);
    Remove(modPickBackBtn);
    delete modPickBrowser;
    delete modPickBackBtn;
    delete pickBackImg;
    delete pickBackTxt;

    if (ret >= 0 && ret < (int)modList.size())
        currentModIdx = ret;
    currentMod = &modList[currentModIdx];
    // Set menu title:
    titleTxt->SetText(currentMod->sectionTitle.c_str());
    SetOptionNames();
    SetOptionValues();

    return currentModIdx;
}

bool RiivolutionModsSM::Show()
{
    int ret = -1;
    selectAnother = false;
    optionBrowser->ResetState();
    if (selectModBtn) selectModBtn->ResetState();

    // If more than one mod, show picker first
    if (modList.size() > 1) {
        ShowModPicker();
    }

    while (ret < 0)
    {
        SetOptionValues();

        Draw();
        Update(&userInput[0]);
        usleep(1000);

        if (saveBtn->GetState() == STATE_CLICKED)
        {
            if (RiivolutionMods::SaveConfig(currentMod->regionalId, *currentMod))
                WindowPrompt(tr("Success"), tr("Mod configuration saved."), tr("OK"));
            else
                WindowPrompt(tr("Error"), tr("Failed to save. Is the SD Card inserted?"), tr("OK"));
            saveBtn->ResetState();
        }

        if (selectModBtn && selectModBtn->GetState() == STATE_CLICKED)
        {
            selectModBtn->ResetState();
            ShowModPicker();
        }

        ret = optionBrowser->GetClickedOption();
    }

    if (ret < 0 || !currentMod || ret >= (int)currentMod->options.size())
        return true;

    // Clicking cycles through choices for that mod option
    RiivoOption& opt = currentMod->options[ret];
    if (!opt.choices.empty())
    {
        opt.selected++;
        if (opt.selected >= (int)opt.choices.size())
            opt.selected = 0;
    }
    SetOptionValues();
    return Show();
}

bool RiivolutionModsSM::wantsSelectAnother() const
{
    return selectAnother;
}