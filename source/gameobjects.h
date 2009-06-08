#ifndef _GAMEOBJECTS_H_
#define _GAMEOBJECTS_H_

class GameObjects
{
	public:
		GameObjects(int obj);
		~GameObjects();
		void SetActive(int obj);
		int GetActive();
		void ReloadAll(struct discHdr * l, int count);
		void Reload(int obj, struct discHdr * l, int count);
		void Reload(struct discHdr * l, int count);
		void * Ptr(int obj);
		void * Ptr();
		void SetFocus(int obj, int f);
		void SetFocus(int f);
		void SetFocus();
	protected:
		GuiGameBrowser * gameBrowser;
		GuiGameGrid * gameGrid;
		GuiGameCarousel * gameCarousel;
		int active;
};
#endif
