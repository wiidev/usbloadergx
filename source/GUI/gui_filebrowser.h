#ifndef GUI_FILEBROWSER_H_
#define GUI_FILEBROWSER_H_

#include "gui.h"
#include "gui_scrollbar.hpp"

//!Display a list of files
class GuiFileBrowser: public GuiElement, public sigslot::has_slots<>
{
	public:
		GuiFileBrowser(int w, int h);
		virtual ~GuiFileBrowser();
		void DisableTriggerUpdate(bool set);
		void ResetState();
		void Draw();
		void UpdateList();
		void Update(GuiTrigger * t);
		GuiButton * fileList[PAGESIZE];
	protected:
		void onListChange(int SelItem, int SelInd);
		int selectedItem;
		bool triggerdisabled;

		GuiText * fileListText[PAGESIZE];
		GuiText * fileListTextOver[PAGESIZE];
		GuiImage * fileListBg[PAGESIZE];
		GuiImage * fileListFolder[PAGESIZE];

		GuiImage * bgFileSelectionImg;

		GuiImageData * bgFileSelection;
		GuiImageData * bgFileSelectionEntry;
		GuiImageData * fileFolder;

		GuiTrigger * trigA;
		GuiScrollbar scrollBar;
};


#endif
