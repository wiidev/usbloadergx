/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_filebrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "prompts/filebrowser.h"
#include "../settings/cfg.h"


#define FILEBROWSERSIZE     8
/**
 * Constructor for the GuiFileBrowser class.
 */
GuiFileBrowser::GuiFileBrowser(int w, int h)
{
	width = w;
	height = h;
	selectedItem = 0;
	selectable = true;
	listChanged = true; // trigger an initial list update
	triggerdisabled = false; // trigger disable
	focus = 1; // allow focus

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbg_browser.png", CFG.theme_path);
	bgFileSelection = new GuiImageData(imgPath, bg_browser_png);
	bgFileSelectionImg = new GuiImage(bgFileSelection);
	bgFileSelectionImg->SetParent(this);
	bgFileSelectionImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	snprintf(imgPath, sizeof(imgPath), "%sbg_browser_selection.png", CFG.theme_path);
	bgFileSelectionEntry = new GuiImageData(imgPath, bg_browser_selection_png);
	fileFolder = new GuiImageData(folder_png);

	snprintf(imgPath, sizeof(imgPath), "%sscrollbar.png", CFG.theme_path);
	scrollbar = new GuiImageData(imgPath, scrollbar_png);
	scrollbarImg = new GuiImage(scrollbar);
	scrollbarImg->SetParent(this);
	scrollbarImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarImg->SetPosition(0, 2);
	scrollbarImg->SetSkew(0,0,0,0,0,-30,0,-30);

	snprintf(imgPath, sizeof(imgPath), "%sscrollbar_arrowdown.png", CFG.theme_path);
	arrowDown = new GuiImageData(imgPath, scrollbar_arrowdown_png);
	arrowDownImg = new GuiImage(arrowDown);
	snprintf(imgPath, sizeof(imgPath), "%sscrollbar_arrowup.png", CFG.theme_path);
	arrowUp = new GuiImageData(imgPath, scrollbar_arrowup_png);
	arrowUpImg = new GuiImage(arrowUp);
	snprintf(imgPath, sizeof(imgPath), "%sscrollbar_box.png", CFG.theme_path);
	scrollbarBox = new GuiImageData(imgPath, scrollbar_box_png);
	scrollbarBoxImg = new GuiImage(scrollbarBox);

	arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
	arrowUpBtn->SetParent(this);
	arrowUpBtn->SetImage(arrowUpImg);
	arrowUpBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	arrowUpBtn->SetPosition(12,-12);
	arrowUpBtn->SetSelectable(false);
	arrowUpBtn->SetClickable(false);
	arrowUpBtn->SetHoldable(true);
	arrowUpBtn->SetTrigger(trigHeldA);
	arrowUpBtn->SetSoundOver(btnSoundOver);
	arrowUpBtn->SetSoundClick(btnSoundClick);

	arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
	arrowDownBtn->SetParent(this);
	arrowDownBtn->SetImage(arrowDownImg);
	arrowDownBtn->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	arrowDownBtn->SetPosition(12,12);
	arrowDownBtn->SetSelectable(false);
	arrowDownBtn->SetClickable(false);
	arrowDownBtn->SetHoldable(true);
	arrowDownBtn->SetTrigger(trigHeldA);
	arrowDownBtn->SetSoundOver(btnSoundOver);
	arrowDownBtn->SetSoundClick(btnSoundClick);

	scrollbarBoxBtn = new GuiButton(scrollbarBoxImg->GetWidth(), scrollbarBoxImg->GetHeight());
	scrollbarBoxBtn->SetParent(this);
	scrollbarBoxBtn->SetImage(scrollbarBoxImg);
	scrollbarBoxBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarBoxBtn->SetMinY(-10);
	scrollbarBoxBtn->SetMaxY(156);
	scrollbarBoxBtn->SetSelectable(false);
	scrollbarBoxBtn->SetClickable(false);
	scrollbarBoxBtn->SetHoldable(true);
	scrollbarBoxBtn->SetTrigger(trigHeldA);

	for(int i=0; i<FILEBROWSERSIZE; i++)
	{
		fileListText[i] = new GuiText(NULL,20, (GXColor){0, 0, 0, 0xff});
		fileListText[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		fileListText[i]->SetPosition(5,0);
		fileListText[i]->SetMaxWidth(bgFileSelectionImg->GetWidth() - (arrowDownImg->GetWidth()+20), GuiText::DOTTED);

		fileListTextOver[i] = new GuiText(NULL,20, (GXColor){0, 0, 0, 0xff});
		fileListTextOver[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		fileListTextOver[i]->SetPosition(5,0);
		fileListTextOver[i]->SetMaxWidth(bgFileSelectionImg->GetWidth() - (arrowDownImg->GetWidth()+20), GuiText::SCROLL);

		fileListBg[i] = new GuiImage(bgFileSelectionEntry);
		fileListFolder[i] = new GuiImage(fileFolder);

		fileList[i] = new GuiButton(350,30);
		fileList[i]->SetParent(this);
		fileList[i]->SetLabel(fileListText[i]);
		fileList[i]->SetLabelOver(fileListTextOver[i]);
		fileList[i]->SetImageOver(fileListBg[i]);
		fileList[i]->SetPosition(2,30*i+3);
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
	delete arrowUpBtn;
	delete arrowDownBtn;
	delete scrollbarBoxBtn;

	delete bgFileSelectionImg;
	delete scrollbarImg;
	delete arrowDownImg;
	delete arrowUpImg;
	delete scrollbarBoxImg;

	delete bgFileSelection;
	delete bgFileSelectionEntry;
	delete fileFolder;
	delete scrollbar;
	delete arrowDown;
	delete arrowUp;
	delete scrollbarBox;

	delete btnSoundOver;
	delete btnSoundClick;
	delete trigHeldA;
	delete trigA;

	for(int i=0; i<FILEBROWSERSIZE; i++)
	{
		delete fileListText[i];
		delete fileListTextOver[i];
		delete fileList[i];
		delete fileListBg[i];
		delete fileListFolder[i];
	}
}

void GuiFileBrowser::SetFocus(int f)
{
	focus = f;

	for(int i=0; i<FILEBROWSERSIZE; i++)
		fileList[i]->ResetState();

	if(f == 1)
		fileList[selectedItem]->SetState(STATE_SELECTED);
}

void GuiFileBrowser::DisableTriggerUpdate(bool set)
{
	triggerdisabled = set;
}

void GuiFileBrowser::ResetState()
{
	state = STATE_DEFAULT;
	stateChan = -1;
	selectedItem = 0;

	for(int i=0; i<FILEBROWSERSIZE; i++)
	{
		fileList[i]->ResetState();
	}
}

void GuiFileBrowser::TriggerUpdate()
{
	listChanged = true;
}

/**
 * Draw the button on screen
 */
void GuiFileBrowser::Draw()
{
	if(!this->IsVisible())
		return;

	bgFileSelectionImg->Draw();

	for(int i=0; i<FILEBROWSERSIZE; i++)
	{
		fileList[i]->Draw();
	}

	scrollbarImg->Draw();
	arrowUpBtn->Draw();
	arrowDownBtn->Draw();
	scrollbarBoxBtn->Draw();

	this->UpdateEffects();
}

void GuiFileBrowser::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t || triggerdisabled)
		return;

	int position = 0;
	int positionWiimote = 0;

	arrowUpBtn->Update(t);
	arrowDownBtn->Update(t);
	scrollbarBoxBtn->Update(t);

	// move the file listing to respond to wiimote cursor movement
	if(scrollbarBoxBtn->GetState() == STATE_HELD &&
		scrollbarBoxBtn->GetStateChan() == t->chan &&
		t->wpad.ir.valid &&
		browser.numEntries > FILEBROWSERSIZE
		)
	{
		scrollbarBoxBtn->SetPosition(20,-10);
		positionWiimote = t->wpad.ir.y - 60 - scrollbarBoxBtn->GetTop();

		if(positionWiimote < scrollbarBoxBtn->GetMinY())
			positionWiimote = scrollbarBoxBtn->GetMinY();
		else if(positionWiimote > scrollbarBoxBtn->GetMaxY())
			positionWiimote = scrollbarBoxBtn->GetMaxY();

		browser.pageIndex = (positionWiimote * browser.numEntries)/166.0 - selectedItem;

		if(browser.pageIndex <= 0)
		{
			browser.pageIndex = 0;
		}
		else if(browser.pageIndex+FILEBROWSERSIZE >= browser.numEntries)
		{
			browser.pageIndex = browser.numEntries-FILEBROWSERSIZE;
		}
		listChanged = true;
		focus = false;
		

	}

	if(arrowDownBtn->GetState() == STATE_HELD && arrowDownBtn->GetStateChan() == t->chan)
	{
		t->wpad.btns_h |= WPAD_BUTTON_DOWN;
		if(!this->IsFocused())
			((GuiWindow *)this->GetParent())->ChangeFocus(this);
			
	}
	else if(arrowUpBtn->GetState() == STATE_HELD && arrowUpBtn->GetStateChan() == t->chan)
	{
		t->wpad.btns_h |= WPAD_BUTTON_UP;
		if(!this->IsFocused())
			((GuiWindow *)this->GetParent())->ChangeFocus(this);
			
	}

	// pad/joystick navigation
	if(!focus)
	{
		goto endNavigation; // skip navigation
		listChanged = false;
	}

	if(t->Right())
	{
		if(browser.pageIndex < browser.numEntries && browser.numEntries > FILEBROWSERSIZE)
		{
			browser.pageIndex += FILEBROWSERSIZE;
			if(browser.pageIndex+FILEBROWSERSIZE >= browser.numEntries)
				browser.pageIndex = browser.numEntries-FILEBROWSERSIZE;
			listChanged = true;
		}
	}
	else if(t->Left())
	{
		if(browser.pageIndex > 0)
		{
			browser.pageIndex -= FILEBROWSERSIZE;
			if(browser.pageIndex < 0)
				browser.pageIndex = 0;
			listChanged = true;
		}
	}
	else if(t->Down())
	{
		if(browser.pageIndex + selectedItem + 1 < browser.numEntries)
		{
			if(selectedItem == FILEBROWSERSIZE-1)
			{
				// move list down by 1
				browser.pageIndex++;
				listChanged = true;
			}
			else if(fileList[selectedItem+1]->IsVisible())
			{
				fileList[selectedItem]->ResetState();
				fileList[++selectedItem]->SetState(STATE_SELECTED, t->chan);
			}
		}
	}
	else if(t->Up())
	{
		if(selectedItem == 0 &&	browser.pageIndex + selectedItem > 0)
		{
			// move list up by 1
			browser.pageIndex--;
			listChanged = true;
		}
		else if(selectedItem > 0)
		{
			fileList[selectedItem]->ResetState();
			fileList[--selectedItem]->SetState(STATE_SELECTED, t->chan);
		}
	}

	endNavigation:

	for(int i=0; i<FILEBROWSERSIZE; i++)
	{
		if(listChanged)
		{
			if(browser.pageIndex+i < browser.numEntries)
			{
				if(fileList[i]->GetState() == STATE_DISABLED)
					fileList[i]->SetState(STATE_DEFAULT);

				fileList[i]->SetVisible(true);

				fileListText[i]->SetText(browserList[browser.pageIndex+i].displayname);
				fileListTextOver[i]->SetText(browserList[browser.pageIndex+i].displayname);

				if(browserList[browser.pageIndex+i].isdir) // directory
				{
					fileList[i]->SetIcon(fileListFolder[i]);
					fileListText[i]->SetPosition(30,0);
					fileListTextOver[i]->SetPosition(30,0);
				}
				else
				{
					fileList[i]->SetIcon(NULL);
					fileListText[i]->SetPosition(10,0);
					fileListTextOver[i]->SetPosition(10,0);
				}
			}
			else
			{
				fileList[i]->SetVisible(false);
				fileList[i]->SetState(STATE_DISABLED);
			}
		}

		if(i != selectedItem && fileList[i]->GetState() == STATE_SELECTED)
			fileList[i]->ResetState();
		else if(focus && i == selectedItem && fileList[i]->GetState() == STATE_DEFAULT)
			fileList[selectedItem]->SetState(STATE_SELECTED, t->chan);

		int currChan = t->chan;

		if(t->wpad.ir.valid && !fileList[i]->IsInside(t->wpad.ir.x, t->wpad.ir.y))
			t->chan = -1;

		fileList[i]->Update(t);
		t->chan = currChan;

		if(fileList[i]->GetState() == STATE_SELECTED)
		{
			selectedItem = i;
			browser.selIndex = browser.pageIndex + i;
		}
	}

	// update the location of the scroll box based on the position in the file list
	if(positionWiimote > 0)
	{
		position = positionWiimote; // follow wiimote cursor
	}
	else
	{
		position = -10+(166*(browser.pageIndex + FILEBROWSERSIZE/2.0) / (browser.numEntries*1.0));

		if(browser.pageIndex/(FILEBROWSERSIZE/2.0) < 1)
			position = -10;
		else if((browser.pageIndex+FILEBROWSERSIZE)/(FILEBROWSERSIZE*1.0) >= (browser.numEntries)/(FILEBROWSERSIZE*1.0))
			position = 156;
	}

	scrollbarBoxBtn->SetPosition(12,position+26);

	listChanged = false;

	if(updateCB)
		updateCB(this);
}
