
/**
 * This code was originally written by Mark Kilgard (mjk@nvidia.com) (c) 1997
 * see: http://www.opengl.org/developers/code/mjktips/TexFont/TexFont.html
 */

/***************************************************************************
                    text.h  -  Font manager for .txf fonts
                             -------------------
    begin                : Mon Jul 7 2003
    copyright            : (C) 2003 by Gabor Torok
    email                : cctorok@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TEXT_H
#define TEXT_H
#pragma once


/**
  *@author Gabor Torok
  */

#define TXF_FORMAT_BYTE  0
#define TXF_FORMAT_BITMAP 1

struct TexGlyphInfo {
	unsigned short c;       /* Potentially support 16-bit glyphs. */
	unsigned char width;
	unsigned char height;
	signed char xoffset;
	signed char yoffset;
	signed char advance;
	char dummy;           /* Space holder for alignment reasons. */
	short x;
	short y;
};

struct TexGlyphVertexInfo {
	GLfloat t0[2];
	GLshort v0[2];
	GLfloat t1[2];
	GLshort v1[2];
	GLfloat t2[2];
	GLshort v2[2];
	GLfloat t3[2];
	GLshort v3[2];
	GLfloat advance;
};

struct TexFont {
	GLuint texobj;
	int tex_width;
	int tex_height;
	int max_ascent;
	int max_descent;
	int num_glyphs;
	int min_glyph;
	int range;
	unsigned char *teximage;
	TexGlyphInfo *tgi;
	TexGlyphVertexInfo *tgvi;
	TexGlyphVertexInfo **lut;
};

enum {
	MONO, TOP_BOTTOM, LEFT_RIGHT, FOUR
};

/* byte swap a 32-bit value */
#define SWAPL(x, n) { \
		n = ((char *) (x))[0];\
		((char *) (x))[0] = ((char *) (x))[3];\
		((char *) (x))[3] = n;\
		n = ((char *) (x))[1];\
		((char *) (x))[1] = ((char *) (x))[2];\
		((char *) (x))[2] = n; }

/* byte swap a short */
#define SWAPS(x, n) { \
		n = ((char *) (x))[0];\
		((char *) (x))[0] = ((char *) (x))[1];\
		((char *) (x))[1] = n; }

class TexturedText  {
private:

	char *lastError;
	int useLuminanceAlpha;
	std::string filename;
	TexFont *txf;

public:
	TexturedText();
	~TexturedText();


	inline char *txfErrorString() {
		return lastError;
	}


	TexFont *txfLoadFont( std::string& filename );

	void txfUnloadFont();

	GLuint txfEstablishTexture( GLuint texobj,
	                            GLboolean setupMipmaps );

	void txfBindFontTexture();

	void txfGetStringMetrics( char *string,
	                          int len,
	                          int *width,
	                          int *max_ascent,
	                          int *max_descent );

	void txfRenderGlyph( int c );

	void txfRenderString( char *string,
	                      int len );

	void txfRenderFancyString( char *string,
	                           int len );

private:
	TexGlyphVertexInfo *getTCVI( int c );
	int txfInFont( int c );
	DECLARE_NOISY_OPENGL_SUPPORT();
};

#endif
