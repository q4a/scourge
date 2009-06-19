/***************************************************************************
                      slider.cpp  -  Drag slider widget
                             -------------------
    begin                : Thu Aug 28 2003
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
#include "../common/constants.h"
#include "slider.h"
#include "window.h"

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

/**
  *@author Gabor Torok
  */

#define BUTTON_SIZE 20

// FIXME: slider does not use highlight texture but demands it in constructor
Slider::Slider( int x1, int y1, int x2, Texture highlight, int minValue, int maxValue, char *label ) :
		Widget( x1, y1, x2 - x1, 30 ) {
	this->x2 = x2;
	this->minValue = minValue;
	this->maxValue = maxValue;
	this->label = new Label( 0, 0, label );
	this->pos = 0;
	this->inside = false;
	this->dragging = false;
	alpha = 0.5f;
	alphaInc = 0.05f;
	lastTick = 0;

}

Slider::~Slider() {
	delete label;
}

void Slider::drawWidget( Window* parent ) {
	GuiTheme *theme = parent->getTheme();

	GLint t = SDL_GetTicks();
	if ( lastTick == 0 || t - lastTick > 50 ) {
		lastTick = t;
		alpha += alphaInc;
		if ( alpha >= 0.7f || alpha < 0.4f ) alphaInc *= -1.0f;
	}

	glPushMatrix();
	glTranslated( 0, 10, 0 );
	label->drawWidget( parent );
	glPopMatrix();

	// draw the drag-button
	if ( theme->getButtonBorder() ) {
		glColor4f( theme->getButtonBorder()->color.r,
		           theme->getButtonBorder()->color.g,
		           theme->getButtonBorder()->color.b,
		           theme->getButtonBorder()->color.a );
	} else {
		applyBorderColor();
	}
	glPushMatrix();

	glBegin( GL_LINES );
	glVertex2i( 0, 20 );
	glVertex2i( x2 - x, 20 );
	glEnd();

	drawButton( parent, pos, 12, pos + BUTTON_SIZE / 2, 12 + BUTTON_SIZE,
	            false, false, false, false, inside );

	glPopMatrix();
}

bool Slider::handleEvent( Window* parent, SDL_Event* event, int x, int y ) {
	inside = isInside( x, y );
	if ( inside ) parent->setLastWidget( this );
	else dragging = false;
	// handle it
	switch ( event->type ) {
	case SDL_KEYUP:
		if ( hasFocus() ) {
			if ( event->key.keysym.sym == SDLK_LEFT ) {
				if ( pos ) pos -= 10 * getStep();
				if ( pos < 0 ) pos = 0;
				return true;
			} else if ( event->key.keysym.sym == SDLK_RIGHT ) {
				if ( pos < getWidth() - BUTTON_SIZE / 2 ) pos += 10 * getStep();
				if ( pos >= getWidth() - BUTTON_SIZE / 2 ) pos = getWidth() - BUTTON_SIZE / 2;
				return true;
			}
		}
		break;
	case SDL_MOUSEMOTION:
		if ( inside && dragging ) {
			pos = x - getX();
			if ( pos >= getWidth() - BUTTON_SIZE / 2 ) pos = getWidth() - BUTTON_SIZE / 2;
			if ( pos < 0 ) pos = 0;
			return true;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if ( inside ) {
			dragging = false;
			pos = x - getX();
			if ( pos >= getWidth() - BUTTON_SIZE / 2 ) pos = getWidth() - BUTTON_SIZE / 2;
			if ( pos < 0 ) pos = 0;
			return true;
		}
	case SDL_MOUSEBUTTONDOWN:
		if ( event->button.button != SDL_BUTTON_LEFT ) return false;
		if ( inside ) {
			dragging = true;
		}
		break;
	default:
		break;
	}
	return false;
}

void Slider::removeEffects() {
	inside = false;
}

void Slider::setValue( int n ) {
	pos = static_cast<int>( static_cast<float>( n * getWidth() ) / static_cast<float>( maxValue - minValue ) );
	if ( pos >= getWidth() - BUTTON_SIZE / 2 ) pos = getWidth() - BUTTON_SIZE / 2;
	if ( pos < 0 ) pos = 0;
}

