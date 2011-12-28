#include "GUI/gui.h"
#include "themes/CTheme.h"
#include "usbloader/GameList.h"
#include "settings/GameTitles.h"
#include "menu/menus.h"

void rockout(int gameSelected, int f)
{
	static bool rockoutSet = false;

	HaltGui();

	if (gameSelected >= 0 && gameSelected < gameList.size() && !rockoutSet && (strcasestr(GameTitles.GetTitle(gameList[gameSelected]), "guitar")
			|| strcasestr(GameTitles.GetTitle(gameList[gameSelected]), "band") || strcasestr(GameTitles.GetTitle(gameList[gameSelected]),
			"rock")))
	{
		pointer[0]->SetImage("rplayer1_point.png");
		pointer[1]->SetImage("rplayer2_point.png");
		pointer[2]->SetImage("rplayer3_point.png");
		pointer[3]->SetImage("rplayer4_point.png");

		rockoutSet = true;
	}
	else if(rockoutSet)
	{
		pointer[0]->SetImage("player1_point.png");
		pointer[1]->SetImage("player2_point.png");
		pointer[2]->SetImage("player3_point.png");
		pointer[3]->SetImage("player4_point.png");

		rockoutSet = false;
	}
	ResumeGui();
}
