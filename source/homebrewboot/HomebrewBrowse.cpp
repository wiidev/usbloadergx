#include "HomebrewBrowser.hpp"

/****************************************************************************
 * MenuHomebrewBrowse
 ***************************************************************************/
int MenuHomebrewBrowse()
{
    HomebrewBrowser * Menu = new HomebrewBrowser();
    mainWindow->Append(Menu);

    Menu->ShowMenu();

    int returnMenu = MENU_NONE;

    while((returnMenu = Menu->MainLoop()) == MENU_NONE);

    delete Menu;

    return returnMenu;
}
