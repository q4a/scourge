/***************************************************************************
                          textscroller.cpp  -  description
                             -------------------
    begin                : Sat Apr 14 2007
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
#include "textscroller.h"
#include "scourge.h"
#include "sdlhandler.h"

using namespace std;

#define LINE_HEIGHT 15
#define LINES_SHOWN 7
#define SCROLL_SPEED 100
#define OFFSET_DELTA 1

TextScroller::TextScroller( Scourge *scourge ) {
	this->scourge = scourge;
	offset = 0;
	lastCheck = SDL_GetTicks();
}

TextScroller::~TextScroller() {
}

void TextScroller::addDescription( char *description, float r, float g, float b ) {
	string s = description;
	// color.push_back( Color( r, g, b, 1 ) );
	text.insert( text.begin(), s );
}

void TextScroller::draw() {

	Uint32 now = SDL_GetTicks();
	if( now - lastCheck > SCROLL_SPEED ) {
		lastCheck = now;
		offset += OFFSET_DELTA;
		if( offset >= LINE_HEIGHT ) {
			offset = 0;
			text.insert( text.begin(), "" );
		}
	}

	glPushMatrix();
	glLoadIdentity();
	glDisable( GL_DEPTH_TEST );
	int x = 50;
	int y = LINES_SHOWN * LINE_HEIGHT - offset;
	int end = -1;
	// cerr << "text.size=" << text.size() << endl;
	for( unsigned int i = 0; i < text.size(); i++ ) {
		if( y < 0 ) {
			if( end == -1 ) end = i;
		} else {
			if( text[ i ] != "" ) {
				scourge->getSDLHandler()->texPrint( x, y, text[ i ].c_str() );
			}
			y -= LINE_HEIGHT;
		}
	}
	if( end > -1 ) {
		while( (int)text.size() > end ) {
			text.erase( text.end() );
		}
	}
	glEnable( GL_DEPTH_TEST );
	glPopMatrix();
}

void TextScroller::scrollUp() {
}

void TextScroller::scrollDown() {
}


