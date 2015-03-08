/****************************************************************************
 * LibWiiGui by Tantric (C) 2009
 * USB Loader GX Team (C) 2009-2011
 *
 * The LibWiiGui library was used as the base for the creation of
 * the GUI in USB Loader GX.
 * Several modifications and additions were made to the library
 * It does no longer match the original LibWiiGui implementation.
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
#ifndef LIBWIIGUI_H
#define LIBWIIGUI_H

#include <gccore.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <vector>
#include <math.h>
#include <asndlib.h>
#include <wiiuse/wpad.h>
#include <wupc/wupc.h>
#include "gui_imagedata.h"
#include "FreeTypeGX.h"
#include "video.h"
#include "input.h"
#include "OptionList.hpp"
#include "SoundOperations/gui_sound.h"
#include "SoundOperations/gui_bgm.h"
#include "utils/timer.h"
#include "sigslot.h"

//! Frequently used variables
extern FreeTypeGX *fontSystem;
extern GuiSound *btnSoundClick;
extern GuiSound *btnSoundClick2;
extern GuiSound *btnSoundOver;
extern GuiBGM *bgMusic;

#define SCROLL_INITIAL_DELAY	20
#define SCROLL_LOOP_DELAY	   3
#define PAGESIZE				9
#define FILEBROWSERSIZE		 8
#define MAX_OPTIONS			 170

typedef void (*UpdateCallback)(void * e);

enum
{
	ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER, ALIGN_TOP, ALIGN_BOTTOM, ALIGN_MIDDLE
};

enum
{
	STATE_DEFAULT, STATE_SELECTED, STATE_CLICKED, STATE_HELD, STATE_DISABLED
};

enum
{
	IMAGE_TEXTURE, IMAGE_COLOR, IMAGE_DATA, IMAGE_COPY
};

enum
{
	TRIGGER_SIMPLE, TRIGGER_HELD, TRIGGER_BUTTON_ONLY
};

enum
{
	WRAP, DOTTED, SCROLL_HORIZONTAL, SCROLL_NONE
};


typedef struct _POINT {
	s16 x;
	s16 y;
} POINT;

typedef struct _paddata
{
		u16 btns_d;
		u16 btns_u;
		u16 btns_h;
		s8 stickX;
		s8 stickY;
		s8 substickX;
		s8 substickY;
		u8 triggerL;
		u8 triggerR;
} PADData;

typedef struct _wupcfulldata {
	u32 btns_d;
	u32 btns_u;
	u32 btns_h;
	s16 stickX;
	s16 stickY;
	s16 substickX;
	s16 substickY;
} WUPCFULLData;

#define EFFECT_SLIDE_TOP			1
#define EFFECT_SLIDE_BOTTOM		 2
#define EFFECT_SLIDE_RIGHT		  4
#define EFFECT_SLIDE_LEFT		   8
#define EFFECT_SLIDE_IN			 16
#define EFFECT_SLIDE_OUT			32
#define EFFECT_FADE				 64
#define EFFECT_SCALE				128
#define EFFECT_COLOR_TRANSITION	 256
#define EFFECT_PULSE				512
#define EFFECT_ROCK_VERTICLE		1024
#define EFFECT_GOROUND			  2048

//!Menu input trigger management. Determine if action is neccessary based on input data by comparing controller input data to a specific trigger element.
class GuiTrigger
{
	public:
		//!Constructor
		GuiTrigger();
		//!Destructor
		virtual ~GuiTrigger();
		//!Sets a simple trigger. Requires: element is selected, and trigger button is pressed
		//!\param ch Controller channel number
		//!\param wiibtns Wii controller trigger button(s) - classic controller buttons are considered separately
		//!\param gcbtns GameCube controller trigger button(s)
		void SetSimpleTrigger(s32 ch, u32 wiibtns, u16 gcbtns);
		//!Sets a held trigger. Requires: element is selected, and trigger button is pressed
		//!\param ch Controller channel number
		//!\param wiibtns Wii controller trigger button(s) - classic controller buttons are considered separately
		//!\param gcbtns GameCube controller trigger button(s)
		void SetHeldTrigger(s32 ch, u32 wiibtns, u16 gcbtns);
		//!Sets a button-only trigger. Requires: Trigger button is pressed
		//!\param ch Controller channel number
		//!\param wiibtns Wii controller trigger button(s) - classic controller buttons are considered separately
		//!\param gcbtns GameCube controller trigger button(s)
		void SetButtonOnlyTrigger(s32 ch, u32 wiibtns, u16 gcbtns);
		//!Get X/Y value from Wii Joystick (classic, nunchuk) input
		//!\param right Controller stick (left = 0, right = 1)
		//!\param axis Controller stick axis (x-axis = 0, y-axis = 1)
		//!\return Stick value
		s8 WPAD_Stick(u8 right, int axis);
		//!Move menu selection left (via pad/joystick). Allows scroll delay and button overriding
		//!\return true if selection should be moved left, false otherwise
		bool Left();
		//!Move menu selection right (via pad/joystick). Allows scroll delay and button overriding
		//!\return true if selection should be moved right, false otherwise
		bool Right();
		//!Move menu selection up (via pad/joystick). Allows scroll delay and button overriding
		//!\return true if selection should be moved up, false otherwise
		bool Up();
		//!Move menu selection down (via pad/joystick). Allows scroll delay and button overriding
		//!\return true if selection should be moved down, false otherwise
		bool Down();

		u8 type; //!< trigger type (TRIGGER_SIMPLE, TRIGGER_HELD, TRIGGER_BUTTON_ONLY)
		s32 chan; //!< Trigger controller channel (0-3, -1 for all)
		WPADData wpad; //!< Wii controller trigger data
		WUPCFULLData wupcdata;//!< Wii U pro controller trigger data
		PADData pad; //!< GameCube controller trigger data
};

extern GuiTrigger userInput[4];

//!Primary GUI class. Most other classes inherit from this class.
class GuiElement
{
	public:
		//!Constructor
		GuiElement();
		//!Destructor
		virtual ~GuiElement();
		//!Set the element's parent
		//!\param e Pointer to parent element
		void SetParent(GuiElement * e);
		//!Gets the element's parent
		//!\return Pointer to parent element
		GuiElement * GetParent() { return parentElement; }
		//!Gets the current leftmost coordinate of the element
		//!Considers horizontal alignment, x offset, width, and parent element's GetLeft() / GetWidth() values
		//!\return left coordinate
		int GetLeft();
		//!Gets the current topmost coordinate of the element
		//!Considers vertical alignment, y offset, height, and parent element's GetTop() / GetHeight() values
		//!\return top coordinate
		int GetTop();
		//!Sets the minimum y offset of the element
		//!\param y Y offset
		void SetMinY(int y);
		//!Gets the minimum y offset of the element
		//!\return Minimum Y offset
		int GetMinY() { return ymin; }
		//!Sets the maximum y offset of the element
		//!\param y Y offset
		void SetMaxY(int y);
		//!Gets the maximum y offset of the element
		//!\return Maximum Y offset
		int GetMaxY() { return ymax; }
		//!Sets the minimum x offset of the element
		//!\param x X offset
		void SetMinX(int x);
		//!Gets the minimum x offset of the element
		//!\return Minimum X offset
		int GetMinX() { return xmin; }
		//!Sets the maximum x offset of the element
		//!\param x X offset
		void SetMaxX(int x);
		//!Gets the maximum x offset of the element
		//!\return Maximum X offset
		int GetMaxX() { return xmax; }
		//!Gets the current width of the element. Does not currently consider the scale
		//!\return width
		virtual int GetWidth() { return width; }
		//!Gets the height of the element. Does not currently consider the scale
		//!\return height
		virtual int GetHeight() { return height; }
		//!Sets the size (width/height) of the element
		//!\param w Width of element
		//!\param h Height of element
		void SetSize(int w, int h);
		//!Checks whether or not the element is visible
		//!\return true if visible, false otherwise
		bool IsVisible() { return visible; }
		//!Checks whether or not the element is selectable
		//!\return true if selectable, false otherwise
		bool IsSelectable();
		//!Checks whether or not the element is clickable
		//!\return true if clickable, false otherwise
		bool IsClickable();
		//!Checks whether or not the element is holdable
		//!\return true if holdable, false otherwise
		bool IsHoldable();
		//!Sets whether or not the element is selectable
		//!\param s Selectable
		void SetSelectable(bool s);
		//!Sets whether or not the element is clickable
		//!\param c Clickable
		void SetClickable(bool c);
		//!Sets whether or not the element is holdable
		//!\param c Holdable
		void SetHoldable(bool d);
		//!Gets the element's current state
		//!\return state
		int GetState() { return state; }
		//!Gets the controller channel that last changed the element's state
		//!\return Channel number (0-3, -1 = no channel)
		int GetStateChan() { return stateChan; }
		//!Sets the element's alpha value
		//!\param a alpha value
		void SetAlpha(int a);
		//!Gets the element's alpha value
		//!Considers alpha, alphaDyn, and the parent element's GetAlpha() value
		//!\return alpha
		int GetAlpha();
		//!Gets the element's AngleDyn value
		//!\return alpha
		float GetAngleDyn();
		//!Sets the element's scale
		//!\param s scale (1 is 100%)
		void SetScale(float s);
		//!Gets the element's current scale
		//!Considers scale, scaleDyn, and the parent element's GetScale() value
		virtual float GetScale();
		//!Set a new GuiTrigger for the element
		//!\param t Pointer to GuiTrigger
		void SetTrigger(GuiTrigger * t);
		//!\overload
		//!\param i Index of trigger array to set
		//!\param t Pointer to GuiTrigger
		void SetTrigger(u8 i, GuiTrigger * t);
		//!Remove GuiTrigger for the element
		//!\param i Index of trigger array to set
		void RemoveTrigger(u8 i);
		//!Checks whether rumble was requested by the element
		//!\return true is rumble was requested, false otherwise
		bool Rumble() { return rumble; }
		//!Sets whether or not the element is requesting a rumble event
		//!\param r true if requesting rumble, false if not
		void SetRumble(bool r);
		//!Set an effect for the element
		//!\param e Effect to enable
		//!\param a Amount of the effect (usage varies on effect)
		//!\param t Target amount of the effect (usage varies on effect)
		void SetEffect(int e, int a, int t = 0);
		//!This SetEffect is for EFFECT_GOROUND only
		//!\param e Effect to enable
		//!\param speed is for Circlespeed
		//!\param circles Circleamount in degree ike 180 for 1/2 circle or 720 for 2 circles
		//!\param r Circle Radius in pixel
		//!\param startdegree Degree where to start circling
		//!\param anglespeedset Set the speed of Angle rotating make 1 for same speed as Circlespeed
		//!	   or 0.5 for half the speed of the circlingspeed. Turn Anglecircling off by 0 to this param.
		//!\param center_x x co-ordinate of the center of circle.
		//!\param center_y y co-ordinate of the center of circle.
		void SetEffect(int e, int speed, f32 circles, int r, f32 startdegree, f32 anglespeedset, int center_x,
				int center_y);
		//!Gets the frequency from the above effect
		//!\return element frequency
		float GetFrequency();
		//!Sets an effect to be enabled on wiimote cursor over
		//!\param e Effect to enable
		//!\param a Amount of the effect (usage varies on effect)
		//!\param t Target amount of the effect (usage varies on effect)
		void SetEffectOnOver(int e, int a, int t = 0);
		//!Shortcut to SetEffectOnOver(EFFECT_SCALE, 4, 110)
		void SetEffectGrow();
		//!Stops the current element effect
		void StopEffect();
		//!Gets the current element effects
		//!\return element effects
		int GetEffect() { return effects; }
		//!Gets the current element on over effects
		//!\return element on over effects
		int GetEffectOnOver() { return effectsOver; }
		//!Checks whether the specified coordinates are within the element's boundaries
		//!\param x X coordinate
		//!\param y Y coordinate
		//!\return true if contained within, false otherwise
		bool IsInside(int x, int y);
		//!Sets the element's position
		//!\param x X coordinate
		//!\param y Y coordinate
		void SetPosition(int x, int y, int z = 0);
		//!Sets the element's relative position
		int GetRelLeft();
		int GetRelTop();
		//!Sets the element's setup position
		int GetLeftPos() const { return xoffset; }
		int GetTopPos() const { return yoffset; }
		//!Updates the element's effects (dynamic values)
		//!Called by Draw(), used for animation purposes
		void UpdateEffects();
		//!Sets a function to called after after Update()
		//!Callback function can be used to response to changes in the state of the element, and/or update the element's attributes
		void SetUpdateCallback(UpdateCallback u);
		//!Sets the element's visibility
		//!\param v Visibility (true = visible)
		virtual void SetVisible(bool v);
		//!Sets the element's state
		//!\param s State (STATE_DEFAULT, STATE_SELECTED, STATE_CLICKED, STATE_DISABLED)
		//!\param c Controller channel (0-3, -1 = none)
		virtual void SetState(int s, int c = -1);
		//!Resets the element's state to STATE_DEFAULT
		virtual void ResetState();
		//!Gets whether or not the element is in STATE_SELECTED
		//!\return true if selected, false otherwise
		virtual int GetSelected();
		//!Sets the element's alignment respective to its parent element
		//!\param hor Horizontal alignment (ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER)
		//!\param vert Vertical alignment (ALIGN_TOP, ALIGN_BOTTOM, ALIGN_MIDDLE)
		virtual void SetAlignment(int hor, int vert);
		//!Called constantly to allow the element to respond to the current input data
		//!\param t Pointer to a GuiTrigger, containing the current input data from PAD/WPAD
		virtual void Update(GuiTrigger * t);
		//!Called constantly to redraw the element
		virtual void Draw();
		virtual void DrawTooltip();
	protected:
		void LockElement();
		void UnlockElement();
		//	  static mutex_t mutex;
		static mutex_t _lock_mutex;
		lwp_t _lock_thread;
		u16 _lock_count;
		lwpq_t _lock_queue;
		friend class SimpleLock;

		//int position2; //! B Scrollbariable
		bool visible; //!< Visibility of the element. If false, Draw() is skipped
		int width; //!< Element width
		int height; //!< Element height
		int xoffset; //!< Element X offset
		int yoffset; //!< Element Y offset
		int zoffset; //!< Element Z offset
		int ymin; //!< Element's min Y offset allowed
		int ymax; //!< Element's max Y offset allowed
		int xmin; //!< Element's min X offset allowed
		int xmax; //!< Element's max X offset allowed
		int xoffsetDyn; //!< Element X offset, dynamic (added to xoffset value for animation effects)
		int yoffsetDyn; //!< Element Y offset, dynamic (added to yoffset value for animation effects)
		int temp_xoffset; //!< Element Temp X offset
		int temp_yoffset; //!< Element Temp Y offset
		f32 degree; //!< Degree where to start for EFFECT_GOROUND enter it in ° like 60°
		f32 frequency; //!< Speed for EFFECT_GOROUND || can also be negative for other direction
		int Radius; //!< The radius in which the Element goes round for EFFECT_GOROUND
		f32 circleamount; //!< Circleamount for the EFFECT_GOROUND effect
		f32 xoffsetDynFloat; //!< Integer sucks float is need by some parts
		f32 yoffsetDynFloat; //!< Integer sucks float is need by some parts
		int changervar; //!< Changervariable for some stuff
		int alpha; //!< Element alpha value (0-255)
		f32 scale; //!< Element scale (1 = 100%)
		f32 angleDyn; //!< AngleDyn for EFFECT_GOROUND
		f32 anglespeed; //!<Anglespeedvariable for EFFECT_GOROUND
		int alphaDyn; //!< Element alpha, dynamic (multiplied by alpha value for blending/fading effects)
		f32 scaleDyn; //!< Element scale, dynamic (multiplied by alpha value for blending/fading effects)
		bool rumble; //!< Wiimote rumble (on/off) - set to on when this element requests a rumble event
		int effects; //!< Currently enabled effect(s). 0 when no effects are enabled
		int effectAmount; //!< Effect amount. Used by different effects for different purposes
		int effectTarget; //!< Effect target amount. Used by different effects for different purposes
		int effectsOver; //!< Effects to enable when wiimote cursor is over this element. Copied to effects variable on over event
		int effectAmountOver; //!< EffectAmount to set when wiimote cursor is over this element
		int effectTargetOver; //!< EffectTarget to set when wiimote cursor is over this element
		int alignmentHor; //!< Horizontal element alignment, respective to parent element (LEFT, RIGHT, CENTRE)
		int alignmentVert; //!< Horizontal element alignment, respective to parent element (TOP, BOTTOM, MIDDLE)
		int state; //!< Element state (DEFAULT, SELECTED, CLICKED, DISABLED)
		int stateChan; //!< Which controller channel is responsible for the last change in state
		bool selectable; //!< Whether or not this element selectable (can change to SELECTED state)
		bool clickable; //!< Whether or not this element is clickable (can change to CLICKED state)
		bool holdable; //!< Whether or not this element is holdable (can change to HELD state)
		GuiTrigger * trigger[6]; //!< GuiTriggers (input actions) that this element responds to
		GuiElement * parentElement; //!< Parent element
		UpdateCallback updateCB; //!< Callback function to call when this element is updated
};
class SimpleLock
{
	public:
		SimpleLock(GuiElement *e);
		~SimpleLock();
	private:
		GuiElement *element;
};
#define LOCK(e) SimpleLock LOCK(e)

//!Allows GuiElements to be grouped together into a "window"
class GuiWindow: public GuiElement
{
	public:
		//!Constructor
		GuiWindow();
		//!\overload
		//!\param w Width of window
		//!\param h Height of window
		GuiWindow(int w, int h);
		//!Destructor
		virtual ~GuiWindow();
		//!Appends a GuiElement to the GuiWindow
		//!\param e The GuiElement to append. If it is already in the GuiWindow, it is removed first
		void Append(GuiElement* e);
		//!Inserts a GuiElement into the GuiWindow at the specified index
		//!\param e The GuiElement to insert. If it is already in the GuiWindow, it is removed first
		//!\param i Index in which to insert the element
		void Insert(GuiElement* e, u32 i);
		//!Removes the specified GuiElement from the GuiWindow
		//!\param e GuiElement to be removed
		void Remove(GuiElement* e);
		//!Removes all GuiElements
		void RemoveAll();
		//!Returns the GuiElement at the specified index
		//!\param index The index of the element
		//!\return A pointer to the element at the index, NULL on error (eg: out of bounds)
		GuiElement* GetGuiElementAt(u32 index) const;
		//!Returns the size of the list of elements
		//!\return The size of the current element list
		u32 GetSize();
		//!Sets the visibility of the window
		//!\param v visibility (true = visible)
		void SetVisible(bool v);
		//!Resets the window's state to STATE_DEFAULT
		void ResetState();
		//!Sets the window's state
		//!\param s State
		void SetState(int s);
		//!Gets the index of the GuiElement inside the window that is currently selected
		//!\return index of selected GuiElement
		int GetSelected();
		//!Moves the selected element to the element to the left or right
		//!\param d Direction to move (-1 = left, 1 = right)
		void MoveSelectionHor(int d);
		//!Moves the selected element to the element above or below
		//!\param d Direction to move (-1 = up, 1 = down)
		void MoveSelectionVert(int d);
		//!Allow dim of screen on disable or not
		void SetAllowDim(bool d) { allowDim = d; }
		void SetDimScreen(bool d) { forceDim = d; }
		//!Draws all the elements in this GuiWindow
		void Draw();
		void DrawTooltip();
		//!Updates the window and all elements contains within
		//!Allows the GuiWindow and all elements to respond to the input data specified
		//!\param t Pointer to a GuiTrigger, containing the current input data from PAD/WPAD
		void Update(GuiTrigger * t);
	protected:
		bool forceDim;
		bool allowDim;
		std::vector<GuiElement*> _elements; //!< Contains all elements within the GuiWindow
};

//!Display, manage, and manipulate images in the GUI
class GuiImage: public GuiElement
{
	public:
		//!Constructor
		GuiImage();
		//!\overload
		//!\param img Pointer to GuiImageData element
		GuiImage(GuiImageData * img);
		//!\overload
		//!Sets up a new image from the image data specified
		//!\param img
		//!\param w Image width
		//!\param h Image height
		GuiImage(u8 * img, int w, int h);
		//!\overload
		//!Creates an image filled with the specified color
		//!\param w Image width
		//!\param h Image height
		//!\param c Image color
		GuiImage(int w, int h, GXColor c);
		//! Copy Constructor
		GuiImage(GuiImage &srcimage);
		GuiImage(GuiImage *srcimage);
		//! = operator for copying images
		GuiImage &operator=(GuiImage &srcimage);
		//!Destructor
		virtual ~GuiImage();
		//!Sets the image rotation angle for drawing
		//!\param a Angle (in degrees)
		void SetAngle(float a);
		//!Gets the image rotation angle for drawing
		float GetAngle();
		//!Sets the number of times to draw the image horizontally
		//!\param t Number of times to draw the image
		void SetTileHorizontal(int t);
		void SetTileVertical(int t);
		// true set horizontal scale to 0.8 //added
		void SetWidescreen(bool w);
		//!Constantly called to draw the image
		void Draw();
		//!Gets the image data
		//!\return pointer to image data
		u8 * GetImage();
		//!Sets up a new image using the GuiImageData object specified
		//!\param img Pointer to GuiImageData object
		void SetImage(GuiImageData * img);
		//!\overload
		//!\param img Pointer to image data
		//!\param w Width
		//!\param h Height
		void SetImage(u8 * img, int w, int h);
		//!Gets the pixel color at the specified coordinates of the image
		//!\param x X coordinate
		//!\param y Y coordinate
		GXColor GetPixel(int x, int y);
		//!Sets the pixel color at the specified coordinates of the image
		//!\param x X coordinate
		//!\param y Y coordinate
		//!\param color Pixel color
		void SetPixel(int x, int y, GXColor color);
		//!Sets the image to grayscale
		void SetGrayscale(void);
		//!Set/disable the use of parentelement angle (default true)
		void SetParentAngle(bool a);
		//!Directly modifies the image data to create a color-striped effect
		//!Alters the RGB values by the specified amount
		//!\param s Amount to increment/decrement the RGB values in the image
		void ColorStripe(int s);
		//!Sets a stripe effect on the image, overlaying alpha blended rectangles
		//!Does not alter the image data
		//!\param s Alpha amount to draw over the image
		void SetStripe(int s);
		s32 z;
		void SetSkew(int XX1, int YY1, int XX2, int YY2, int XX3, int YY3, int XX4, int YY4);
		void SetSkew(int *skew /* int skew[8] */);
		int xx1;
		int yy1;
		int xx2;
		int yy2;
		int xx3;
		int yy3;
		int xx4;
		int yy4;
		int rxx1;
		int ryy1;
		int rxx2;
		int ryy2;
		int rxx3;
		int ryy3;
		int rxx4;
		int ryy4;
	protected:
		int imgType; //!< Type of image data (IMAGE_TEXTURE, IMAGE_COLOR, IMAGE_DATA)
		u8 * image; //!< Poiner to image data. May be shared with GuiImageData data
		f32 imageangle; //!< Angle to draw the image
		int tileHorizontal; //!< Number of times to draw (tile) the image horizontally
		int tileVertical; //!< Number of times to draw (tile) the image vertically
		u8 stripe; //!< Alpha value (0-255) to apply a stripe effect to the texture
		short widescreen; //added
		bool parentangle;
};
//!Display, manage, and manipulate text in the GUI
class GuiText: public GuiElement
{
	public:
		//!Constructor
		//!\param t Text
		//!\param s Font size
		//!\param c Font color
		GuiText(const char * t, int s, GXColor c);
		//!\overload
		//!\param t Text
		//!\param s Font size
		//!\param c Font color
		GuiText(const wchar_t * t, int s, GXColor c);
		//!\overload
		//!\Assumes SetPresets() has been called to setup preferred text attributes
		//!\param t Text
		GuiText(const char * t);
		//!Destructor
		virtual ~GuiText();
		//!Sets the text of the GuiText element
		//!\param t Text
		virtual void SetText(const char * t);
		virtual void SetText(const wchar_t * t);
		virtual void SetTextf(const char *format, ...) __attribute__( ( format( printf, 2, 3 ) ) );
		//!Sets up preset values to be used by GuiText(t)
		//!Useful when printing multiple text elements, all with the same attributes set
		//!\param sz Font size
		//!\param c Font color
		//!\param w Maximum width of texture image (for text wrapping)
		//!\param wrap Wrapmode when w>0
		//!\param s Font style
		//!\param h Text alignment (horizontal)
		//!\param v Text alignment (vertical)
		static void SetPresets(int sz, GXColor c, int w, u16 s, int h, int v);
		//!Sets the font size
		//!\param s Font size
		void SetFontSize(int s);
		//!Sets the maximum width of the drawn texture image
		//!If the text exceeds this, it is wrapped to the next line
		//!\param w Maximum width
		//!\param m WrapMode
		void SetMaxWidth(int w = 0, int m = WRAP);
		//!Sets the font color
		//!\param c Font color
		void SetColor(GXColor c);
		//!Sets the FreeTypeGX style attributes
		//!\param s Style attributes
		//!\param m Style-Mask attributes
		void SetStyle(u16 s);
		//!Sets the text alignment
		//!\param hor Horizontal alignment (ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER)
		//!\param vert Vertical alignment (ALIGN_TOP, ALIGN_BOTTOM, ALIGN_MIDDLE)
		void SetAlignment(int hor, int vert);
		//!Set PassChar
		void SetPassChar(wchar_t p);
		//!Sets the font
		//!\param f Font
		void SetFont(FreeTypeGX *f);
		//!Get the original text as char
		virtual const wchar_t * GetText();
		//!Overload for GetWidth()
		int GetWidth() { return GetTextWidth(); }
		//!Get the Horizontal Size of Text
		int GetTextWidth();
		int GetTextWidth(int ind);
		//!Get the max textwidth
		int GetTextMaxWidth();
		//!Gets the total line number
		virtual int GetLinesCount() { return 1; }
		//!Get fontsize
		int GetFontSize() { return size; }
		//!Set max lines to draw
		void SetLinesToDraw(int l);
		void SetWidescreen(bool b) { widescreen = b; }
		//!Get current Textline (for position calculation)
		const wchar_t * GetDynText(int ind = 0);
		virtual const wchar_t * GetTextLine(int ind) { return GetDynText(ind); }
		//!Change the font
		//!\param font bufferblock
		//!\param font filesize
		bool SetFont(const u8 *font, const u32 filesize);
		//!Constantly called to draw the text
		void Draw();
	protected:
		//!Clear the dynamic text
		void ClearDynamicText();
		//!Create a dynamic dotted text if the text is too long
		void MakeDottedText();
		//!Scroll the text once
		void ScrollText();
		//!Wrap the text to several lines
		void WrapText();

