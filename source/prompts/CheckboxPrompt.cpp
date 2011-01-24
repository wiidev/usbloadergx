/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include <unistd.h>
#include "CheckboxPrompt.hpp"
#include "themes/CTheme.h"
#include "menu/menus.h"
#include "language/gettext.h"

CheckboxPrompt::CheckboxPrompt(const char * title, const char *msg)
    : PromptWindow(title, msg)
{
    PromptWindow::AddButton(tr("OK"));
    PromptWindow::AddButton(tr("Cancel"));
}

CheckboxPrompt::~CheckboxPrompt()
{
    ResumeGui();

    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(parentElement && this->GetEffect() > 0) usleep(100);

    HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);
    parentElement = NULL;

    RemoveAll();

	for(u32 i = 0; i < Checkbox.size(); ++i)
	{
		delete CheckboxTxt[i];
		delete Checkbox[i];
	}

}

void CheckboxPrompt::AddCheckBox(const char *text)
{
    int size = Checkbox.size();
    if(size > 3)
        return;

    CheckboxTxt.resize(size+1);
    Checkbox.resize(size+1);

    CheckboxTxt[size] = new GuiText(text, 20, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    CheckboxTxt[size]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    CheckboxTxt[size]->SetPosition(40, 0);

    Checkbox[size] = new GuiCheckbox(24, 24);
    Checkbox[size]->SetLabel(CheckboxTxt[size]);
    Checkbox[size]->SetSoundClick(btnSoundClick);
    Checkbox[size]->SetSoundOver(btnSoundOver);
    Checkbox[size]->SetTrigger(trigA);
    Append(Checkbox[size]);

    if (Settings.wsprompt && Settings.widescreen)
    {
        if(size == 0)
        {
            Checkbox[size]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            Checkbox[size]->SetPosition(80, -170);
        }
        else if(size == 1)
        {
            Checkbox[size]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            Checkbox[size]->SetPosition(80, -115);
        }
        else if(size == 2)
        {
            Checkbox[size]->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            Checkbox[size]->SetPosition(-210, -170);
        }
        else if(size == 3)
        {
            Checkbox[size]->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            Checkbox[size]->SetPosition(-210, -115);
        }
    }
    else
    {
        if(size == 0)
        {
            Checkbox[size]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            Checkbox[size]->SetPosition(40, -170);
        }
        else if(size == 1)
        {
            Checkbox[size]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            Checkbox[size]->SetPosition(40, -115);
        }
        else if(size == 2)
        {
            Checkbox[size]->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            Checkbox[size]->SetPosition(-210, -170);
        }
        else if(size == 3)
        {
            Checkbox[size]->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            Checkbox[size]->SetPosition(-210, -115);
        }
    }
}

int CheckboxPrompt::GetChoice()
{
    int choice  = PromptWindow::GetChoice();
    if(choice == 0)
        return 0;

    else if(choice == 1)
    {
        int ret = 0;

        for(u32 i = 0; i < Checkbox.size(); ++i)
        {
            if(Checkbox[i]->IsChecked())
            {
                ret ^= (int) pow(2, i);
            }
        }

        return ret;
    }

    return -1;
}


int CheckboxPrompt::Show(const char *title, const char *msg, const char *chbx1, const char *chbx2, const char *chbx3, const char *chbx4)
{
    CheckboxPrompt * Window = new CheckboxPrompt(title, msg);
    if(chbx1)
        Window->AddCheckBox(chbx1);
    if(chbx2)
        Window->AddCheckBox(chbx2);
    if(chbx3)
        Window->AddCheckBox(chbx3);
    if(chbx4)
        Window->AddCheckBox(chbx4);

    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(Window);
    mainWindow->ChangeFocus(Window);

    int choice = -1;

    while (choice == -1)
    {
        usleep(100);

        if (shutdown)
            Sys_Shutdown();
        if (reset)
            Sys_Reboot();

        choice = Window->GetChoice();
    }

    delete Window;
    mainWindow->SetState(STATE_DEFAULT);

    return choice;
}
