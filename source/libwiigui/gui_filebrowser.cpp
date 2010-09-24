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
#include "settings/CSettings.h"

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
    focus = 0; // allow focus

    trigA = new GuiTrigger;
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    trigHeldA = new GuiTrigger;
    trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
    btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, Settings.sfxvolume);

    char imgPath[100];
    snprintf(imgPath, sizeof(imgPath), "%sbg_browser.png", Settings.theme_path);
    bgFileSelection = new GuiImageData(imgPath, bg_browser_png);
    bgFileSelectionImg = new GuiImage(bgFileSelection);
    bgFileSelectionImg->SetParent(this);
    bgFileSelectionImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

    snprintf(imgPath, sizeof(imgPath), "%sbg_browser_selection.png", Settings.theme_path);
    bgFileSelectionEntry = new GuiImageData(imgPath, bg_browser_selection_png);
    //  fileArchives = new GuiImageData(icon_archives_png);
    //  fileDefault = new GuiImageData(icon_default_png);
    fileFolder = new GuiImageData(icon_folder_png);
    //  fileGFX = new GuiImageData(icon_gfx_png);
    //  filePLS = new GuiImageData(icon_pls_png);
    //  fileSFX = new GuiImageData(icon_sfx_png);
    //  fileTXT = new GuiImageData(icon_txt_png);
    //  fileXML = new GuiImageData(icon_xml_png);

    snprintf(imgPath, sizeof(imgPath), "%sscrollbar.png", Settings.theme_path);
    scrollbar = new GuiImageData(imgPath, scrollbar_png);
    scrollbarImg = new GuiImage(scrollbar);
    scrollbarImg->SetParent(this);
    scrollbarImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    scrollbarImg->SetPosition(0, 2);
    scrollbarImg->SetSkew(0, 0, 0, 0, 0, -30, 0, -30);

    snprintf(imgPath, sizeof(imgPath), "%sscrollbar_arrowdown.png", Settings.theme_path);
    arrowDown = new GuiImageData(imgPath, scrollbar_arrowdown_png);
    arrowDownImg = new GuiImage(arrowDown);
    snprintf(imgPath, sizeof(imgPath), "%sscrollbar_arrowup.png", Settings.theme_path);
    arrowUp = new GuiImageData(imgPath, scrollbar_arrowup_png);
    arrowUpImg = new GuiImage(arrowUp);
    snprintf(imgPath, sizeof(imgPath), "%sscrollbar_box.png", Settings.theme_path);
    scrollbarBox = new GuiImageData(imgPath, scrollbar_box_png);
    scrollbarBoxImg = new GuiImage(scrollbarBox);

    arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
    arrowUpBtn->SetParent(this);
    arrowUpBtn->SetImage(arrowUpImg);
    arrowUpBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    arrowUpBtn->SetPosition(12, -12);
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
    arrowDownBtn->SetPosition(12, 12);
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
    scrollbarBoxBtn->SetPosition(-10, 0);
    scrollbarBoxBtn->SetMinY(-10);
    scrollbarBoxBtn->SetMaxY(156);
    scrollbarBoxBtn->SetSelectable(false);
    scrollbarBoxBtn->SetClickable(false);
    scrollbarBoxBtn->SetHoldable(true);
    scrollbarBoxBtn->SetTrigger(trigHeldA);

    for (int i = 0; i < FILEBROWSERSIZE; i++)
    {
        fileListText[i] = new GuiText((char *) NULL, 20, ( GXColor )
        {   0, 0, 0, 0xff});
        fileListText[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
        fileListText[i]->SetPosition(5, 0);
        fileListText[i]->SetMaxWidth(bgFileSelectionImg->GetWidth() - (arrowDownImg->GetWidth() + 20), DOTTED);

        fileListTextOver[i] = new GuiText((char *) NULL, 20, ( GXColor )
        {   0, 0, 0, 0xff});
        fileListTextOver[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
        fileListTextOver[i]->SetPosition(5, 0);
        fileListTextOver[i]->SetMaxWidth(bgFileSelectionImg->GetWidth() - (arrowDownImg->GetWidth() + 20),
                SCROLL_HORIZONTAL);

        fileListBg[i] = new GuiImage(bgFileSelectionEntry);
        //fileListArchives[i] = new GuiImage(fileArchives);
        //fileListDefault[i] = new GuiImage(fileDefault);
        fileListFolder[i] = new GuiImage(fileFolder);
        //fileListGFX[i] = new GuiImage(fileGFX);
        //fileListPLS[i] = new GuiImage(filePLS);
        //fileListSFX[i] = new GuiImage(fileSFX);
        //fileListTXT[i] = new GuiImage(fileTXT);
        //fileListXML[i] = new GuiImage(fileXML);
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
    //delete fileArchives;
    //delete fileDefault;
    delete fileFolder;
    //delete fileGFX;
    //delete filePLS;
    //delete fileSFX;
    //delete fileTXT;
    //delete fileXML;
    delete scrollbar;
    delete arrowDown;
    delete arrowUp;
    delete scrollbarBox;

    delete btnSoundOver;
    delete btnSoundClick;
    delete trigHeldA;
    delete trigA;

    for (int i = 0; i < FILEBROWSERSIZE; i++)
    {
        delete fileListText[i];
        delete fileListTextOver[i];
        delete fileList[i];
        delete fileListBg[i];
        //delete fileListArchives[i];
        //delete fileListDefault[i];
        delete fileListFolder[i];
        //delete fileListGFX[i];
        //delete fileListPLS[i];
        //delete fileListSFX[i];
        //delete fileListTXT[i];
        //delete fileListXML[i];
    }
}

void GuiFileBrowser::SetFocus(int f)
{
    LOCK( this );
    focus = f;

    for (int i = 0; i < FILEBROWSERSIZE; i++)
        fileList[i]->ResetState();

    if (f == 1) fileList[selectedItem]->SetState(STATE_SELECTED);
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

void GuiFileBrowser::TriggerUpdate()
{
    LOCK( this );
    listChanged = true;
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

    scrollbarImg->Draw();
    arrowUpBtn->Draw();
    arrowDownBtn->Draw();
    scrollbarBoxBtn->Draw();

    this->UpdateEffects();
}

void GuiFileBrowser::Update(GuiTrigger * t)
{
    LOCK( this );
    if (state == STATE_DISABLED || !t || triggerdisabled) return;

    int position = 0;
    int positionWiimote = 0;

    arrowUpBtn->Update(t);
    arrowDownBtn->Update(t);
    scrollbarBoxBtn->Update(t);

    // move the file listing to respond to wiimote cursor movement
    if (scrollbarBoxBtn->GetState() == STATE_HELD && scrollbarBoxBtn->GetStateChan() == t->chan && t->wpad.ir.valid
            && browser->browserList.size() > FILEBROWSERSIZE)
    {
        scrollbarBoxBtn->SetPosition(20, -10);
        positionWiimote = t->wpad.ir.y - 60 - scrollbarBoxBtn->GetTop();

        if (positionWiimote < scrollbarBoxBtn->GetMinY())
            positionWiimote = scrollbarBoxBtn->GetMinY();
        else if (positionWiimote > scrollbarBoxBtn->GetMaxY()) positionWiimote = scrollbarBoxBtn->GetMaxY();

        browser->pageIndex = (positionWiimote * browser->browserList.size()) / 136.0 - selectedItem;

        if (browser->pageIndex <= 0)
        {
            browser->pageIndex = 0;
        }
        else if (browser->pageIndex + FILEBROWSERSIZE >= (int) browser->browserList.size())
        {
            browser->pageIndex = browser->browserList.size() - FILEBROWSERSIZE;
        }
        listChanged = true;
        focus = false;

    }

    if (arrowDownBtn->GetState() == STATE_HELD && arrowDownBtn->GetStateChan() == t->chan)
    {
        t->wpad.btns_h |= WPAD_BUTTON_DOWN;
        if (!this->IsFocused()) ((GuiWindow *) this->GetParent())->ChangeFocus(this);

    }
    else if (arrowUpBtn->GetState() == STATE_HELD && arrowUpBtn->GetStateChan() == t->chan)
    {
        t->wpad.btns_h |= WPAD_BUTTON_UP;
        if (!this->IsFocused()) ((GuiWindow *) this->GetParent())->ChangeFocus(this);

    }

    /*  // pad/joystick navigation
     if(!focus)
     {
     goto endNavigation; // skip navigation
     listChanged = false;
     }
     */
    if (t->Right())
    {
        if (browser->pageIndex < (int) browser->browserList.size() && browser->browserList.size() > FILEBROWSERSIZE)
        {
            browser->pageIndex += FILEBROWSERSIZE;
            if (browser->pageIndex + FILEBROWSERSIZE >= (int) browser->browserList.size()) browser->pageIndex
                    = browser->browserList.size() - FILEBROWSERSIZE;
            listChanged = true;
        }
    }
    else if (t->Left())
    {
        if (browser->pageIndex > 0)
        {
            browser->pageIndex -= FILEBROWSERSIZE;
            if (browser->pageIndex < 0) browser->pageIndex = 0;
            listChanged = true;
        }
    }
    else if (t->Down())
    {
        if (browser->pageIndex + selectedItem + 1 < (int) browser->browserList.size())
        {
            if (selectedItem == FILEBROWSERSIZE - 1)
            {
                // move list down by 1
                browser->pageIndex++;
                listChanged = true;
            }
            else if (fileList[selectedItem + 1]->IsVisible())
            {
                fileList[selectedItem]->ResetState();
                fileList[++selectedItem]->SetState(STATE_SELECTED, t->chan);
            }
        }
    }
    else if (t->Up())
    {
        if (selectedItem == 0 && browser->pageIndex + selectedItem > 0)
        {
            // move list up by 1
            browser->pageIndex--;
            listChanged = true;
        }
        else if (selectedItem > 0)
        {
            fileList[selectedItem]->ResetState();
            fileList[--selectedItem]->SetState(STATE_SELECTED, t->chan);
        }
    }

    //endNavigation:

    for (int i = 0; i < FILEBROWSERSIZE; i++)
    {
        if (listChanged)
        {
            bool haveselected = false;
            if (browser->pageIndex + i < (int) browser->browserList.size())
            {
                if (fileList[i]->GetState() == STATE_DISABLED) fileList[i]->SetState(STATE_DEFAULT);

                if (fileList[i]->GetState() == STATE_SELECTED) haveselected = true;

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
                    /*
                     char *fileext = strrchr(browserList[browser.pageIndex+i].displayname, '.');
                     fileListText[i]->SetPosition(32,0);
                     fileListTextOver[i]->SetPosition(32,0);
                     if(fileext)
                     {
                     if(!strcasecmp(fileext, ".png") || !strcasecmp(fileext, ".jpg") || !strcasecmp(fileext, ".jpeg") ||
                     !strcasecmp(fileext, ".gif") || !strcasecmp(fileext, ".tga") || !strcasecmp(fileext, ".tpl") ||
                     !strcasecmp(fileext, ".bmp")) {
                     fileList[i]->SetIcon(fileListGFX[i]);
                     } else if(!strcasecmp(fileext, ".mp3") || !strcasecmp(fileext, ".ogg") || !strcasecmp(fileext, ".flac") ||
                     !strcasecmp(fileext, ".mpc") || !strcasecmp(fileext, ".m4a") || !strcasecmp(fileext, ".wav")) {
                     fileList[i]->SetIcon(fileListSFX[i]);
                     } else if(!strcasecmp(fileext, ".pls") || !strcasecmp(fileext, ".m3u")) {
                     fileList[i]->SetIcon(fileListPLS[i]);
                     } else if(!strcasecmp(fileext, ".txt")) {
                     fileList[i]->SetIcon(fileListTXT[i]);
                     } else if(!strcasecmp(fileext, ".xml")) {
                     fileList[i]->SetIcon(fileListXML[i]);
                     } else if(!strcasecmp(fileext, ".rar") || !strcasecmp(fileext, ".zip") ||
                     !strcasecmp(fileext, ".gz") || !strcasecmp(fileext, ".7z")) {
                     fileList[i]->SetIcon(fileListArchives[i]);
                     } else {
                     fileList[i]->SetIcon(fileListDefault[i]);
                     }
                     } else {
                     fileList[i]->SetIcon(fileListDefault[i]);
                     }
                     */
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
            if (!haveselected && browser->pageIndex < (int) browser->browserList.size()) fileList[i]->SetState(
                    STATE_SELECTED, t->chan);

        }

        if (i != selectedItem && fileList[i]->GetState() == STATE_SELECTED)
            fileList[i]->ResetState();
        else if (focus && i == selectedItem && fileList[i]->GetState() == STATE_DEFAULT) fileList[selectedItem]->SetState(
                STATE_SELECTED, t->chan);

        int currChan = t->chan;

        if (t->wpad.ir.valid && !fileList[i]->IsInside(t->wpad.ir.x, t->wpad.ir.y)) t->chan = -1;

        fileList[i]->Update(t);
        t->chan = currChan;

        if (fileList[i]->GetState() == STATE_SELECTED)
        {
            selectedItem = i;
        }
    }

    // update the location of the scroll box based on the position in the file list
    if (positionWiimote > 0)
    {
        position = positionWiimote; // follow wiimote cursor
    }
    else
    {
        position = 136 * (browser->pageIndex + FILEBROWSERSIZE / 2.0) / (browser->browserList.size() * 1.0);

        if (browser->pageIndex / (FILEBROWSERSIZE / 2.0) < 1)
            position = -10;
        else if ((browser->pageIndex + FILEBROWSERSIZE) / (FILEBROWSERSIZE * 1.0) >= (browser->browserList.size())
                / (FILEBROWSERSIZE * 1.0)) position = 156;
    }

    scrollbarBoxBtn->SetPosition(12, position + 26);

    listChanged = false;

    if (updateCB) updateCB(this);
}