		wchar_t *text;
		std::vector<wchar_t *> textDyn;
		int wrapMode; //!< Wrapping toggle
		int textScrollPos; //!< Current starting index of text string for scrolling
		int textScrollInitialDelay; //!< Delay to wait before starting to scroll
		int textScrollDelay; //!< Scrolling speed
		int size; //!< Font size
		int maxWidth; //!< Maximum width of the generated text object (for text wrapping)
		u16 style; //!< FreeTypeGX style attributes
		GXColor color; //!< Font color
		FreeTypeGX *font;
		int textWidth;
		int currentSize;
		int linestodraw;
		wchar_t passChar;
		bool widescreen;
};

//!Display, manage, and manipulate tooltips in the GUI.
class GuiTooltip: public GuiElement
{
	public:
		//!Constructor
		//!\param t Text
		GuiTooltip(const char *t, int Alpha = 255);
		//!Destructor
		virtual ~GuiTooltip();
		//!Gets the element's current scale
		//!Considers scale, scaleDyn, and the parent element's GetScale() value
		float GetScale();
		//!Sets the text of the GuiTooltip element
		//!\param t Text
		void SetText(const char * t);
		void SetWidescreen(bool w); // timely a dummy
		//!Constantly called to draw the GuiButton
		void Draw();
	protected:
		GuiImageData * tooltipLeft;
		GuiImageData * tooltipTile;
		GuiImageData * tooltipRight;
		GuiImage * leftImage; //!< Tooltip left-image
		GuiImage * tileImage; //!< Tooltip tile-image
		GuiImage * rightImage; //!< Tooltip right-image
		GuiText *text;
};

