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
//mutex_t GuiElement::mutex = LWP_MUTEX_NULL;
mutex_t GuiElement::_lock_mutex = LWP_MUTEX_NULL;
GuiElement::GuiElement()
{
	xoffset = 0;
	yoffset = 0;
	zoffset = 0;
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
	trigger[5] = NULL;
	parentElement = NULL;
	rumble = true;
	selectable = false;
	clickable = false;
	holdable = false;
	visible = true;
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
	frequency = 0.0f;
	changervar = 0;
	degree = -90.0f;
	circleamount = 360.0f;
	Radius = 150;
	angleDyn = 0.0f;
	anglespeed = 0.0f;

	// default alignment - align to top left
	alignmentVert = ALIGN_TOP;
	alignmentHor = ALIGN_LEFT;
	//  if(mutex == LWP_MUTEX_NULL) LWP_MutexInit(&mutex, true);
	if (_lock_mutex == LWP_MUTEX_NULL) LWP_MutexInit(&_lock_mutex, true);
	_lock_thread = LWP_THREAD_NULL;
	_lock_count = 0;
	_lock_queue = LWP_TQUEUE_NULL;

}

/**
 * Destructor for the GuiElement class.
 */
GuiElement::~GuiElement()
{
	//  LWP_MutexDestroy(mutex);
}

void GuiElement::SetParent(GuiElement * e)
{
	LOCK( this );
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

	if (parentElement)
	{
		pWidth = parentElement->GetWidth();
		pLeft = parentElement->GetLeft();
	}

	if (effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT | EFFECT_GOROUND | EFFECT_ROCK_VERTICLE)) pLeft += xoffsetDyn;

	switch (alignmentHor)
	{
		case ALIGN_LEFT:
			x = pLeft;
			break;
		case ALIGN_CENTER:
			x = pLeft + (pWidth / 2) - (width / 2);
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

	if (parentElement)
	{
		pHeight = parentElement->GetHeight();
		pTop = parentElement->GetTop();
	}

	if (effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT | EFFECT_GOROUND | EFFECT_ROCK_VERTICLE)) pTop += yoffsetDyn;

	switch (alignmentVert)
	{
		case ALIGN_TOP:
			y = pTop;
			break;
		case ALIGN_MIDDLE:
			y = pTop + (pHeight / 2) - (height / 2);
			break;
		case ALIGN_BOTTOM:
			y = pTop + pHeight - height;
			break;
	}
	return y + yoffset;
}

int GuiElement::GetRelLeft()
{
	GuiElement *tmp = parentElement;
	parentElement = NULL;
	int pos = GetLeft();
	parentElement = tmp;

	return pos;
}

int GuiElement::GetRelTop()
{
	GuiElement *tmp = parentElement;
	parentElement = NULL;
	int pos = GetTop();
	parentElement = tmp;

	return pos;
}

void GuiElement::SetMinX(int x)
{
	LOCK( this );
	xmin = x;
}

void GuiElement::SetMaxX(int x)
{
	LOCK( this );
	xmax = x;
}

void GuiElement::SetMinY(int y)
{
	LOCK( this );
	ymin = y;
}

void GuiElement::SetMaxY(int y)
{
	LOCK( this );
	ymax = y;
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
	LOCK( this );

	width = w;
	height = h;
}

/**
 * Set visible.
 * @param[in] Visible Set to true to show GuiElement.
 * @see IsVisible()
 */
void GuiElement::SetVisible(bool v)
{
	LOCK( this );
	visible = v;
}

void GuiElement::SetAlpha(int a)
{
	LOCK( this );
	alpha = a;
}

int GuiElement::GetAlpha()
{
	int a;

	if (alphaDyn >= 0)
		a = alphaDyn;
	else a = alpha;

	if (parentElement) a *= parentElement->GetAlpha() / 255.0;

	return a;
}

float GuiElement::GetAngleDyn()
{
	float a = 0.0;

	if (angleDyn) a = angleDyn;

	if (parentElement && !angleDyn) a = parentElement->GetAngleDyn();

	return a;
}

