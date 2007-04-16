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
#define MAX_QUEUE_SIZE 200

TextScroller::TextScroller( Scourge *scourge ) {
	this->scourge = scourge;
	this->xp = 300;
	this->yp = 0;
	offset = 0;
	lastCheck = SDL_GetTicks();
}

TextScroller::~TextScroller() {
}

void TextScroller::addDescription( char *description, float r, float g, float b ) {
	string s = description;
	color.insert( color.begin(), new Color( r, g, b, 1 ) );
	text.insert( text.begin(), s );
	if( text.size() == MAX_QUEUE_SIZE ) {
		text.erase( text.end() );
		Color *c = color[ color.size() - 1 ];
		delete( c );
		color.erase( color.end() );
	}
}

void TextScroller::draw() {

	if( scourge->getParty()->isRealTimeMode() ) {
		Uint32 now = SDL_GetTicks();
		if( now - lastCheck > SCROLL_SPEED ) {
			lastCheck = now;
			offset += OFFSET_DELTA;
			if( offset >= LINE_HEIGHT ) {
				offset = 0;
				text.insert( text.begin(), "" );
			}
		}
	}

	glPushMatrix();
	glLoadIdentity();
	glTranslatef( xp, yp, 0 );
	glDisable( GL_DEPTH_TEST );
	int x = 0;
	int y = LINES_SHOWN * LINE_HEIGHT - offset;
	// cerr << "text.size=" << text.size() << endl;
	for( unsigned int i = 0; i < LINES_SHOWN && i < text.size(); i++ ) {
		if( text[ i ] != "" ) {
			Color *c = color[ i ];
			float a = c->a * ( ( LINES_SHOWN - ( i + ( offset / (float)LINE_HEIGHT ) ) ) / (float)LINES_SHOWN );
			glColor4f( c->r, c->g, c->b, a  );
			scourge->getSDLHandler()->texPrint( x, y, text[ i ].c_str() );
		}
		y -= LINE_HEIGHT;
	}
	glEnable( GL_DEPTH_TEST );
	glPopMatrix();
	glColor4f( 1, 1, 1, 1 );
}

void TextScroller::scrollUp() {
}

void TextScroller::scrollDown() {
}