//!Display, manage, and manipulate buttons in the GUI. Buttons can have images, icons, text, and sound set (all of which are optional)
class GuiButton: public GuiElement
{
	public:
		//!Constructor
		//!\param w Width
		//!\param h Height
		GuiButton(int w, int h);
		//!\param img is the button GuiImage.  it uses the height & width of this image for the button
		//!\param imgOver is the button's over GuiImage
		//!\param hor is horizontal alingment of the button
		//!\param vert is verticle alignment of the button
		//!\param x is xposition of the button
		//!\param y is yposition of the button
		//!\param trig is a GuiTrigger to assign to this button
		//!\param sndOver is a GuiSound used for soundOnOver for this button
		//!\param sndClick is a GuiSound used for clickSound of this button
		//!\param grow sets effect grow for this button.  1 for yes ;0 for no
		GuiButton(GuiImage* img, GuiImage* imgOver, int hor, int vert, int x, int y, GuiTrigger* trig,
				GuiSound* sndOver, GuiSound* sndClick, u8 grow);
		//!\param same as all the parameters for the above button plus the following
		//!\param tt is a GuiTooltip assigned to this button
		//!\param ttx and tty are the xPOS and yPOS for this tooltip in relationship to the button
		//!\param h_align and v_align are horizontal and verticle alignment for the tooltip in relationship to the button
		GuiButton(GuiImage* img, GuiImage* imgOver, int hor, int vert, int x, int y, GuiTrigger* trig,
				GuiSound* sndOver, GuiSound* sndClick, u8 grow, GuiTooltip* tt, int ttx, int tty, int h_align,
				int v_align);
		//!Destructor
		virtual ~GuiButton();
		//!Sets the button's image
		//!\param i Pointer to GuiImage object
		void SetImage(GuiImage* i);
		//!Sets the button's image on over
		//!\param i Pointer to GuiImage object
		void SetImageOver(GuiImage* i);
		//!Sets the button's image on hold
		//!\param i Pointer to GuiImage object
		void SetAngle(float a);
		void SetImageHold(GuiImage* i);
		//!Sets the button's image on click
		//!\param i Pointer to GuiImage object
		void SetImageClick(GuiImage* i);
		//!Sets the button's icon
		//!\param i Pointer to GuiImage object
		void SetIcon(GuiImage* i);
		//!Sets the button's icon on over
		//!\param i Pointer to GuiImage object
		void SetIconOver(GuiImage* i);
		//!Sets the button's icon on hold
		//!\param i Pointer to GuiImage object
		void SetIconHold(GuiImage* i);
		//!Sets the button's icon on click
		//!\param i Pointer to GuiImage object
		void SetIconClick(GuiImage* i);
		//!Sets the button's label
		//!\param t Pointer to GuiText object
		//!\param n Index of label to set (optional, default is 0)
		void SetLabel(GuiText* t, int n = 0);
		//!Sets the button's label on over (eg: different colored text)
		//!\param t Pointer to GuiText object
		//!\param n Index of label to set (optional, default is 0)
		void SetLabelOver(GuiText* t, int n = 0);
		//!Sets the button's label on hold
		//!\param t Pointer to GuiText object
		//!\param n Index of label to set (optional, default is 0)
		void SetLabelHold(GuiText* t, int n = 0);
		//!Sets the button's label on click
		//!\param t Pointer to GuiText object
		//!\param n Index of label to set (optional, default is 0)
		void SetLabelClick(GuiText* t, int n = 0);
		//!Sets the sound to play on over
		//!\param s Pointer to GuiSound object
		void SetSoundOver(GuiSound * s);
		//!Sets the sound to play on hold
		//!\param s Pointer to GuiSound object
		void SetSoundHold(GuiSound * s);
		//!Sets the sound to play on click
		//!\param s Pointer to GuiSound object
		void SetSoundClick(GuiSound * s);
		//!\param reset the soundover to NULL
		void RemoveSoundOver();
		//!\param reset the soundclick to NULL
		void RemoveSoundClick();
		//!Constantly called to draw the GuiButtons ToolTip
		//!Sets the button's Tooltip on over
		//!\param tt Pointer to GuiElement object, x & y Positioning, h & v Align
		void SetToolTip(GuiTooltip* tt, int x, int y, int h = ALIGN_RIGHT, int v = ALIGN_TOP);

