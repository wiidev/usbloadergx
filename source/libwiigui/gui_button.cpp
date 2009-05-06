/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_button.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

static int scrollison;

/**
 * Constructor for the GuiButton class.
 */

GuiButton::GuiButton(int w, int h)
{
	width = w;
	height = h;
	image = NULL;
	imageOver = NULL;
	imageHold = NULL;
	imageClick = NULL;
	icon = NULL;
	iconOver = NULL;
	iconHold = NULL;
	iconClick = NULL;
	toolTip = NULL;
	toolTip2 = NULL;
	toolTipTxt = NULL;

	for(int i=0; i < 3; i++)
	{
		label[i] = NULL;
		labelOver[i] = NULL;
		labelHold[i] = NULL;
		labelClick[i] = NULL;
	}

	soundOver = NULL;
	soundHold = NULL;
	soundClick = NULL;
	selectable = true;
	holdable = false;
	clickable = true;
}

/**
 * Destructor for the GuiButton class.
 */
GuiButton::~GuiButton()
{
}

void GuiButton::SetImage(GuiImage* img)
{
	LOCK(this);
	image = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageOver(GuiImage* img)
{
	LOCK(this);
	imageOver = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageHold(GuiImage* img)
{
	LOCK(this);
	imageHold = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageClick(GuiImage* img)
{
	LOCK(this);
	imageClick = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIcon(GuiImage* img)
{
	LOCK(this);
	icon = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconOver(GuiImage* img)
{
	LOCK(this);
	iconOver = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconHold(GuiImage* img)
{
	LOCK(this);
	iconHold = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconClick(GuiImage* img)
{
	LOCK(this);
	iconClick = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetLabel(GuiText* txt, int n)
{
	LOCK(this);
	label[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelOver(GuiText* txt, int n)
{
	LOCK(this);
	labelOver[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelHold(GuiText* txt, int n)
{
	LOCK(this);
	labelHold[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelClick(GuiText* txt, int n)
{
	LOCK(this);
	labelClick[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetSoundOver(GuiSound * snd)
{
	LOCK(this);
	soundOver = snd;
}
void GuiButton::SetSoundHold(GuiSound * snd)
{
	LOCK(this);
	soundHold = snd;
}
void GuiButton::SetSoundClick(GuiSound * snd)
{
	LOCK(this);
	soundClick = snd;
}

//No delay for now
void GuiButton::SetToolTip(GuiImage* img, GuiText * txt, int x, int y)
{
	LOCK(this);
	if(img)
	{

		toolTip = img;
		img->SetParent(this);
		img->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
		img->SetPosition(x,y);
		if(txt)
		{
			toolTipTxt = txt;
			txt->SetParent(img);
		}

	}
}
void GuiButton::SetToolTip(GuiElement* tt, int x, int y, int h_align, int v_align)
{
	LOCK(this);
	if(tt)
	{
		toolTip2 = tt;
		toolTip2->SetParent(this);
		toolTip2->SetAlignment(h_align, v_align);
		toolTip2->SetPosition(x,y);
		
	}
}
/**
 * Draw the button on screen
 */
void GuiButton::Draw()
{
	LOCK(this);
	if(!this->IsVisible())
		return;

	// draw image
	if((state == STATE_SELECTED || state == STATE_HELD) && imageOver)
		imageOver->Draw();
	else if(image)
		image->Draw();
	// draw icon
	if((state == STATE_SELECTED || state == STATE_HELD) && iconOver)
		iconOver->Draw();
	else if(icon)
		icon->Draw();
	// draw text
	for(int i=0; i<3; i++)
	{
		if((state == STATE_SELECTED || state == STATE_HELD) && labelOver[i])
			labelOver[i]->Draw();
		else if(label[i])
			label[i]->Draw();
	}

	this->UpdateEffects();
}
void GuiButton::DrawTooltip()
{
	LOCK(this);
	if(state == STATE_SELECTED && (toolTip || toolTip2))
	{
	    if (time2 == 0)
		    time(&time2);

		    time(&time1);

        if (difftime(time1, time2) >= 2) {
		if(toolTip) toolTip->Draw();
		if(toolTip2) toolTip2->Draw();
		if (toolTipTxt)
        {
			toolTipTxt->Draw();
        }
        }
	}
	else
		time2 = 0;
}
void GuiButton::ScrollIsOn(int f)
{
	LOCK(this);
    scrollison = f;
}

void GuiButton::Update(GuiTrigger * t)
{
	LOCK(this);
	if(state == STATE_CLICKED || state == STATE_DISABLED || !t)
		return;
	else if(parentElement && parentElement->GetState() == STATE_DISABLED)
		return;

//    if(state != STATE_SELECTED && toolTip) {
//    time2 = 0;
//    }


	#ifdef HW_RVL
	// cursor
	if(t->wpad.ir.valid)
	{
		if(this->IsInside(t->wpad.ir.x, t->wpad.ir.y))
		{
			if(state == STATE_DEFAULT) // we weren't on the button before!
			{
			    if(scrollison == 0) {
				this->SetState(STATE_SELECTED, t->chan);
			    }

				if(this->Rumble() && scrollison == 0)
					rumbleRequest[t->chan] = 1;

				if(soundOver && scrollison == 0)
					soundOver->Play();

				if(effectsOver && !effects && scrollison == 0)
				{
					// initiate effects
					effects = effectsOver;
					effectAmount = effectAmountOver;
					effectTarget = effectTargetOver;
				}
			}
		}
		else
		{
			if(state == STATE_SELECTED && (stateChan == t->chan || stateChan == -1))
				this->ResetState();

			if(effectTarget == effectTargetOver && effectAmount == effectAmountOver)
			{
				// initiate effects (in reverse)
				effects = effectsOver;
				effectAmount = -effectAmountOver;
				effectTarget = 100;
			}
		}
	}
	#endif

	// button triggers
	if(this->IsClickable() && scrollison == 0)
	{
		s32 wm_btns, wm_btns_trig, cc_btns, cc_btns_trig;
		for(int i=0; i<6; i++)
		{
			if(trigger[i] && (trigger[i]->chan == -1 || trigger[i]->chan == t->chan))
			{
				// higher 16 bits only (wiimote)
				wm_btns = t->wpad.btns_d << 16;
				wm_btns_trig = trigger[i]->wpad.btns_d << 16;

				// lower 16 bits only (classic controller)
				cc_btns = t->wpad.btns_d >> 16;
				cc_btns_trig = trigger[i]->wpad.btns_d >> 16;

				if(
					(t->wpad.btns_d > 0 &&
					wm_btns == wm_btns_trig ||
					(cc_btns == cc_btns_trig && t->wpad.exp.type == EXP_CLASSIC)) ||
					(t->pad.btns_d == trigger[i]->pad.btns_d && t->pad.btns_d > 0))
				{
					if(t->chan == stateChan || stateChan == -1)
					{
						if(state == STATE_SELECTED)
						{
							this->SetState(STATE_CLICKED, t->chan);

							if(soundClick)
								soundClick->Play();
						}
						else if(trigger[i]->type == TRIGGER_BUTTON_ONLY)
						{
							this->SetState(STATE_CLICKED, t->chan);
						}
						else if(trigger[i]->type == TRIGGER_BUTTON_ONLY_IN_FOCUS &&
								parentElement->IsFocused())
						{
							this->SetState(STATE_CLICKED, t->chan);
						}
					}
				}
			}
		}
	}

	if(this->IsHoldable())
	{
		bool held = false;
		s32 wm_btns, wm_btns_h, wm_btns_trig, cc_btns, cc_btns_h, cc_btns_trig;

		for(int i=0; i<6; i++)
		{
			if(trigger[i] && (trigger[i]->chan == -1 || trigger[i]->chan == t->chan))
			{
				// higher 16 bits only (wiimote)
				wm_btns = t->wpad.btns_d << 16;
				wm_btns_h = t->wpad.btns_h << 16;
				wm_btns_trig = trigger[i]->wpad.btns_h << 16;

				// lower 16 bits only (classic controller)
				cc_btns = t->wpad.btns_d >> 16;
				cc_btns_h = t->wpad.btns_h >> 16;
				cc_btns_trig = trigger[i]->wpad.btns_h >> 16;

				if(
					(t->wpad.btns_d > 0 &&
					wm_btns == wm_btns_trig ||
					(cc_btns == cc_btns_trig && t->wpad.exp.type == EXP_CLASSIC)) ||
					(t->pad.btns_d == trigger[i]->pad.btns_h && t->pad.btns_d > 0))
				{
					if(trigger[i]->type == TRIGGER_HELD && state == STATE_SELECTED &&
						(t->chan == stateChan || stateChan == -1))
						this->SetState(STATE_CLICKED, t->chan);
				}

				if(
					(t->wpad.btns_h > 0 &&
					wm_btns_h == wm_btns_trig ||
					(cc_btns_h == cc_btns_trig && t->wpad.exp.type == EXP_CLASSIC)) ||
					(t->pad.btns_h == trigger[i]->pad.btns_h && t->pad.btns_h > 0))
				{
					if(trigger[i]->type == TRIGGER_HELD)
						held = true;
				}

				if(!held && state == STATE_HELD && stateChan == t->chan)
				{
					this->ResetState();
				}
				else if(held && state == STATE_CLICKED && stateChan == t->chan)
				{
					this->SetState(STATE_HELD, t->chan);
				}
			}
		}
	}

	if(updateCB)
		updateCB(this);
}
