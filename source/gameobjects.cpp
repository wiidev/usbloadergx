/****************************************************************************
 * gameobjects.cpp
 *
 * class definition
 ***************************************************************************/

#include "libwiigui/gui_gamebrowser.h"
#include "libwiigui/gui_gamegrid.h"
#include "libwiigui/gui_gamecarousel.h"

/**
 * Constructor for the GameObjects class.
 */
GameObjects::GameObjects(int obj)
{
	gameBroswer = new GuiGameBrowser(/* Params */);
	gameGrid = new GuiGameGrid(/* Params */);
	gameCarousel = new GuiGameCarousel(/* Params */);
	active = obj;
}

/**
 * Destructor for the GuiGameCarousel class.
 */
GameObjects::~GameObjects()
{
	delete gameBrowser;
	delete gameGrid;
	delete gameCarousel;
}

/**
 * Functions to set and get teh active object.
 */
void GameObjects::SetActive(int obj)
{
	active = obj;
}

int GameObjects::GetActive()
{
	return active;
}

/**
 * Reload Functions.
 */
void GameObjects::ReloadAll(struct discHdr * l, int count)
{
	gameBrowser->Reload(l, count);
	gameGrid->Reload(l, count);
	gameCarousel->Reload(l, count);
}

void GameObjects::Reload(int obj, struct discHdr * l, int count)
{
	if(obj == LIST) gameBrowser->Reload(l, count);
	else if(obj == GRID) gameGrid->Reload(l, count);
	else if(obj == CAROUSEL) gameCarousel->Reload(l, count);
}

void GameObjects::Reload(struct discHdr * l, int count)
{
	Reload(active, l, count);
}

/**
 * Functions to get pointers to the various objects.
 */
void * GameObjects::Ptr(int obj)
{
	if(obj == LIST) return gameBrowser;
	else if(obj == GRID) return gameGrid;
	else if(obj == CAROUSEL) return gameCarousel;
	else return NULL;
}

void * GameObjects::Ptr()
{
	return Ptr(active);
}

/**
 * Functions to set focus.
 */
void GameObjects::SetFocus(int obj, int f)
{
	if(obj == LIST) gameBrowser->SetFocus(f);
	else if(obj == GRID) gameGrid->SetFocus(f);
	else if(obj == CAROUSEL) gameCarousel->SetFocus(f);
}

void GameObjects::SetFocus(int f)
{
	SetFocus(active, f);
}

void GameObjects::SetFocus()
{
	SetFocus(active, 1);
}
