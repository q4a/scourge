/*!
    \file  og_font.c
    \brief Bitmap and stroke fonts
*/

/*
 * Portions copyright (C) 2004, the OpenGLUT project contributors.
 * OpenGLUT branched from freeglut in February, 2004.
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Thu Dec 16 1999
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

#include "glutpart.h"

/* -- IMPORT DECLARATIONS -------------------------------------------------- */

/* -- PRIVATE FUNCTIONS ---------------------------------------------------- */

/*!
    Matches a font ID with a SOG_Font structure pointer.
    This was changed to match the GLUT header style.

    \bug  Only matches bitmapped fonts; the name should reflect that
          restriction.
*/
static SOG_Font *oghFontByID( void *font )
{

    if( font == GLUT_BITMAP_8_BY_13        )
	  return (SOG_Font*)&ogFontFixed8x13;
    if( font == GLUT_BITMAP_9_BY_15        )
        return (SOG_Font*)&ogFontFixed9x15;
    if( font == GLUT_BITMAP_HELVETICA_10   )
        return (SOG_Font*)&ogFontHelvetica10;
    if( font == GLUT_BITMAP_HELVETICA_12   )
        return (SOG_Font*)&ogFontHelvetica12;
    if( font == GLUT_BITMAP_HELVETICA_18   )
        return (SOG_Font*)&ogFontHelvetica18;
    if( font == GLUT_BITMAP_TIMES_ROMAN_10 )
        return (SOG_Font*)&ogFontTimesRoman10;
    if( font == GLUT_BITMAP_TIMES_ROMAN_24 )
        return (SOG_Font*)&ogFontTimesRoman24;

    cerr << "font 0x%08x not found" << font << endl;
    return 0;
}

/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*!
    \fn
    \brief    Draw a bitmapped character
    \ingroup  bitmapfont
    \param    font      A bitmapped font identifier.
    \param    character A character code.

              Draw a \a character at the current OpenGL raster position
              using a bitmapped \a font.  The raster position is advanced
              by the width of the character.

              Nothing is drawn, and the raster position is unaffected when
              either:
              - \a character is out of range
              - \a font is not a valid OpenGLUT bitmap font
              - The current OpenGL raster position is invalid

    \note     glutBitmapString() is generally more efficient for 
              strings of characters.

    \see      glRasterPos(), glutBitmapString(), glutBitmapWidth(), 
              glutBitmapHeight()
*/
void OGAPIENTRY glutBitmapCharacter( void *font, int character )
{
    const GLubyte *face;
    SOG_Font *f = oghFontByID( font );

    if( !f ||
        ( 0 > character ) ||
        (255 < character ) )
        return;

    /* Find the glyph we want to draw. */
    face = f->Characters[ character ];

    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );

        glPixelStorei( GL_UNPACK_SWAP_BYTES,  GL_FALSE );
        glPixelStorei( GL_UNPACK_LSB_FIRST,   GL_FALSE );
        glPixelStorei( GL_UNPACK_ROW_LENGTH,  0        );
        glPixelStorei( GL_UNPACK_SKIP_ROWS,   0        );
        glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0        );
        glPixelStorei( GL_UNPACK_ALIGNMENT,   1        );
        glBitmap(
            face[ 0 ], f->Height,         /* The bitmap's width and height  */
            f->xorig, f->yorig,           /* The origin in the font glyph   */
            ( float )( face[ 0 ] ), 0.0,  /* The raster advance -- inc. x,y */
            ( face + 1 )                  /* The packed bitmap data...      */
        );

    glPopClientAttrib( );
}

