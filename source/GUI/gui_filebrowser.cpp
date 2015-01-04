/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_filebrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui_filebrowser.h"
#include "prompts/filebrowser.h"
#include "settings/CSettings.h"
#include "themes/CTheme.h"

/**
 * Constructor for the GuiFileBrowser class.
 */
GuiFileBrowser::GuiFileBrowser(int w, int h)
	: scrollBar(h-10)
{
	width = w;
	height = h;
	selectedItem = 0;
	selectable = true;
	triggerdisabled = false; // trigger disable

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	bgFileSelection = new GuiImageData(Resources::GetFile("bg_browser.png"), Resources::GetFileSize("bg_browser.png"));
	bgFileSelectionImg = new GuiImage(bgFileSelection);
	bgFileSelectionImg->SetParent(this);
	bgFileSelectionImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	bgFileSelectionEntry = Resources::GetImageData("bg_browser_selection.png");

	fileFolder = Resources::GetImageData("icon_folder.png");

	scrollBar.SetParent(this);
	scrollBar.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollBar.SetPosition(0, 5);
	scrollBar.listChanged.connect(this, &GuiFileBrowser::onListChange);

	for (int i = 0; i < FILEBROWSERSIZE; i++)
	{
		fileListText[i] = new GuiText((char *) NULL, 20, ( GXColor ) {0, 0, 0, 0xff});
		fileListText[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		fileListText[i]->SetPosition(5, 0);
		fileListText[i]->SetMaxWidth(bgFileSelectionImg->GetWidth() - (scrollBar.GetWidth() + 40), DOTTED);

		fileListTextOver[i] = new GuiText((char *) NULL, 20, ( GXColor ) {0, 0, 0, 0xff});
		fileListTextOver[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		fileListTextOver[i]->SetPosition(5, 0);
		fileListTextOver[i]->SetMaxWidth(bgFileSelectionImg->GetWidth() - (scrollBar.GetWidth() + 40), SCROLL_HORIZONTAL);

		fileListBg[i] = new GuiImage(bgFileSelectionEntry);
		fileListFolder[i] = new GuiImage(fileFolder);
		fileList[i] = new GuiButton(350, 30);
		fileList[i]->SetParent(this);
		fileList[i]->SetLabel(fileListText[i]);
		fileList[i]->SetLabelOver(fileListTextOver[i]);
		fileList[i]->SetImageOver(fileListBg[i]);
		fileList[i]->SetPosition(2, 30 * i + 3);
		fileList[i]->SetTrigger(trigA);
		fileList[i]->SetRumble(false);
		fileList[i]->SetSoundClick(btnSoundClick);
	}
}

/**
 * Destructor for the GuiFileBrowser class.
 */
GuiFileBrowser::~GuiFileBrowser()
{
	delete bgFileSelectionImg;

	delete bgFileSelection;
	delete bgFileSelectionEntry;
	delete fileFolder;

	delete trigA;

	for (int i = 0; i < FILEBROWSERSIZE; i++)
	{
		delete fileListText[i];
		delete fileListTextOver[i];
		delete fileList[i];
		delete fileListBg[i];
		delete fileListFolder[i];
	}
}

void GuiFileBrowser::DisableTriggerUpdate(bool set)
{
	LOCK( this );
	triggerdisabled = set;
}

void GuiFileBrowser::ResetState()
{
	LOCK( this );
	state = STATE_DEFAULT;
	stateChan = -1;
	selectedItem = 0;

	for (int i = 0; i < FILEBROWSERSIZE; i++)
	{
		fileList[i]->ResetState();
	}
}

void GuiFileBrowser::UpdateList()
{
	LOCK( this );
	for (int i = 0; i < FILEBROWSERSIZE; i++)
	{
		if (browser->pageIndex + i < (int) browser->browserList.size())
		{
			if (fileList[i]->GetState() == STATE_DISABLED)
				fileList[i]->SetState(STATE_DEFAULT);

			fileList[i]->SetVisible(true);

			fileListText[i]->SetText(browser->browserList[browser->pageIndex + i].displayname);
			fileListTextOver[i]->SetText(browser->browserList[browser->pageIndex + i].displayname);

			if (browser->browserList[browser->pageIndex + i].isdir) // directory
			{
				fileList[i]->SetIcon(fileListFolder[i]);
				fileListText[i]->SetPosition(30, 0);
				fileListTextOver[i]->SetPosition(30, 0);
			}
			else
			{
				fileList[i]->SetIcon(NULL);
				fileListText[i]->SetPosition(10, 0);
				fileListTextOver[i]->SetPosition(10, 0);
			}
		}
		else
		{
			fileList[i]->SetVisible(false);
			fileList[i]->SetState(STATE_DISABLED);
		}
	}
}

void GuiFileBrowser::onListChange(int SelItem, int SelInd)
{
	selectedItem = SelItem;
	browser->pageIndex = SelInd;
	UpdateList();
}

/**
 * Draw the button on screen
 */
void GuiFileBrowser::Draw()
{
	LOCK( this );
	if (!this->IsVisible()) return;

	bgFileSelectionImg->Draw();

	for (int i = 0; i < FILEBROWSERSIZE; i++)
	{
		fileList[i]->Draw();
	}

	if(browser->browserList.size() > FILEBROWSERSIZE)
		scrollBar.Draw();

	this->UpdateEffects();
}

void GuiFileBrowser::Update(GuiTrigger * t)
{
	LOCK( this );
	if (state == STATE_DISABLED || !t || triggerdisabled)
		return;

	static int pressedChan = -1;

	if((t->wpad.btns_d & (WPAD_BUTTON_B | WPAD_BUTTON_DOWN | WPAD_BUTTON_UP | WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT |
						  WPAD_CLASSIC_BUTTON_B | WPAD_CLASSIC_BUTTON_UP | WPAD_CLASSIC_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_RIGHT)) ||
		(t->pad.btns_d & (PAD_BUTTON_UP | PAD_BUTTON_DOWN)) ||
		(t->wupcdata.btns_d & (WPAD_CLASSIC_BUTTON_B | WPAD_CLASSIC_BUTTON_UP | WPAD_CLASSIC_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_RIGHT)))
		pressedChan = t->chan;

	if(browser->browserList.size() > FILEBROWSERSIZE)
		scrollBar.Update(t);

	if(pressedChan == -1 || (!t->wpad.btns_h && !t->pad.btns_h && !t->wupcdata.btns_h))
	{
		for (int i = 0; i < FILEBROWSERSIZE; i++)
		{
			if (i != selectedItem && fileList[i]->GetState() == STATE_SELECTED)
				fileList[i]->ResetState();
			else if (i == selectedItem && fileList[i]->GetState() == STATE_DEFAULT)
				fileList[selectedItem]->SetState(STATE_SELECTED, -1);

			fileList[i]->Update(t);

			if (fileList[i]->GetState() == STATE_SELECTED)
				selectedItem = i;
		}
	}


	if(pressedChan == t->chan && !t->wpad.btns_d && !t->wpad.btns_h && !t->wupcdata.btns_d && !t->wupcdata.btns_h)
		pressedChan = -1;

	scrollBar.SetPageSize(FILEBROWSERSIZE);
	scrollBar.SetSelectedItem(selectedItem);
	scrollBar.SetSelectedIndex(browser->pageIndex);
	scrollBar.SetEntrieCount(browser->browserList.size());
}
