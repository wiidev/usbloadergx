#include "gui.h"
#include "gui_searchbar.h"

#include "../wpad.h"
#include "../main.h"
#include "../settings/cfg.h"
#include "../usbloader/getentries.h"

extern GuiWindow * mainWindow;

class cSearchButton {
public:
    cSearchButton(wchar_t *Char, GuiImageData *keyImageData, GuiImageData *keyOverImageData, int x, int y, GuiTrigger* trig, GuiSound* sndOver, GuiSound* sndClick)
            :
            wchar(*Char),
            image(keyImageData),
            imageOver(keyOverImageData),
	    text(NULL, 20, (GXColor) {0, 0, 0, 0xff}),
	button(&image, &imageOver, ALIGN_LEFT, ALIGN_TOP, x, y, trig, sndOver, sndClick, 1) {
        text.SetText(Char);
        button.SetLabel(&text);
    }
    wchar_t		wchar;
    GuiImage	image;
    GuiImage	imageOver;
    GuiText		text;
    GuiButton	button;
private:

};


GuiSearchBar::GuiSearchBar(const wchar_t *SearchChars)
        :
        inSide(0),
	text(NULL, 22, (GXColor) {0, 0, 0, 255}),
    buttons(0),
    keyImageData(keyboard_key_png),
    keyOverImageData(keyboard_key_over_png),
    sndOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume),
    sndClick(button_click_pcm, button_click_pcm_size, Settings.sfxvolume) {
    char imgPath[100];
    trig.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

    cnt = wcslen(SearchChars);
    buttons = new cSearchButton*[cnt];

    wchar_t charstr[2] = {0, 0};
    int lines = (cnt+9)/10;
    int buttonsPerLine = (cnt+lines-1)/lines;
    width = 10+buttonsPerLine*42+10;
    int x_start=10, x=0, y_start=10+42, y=0;
    if (width < 200) {
        x_start += (200-width)>>1;
        width=200;
    }
    for (int i=0; i<cnt; i++, x++) {
        if (x >= buttonsPerLine) x=0;
        if (x == 0) y++;
        charstr[0] = SearchChars[i];
        buttons[i] = new cSearchButton(charstr, &keyImageData, &keyOverImageData, x_start+x*42, y_start-42+y*42, &trig, &sndOver, &sndClick);
        this->Append(&(buttons[i]->button));
    }
    height = 10+42+y*42+10;


    text.SetText(gameFilter);
    text.SetPosition(10, 15);
    text.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    text.SetWidescreen(CFG.widescreen);
    text.SetMaxWidth(width-(10+2*42+10), GuiText::SCROLL);
    this->Append(&text);

    snprintf(imgPath, sizeof(imgPath), "%skeyboard_backspace_over.png", CFG.theme_path);
    imgBacspaceBtn = new GuiImageData(imgPath, keyboard_backspace_over_png);
    BacspaceBtnImg_Over = new GuiImage(imgBacspaceBtn);
    BacspaceBtnImg = new GuiImage(BacspaceBtnImg_Over);
    BacspaceBtnImg->SetGrayscale();
    BacspaceBtn = new GuiButton(BacspaceBtnImg,BacspaceBtnImg_Over, ALIGN_RIGHT, ALIGN_TOP, -52, 10, &trig, &sndOver, &sndClick,1);
    this->Append(BacspaceBtn);

    snprintf(imgPath, sizeof(imgPath), "%skeyboard_clear_over.png", CFG.theme_path);
    imgClearBtn = new GuiImageData(imgPath, keyboard_clear_over_png);
    ClearBtnImg_Over = new GuiImage(imgClearBtn);
    ClearBtnImg = new GuiImage(ClearBtnImg_Over);
    ClearBtnImg->SetGrayscale();
    ClearBtn = new GuiButton(ClearBtnImg,ClearBtnImg_Over, ALIGN_RIGHT, ALIGN_TOP, -10, 10, &trig, &sndOver, &sndClick,1);
    this->Append(ClearBtn);


//	SetPosition(100,100);

}
GuiSearchBar::~GuiSearchBar() {
    if (buttons) {
        for (int i=0; i<cnt; i++)
            delete buttons[i];
        delete [] buttons;
    }
    delete ClearBtn;
    delete ClearBtnImg;
    delete ClearBtnImg_Over;
    delete imgClearBtn;

    delete BacspaceBtn;
    delete BacspaceBtnImg;
    delete BacspaceBtnImg_Over;
    delete imgBacspaceBtn;
    if (inSide)
        mainWindow->SetState(STATE_DEFAULT);
}
void GuiSearchBar::Draw() {
    Menu_DrawRectangle(this->GetLeft(),this->GetTop(),width, height,(GXColor) {0, 0, 0, 0xa0},1);
    Menu_DrawRectangle(this->GetLeft()+10,this->GetTop()+15,width-(10+2*42+10), 22,(GXColor) {255, 255, 255, 255},1);
    GuiWindow::Draw();
}
void GuiSearchBar::Update(GuiTrigger * t) {
    LOCK(this);
    if (_elements.size() == 0 || (state == STATE_DISABLED && parentElement))
        return;
    // cursor
    if (t->wpad.ir.valid && state != STATE_DISABLED) {
        if (this->IsInside(t->wpad.ir.x, t->wpad.ir.y)) {
            if (inSide == 0) {
                mainWindow->SetState(STATE_DISABLED);
                this->SetState(STATE_DEFAULT);
            }
            inSide |= 1 << t->chan;
        } else if (inSide) {
            inSide &= ~(1 << t->chan);
            if (inSide == 0)
                mainWindow->SetState(STATE_DEFAULT);
        }
    }
    GuiWindow::Update(t);
}
wchar_t GuiSearchBar::GetClicked() {
    if (buttons) {
        for (int i=0; i<cnt; i++) {
            if (buttons[i]->button.GetState() == STATE_CLICKED) {
                buttons[i]->button.ResetState();
                return buttons[i]->wchar;
            }
        }
    }
    if (BacspaceBtn->GetState() == STATE_CLICKED)
        return 8;
    if (ClearBtn->GetState() == STATE_CLICKED)
        return 7;

    return 0;
}

/*
private:
	SearchButtons *buttons;
}*/


