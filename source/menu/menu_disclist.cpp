#include "GameBrowseMenu.hpp"
#include "menus.h"

/****************************************************************************
 * MenuDiscList
 ***************************************************************************/
int MenuDiscList()
{
    int retMenu = MENU_NONE;

    GameBrowseMenu * Menu = new GameBrowseMenu();
    mainWindow->Append(Menu);

    retMenu = Menu->Show();

    delete Menu;

    return retMenu;
}