void GuiElement::SetScale(float s)
{
	LOCK( this );
	scale = s;
}

float GuiElement::GetScale()
{
	float s = scale * scaleDyn;

	if (parentElement) s *= parentElement->GetScale();

	return s;
}

void GuiElement::SetState(int s, int c)
{
	LOCK( this );
	state = s;
	stateChan = c;
}

void GuiElement::ResetState()
{
	LOCK( this );
	if (state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}
}

void GuiElement::SetClickable(bool c)
{
	LOCK( this );
	clickable = c;
}

void GuiElement::SetSelectable(bool s)
{
	LOCK( this );
	selectable = s;
}

void GuiElement::SetHoldable(bool d)
{
	LOCK( this );
	holdable = d;
}

bool GuiElement::IsSelectable()
{
	if (state == STATE_DISABLED || state == STATE_CLICKED)
		return false;
	else return selectable;
}

bool GuiElement::IsClickable()
{
	if (state == STATE_DISABLED || state == STATE_CLICKED || state == STATE_HELD)
		return false;
	else return clickable;
}

bool GuiElement::IsHoldable()
{
	if (state == STATE_DISABLED)
		return false;
	else return holdable;
}

void GuiElement::SetTrigger(GuiTrigger * t)
{
	LOCK( this );
	if (!trigger[0])
		trigger[0] = t;
	else if (!trigger[1])
		trigger[1] = t;
	else if (!trigger[2])
		trigger[2] = t;
	else if (!trigger[3])
		trigger[3] = t;
	else if (!trigger[4])
		trigger[4] = t;
	else if (!trigger[5])
		trigger[5] = t;
	else // both were assigned, so we'll just overwrite the first one
	trigger[0] = t;
}

void GuiElement::SetTrigger(u8 i, GuiTrigger * t)
{
	LOCK( this );
	trigger[i] = t;
}

void GuiElement::RemoveTrigger(u8 i)
{
	LOCK( this );
	trigger[i] = NULL;
}

void GuiElement::SetRumble(bool r)
{
	LOCK( this );
	rumble = r;
}

float GuiElement::GetFrequency()
{
	LOCK( this );
	return frequency;
}

void GuiElement::SetEffect(int eff, int speed, f32 circles, int r, f32 startdegree, f32 anglespeedset, int center_x,
		int center_y)
{

	if (eff & EFFECT_GOROUND)
	{
		xoffsetDyn = 0; //!position of circle in x
		yoffsetDyn = 0; //!position of circle in y
		Radius = r; //!radius of the circle
		degree = startdegree; //!for example -90 (°) to start at top of circle
		circleamount = circles; //!circleamoutn in degrees for example 360 for 1 circle
		angleDyn = 0.0f; //!this is used by the code to calc the angle
		anglespeed = anglespeedset; //!This is anglespeed depending on circle speed 1 is same speed and 0.5 half speed
		temp_xoffset = center_x; //!position of center in x
		temp_yoffset = center_y; //!position of center in y
	}
	effects |= eff;
	effectAmount = speed; //!Circlespeed
}

