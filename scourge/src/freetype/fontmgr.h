/***************************************************************************
                          fontmgr.h  -  description
                             -------------------
    begin                : Tue Jan 23 2007
    copyright            : (C) 2003 by Gabor Torok
    email                : cctorok@yahoo.com
		
		The original code for this class was written by Bob Pendleton and is 
		covered under the LGPL. Thanks Bob!
		
		http://www.gamedev.net/community/forums/topic.asp?topic_id=284259
		
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include <iostream>
#include <map>
#include "../common/constants.h"

/**
  *@author Gabor Torok
  */
	
class GlyphInfo {
public:
	int minx, maxx;
	int miny, maxy;
	int advance;
	int w, h;
	GLuint tex;
	GLfloat texMinX, texMinY;
	GLfloat texMaxX, texMaxY;

	GlyphInfo();
	~GlyphInfo();
};
	
class FontMgr {
private:  
	static int initCounter;

  int height;
  int ascent;
  int descent;
  int lineSkip;

	std::map<Uint16, GlyphInfo*> glyphs;

  unsigned char *address;
  int length;
  int pointSize;
  int style;
  float fgRed, fgGreen, fgBlue;
  float bgRed, bgGreen, bgBlue;

  TTF_Font *ttfFont;

  SDL_Color foreground;
  SDL_Color background;

	Uint16 unicodeBuffer[5000];

  GlyphInfo *loadChar( Uint16 c );
	GLuint loadTextureColorKey( SDL_Surface *surface, 
															GLfloat *texcoord,
															int ckr, 
															int ckg, 
															int ckb );

	// from SDL_ttf
	static Uint16 *LATIN1_to_UNICODE(Uint16 *unicode, const char *text, int len);
	static Uint16 *UTF8_to_UNICODE(Uint16 *unicode, const char *utf8, int len);

	void textSizeUNICODE( Uint16 *p, SDL_Rect *r );
	void drawUNICODE( Uint16 *p, int x, int y );

public:

  FontMgr( TTF_Font *ttfFont, 
					 float fgRed, float fgGreen, float fgBlue,
					 float bgRed, float bgGreen, float bgBlue );

  ~FontMgr();
  inline int getLineSkip() { return lineSkip; }
  inline int getHeight() { return height; }
  void textSizeUTF8( char *text, SDL_Rect *r );
	void textSize( char *text, SDL_Rect *r );
  void drawTextUTF8( char *text, int x, int y );
	void drawText(char *text, int x, int y);

};

#endif

