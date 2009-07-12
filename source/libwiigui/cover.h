#ifndef COVER_H
#define COVER_H

#include "gui.h"

//!Display, manage, and manipulate images in the GUI
class CoverImage : public GuiElement
{
	public:
		//!Constructor
		CoverImage(const char * imgPath, const u8 * buffer);
		//!\overload
		//!Sets up a new image from the image data specified
		//!\param img
		//!\param w Image width
		//!\param h Image height
		CoverImage(u8 * img, int w, int h);
		CoverImage(GuiImageData *img);
		//!\overload
		//!Creates an image filled with the specified color
		//!\param w Image width
		//!\param h Image height
		//!\param c Image color
		CoverImage(int w, int h, GXColor c);
		//! Copy Constructor
		CoverImage(CoverImage &srcimage);
		CoverImage(CoverImage *srcimage);
		//! = operator for copying images
		CoverImage &operator=(CoverImage &srcimage);
		//!Destructor
		~CoverImage();
		//!Sets the image rotation angle for drawing
		//!\param a Angle (in degrees)
		void SetAngle(float a);
		//!Gets the image rotation angle for drawing
		float GetAngle();
		//!Sets the number of times to draw the image horizontally
		//!\param t Number of times to draw the image
		void SetTile(int t);
		// true set horizontal scale to 0.8 //added
		void SetWidescreen(bool w);
		//!Constantly called to draw the image
		void Draw();
		//!Gets the image data
		//!\return pointer to image data
		u8 * GetImage();
		//!Sets up a new image using the CoverImageData object specified
		//!\param img Pointer to CoverImageData object
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
		//!Directly modifies the image data to create a color-striped effect
		//!Alters the RGB values by the specified amount
		//!\param s Amount to increment/decrement the RGB values in the image
		void ColorStripe(int s);
		//!Sets a stripe effect on the image, overlaying alpha blended rectangles
		//!Does not alter the image data
		//!\param s Alpha amount to draw over the image
		void SetStripe(int s);
		s32 z;
		void SetSkew(int XX1, int YY1,int XX2, int YY2,int XX3, int YY3,int XX4, int YY4);

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
		u8 * image; //!< Poiner to image data. May be shared with CoverImageData data
		f32 imageangle; //!< Angle to draw the image
		int tile; //!< Number of times to draw (tile) the image horizontally
		int stripe; //!< Alpha value (0-255) to apply a stripe effect to the texture
		short widescreen; //added
};

#endif
