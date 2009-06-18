/***************************************************************************
             textscroller.cpp  -  "Scrolling text lines" widget
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
#include "common/constants.h"
#include "textscroller.h"
#include "scourge.h"
#include "sdlhandler.h"
#include "render/map.h"
#include "shapepalette.h"

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

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
	visible = false;
	scrollTexture = 0;
}

TextScroller::~TextScroller() {
	for ( size_t i = 0; i < color.size(); ++i ) {
		delete color[i];
	}
}

void TextScroller::addDescription( char const* description, float r, float g, float b, int logLevel ) {
	if ( scourge->getUserConfiguration()->getLogLevel() < logLevel ) return;

	string s = description;
	color.insert( color.begin(), new Color( r, g, b, 1 ) );
	text.insert( text.begin(), s );
	if ( text.size() == MAX_QUEUE_SIZE ) {
		text.erase( text.end() - 1 );
		Color *c = color[ color.size() - 1 ];
		delete( c );
		color.erase( color.end() - 1 );
	}
}

void TextScroller::writeLogMessage( char const* message, int messageType, int logLevel ) {

	if ( messageType == Constants::MSGTYPE_NORMAL ) {
		addDescription( message, 1, 1, 0, logLevel );
	} else if ( messageType == Constants::MSGTYPE_MISSION ) {
		addDescription( message, 1, 1, 1, logLevel );
	} else if ( messageType == Constants::MSGTYPE_PLAYERDAMAGE ) {
		addDescription( message, 1, 0, 0, logLevel );
	} else if ( messageType == Constants::MSGTYPE_NPCDAMAGE ) {
		addDescription( message, 0, 1, 0, logLevel );
	} else if ( messageType == Constants::MSGTYPE_PLAYERMAGIC ) {
		addDescription( message, 1, 0, 1, logLevel );
	} else if ( messageType == Constants::MSGTYPE_NPCMAGIC ) {
		addDescription( message, 0.7f, 0, 1, logLevel );
	} else if ( messageType == Constants::MSGTYPE_PLAYERITEM ) {
		addDescription( message, 0, 0, 1, logLevel );
	} else if ( messageType == Constants::MSGTYPE_NPCITEM ) {
		addDescription( message, 0, 0.5f, 1, logLevel );
	} else if ( messageType == Constants::MSGTYPE_PLAYERBATTLE ) {
		addDescription( message, 1, 0.8f, 0.8f, logLevel );
	} else if ( messageType == Constants::MSGTYPE_NPCBATTLE ) {
		addDescription( message, 1, 0.5f, 0.5f, logLevel );
	} else if ( messageType == Constants::MSGTYPE_PLAYERDEATH ) {
		addDescription( message, 0.7f, 0.7f, 0, logLevel );
	} else if ( messageType == Constants::MSGTYPE_NPCDEATH ) {
		addDescription( message, 0, 0.7f, 0, logLevel );
	} else if ( messageType == Constants::MSGTYPE_FAILURE ) {
		addDescription( message, 0.7f, 0.7f, 0.7f, logLevel );
	} else if ( messageType == Constants::MSGTYPE_STATS ) {
		addDescription( message, 1, 0.5f, 0, logLevel );
	} else if ( messageType == Constants::MSGTYPE_SYSTEM ) {
		addDescription( message, 0, 0.8f, 0.8f, logLevel );
	} else if ( messageType == Constants::MSGTYPE_SKILL ) {
		addDescription( message, 0, 1, 1, logLevel );
	}
}

void TextScroller::draw() {

	if ( !visible ) {
		lineOffset = 0;
	}

	if ( scourge->getParty()->isRealTimeMode() && !visible ) {
		Uint32 now = SDL_GetTicks();
		if ( now - lastCheck > SCROLL_SPEED ) {
			lastCheck = now;
			offset += OFFSET_DELTA;
			if ( offset >= LINE_HEIGHT ) {
				offset = 0;
				text.insert( text.begin(), "" );
				color.insert( color.begin(), new Color( 0, 0, 0, 0 ) );
				for ( unsigned int i = LINES_SHOWN; i < text.size(); i++ ) {
					if ( text[i] == "" ) {
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
	glsDisable( GLS_DEPTH_TEST );

	if ( visible ) {
		int margin = 10;
		glsEnable( GLS_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glsDisable( GLS_TEXTURE_2D );
		// glColor4f( 1, 1, 1, 0.25f );
		glColor4f( 0, 0, 0, 0.5f );
		glBegin( GL_TRIANGLE_STRIP );
		glVertex2i( -margin, 0 );
		glVertex2i( SCROLL_WIDTH + margin, 0 );
		glVertex2i( -margin, height + margin );
		glVertex2i( SCROLL_WIDTH + margin, height + margin );
		glEnd();
		glColor4f( 0.5f, 0.5f, 0.5f, 1 );
		glBegin( GL_LINE_LOOP );
		glVertex2i( -margin, height + margin );
		glVertex2i( -margin, 0 );
		glVertex2i( SCROLL_WIDTH + margin, 0 );
		glVertex2i( SCROLL_WIDTH + margin, height + margin );
		glEnd();

		glBegin( GL_LINE_LOOP );
		glVertex2i( SCROLL_WIDTH + margin - 10, height );
		glVertex2i( SCROLL_WIDTH + margin - 10, 10 );
		glVertex2i( SCROLL_WIDTH + margin - 7, 10 );
		glVertex2i( SCROLL_WIDTH + margin - 7, height );
		glEnd();
		int ypos = static_cast<int>( ( lineOffset * ( height - 10 ) ) / static_cast<float>( text.size() - 1 ) );
		glBegin( GL_TRIANGLE_STRIP );
		glVertex2i( SCROLL_WIDTH + margin - 10, height - ( ypos + 10 ) );
		glVertex2i( SCROLL_WIDTH + margin - 7, height - ( ypos + 10 ) );
		glVertex2i( SCROLL_WIDTH + margin - 10, height - ypos );
		glVertex2i( SCROLL_WIDTH + margin - 7, height - ypos );
		glEnd();

		glsEnable( GLS_TEXTURE_2D );
		glsDisable( GLS_BLEND );
	}

	glScissor( xp, scourge->getScreenHeight() - ( ytop + height ), SCROLL_WIDTH, height );
	glsEnable( GLS_SCISSOR_TEST );

	int currentOffset = ( visible ? 0 : offset );
	int x = 0;
	int y = height - currentOffset;
	// cerr << "text.size=" << text.size() << endl;
	for ( unsigned int i = lineOffset; static_cast<int>( i ) < lineOffset + LINES_SHOWN && i < text.size(); i++ ) {
		if ( text[ i ] != "" ) {
			Color *c = color[ i ];
			float a;
			if ( visible ) a = 1;
			else a = c->a * ( ( LINES_SHOWN - ( ( i - lineOffset ) + ( currentOffset / static_cast<float>( LINE_HEIGHT ) ) ) ) / static_cast<float>( LINES_SHOWN ) );
			glColor4f( c->r, c->g, c->b, a  );
			scourge->getSDLHandler()->texPrint( x, y, text[ i ].c_str() );
		}
		y -= LINE_HEIGHT;
	}
	glsDisable( GLS_SCISSOR_TEST );
	glsEnable( GLS_DEPTH_TEST );
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
	bool before = visible;
	inside = ( mx >= xp && mx < xp + SCROLL_WIDTH &&
	           my >= ytop && my < ytop + LINES_SHOWN * LINE_HEIGHT &&
	           !( scourge->getSession()->getMap()->isMouseRotating() ||
	              scourge->getSession()->getMap()->isMouseZooming() ||
	              scourge->getSession()->getMap()->isMapMoving() ) );
	if ( inside && !before && my >= ytop && my <= ytop + 15 ) {
		visible = true;
		startOffset = lineOffset = 0;
		for ( unsigned int i = 0; i < text.size(); i++ ) {
			if ( text[i] != "" ) {
				startOffset = lineOffset = i;
				break;
			}
		}
	}

	if ( !inside && before ) visible = false;

	if ( event->type == SDL_MOUSEBUTTONDOWN ) {
		if ( inside && visible ) {
			if ( event->button.button == SDL_BUTTON_WHEELUP ) {
				lineOffset++;
				if ( lineOffset + LINES_SHOWN >= static_cast<int>( text.size() ) ) {
					lineOffset = text.size() - LINES_SHOWN + 1;
				}
			} else if ( event->button.button == SDL_BUTTON_WHEELDOWN ) {
				lineOffset--;
				if ( lineOffset < startOffset ) lineOffset = startOffset;
			}
		}
	}
	return visible;
}