		void RemoveToolTip();
		//!Constantly called to draw the GuiButton
		void Draw();
		void DrawTooltip();
		//!Constantly called to allow the GuiButton to respond to updated input data
		//!\param t Pointer to a GuiTrigger, containing the current input data from PAD/WPAD
		void Update(GuiTrigger * t);
		//!Deactivate/Activate pointing on Games while B scrolling
		void ScrollIsOn(int f);
		void SetSkew(int XX1, int YY1, int XX2, int YY2, int XX3, int YY3, int XX4, int YY4);
		void SetSkew(int *skew /* int skew[8] */);
		virtual void SetState(int s, int c = -1);
		sigslot::signal3<GuiButton *, int, const POINT&> Clicked;
		sigslot::signal3<GuiButton *, int, const POINT&> Held;
	protected:
		GuiImage * image; //!< Button image (default)
		GuiImage * imageOver; //!< Button image for STATE_SELECTED
		GuiImage * imageHold; //!< Button image for STATE_HELD
		GuiImage * imageClick; //!< Button image for STATE_CLICKED
		GuiImage * icon; //!< Button icon (drawn after button image)
		GuiImage * iconOver; //!< Button icon for STATE_SELECTED
		GuiImage * iconHold; //!< Button icon for STATE_HELD
		GuiImage * iconClick; //!< Button icon for STATE_CLICKED
		GuiTooltip *toolTip;
		Timer ToolTipDelay;
		bool bOldTooltipVisible;
		GuiText * label[3]; //!< Label(s) to display (default)
		GuiText * labelOver[3]; //!< Label(s) to display for STATE_SELECTED
		GuiText * labelHold[3]; //!< Label(s) to display for STATE_HELD
		GuiText * labelClick[3]; //!< Label(s) to display for STATE_CLICKED
		GuiSound * soundOver; //!< Sound to play for STATE_SELECTED
		GuiSound * soundHold; //!< Sound to play for STATE_HELD
		GuiSound * soundClick; //!< Sound to play for STATE_CLICKED
};

