/*
 * Portions copyright (C) 2004, the OpenGLUT project contributors.
 * OpenGLUT branched from freeglut in February, 2004.
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Fri Dec 3 1999
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PAWEL W. OLSZTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/***************************************************************************
                          glutpart.h  -  description
                             -------------------
    begin                : Thu Jul 10 2003
    copyright            : (C) 2003 by Gabor Torok
    email                : cctorok@yahoo.com
 ***************************************************************************/

#ifndef GLUT_PART_H
#define GLUT_PART_H

#include "../constants.h"

/*
 * The bitmap font structure
 */
typedef struct tagSOG_Font SOG_Font;
struct tagSOG_Font
{
    char           *Name;         /* The source font name             */
    int             Quantity;     /* Number of chars in font          */
    int             Height;       /* Height of the characters         */
    const GLubyte **Characters;   /* The characters mapping           */

    float           xorig, yorig; /* Relative origin of the character */
};

/* Bitmap font constants (use these in GLUT program). */
#define GLUT_BITMAP_9_BY_15		((void*)2)
#define GLUT_BITMAP_8_BY_13		((void*)3)
#define GLUT_BITMAP_TIMES_ROMAN_10	((void*)4)
#define GLUT_BITMAP_TIMES_ROMAN_24	((void*)5)
#define GLUT_BITMAP_HELVETICA_10	((void*)6)
#define GLUT_BITMAP_HELVETICA_12	((void*)7)
#define GLUT_BITMAP_HELVETICA_18	((void*)8)

#define OGAPIENTRY

extern void OGAPIENTRY glutBitmapCharacter( void *font, int character );

/*
 * These are the font faces defined in og_font_data.c file:
 */
extern const SOG_Font ogFontFixed8x13;
extern const SOG_Font ogFontFixed9x15;
extern const SOG_Font ogFontHelvetica10;
extern const SOG_Font ogFontHelvetica12;
extern const SOG_Font ogFontHelvetica18;
extern const SOG_Font ogFontTimesRoman10;
extern const SOG_Font ogFontTimesRoman24;

#endif
