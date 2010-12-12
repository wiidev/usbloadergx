#include "libwiigui/gui.h"
#include "themes/CTheme.h"
#include "usbloader/GameList.h"
#include "settings/GameTitles.h"
#include "menu/menus.h"

extern GuiImageData * pointer[4];

void rockout(int gameSelected, int f)
{
    HaltGui();

    if (gameSelected >= 0 && gameSelected < gameList.size() && (strcasestr(GameTitles.GetTitle(gameList[gameSelected]), "guitar")
            || strcasestr(GameTitles.GetTitle(gameList[gameSelected]), "band") || strcasestr(GameTitles.GetTitle(gameList[gameSelected]),
            "rock")))
    {
        for (int i = 0; i < 4; i++)
            delete pointer[i];
        pointer[0] = Resources::GetImageData("rplayer1_point.png");
        pointer[1] = Resources::GetImageData("rplayer2_point.png");
        pointer[2] = Resources::GetImageData("rplayer3_point.png");
        pointer[3] = Resources::GetImageData("rplayer4_point.png");
    }
    else
    {

        for (int i = 0; i < 4; i++)
            delete pointer[i];
        pointer[0] = Resources::GetImageData("player1_point.png");
        pointer[1] = Resources::GetImageData("player2_point.png");
        pointer[2] = Resources::GetImageData("player3_point.png");
        pointer[3] = Resources::GetImageData("player4_point.png");
    }
    ResumeGui();
}
