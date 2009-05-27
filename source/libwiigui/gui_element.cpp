/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_element.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

/**
 * Constructor for the Object class.
 */
mutex_t GuiElement::mutex = 0;
GuiElement::GuiElement()
{
	xoffset = 0;
	yoffset = 0;
	xmin = 0;
	xmax = 0;
	ymin = 0;
	ymax = 0;
	width = 0;
	height = 0;
	alpha = 255;
	scale = 1;
	state = STATE_DEFAULT;
	stateChan = -1;
	trigger[0] = NULL;
	trigger[1] = NULL;
	trigger[2] = NULL;
	trigger[3] = NULL;
	trigger[4] = NULL;
	trigger[6] = NULL;
	parentElement = NULL;
	rumble = true;
	selectable = false;
	clickable = false;
	holdable = false;
	visible = true;
	focus = -1; // cannot be focused
	updateCB = NULL;
	yoffsetDyn = 0;
	xoffsetDyn = 0;
	yoffsetDynFloat = 0;
	alphaDyn = -1;
	scaleDyn = 1;
	effects = 0;
	effectAmount = 0;
	effectTarget = 0;
	effectsOver = 0;
	effectAmountOver = 0;
	effectTargetOver = 0;
	frequency = 0.0;
	changervar = 0;
    degree = -90*PI/180;
    circleamount = 360;
    Radius = 150;
    angleDyn = 0.0;
    anglespeed = 0.0;

	// default alignment - align to top left
	alignmentVert = ALIGN_TOP;
	alignmentHor = ALIGN_LEFT;
	if(mutex == 0)	LWP_MutexInit(&mutex, true);
}

/**
 * Destructor for the GuiElement class.
 */
GuiElement::~GuiElement()
{
//	LWP_MutexDestroy(mutex);
}

void GuiElement::SetParent(GuiElement * e)
{
	LOCK(this);
	parentElement = e;
}
/**
 * Get the left position of the GuiElement.
 * @see SetLeft()
 * @return Left position in pixel.
 */
int GuiElement::GetLeft()
{
	int x = 0;
	int pWidth = 0;
	int pLeft = 0;

	if(parentElement)
	{
		pWidth = parentElement->GetWidth();
		pLeft = parentElement->GetLeft();
	}

	if(effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT | EFFECT_GOROUND | EFFECT_ROCK_VERTICLE))
		pLeft += xoffsetDyn;

	switch(alignmentHor)
	{
		case ALIGN_LEFT:
			x = pLeft;
			break;
		case ALIGN_CENTRE:
			x = pLeft + (pWidth/2) - (width/2);
			break;
		case ALIGN_RIGHT:
			x = pLeft + pWidth - width;
			break;
	}
	return x + xoffset;
}

/**
 * Get the top position of the GuiElement.
 * @see SetTop()
 * @return Top position in pixel.
 */
int GuiElement::GetTop()
{
	int y = 0;
	int pHeight = 0;
	int pTop = 0;

	if(parentElement)
	{
		pHeight = parentElement->GetHeight();
		pTop = parentElement->GetTop();
	}

	if(effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT | EFFECT_GOROUND | EFFECT_ROCK_VERTICLE))
		pTop += yoffsetDyn;


	switch(alignmentVert)
	{
		case ALIGN_TOP:
			y = pTop;
			break;
		case ALIGN_MIDDLE:
			y = pTop + (pHeight/2) - (height/2);
			break;
		case ALIGN_BOTTOM:
			y = pTop + pHeight - height;
			break;
	}
	return y + yoffset;
}

void GuiElement::SetMinX(int x)
{
	LOCK(this);
	xmin = x;
}

int GuiElement::GetMinX()
{
	return xmin;
}

void GuiElement::SetMaxX(int x)
{
	LOCK(this);
	xmax = x;
}

int GuiElement::GetMaxX()
{
	return xmax;
}

void GuiElement::SetMinY(int y)
{
	LOCK(this);
	ymin = y;
}

int GuiElement::GetMinY()
{
	return ymin;
}

void GuiElement::SetMaxY(int y)
{
	LOCK(this);
	ymax = y;
}

int GuiElement::GetMaxY()
{
	return ymax;
}

/**
 * Get the width of the GuiElement.
 * @see SetWidth()
 * @return Width of the GuiElement.
 */
int GuiElement::GetWidth()
{
	return width;
}

/**
 * Get the height of the GuiElement.
 * @see SetHeight()
 * @return Height of the GuiElement.
 */
