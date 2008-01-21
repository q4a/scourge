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
#include "render/map.h"
#include "shapepalette.h"

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
  startOffset = lineOffset = 0;
	offset = 0;
	lastCheck = SDL_GetTicks();
  inside = false;
	scrollTexture = 0;
}

TextScroller::~TextScroller() {
}

void TextScroller::addDescription( char const* description, float r, float g, float b ) {
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
				for( unsigned int i = LINES_SHOWN; i < text.size(); i++ ) {
					if( text[i] == "" ) {
						text.erase( text.begin() + i );
						Color *c = color[ i ];
						delete( c );
						color.erase( color.begin() + i );
						i--;
					}
				}
			}
		}
	}

  int height = LINES_SHOWN * LINE_HEIGHT;

	glPushMatrix();
	glLoadIdentity();
	int ytop = ( scourge->inTurnBasedCombat() ? yp + 50 : yp );
	glTranslatef( xp, ytop, 0 );
	glDisable( GL_DEPTH_TEST );

  if( inside ) {
    int margin = 10;
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDisable( GL_TEXTURE_2D );
    // glColor4f( 1, 1, 1, 0.25f );
		glColor4f( 0, 0, 0, 0.5f );
    glBegin( GL_QUADS );
    glVertex2d( -margin, height + margin );
    glVertex2d( -margin, 0 );
    glVertex2d( SCROLL_WIDTH + margin, 0 );
    glVertex2d( SCROLL_WIDTH + margin, height + margin );
    glEnd();
    glColor4f( 0.5f, 0.5f, 0.5f, 1 );
    glBegin( GL_LINE_LOOP );
    glVertex2d( -margin, height + margin );
    glVertex2d( -margin, 0 );
    glVertex2d( SCROLL_WIDTH + margin, 0 );
    glVertex2d( SCROLL_WIDTH + margin, height + margin );
    glEnd();

		glBegin( GL_LINE_LOOP );
		glVertex2d( SCROLL_WIDTH + margin - 10, height );
    glVertex2d( SCROLL_WIDTH + margin - 10, 10 );
    glVertex2d( SCROLL_WIDTH + margin - 7, 10 );
    glVertex2d( SCROLL_WIDTH + margin - 7, height );
		glEnd();
		int ypos = (int)( ( lineOffset * ( height - 10 ) ) / (float)( text.size() - 1 ) );
		glBegin( GL_QUADS );
		glVertex2d( SCROLL_WIDTH + margin - 10, height - ( ypos + 10 ) );
    glVertex2d( SCROLL_WIDTH + margin - 10, height - ypos );
    glVertex2d( SCROLL_WIDTH + margin - 7, height - ypos );
    glVertex2d( SCROLL_WIDTH + margin - 7, height - ( ypos + 10 ) );
		glEnd();

    glEnable( GL_TEXTURE_2D );
		glDisable( GL_BLEND );
  }

	glScissor( xp, scourge->getScreenHeight() - ( ytop + height ), SCROLL_WIDTH, height );
  glEnable( GL_SCISSOR_TEST );

	int currentOffset = ( inside ? 0 : offset );
	int x = 0;
	int y = height - currentOffset;
	// cerr << "text.size=" << text.size() << endl;
	for( unsigned int i = lineOffset; (int)i < lineOffset + LINES_SHOWN && i < text.size(); i++ ) {
		if( text[ i ] != "" ) {
			Color *c = color[ i ];
      float a;
      if( inside ) a = 1;
      else a = c->a * ( ( LINES_SHOWN - ( ( i - lineOffset ) + ( currentOffset / (float)LINE_HEIGHT ) ) ) / (float)LINES_SHOWN );
			glColor4f( c->r, c->g, c->b, a  );
			scourge->getSDLHandler()->texPrint( x, y, text[ i ].c_str() );
		}
		y -= LINE_HEIGHT;
	}
	glDisable( GL_SCISSOR_TEST );
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
	int ytop = ( scourge->inTurnBasedCombat() ? yp + 50 : yp );
	bool before = inside;
	inside = ( mx >= xp && mx < xp + SCROLL_WIDTH &&
						 my >= ytop && my < ytop + LINES_SHOWN * LINE_HEIGHT &&
						 !( scourge->getSession()->getMap()->isMouseRotating() || 
								scourge->getSession()->getMap()->isMouseZooming() ||
								scourge->getSession()->getMap()->isMapMoving() ) );
	if( inside && !before ) {
		startOffset = lineOffset = 0;
		for( unsigned int i = 0; i < text.size(); i++ ) {
			if( text[i] != "" ) {
				startOffset = lineOffset = i;
				break;
			}
		}
	}
	if( event->type == SDL_MOUSEBUTTONDOWN ) {
		if( inside ) {
			if( event->button.button == SDL_BUTTON_WHEELUP ) {
				lineOffset++;
				if( lineOffset + LINES_SHOWN >= (int)text.size() ) {
					lineOffset = text.size() - LINES_SHOWN + 1;
				}
			} else if( event->button.button == SDL_BUTTON_WHEELDOWN ) {
				lineOffset--;
				if( lineOffset < startOffset ) lineOffset = startOffset;
			}
		}
	}
  return inside;
}