/*!
    \fn
    \brief    Draw a string of bitmapped characters
    \ingroup  bitmapfont
    \param    font    A bitmapped font identifier.
    \param    string  A NUL-terminated ASCII string.

              Draw a \a string the current OpenGL raster position
              using a bitmapped \a font.  The raster position is advanced
              by the width of the string.

              The starting raster position is used as
              the left margin for multi-line strings.
              Each newline character repositions the raster 
              position at the beginning of the next line.  

              Nothing is drawn, and the raster position is unaffected when
              either:
              - \a font is not a valid OpenGLUT bitmap font
              - \a string is an empty string or NULL pointer
              - The current OpenGL raster position is invalid

    \see      glRasterPos(), glutBitmapCharacter()
*/
void OGAPIENTRY glutBitmapString( void *font, const unsigned char *string )
{
    SOG_Font *f = oghFontByID( font );
    short x = 0;
    unsigned char c;

    if( !f )
        return;

    if( !string || !*string )
        return;

    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );

    glPixelStorei( GL_UNPACK_SWAP_BYTES,  GL_FALSE );
    glPixelStorei( GL_UNPACK_LSB_FIRST,   GL_FALSE );
    glPixelStorei( GL_UNPACK_ROW_LENGTH,  0        );
    glPixelStorei( GL_UNPACK_SKIP_ROWS,   0        );
    glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0        );
    glPixelStorei( GL_UNPACK_ALIGNMENT,   1        );

    /*
     * Step through the string, drawing each character.
     * A newline will simply translate the next character's insertion
     * point back to the start of the line and down one line.
     */
    while( c = *string++ )
        if( c == '\n' )
        {
            glBitmap( 0, 0, 0, 0, - ( float ) x, - ( float ) f->Height, NULL );
            x = 0;
        }
        else  /* Not an EOL, draw the bitmap character */
        {
            const GLubyte* face = f->Characters[ c ];
            glBitmap(
                face[ 0 ], f->Height,     /* Bitmap's width and height    */
                f->xorig, f->yorig,       /* The origin in the font glyph */
                face[ 0 ], 0.0,           /* The raster advance; inc. x,y */
                face + 1                  /* The packed bitmap data...    */
            );

            x += face[ 0 ];
        }

    glPopClientAttrib( );
}

/*!
    \fn
    \brief    Return the width of a bitmapped character, in pixels.
    \ingroup  bitmapfont
    \param    font      A bitmapped font identifier.
    \param    character A character code.

              Returns the horizontal OpenGL raster position
              offset for a \a character in a bitmapped \a font.

              It is also an upper bound on the width of the bitmapped glyph 
              for \a character, though not all letters will use their full 
              width, especially fixed-width fonts.

              Returns 0 if the \a character is out of the inclusive
              range [0,255] or if the \a font is invalid.

    \note     In GLUT, some glyphs could render to the left of the
              starting position, in some fonts.  OpenGLUT's fonts all
              position all of their glyphs to start at, or to the right of,
              the initial position.

    \see      glutBitmapCharacter(), glutBitmapLength(), glutBitmapHeight(),
              glutStrokeWidth()
*/
int OGAPIENTRY glutBitmapWidth( void *font, int character )
{
    int ret = 0;
    SOG_Font *f = oghFontByID( font );

    if( f &&
        ( 0 <= character ) &&
        ( 256 > character ) )
        ret = *( f->Characters[ character ] );

    return ret;
}

/*!
    \fn
    \brief    Return the width of a bitmapped string, in pixels.
    \ingroup  bitmapfont
    \param    font    A bitmapped font identifier.
    \param    string  A NUL-terminated ASCII string.

              Returns the maximum horizontal OpenGL raster position
              offset for a \a string in a bitmapped \a font.

              As with glutBitmapString(), newlines are taken into
              consideration.

              Returns 0 if the \a font is invalid or if the 
              \a string is empty or \a NULL.

    \see      glutBitmapString(), glutBitmapWidth(), glutBitmapHeight(),
              glutStrokeLength()
*/
int OGAPIENTRY glutBitmapLength( void *font, const unsigned char *string )
{
    int length = 0, this_line_length = 0;
    SOG_Font *f = oghFontByID( font );
    unsigned char c;

    if( f && string )
    {
        while( c = *string++ )
            if( c != '\n' ) /* Not an EOL, increment length of line */
                this_line_length += *( f->Characters[ c ]);
            else  /* EOL; reset the length of this line */
            {
                if( length < this_line_length )
                    length = this_line_length;
                this_line_length = 0;
            }
    }

    if ( length < this_line_length )
        length = this_line_length;
    return length;
}

/*!
    \fn
    \brief    Return the height of a given font, in pixels.
    \ingroup  bitmapfont
    \param    font    A bitmapped font identifier.

              Return the line-to-line vertical spacing (in pixels) 
              between lines of a bitmapped \a font.

              Returns 0 if \a font is invalid.

    \note     Does <i>not</i> report the height used by individual
              characters.  This may limit its usefulness.  (Compare
              with other font-metric queries.)
    \see      glutBitmapCharacter(), glutBitmapString(), glutBitmapWidth(),
              glutBitmapLength(), glutStrokeHeight()

    \internal
    \todo     We have discussed adding a "font descender" query.
              We should go ahead and do it.
*/
int OGAPIENTRY glutBitmapHeight( void *font )
{
    SOG_Font *f = oghFontByID( font );
    int ret = 0;

    if( f )
        ret = f->Height;

    return ret;
}