int GuiElement::GetHeight()
{
	return height;
}

/**
 * Set the width and height of the GuiElement.
 * @param[in] Width Width in pixel.
 * @param[in] Height Height in pixel.
 * @see SetWidth()
 * @see SetHeight()
 */
void GuiElement::SetSize(int w, int h)
{
	LOCK(this);

	width = w;
	height = h;
}

/**
 * Get visible.
 * @see SetVisible()
 * @return true if visible, false otherwise.
 */
bool GuiElement::IsVisible()
{
	return visible;
}

/**
 * Set visible.
 * @param[in] Visible Set to true to show GuiElement.
 * @see IsVisible()
 */
void GuiElement::SetVisible(bool v)
{
	LOCK(this);
	visible = v;
}

void GuiElement::SetAlpha(int a)
{
	LOCK(this);
	alpha = a;
}

int GuiElement::GetAlpha()
{
	int a;

	if(alphaDyn >= 0)
		a = alphaDyn;
	else
		a = alpha;

	if(parentElement)
		a *= parentElement->GetAlpha()/255.0;

	return a;
}

float GuiElement::GetAngleDyn()
{
    float a = 0.0;

    if(angleDyn)
    a = angleDyn;

    if(parentElement && !angleDyn)
    a = parentElement->GetAngleDyn();

    return a;
}

void GuiElement::SetScale(float s)
{
	LOCK(this);
	scale = s;
}

float GuiElement::GetScale()
{
	float s = scale * scaleDyn;

	if(parentElement)
		s *= parentElement->GetScale();

	return s;
}

int GuiElement::GetState()
{
	return state;
}

int GuiElement::GetStateChan()
{
	return stateChan;
}

void GuiElement::SetState(int s, int c)
{
	LOCK(this);
	state = s;
	stateChan = c;
}

void GuiElement::ResetState()
{
	LOCK(this);
	if(state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}
}

void GuiElement::SetClickable(bool c)
{
	LOCK(this);
	clickable = c;
}

void GuiElement::SetSelectable(bool s)
{
	LOCK(this);
	selectable = s;
}

void GuiElement::SetHoldable(bool d)
{
	LOCK(this);
	holdable = d;
}

bool GuiElement::IsSelectable()
{
	if(state == STATE_DISABLED || state == STATE_CLICKED)
		return false;
	else
		return selectable;
}

bool GuiElement::IsClickable()
{
	if(state == STATE_DISABLED ||
		state == STATE_CLICKED ||
		state == STATE_HELD)
		return false;
	else
		return clickable;
}

bool GuiElement::IsHoldable()
{
	if(state == STATE_DISABLED)
		return false;
	else
		return holdable;
}

void GuiElement::SetFocus(int f)
{
	LOCK(this);
	focus = f;
}

int GuiElement::IsFocused()
{
	return focus;
}

void GuiElement::SetTrigger(GuiTrigger * t)
{
	LOCK(this);
	if(!trigger[0])
		trigger[0] = t;
	else if(!trigger[1])
		trigger[1] = t;
    else if(!trigger[2])
		trigger[2] = t;
    else if(!trigger[3])
		trigger[3] = t;
    else if(!trigger[4])
		trigger[4] = t;
    else if(!trigger[5])
		trigger[5] = t;
	else // both were assigned, so we'll just overwrite the first one
		trigger[0] = t;
}

void GuiElement::SetTrigger(u8 i, GuiTrigger * t)
{
	LOCK(this);
	trigger[i] = t;
}

void GuiElement::RemoveTrigger(u8 i)
{
	LOCK(this);
	trigger[i] = NULL;
}

bool GuiElement::Rumble()
{
	return rumble;
}

void GuiElement::SetRumble(bool r)
{
	LOCK(this);
	rumble = r;
}

int GuiElement::GetEffect()
{
	LOCK(this);
	return effects;
}

void GuiElement::SetEffect(int eff, int speed, int circles, int r, int startdegree, f32 anglespeedset) {

    if(eff & EFFECT_GOROUND) {
        xoffsetDyn = 0;             //!position of circle in x
        yoffsetDyn = 0;             //!position of circle in y
        Radius = r;                 //!Radius of the circle
        degree = startdegree*PI/180;//!for example -90 (°) to start at top of circle
        circleamount = circles;     //!circleamoutn in degrees for example 360 for 1 circle
        angleDyn = 0.0;             //!this is used by the code to calc the angle
        anglespeed = anglespeedset; //!This is anglespeed depending on circle speed 1 is same speed and 0.5 half speed
    }
    effects |= eff;
    effectAmount = speed;           //!Circlespeed
}

