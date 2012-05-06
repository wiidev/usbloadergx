/*
 * FreeTypeGX is a wrapper class for libFreeType which renders a compiled
 * FreeType parsable font into a GX texture for Wii homebrew development.
 * Copyright (C) 2008 Armin Tamzarian
 * Modified by Dimok, 2010
 *
 * This file is part of FreeTypeGX.
 *
 * FreeTypeGX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FreeTypeGX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with FreeTypeGX.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FreeTypeGX.h"
#include "memory/mem2.h"

using namespace std;

#define ALIGN8(x) (((x) + 7) & ~7)

/**
 * Convert a short char string to a wide char string.
 *
 * This routine converts a supplied short character string into a wide character string.
 * Note that it is the user's responsibility to clear the returned buffer once it is no longer needed.
 *
 * @param strChar   Character string to be converted.
 * @return Wide character representation of supplied character string.
 */

wchar_t* charToWideChar(const char* strChar)
{
	if (!strChar) return NULL;

	wchar_t *strWChar = new (std::nothrow) wchar_t[strlen(strChar) + 1];
	if (!strWChar) return NULL;

	int bt = mbstowcs(strWChar, strChar, strlen(strChar));
	if (bt > 0)
	{
		strWChar[bt] = 0;
		return strWChar;
	}

	wchar_t *tempDest = strWChar;
	while ((*tempDest++ = *strChar++))
		;

	return strWChar;
}

/**
 * Default constructor for the FreeTypeGX class for WiiXplorer.
 */
FreeTypeGX::FreeTypeGX(const uint8_t* fontBuffer, FT_Long bufferSize, bool lastFace)
{
	int faceIndex = 0;
	ftPointSize = 0;

	FT_Init_FreeType(&ftLibrary);
	if(lastFace)
	{
		FT_New_Memory_Face(ftLibrary, (FT_Byte *)fontBuffer, bufferSize, -1, &ftFace);
		faceIndex = ftFace->num_faces - 1; // Use the last face
		FT_Done_Face(ftFace);
		ftFace = NULL;
	}
	FT_New_Memory_Face(ftLibrary, (FT_Byte *) fontBuffer, bufferSize, faceIndex, &ftFace);

	setVertexFormat(GX_VTXFMT1);
	ftKerningEnabled = false;//FT_HAS_KERNING(ftFace);
}

/**
 * Default destructor for the FreeTypeGX class.
 */
FreeTypeGX::~FreeTypeGX()
{
	unloadFont();
	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLibrary);
}

/**
 * Setup the vertex attribute formats for the glyph textures.
 *
 * This function sets up the vertex format for the glyph texture on the specified vertex format index.
 * Note that this function should not need to be called except if the vertex formats are cleared or the specified
 * vertex format index is modified.
 *
 * @param vertexIndex   Vertex format index (GX_VTXFMT*) of the glyph textures as defined by the libogc gx.h header file.
 */
