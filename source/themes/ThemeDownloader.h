#ifndef _THEME_DOWNLOADER_H_
#define _THEME_DOWNLOADER_H_

#include "settings/menus/FlyingButtonsMenu.hpp"
#include "themes/Theme_List.h"

class ThemeDownloader : public FlyingButtonsMenu
{
    public:
        ThemeDownloader();
        ~ThemeDownloader();
        static int Run();
        int MainLoop();
    protected:
        void CreateSettingsMenu(int index) { MainButtonClicked(index); };
        void MainButtonClicked(int button);
        void AddMainButtons();
        void SetupMainButtons();
        void SetMainButton(int position, const char * ButtonText, GuiImageData * imageData, GuiImageData * imageOver);
        GuiImageData * GetImageData(int theme);
        int DownloadTheme(const char *url, const char *title);

        Theme_List * ThemeList;
        GuiText * urlTxt;
        GuiText * defaultBtnTxt;
        GuiImage * defaultBtnImg;
        GuiButton * defaultBtn;
        GuiImageData * ThemePreviews[4];
        std::string ThemeListURL;
};

#endif