void GuiElement::SetEffect(int eff, int amount, int target)
{
	LOCK(this);
	if(eff & EFFECT_SLIDE_IN)
	{
		// these calculations overcompensate a little
		if(eff & EFFECT_SLIDE_TOP)
			yoffsetDyn = -screenheight;
		else if(eff & EFFECT_SLIDE_LEFT)
			xoffsetDyn = -screenwidth;
		else if(eff & EFFECT_SLIDE_BOTTOM)
			yoffsetDyn = screenheight;
		else if(eff & EFFECT_SLIDE_RIGHT)
			xoffsetDyn = screenwidth;
	}

	if(eff & EFFECT_FADE && amount > 0)
	{
		alphaDyn = 0;
	}
	else if(eff & EFFECT_FADE && amount < 0)
	{
		alphaDyn = alpha;

	} else if(eff & EFFECT_ROCK_VERTICLE) {
	    changervar = 0;
        yoffsetDyn = 0;
        yoffsetDynFloat = 0.0;
	}

	effects |= eff;
	effectAmount = amount;
	effectTarget = target;
}

void GuiElement::SetEffectOnOver(int eff, int amount, int target)
{
	LOCK(this);
	effectsOver |= eff;
	effectAmountOver = amount;
	effectTargetOver = target;
}

void GuiElement::SetEffectGrow()
{
	SetEffectOnOver(EFFECT_SCALE, 4, 110);
}

void GuiElement::StopEffect()
{
    xoffsetDyn = 0;
	yoffsetDyn = 0;
	effects = 0;
	effectsOver = 0;
	effectAmount = 0;
	effectAmountOver = 0;
	effectTarget = 0;
	effectTargetOver = 0;
	scaleDyn = 1;
	frequency = 0;
	changervar = 0;
	angleDyn = 0;
	anglespeed = 0.0;
}