void FreeTypeGX::setVertexFormat(uint8_t vertexInd)
{
	vertexIndex = vertexInd;
	GX_SetVtxAttrFmt(vertexIndex, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
	GX_SetVtxAttrFmt(vertexIndex, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetVtxAttrFmt(vertexIndex, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
}

/**
 * Clears all loaded font glyph data.
 *
 * This routine clears all members of the font map structure and frees all allocated memory back to the system.
 */
void FreeTypeGX::unloadFont()
{
	if (this->fontData.size() == 0) return;

	map<int16_t, map<wchar_t, ftgxCharData> >::iterator itr;
	map<wchar_t, ftgxCharData>::iterator itr2;

	for (itr = fontData.begin(); itr != fontData.end(); itr++)
	{
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++)
			MEM2_free(itr2->second.glyphDataTexture);

		itr->second.clear();
	}

	fontData.clear();
	ftgxAlign.clear();
}

/**
 * Caches the given font glyph in the instance font texture buffer.
 *
 * This routine renders and stores the requested glyph's bitmap and relevant information into its own quickly addressible
 * structure within an instance-specific map.
 *
 * @param charCode  The requested glyph's character code.
 * @return A pointer to the allocated font structure.
 */
ftgxCharData * FreeTypeGX::cacheGlyphData(wchar_t charCode, int16_t pixelSize)
{
	map<int16_t, map<wchar_t, ftgxCharData> >::iterator itr = fontData.find(pixelSize);
	if (itr != fontData.end())
	{
		map<wchar_t, ftgxCharData>::iterator itr2 = itr->second.find(charCode);
		if (itr2 != itr->second.end())
		{
			return &itr2->second;
		}
	}

	FT_UInt gIndex;
	uint16_t textureWidth = 0, textureHeight = 0;

	if (ftPointSize != pixelSize)
	{
		ftPointSize = pixelSize;
		FT_Set_Pixel_Sizes(ftFace, 0, ftPointSize);

		//!Cache ascender and decender as well
		map<int16_t, ftgxDataOffset>::iterator itrAlign = ftgxAlign.find(ftPointSize);
		if (itrAlign == ftgxAlign.end())
		{
			ftgxAlign[ftPointSize].ascender = (int16_t) ftFace->size->metrics.ascender >> 6;
			ftgxAlign[ftPointSize].descender = (int16_t) ftFace->size->metrics.descender >> 6;
			ftgxAlign[ftPointSize].max = 0;
			ftgxAlign[ftPointSize].min = 0;
		}
	}

	gIndex = FT_Get_Char_Index(ftFace, (FT_ULong) charCode);
	if (gIndex != 0 && FT_Load_Glyph(ftFace, gIndex, FT_LOAD_DEFAULT | FT_LOAD_RENDER) == 0)
	{
		if (ftFace->glyph->format == FT_GLYPH_FORMAT_BITMAP)
		{
			FT_Bitmap *glyphBitmap = &ftFace->glyph->bitmap;

			textureWidth = ALIGN8(glyphBitmap->width);
			textureHeight = ALIGN8(glyphBitmap->rows);
			if(textureWidth == 0)
				textureWidth = 8;
			if(textureHeight == 0)
				textureHeight = 8;

			fontData[pixelSize][charCode].renderOffsetX = (int16_t) ftFace->glyph->bitmap_left;
			fontData[pixelSize][charCode].glyphAdvanceX = (uint16_t) (ftFace->glyph->advance.x >> 6);
			fontData[pixelSize][charCode].glyphIndex = (uint32_t) gIndex;
			fontData[pixelSize][charCode].textureWidth = (uint16_t) textureWidth;
			fontData[pixelSize][charCode].textureHeight = (uint16_t) textureHeight;
			fontData[pixelSize][charCode].renderOffsetY = (int16_t) ftFace->glyph->bitmap_top;
			fontData[pixelSize][charCode].renderOffsetMax = (int16_t) ftFace->glyph->bitmap_top;
			fontData[pixelSize][charCode].renderOffsetMin = (int16_t) glyphBitmap->rows - ftFace->glyph->bitmap_top;
			fontData[pixelSize][charCode].glyphDataTexture = NULL;

			loadGlyphData(glyphBitmap, &fontData[pixelSize][charCode]);

			return &fontData[pixelSize][charCode];
		}
	}
	return NULL;
}

/**
 * Locates each character in this wrapper's configured font face and proccess them.
 *
 * This routine locates each character in the configured font face and renders the glyph's bitmap.
 * Each bitmap and relevant information is loaded into its own quickly addressible structure within an instance-specific map.
 */
uint16_t FreeTypeGX::cacheGlyphDataComplete(int16_t pixelSize)
{
	uint32_t i = 0;
	FT_UInt gIndex;

	FT_ULong charCode = FT_Get_First_Char(ftFace, &gIndex);
	while (gIndex != 0)
	{
		if (cacheGlyphData(charCode, pixelSize) != NULL) ++i;
		charCode = FT_Get_Next_Char(ftFace, charCode, &gIndex);
	}
	return (uint16_t) (i);
}

/**
 * Loads the rendered bitmap into the relevant structure's data buffer.
 *
 * This routine does a simple byte-wise copy of the glyph's rendered 8-bit grayscale bitmap into the structure's buffer.
 * Each byte is converted from the bitmap's intensity value into the a uint32_t RGBA value.
 *
 * @param bmp   A pointer to the most recently rendered glyph's bitmap.
 * @param charData  A pointer to an allocated ftgxCharData structure whose data represent that of the last rendered glyph.
 */

void FreeTypeGX::loadGlyphData(FT_Bitmap *bmp, ftgxCharData *charData)
{
	int glyphSize = (charData->textureWidth * charData->textureHeight) >> 1;

	uint8_t *glyphData = (uint8_t *) MEM2_alloc(glyphSize);
	memset(glyphData, 0x00, glyphSize);

	uint8_t *src = (uint8_t *)bmp->buffer;
	uint8_t *dst = glyphData;
	int32_t pos, x1, y1, x, y;

	for(y1 = 0; y1 < bmp->rows; y1 += 8)
	{
		for(x1 = 0; x1 < bmp->width; x1 += 8)
		{
			for(y = y1; y < (y1 + 8); y++)
			{
				for(x = x1; x < (x1 + 8); x += 2, dst++)
				{
					if(x >= bmp->width || y >= bmp->rows)
						continue;

					pos = y * bmp->width + x;
					*dst = (src[pos] & 0xF0);

					if(x+1 < bmp->width)
						*dst |= (src[pos + 1] >> 4);
				}
			}
		}
	}

	DCFlushRange(glyphData, glyphSize);
	charData->glyphDataTexture = glyphData;
}

/**
 * Determines the x offset of the rendered string.
 *
 * This routine calculates the x offset of the rendered string based off of a supplied positional format parameter.
 *
 * @param width Current pixel width of the string.
 * @param format	Positional format of the string.
 */
int16_t FreeTypeGX::getStyleOffsetWidth(uint16_t width, uint16_t format)
{
	if (format & FTGX_JUSTIFY_LEFT)
		return 0;
	else if (format & FTGX_JUSTIFY_CENTER)
		return -(width >> 1);
	else if (format & FTGX_JUSTIFY_RIGHT) return -width;
	return 0;
}

/**
 * Determines the y offset of the rendered string.
 *
 * This routine calculates the y offset of the rendered string based off of a supplied positional format parameter.
 *
 * @param offset	Current pixel offset data of the string.
 * @param format	Positional format of the string.
 */
int16_t FreeTypeGX::getStyleOffsetHeight(int16_t format, uint16_t pixelSize)
{
	map<int16_t, ftgxDataOffset>::iterator itrAlign = ftgxAlign.find(pixelSize);
	if (itrAlign == ftgxAlign.end()) return 0;

	switch (format & FTGX_ALIGN_MASK)
	{
		case FTGX_ALIGN_TOP:
			return itrAlign->second.ascender;

		case FTGX_ALIGN_MIDDLE:
		default:
			return (itrAlign->second.ascender + itrAlign->second.descender + 1) >> 1;

		case FTGX_ALIGN_BOTTOM:
			return itrAlign->second.descender;

		case FTGX_ALIGN_BASELINE:
			return 0;

		case FTGX_ALIGN_GLYPH_TOP:
			return itrAlign->second.max;

		case FTGX_ALIGN_GLYPH_MIDDLE:
			return (itrAlign->second.max + itrAlign->second.min + 1) >> 1;

		case FTGX_ALIGN_GLYPH_BOTTOM:
			return itrAlign->second.min;
	}
	return 0;
}

/**
 * Processes the supplied text string and prints the results at the specified coordinates.
 *
 * This routine processes each character of the supplied text string, loads the relevant preprocessed bitmap buffer,
 * a texture from said buffer, and loads the resultant texture into the EFB.
 *
 * @param x Screen X coordinate at which to output the text.
 * @param y Screen Y coordinate at which to output the text. Note that this value corresponds to the text string origin and not the top or bottom of the glyphs.
 * @param text  NULL terminated string to output.
 * @param color Optional color to apply to the text characters. If not specified default value is ftgxWhite: (GXColor){0xff, 0xff, 0xff, 0xff}
 * @param textStyle Flags which specify any styling which should be applied to the rendered string.
 * @return The number of characters printed.
 */

uint16_t FreeTypeGX::drawText(int16_t x, int16_t y, int16_t z, const wchar_t *text, int16_t pixelSize, GXColor color,
		uint16_t textStyle, uint16_t textWidth, uint16_t widthLimit)
{
	if (!text) return 0;

	uint16_t fullTextWidth = textWidth > 0 ? textWidth : getWidth(text, pixelSize);
	uint16_t x_pos = x, printed = 0;
	uint16_t x_offset = 0, y_offset = 0;
	GXTexObj glyphTexture;
	FT_Vector pairDelta;

	if (textStyle & FTGX_JUSTIFY_MASK)
	{
		x_offset = getStyleOffsetWidth(fullTextWidth, textStyle);
	}
	if (textStyle & FTGX_ALIGN_MASK)
	{
		y_offset = getStyleOffsetHeight(textStyle, pixelSize);
	}

	int i = 0;

	while (text[i])
	{
		if (widthLimit > 0 && (x_pos - x) > widthLimit) break;

		ftgxCharData* glyphData = cacheGlyphData(text[i], pixelSize);

		if (glyphData != NULL)
		{
			if (ftKerningEnabled && i > 0)
			{
				FT_Get_Kerning(ftFace, fontData[pixelSize][text[i - 1]].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta);
				x_pos += pairDelta.x >> 6;
			}

			GX_InitTexObj(&glyphTexture, glyphData->glyphDataTexture, glyphData->textureWidth, glyphData->textureHeight, GX_TF_I4, GX_CLAMP, GX_CLAMP, GX_FALSE);
			copyTextureToFramebuffer(&glyphTexture, glyphData->textureWidth, glyphData->textureHeight, x_pos + glyphData->renderOffsetX + x_offset, y - glyphData->renderOffsetY + y_offset, z, color);

			x_pos += glyphData->glyphAdvanceX;
			++printed;
		}
		++i;
	}

	if (textStyle & FTGX_STYLE_MASK)
	{
		getOffset(text, pixelSize, widthLimit);
		drawTextFeature(x + x_offset, y + y_offset, z, pixelSize, fullTextWidth, &ftgxAlign[pixelSize], textStyle,
				color);
	}

	return printed;
}

void FreeTypeGX::drawTextFeature(int16_t x, int16_t y, int16_t z, int16_t pixelSize, uint16_t width,
		ftgxDataOffset *offsetData, uint16_t format, GXColor color)
{
	uint16_t featureHeight = pixelSize >> 4 > 0 ? pixelSize >> 4 : 1;

	if (format & FTGX_STYLE_UNDERLINE) this->copyFeatureToFramebuffer(width, featureHeight, x, y + 1, z, color);

	if (format & FTGX_STYLE_STRIKE) this->copyFeatureToFramebuffer(width, featureHeight, x, y
			- ((offsetData->max) >> 1), z, color);
}

/**
 * Processes the supplied string and return the width of the string in pixels.
 *
 * This routine processes each character of the supplied text string and calculates the width of the entire string.
 * Note that if precaching of the entire font set is not enabled any uncached glyph will be cached after the call to this function.
 *
 * @param text  NULL terminated string to calculate.
 * @return The width of the text string in pixels.
 */
uint16_t FreeTypeGX::getWidth(const wchar_t *text, int16_t pixelSize)
{
	if (!text) return 0;

	uint16_t strWidth = 0;
	FT_Vector pairDelta;

	int i = 0;
	while (text[i])
	{
		ftgxCharData* glyphData = cacheGlyphData(text[i], pixelSize);

		if (glyphData != NULL)
		{
			if (ftKerningEnabled && (i > 0))
			{
				FT_Get_Kerning(ftFace, fontData[pixelSize][text[i - 1]].glyphIndex, glyphData->glyphIndex,
						FT_KERNING_DEFAULT, &pairDelta);
				strWidth += pairDelta.x >> 6;
			}

			strWidth += glyphData->glyphAdvanceX;
		}
		++i;
	}
	return strWidth;
}

/**
 * Single char width
 */
uint16_t FreeTypeGX::getCharWidth(const wchar_t wChar, int16_t pixelSize, const wchar_t prevChar)
{
	uint16_t strWidth = 0;
	ftgxCharData * glyphData = cacheGlyphData(wChar, pixelSize);

	if (glyphData != NULL)
	{
		if (ftKerningEnabled && prevChar != 0x0000)
		{
			FT_Vector pairDelta;
			FT_Get_Kerning(ftFace, fontData[pixelSize][prevChar].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT,
					&pairDelta);
			strWidth += pairDelta.x >> 6;
		}
		strWidth += glyphData->glyphAdvanceX;
	}

	return strWidth;
}

/**
 * Processes the supplied string and return the height of the string in pixels.
 *
 * This routine processes each character of the supplied text string and calculates the height of the entire string.
 * Note that if precaching of the entire font set is not enabled any uncached glyph will be cached after the call to this function.
 *
 * @param text  NULL terminated string to calculate.
 * @return The height of the text string in pixels.
 */
uint16_t FreeTypeGX::getHeight(const wchar_t *text, int16_t pixelSize)
{
	getOffset(text, pixelSize);

	return ftgxAlign[pixelSize].max - ftgxAlign[pixelSize].min;
}

/**
 * Get the maximum offset above and minimum offset below the font origin line.
 *
 * This function calculates the maximum pixel height above the font origin line and the minimum
 * pixel height below the font origin line and returns the values in an addressible structure.
 *
 * @param text  NULL terminated string to calculate.
 * @param offset returns the max and min values above and below the font origin line
 *
 */
void FreeTypeGX::getOffset(const wchar_t *text, int16_t pixelSize, uint16_t widthLimit)
{
	if (ftgxAlign.find(pixelSize) != ftgxAlign.end()) return;

	int16_t strMax = 0, strMin = 9999;
	uint16_t currWidth = 0;

	int i = 0;

	while (text[i])
	{
		if (widthLimit > 0 && currWidth >= widthLimit) break;

		ftgxCharData* glyphData = cacheGlyphData(text[i], pixelSize);

		if (glyphData != NULL)
		{
			strMax = glyphData->renderOffsetMax > strMax ? glyphData->renderOffsetMax : strMax;
			strMin = glyphData->renderOffsetMin < strMin ? glyphData->renderOffsetMin : strMin;
			currWidth += glyphData->glyphAdvanceX;
		}

		++i;
	}

	if (ftPointSize != pixelSize)
	{
		ftPointSize = pixelSize;
		FT_Set_Pixel_Sizes(ftFace, 0, ftPointSize);
	}

	ftgxAlign[pixelSize].ascender = ftFace->size->metrics.ascender >> 6;
	ftgxAlign[pixelSize].descender = ftFace->size->metrics.descender >> 6;
	ftgxAlign[pixelSize].max = strMax;
	ftgxAlign[pixelSize].min = strMin;
}

/**
 * Copies the supplied texture quad to the EFB.
 *
 * This routine uses the in-built GX quad builder functions to define the texture bounds and location on the EFB target.
 *
 * @param texObj	A pointer to the glyph's initialized texture object.
 * @param texWidth  The pixel width of the texture object.
 * @param texHeight The pixel height of the texture object.
 * @param screenX   The screen X coordinate at which to output the rendered texture.
 * @param screenY   The screen Y coordinate at which to output the rendered texture.
 * @param color Color to apply to the texture.
 */
void FreeTypeGX::copyTextureToFramebuffer(GXTexObj *texObj, f32 texWidth, f32 texHeight, int16_t screenX,
		int16_t screenY, int16_t screenZ, GXColor color)
{
	GX_LoadTexObj(texObj, GX_TEXMAP0);
	GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	GX_Begin(GX_QUADS, this->vertexIndex, 4);
	GX_Position3s16(screenX, screenY, screenZ);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(0.0f, 0.0f);

	GX_Position3s16(texWidth + screenX, screenY, screenZ);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(1.0f, 0.0f);

	GX_Position3s16(texWidth + screenX, texHeight + screenY, screenZ);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(1.0f, 1.0f);

	GX_Position3s16(screenX, texHeight + screenY, screenZ);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_TexCoord2f32(0.0f, 1.0f);
	GX_End();
}

/**
 * Creates a feature quad to the EFB.
 *
 * This function creates a simple quad for displaying underline or strikeout text styling.
 *
 * @param featureWidth  The pixel width of the quad.
 * @param featureHeight The pixel height of the quad.
 * @param screenX   The screen X coordinate at which to output the quad.
 * @param screenY   The screen Y coordinate at which to output the quad.
 * @param color Color to apply to the texture.
 */
void FreeTypeGX::copyFeatureToFramebuffer(f32 featureWidth, f32 featureHeight, int16_t screenX, int16_t screenY,
		int16_t screenZ, GXColor color)
{
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

	GX_Begin(GX_QUADS, this->vertexIndex, 4);
	GX_Position3s16(screenX, screenY, screenZ);
	GX_Color4u8(color.r, color.g, color.b, color.a);

	GX_Position3s16(featureWidth + screenX, screenY, screenZ);
	GX_Color4u8(color.r, color.g, color.b, color.a);

	GX_Position3s16(featureWidth + screenX, featureHeight + screenY, screenZ);
	GX_Color4u8(color.r, color.g, color.b, color.a);

	GX_Position3s16(screenX, featureHeight + screenY, screenZ);
	GX_Color4u8(color.r, color.g, color.b, color.a);
	GX_End();

	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
}
