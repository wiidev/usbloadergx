#include <string.h>
#include <unistd.h>
#include "settings/menus/GlobalSettings.hpp"
#include "settings/menus/GameSettingsMenu.hpp"

/****************************************************************************
 * MenuSettings
 ***************************************************************************/
int MenuSettings()
{
    GlobalSettings * Menu = new GlobalSettings();
    mainWindow->Append(Menu);

    Menu->ShowMenu();

    int returnMenu = MENU_NONE;

    while((returnMenu = Menu->MainLoop()) == MENU_NONE);

    delete Menu;

    return returnMenu;
}

/********************************************************************************
 *Game specific settings
 *********************************************************************************/
int MenuGameSettings(struct discHdr * header)
{
    GameSettingsMenu * Menu = new GameSettingsMenu(header);
    mainWindow->Append(Menu);

    Menu->ShowMenu();

    int returnMenu = MENU_NONE;

    while((returnMenu = Menu->MainLoop()) == MENU_NONE);

    delete Menu;

    return returnMenu;
}
