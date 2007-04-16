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
#define SCROLL_WIDTH 500

TextScroller::TextScroller( Scourge *scourge ) {
	this->scourge = scourge;
	this->xp = 300;
	this->yp = 0;
  lineOffset = 0;
	offset = 0;
	lastCheck = SDL_GetTicks();
  inside = false;
}

TextScroller::~TextScroller() {
}

void TextScroller::addDescription( char *description, float r, float g, float b ) {
	string s = description;
	color.insert( color.begin(), new Color( r, g, b, 1 ) );
	text.insert( text.begin(), s );
	if( text.size() == MAX_QUEUE_SIZE ) {
		text.erase( text.end() - 1 );
		Color *c = color[ color.size() - 1 ];
		delete( c );
		color.erase( color.end() - 1 );
	}
}

void TextScroller::draw() {

  if( !inside ) {
    lineOffset = 0;
  }

	if( scourge->getParty()->isRealTimeMode() && !inside ) {
		Uint32 now = SDL_GetTicks();
		if( now - lastCheck > SCROLL_SPEED ) {
			lastCheck = now;
			offset += OFFSET_DELTA;
			if( offset >= LINE_HEIGHT ) {
				offset = 0;
				text.insert( text.begin(), "" );
        color.insert( color.begin(), new Color( 0, 0, 0, 0 ) );
			}
		}
	}

  int height = LINES_SHOWN * LINE_HEIGHT;

	glPushMatrix();
	glLoadIdentity();
	glTranslatef( xp, yp, 0 );
	glDisable( GL_DEPTH_TEST );

  if( inside ) {
    int margin = 10;
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDisable( GL_TEXTURE_2D );
    glColor4f( 1, 1, 1, 0.25f );
    glBegin( GL_QUADS );
    glVertex2d( -margin, height );
    glVertex2d( -margin, 0 );
    glVertex2d( SCROLL_WIDTH + margin, 0 );
    glVertex2d( SCROLL_WIDTH + margin, height );
    glEnd();
    glColor4f( 0.75f, 0.75f, 0.75f, 1 );
    glBegin( GL_LINE_LOOP );
    glVertex2d( -margin, height );
    glVertex2d( -margin, 0 );
    glVertex2d( SCROLL_WIDTH + margin, 0 );
    glVertex2d( SCROLL_WIDTH + margin, height );
    glEnd();
    glDisable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
  }

	int x = 0;
	int y = height - offset;
	// cerr << "text.size=" << text.size() << endl;
	for( unsigned int i = lineOffset; (int)i < lineOffset + LINES_SHOWN && i < text.size(); i++ ) {
		if( text[ i ] != "" ) {
			Color *c = color[ i ];
      float a;
      if( inside ) a = 1;
      else a = c->a * ( ( LINES_SHOWN - ( ( i - lineOffset ) + ( offset / (float)LINE_HEIGHT ) ) ) / (float)LINES_SHOWN );
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

bool TextScroller::handleEvent( SDL_Event *event ) {
  int mx = scourge->getSDLHandler()->mouseX;
  int my = scourge->getSDLHandler()->mouseY;
  inside = ( mx >= xp && mx < xp + SCROLL_WIDTH &&
             my >= yp && my < yp + LINES_SHOWN * LINE_HEIGHT );
  if( event->type == SDL_MOUSEBUTTONDOWN ) {
    if( inside ) {
      if( event->button.button == SDL_BUTTON_WHEELUP ) {
        lineOffset++;
        if( lineOffset + LINES_SHOWN >= (int)text.size() ) {
          lineOffset = text.size() - LINES_SHOWN + 1;
        }
      } else if( event->button.button == SDL_BUTTON_WHEELDOWN ) {
        lineOffset--;
        if( lineOffset < 0 ) lineOffset = 0;
      }
    }
  }
  return inside;
}