void GuiElement::SetEffect(int eff, int amount, int target)
{
	LOCK( this );
	if (eff & EFFECT_SLIDE_IN)
	{
		// these calculations overcompensate a little
		if (eff & EFFECT_SLIDE_TOP)
			yoffsetDyn = -screenheight;
		else if (eff & EFFECT_SLIDE_LEFT)
			xoffsetDyn = -screenwidth;
		else if (eff & EFFECT_SLIDE_BOTTOM)
			yoffsetDyn = screenheight;
		else if (eff & EFFECT_SLIDE_RIGHT) xoffsetDyn = screenwidth;
	}

	if ((eff & EFFECT_FADE) && amount > 0)
	{
		alphaDyn = 0;
	}
	else if ((eff & EFFECT_FADE) && amount < 0)
	{
		alphaDyn = alpha;

	}
	else if (eff & EFFECT_ROCK_VERTICLE)
	{
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
	LOCK( this );
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
	frequency = 0.0f;
	changervar = 0;
	//angleDyn = 0.0f;
	anglespeed = 0.0f;
}

void GuiElement::UpdateEffects()
{
	LOCK( this );

	if (effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT | EFFECT_GOROUND))
	{
		if (effects & EFFECT_SLIDE_IN)
		{
			if (effects & EFFECT_SLIDE_LEFT)
			{
				xoffsetDyn += effectAmount;

				if (xoffsetDyn >= 0)
				{
					xoffsetDyn = 0;
					effects = 0;
				}
			}
			else if (effects & EFFECT_SLIDE_RIGHT)
			{
				xoffsetDyn -= effectAmount;

				if (xoffsetDyn <= 0)
				{
					xoffsetDyn = 0;
					effects = 0;
				}
			}
			else if (effects & EFFECT_SLIDE_TOP)
			{
				yoffsetDyn += effectAmount;

				if (yoffsetDyn >= 0)
				{
					yoffsetDyn = 0;
					effects = 0;
				}
			}
			else if (effects & EFFECT_SLIDE_BOTTOM)
			{
				yoffsetDyn -= effectAmount;

				if (yoffsetDyn <= 0)
				{
					yoffsetDyn = 0;
					effects = 0;
				}
			}
		}
		else
		{
			if (effects & EFFECT_SLIDE_LEFT)
			{
				xoffsetDyn -= effectAmount;

				if (xoffsetDyn <= -screenwidth) effects = 0; // shut off effect
			}
			else if (effects & EFFECT_SLIDE_RIGHT)
			{
				xoffsetDyn += effectAmount;

				if (xoffsetDyn >= screenwidth) effects = 0; // shut off effect
			}
			else if (effects & EFFECT_SLIDE_TOP)
			{
				yoffsetDyn -= effectAmount;

				if (yoffsetDyn <= -screenheight) effects = 0; // shut off effect
			}
			else if (effects & EFFECT_SLIDE_BOTTOM)
			{
				yoffsetDyn += effectAmount;

				if (yoffsetDyn >= screenheight) effects = 0; // shut off effect
			}
		}
	}

	if (effects & EFFECT_GOROUND)
	{
		//!< check out gui.h for info
		xoffset = temp_xoffset;
		yoffset = temp_yoffset;
		if (fabs(frequency) < circleamount)
		{
			angleDyn = (frequency + degree + 90.0f) * anglespeed;
			xoffsetDyn = (int) lround(((f32) Radius) * cosf((frequency + degree) * PI / 180.0f));
			yoffsetDyn = (int) lround(((f32) Radius) * sinf((frequency + degree) * PI / 180.0f));
			frequency += ((f32) effectAmount) * 0.01f;
		}
		else
		{
			f32 temp_frequency = ((effectAmount < 0) ? -1.0f : 1.0f) * circleamount;
			angleDyn = (temp_frequency + degree + 90.0f) * anglespeed;
			xoffsetDyn = (int) lround(((f32) Radius) * cosf((temp_frequency + degree) * PI / 180.0f));
			yoffsetDyn = (int) lround(((f32) Radius) * sinf((temp_frequency + degree) * PI / 180.0f));
			xoffset += xoffsetDyn;
			yoffset += yoffsetDyn;
			effects ^= EFFECT_GOROUND;
			frequency = 0.0f;
		}
	}

	if (effects & EFFECT_ROCK_VERTICLE)
	{
		//move up to 10pixel above 0
		if (changervar == 0 && yoffsetDynFloat < 11.0f)
		{
			yoffsetDynFloat += (effectAmount * 0.01f);
		}
		else if (yoffsetDynFloat > 10.0f)
		{
			changervar = 1;
		}
		//move down till 10pixel under 0
		if (changervar == 1 && yoffsetDynFloat > -11.0f)
		{
			yoffsetDynFloat -= (effectAmount * 0.01f);
		}
		else if (yoffsetDynFloat < -10.0f)
		{
			changervar = 0;
		}
		yoffsetDyn = (int) (yoffsetDynFloat);
	}

	if (effects & EFFECT_FADE)
	{
		alphaDyn += effectAmount;

		if (effectAmount < 0 && alphaDyn <= 0)
		{
			alphaDyn = 0;
			effects = 0; // shut off effect
		}
		else if (effectAmount > 0 && alphaDyn >= alpha)
		{
			alphaDyn = alpha;
			effects = 0; // shut off effect
		}
	}
	if (effects & EFFECT_SCALE)
	{
		scaleDyn += effectAmount / 100.0f;

		if ((effectAmount < 0 && scaleDyn <= effectTarget / 100.0f) || (effectAmount > 0 && scaleDyn >= effectTarget
				/ 100.0f))
		{
			scaleDyn = effectTarget / 100.0f;
			effects = 0; // shut off effect
		}
	}
	if (effects & EFFECT_PULSE)
	{
		int percent = 10; //go down from target by this

		if ((scaleDyn <= (effectTarget * 0.01f)) && (!changervar))
		{
			scaleDyn += (effectAmount * 0.001f);
		}
		else if (scaleDyn > (effectTarget * 0.01f))
		{
			changervar = 1;
		}
		if ((scaleDyn >= ((effectTarget - percent) * 0.01f)) && (changervar))
		{
			scaleDyn -= (effectAmount * 0.001f);
		}
		else if (scaleDyn < ((effectTarget - percent) * 0.01f))
		{
			changervar = 0;
		}
	}
}

