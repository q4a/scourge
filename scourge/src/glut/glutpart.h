/***************************************************************************
                          glutpart.h  -  description
                             -------------------
    begin                : Thu Jul 10 2003
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