typedef struct _keytype
{
		char ch, chShift, chalt, chalt2;
} Key;

//!On-screen keyboard
class GuiKeyboard: public GuiWindow
{
	public:
		GuiKeyboard(char * t, u32 m, int min, int lang);
		virtual ~GuiKeyboard();
		void SetVisibleText(bool v) { textVisible = v; }
		const char *GetText() { return kbtextstr; }
		void Update(GuiTrigger * t);
	protected:
		void SetDisplayText(const char *text);

		bool textVisible;
		char kbtextstr[256];
		u32 kbtextmaxlen;
		Key keys[4][11];
		int shift;
		int caps;
		int alt;
		int alt2;
		u16 min;
		GuiText * kbText;
		GuiImage * keyTextboxImg;
		GuiText * keyCapsText;
		GuiImage * keyCapsImg;
		GuiImage * keyCapsOverImg;
		GuiButton * keyCaps;
		GuiText * keyAltText;
		GuiImage * keyAltImg;
		GuiImage * keyAltOverImg;
		GuiButton * keyAlt;
		GuiText * keyAlt2Text;
		GuiImage * keyAlt2Img;
		GuiImage * keyAlt2OverImg;
		GuiButton * keyAlt2;
		GuiText * keyShiftText;
		GuiImage * keyShiftImg;
		GuiImage * keyShiftOverImg;
		GuiButton * keyShift;
		GuiText * keyBackText;
		GuiImage * keyBackImg;
		GuiImage * keyBackOverImg;
		GuiButton * keyBack;
		GuiText * keyClearText;
		GuiImage * keyClearImg;
		GuiImage * keyClearOverImg;
		GuiButton * keyClear;
		GuiImage * keySpaceImg;
		GuiImage * keySpaceOverImg;
		GuiButton * keySpace;
		GuiButton * keyBtn[4][11];
		GuiImage * keyImg[4][11];
		GuiImage * keyImgOver[4][11];
		GuiText * keyTxt[4][11];
		GuiImageData * keyTextbox;
		GuiImageData * key;
		GuiImageData * keyOver;
		GuiImageData * keyMedium;
		GuiImageData * keyLarge;
		GuiTrigger * trigA;
		GuiTrigger * trigB;
};

#endif