void GuiElement::Update(GuiTrigger * t)
{
	LOCK( this );
	if (updateCB) updateCB(this);
}

void GuiElement::SetUpdateCallback(UpdateCallback u)
{
	LOCK( this );
	updateCB = u;
}

void GuiElement::SetPosition(int xoff, int yoff, int zoff)
{
	LOCK( this );
	xoffset = xoff;
	yoffset = yoff;
	zoffset = zoff;
}

void GuiElement::SetAlignment(int hor, int vert)
{
	LOCK( this );
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
	if (x > this->GetLeft() && x < (this->GetLeft() + width) && y > this->GetTop() && y < (this->GetTop() + height)) return true;
	return false;
}
void GuiElement::LockElement()
{
	//  LWP_MutexLock(mutex);
	for (;;) // loop while element is locked by self
	{
		LWP_MutexLock(_lock_mutex);

		if (_lock_thread == LWP_THREAD_NULL) // element is not locked
		{
			_lock_thread = LWP_GetSelf(); // mark as locked
			_lock_count = 1; // set count of lock to 1
			LWP_MutexUnlock(_lock_mutex);
			return;
		}
		else if (_lock_thread == LWP_GetSelf()) // thread is locked by my self
		{
			_lock_count++; // inc count of locks;
			LWP_MutexUnlock(_lock_mutex);
			return;
		}
		else // otherwise the element is locked by an other thread
		{
			if (_lock_queue == LWP_TQUEUE_NULL) // no queue - meens it is the first access to the locked element
			LWP_InitQueue(&_lock_queue); // init queue
			LWP_MutexUnlock(_lock_mutex);
			LWP_ThreadSleep(_lock_queue); // and sleep
			// try lock again;
		}
	}
}
void GuiElement::UnlockElement()
{
	//  LWP_MutexUnlock(mutex);
	LWP_MutexLock(_lock_mutex);
	// only the thread was locked this element, can call unlock
	if (_lock_thread == LWP_GetSelf()) // but we check it here safe is safe
	{
		if (--_lock_count == 0) // dec count of locks and check if it last lock;
		{
			_lock_thread = LWP_THREAD_NULL; // mark as unlocked
			if (_lock_queue != LWP_TQUEUE_NULL) // has a queue
			{
				LWP_CloseQueue(_lock_queue); // close the queue and wake all waited threads
				_lock_queue = LWP_TQUEUE_NULL;
			}
		}
	}
	LWP_MutexUnlock(_lock_mutex);
}

SimpleLock::SimpleLock(GuiElement *e) :
	element(e)
{
	element->LockElement();
}
SimpleLock::~SimpleLock()
{
	element->UnlockElement();
}