void GuiElement::UpdateEffects()
{
	LOCK(this);

	if(effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT | EFFECT_GOROUND))
	{
		if(effects & EFFECT_SLIDE_IN)
		{
			if(effects & EFFECT_SLIDE_LEFT)
			{
				xoffsetDyn += effectAmount;

				if(xoffsetDyn >= 0)
				{
					xoffsetDyn = 0;
					effects = 0;
				}
			}
			else if(effects & EFFECT_SLIDE_RIGHT)
			{
				xoffsetDyn -= effectAmount;

				if(xoffsetDyn <= 0)
				{
					xoffsetDyn = 0;
					effects = 0;
				}
			}
			else if(effects & EFFECT_SLIDE_TOP)
			{
				yoffsetDyn += effectAmount;

				if(yoffsetDyn >= 0)
				{
					yoffsetDyn = 0;
					effects = 0;
				}
			}
			else if(effects & EFFECT_SLIDE_BOTTOM)
			{
				yoffsetDyn -= effectAmount;

				if(yoffsetDyn <= 0)
				{
					yoffsetDyn = 0;
					effects = 0;
				}
			}
		}
		else
		{
			if(effects & EFFECT_SLIDE_LEFT)
			{
				xoffsetDyn -= effectAmount;

				if(xoffsetDyn <= -screenwidth)
					effects = 0; // shut off effect
			}
			else if(effects & EFFECT_SLIDE_RIGHT)
			{
				xoffsetDyn += effectAmount;

				if(xoffsetDyn >= screenwidth)
					effects = 0; // shut off effect
			}
			else if(effects & EFFECT_SLIDE_TOP)
			{
				yoffsetDyn -= effectAmount;

				if(yoffsetDyn <= -screenheight)
					effects = 0; // shut off effect
			}
			else if(effects & EFFECT_SLIDE_BOTTOM)
			{
				yoffsetDyn += effectAmount;

				if(yoffsetDyn >= screenheight)
					effects = 0; // shut off effect
			}
		}
	}

    if(effects & EFFECT_GOROUND) {

        //!< check out gui.h for info
        if(abs(frequency) < PI*circleamount/180) {

        angleDyn = (frequency+degree) * 180/PI * anglespeed;
        frequency += effectAmount*0.001;

        xoffsetDyn = (int)(Radius*cos(frequency+degree));
        yoffsetDyn = (int)(Radius*sin(frequency+degree));

        } else {

            //Angle go back to start value (has to be 0.0001 when near 0 but not 0 so that the if state goes through)
            //value 0.0001 isnt noticeable that's why i chose it.
            angleDyn = degree* 180/PI * anglespeed +0.0001;
            //Reset Angle to 0
            //angleDyn = 0.0001;

            //fly back to the middle tolerance to 0 is +- 10pixel
            if(xoffsetDyn < -10)
            xoffsetDyn += abs(effectAmount)*0.1;
            else if(xoffsetDyn > 10)
            xoffsetDyn -= abs(effectAmount)*0.1;
            else xoffsetDyn = 0;

            if(yoffsetDyn < -10)
            yoffsetDyn += abs(effectAmount)*0.1;
            else if(yoffsetDyn > 10)
            yoffsetDyn -= abs(effectAmount)*0.1;
            else yoffsetDyn = 0;

            if(xoffsetDyn == 0 && yoffsetDyn == 0) {
                effects = 0;
                frequency = 0;
                Radius = 0;
            }
        }
    }

    if(effects & EFFECT_ROCK_VERTICLE) {
        //move up to 10pixel above 0
        if(changervar == 0 && yoffsetDynFloat < 11.0) {
            yoffsetDynFloat += (effectAmount*0.01);
        } else if(yoffsetDynFloat > 10.0) {
            changervar = 1;
        }
        //move down till 10pixel under 0
        if(changervar == 1 && yoffsetDynFloat > -11.0) {
            yoffsetDynFloat -= (effectAmount*0.01);
        } else if(yoffsetDynFloat < -10.0) {
            changervar = 0;
        }
        yoffsetDyn = (int)(yoffsetDynFloat);
    }

	if(effects & EFFECT_FADE)
	{
		alphaDyn += effectAmount;

		if(effectAmount < 0 && alphaDyn <= 0)
		{
			alphaDyn = 0;
			effects = 0; // shut off effect
		}
		else if(effectAmount > 0 && alphaDyn >= alpha)
		{
			alphaDyn = alpha;
			effects = 0; // shut off effect
		}
	}
    if(effects & EFFECT_SCALE)
	{
		scaleDyn += effectAmount/100.0;

		if((effectAmount < 0 && scaleDyn <= effectTarget/100.0)
			|| (effectAmount > 0 && scaleDyn >= effectTarget/100.0))
		{
			scaleDyn = effectTarget/100.0;
			effects = 0; // shut off effect
		}
	}
	if(effects & EFFECT_PULSE)
	{
	    int percent = 10; //go down from target by this

	    if((scaleDyn <= (effectTarget*0.01)) && (!changervar)) {
            scaleDyn += (effectAmount*0.001);
	    } else if(scaleDyn > (effectTarget*0.01)) {
            changervar = 1;
	    }
	    if((scaleDyn >= ((effectTarget-percent)*0.01)) && (changervar)) {
            scaleDyn -= (effectAmount*0.001);
	    } else if(scaleDyn < ((effectTarget-percent)*0.01)) {
            changervar = 0;
	    }
	}
}

void GuiElement::Update(GuiTrigger * t)
{
	LOCK(this);
	if(updateCB)
		updateCB(this);
}

void GuiElement::SetUpdateCallback(UpdateCallback u)
{
	LOCK(this);
	updateCB = u;
}

void GuiElement::SetPosition(int xoff, int yoff)
{
	LOCK(this);
	xoffset = xoff;
	yoffset = yoff;
}

void GuiElement::SetAlignment(int hor, int vert)
{
	LOCK(this);
	alignmentHor = hor;
	alignmentVert = vert;
}

int GuiElement::GetSelected()
{
	return -1;
}

/**
 * Draw an element on screen.
 */
void GuiElement::Draw()
{
}

/**
 * Draw Tooltips on screen.
 */
void GuiElement::DrawTooltip()
{
}

/**
 * Check if a position is inside the GuiElement.
 * @param[in] x X position in pixel.
 * @param[in] y Y position in pixel.
 */
bool GuiElement::IsInside(int x, int y)
{
	if(x > this->GetLeft() && x < (this->GetLeft()+width)
	&& y > this->GetTop() && y < (this->GetTop()+height))
		return true;
	return false;
}
void GuiElement::Lock()
{
	LWP_MutexLock(mutex);
}
void GuiElement::Unlock()
{
	LWP_MutexUnlock(mutex);
}


SimpleLock::SimpleLock(GuiElement *e) : element(e)
{
	element->Lock();
}
SimpleLock::~SimpleLock()
{
	element->Unlock();
}
