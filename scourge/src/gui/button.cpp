/***************************************************************************
                        button.cpp  -  Button widget
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
#include "button.h"
#include "window.h"
#include "guitheme.h"
#include "../render/texture.h"

/**
  *@author Gabor Torok
  */

Button::Button( int x1, int y1, int x2, int y2, Texture const& highlight, char const* label, Texture const& texture ) 
		: Widget( x1, y1, x2 - x1, y2 - y1 )
		, x2( x2 )
		, y2( y2 )
		, inside( false )
		, lastTick( 0 )
		, labelPos( CENTER )
		, toggle( false )
		, selected( false )
		, highlight( highlight )
		, glowing( false )
		, inverse( false )
		, fontType( 0 )
		, texture( texture )
		, pressed( false ) {
	setLabel( label );
}

Button::~Button() {
}

void Button::drawWidget( Window* parent ) {

	GuiTheme *theme = parent->getTheme();

	drawButton( parent, 0, 0, x2 - x, y2 - y, toggle, selected, inverse, glowing, inside );

	if ( texture.isSpecified() ) {
		glsDisable( GLS_CULL_FACE );
		glsEnable( GLS_TEXTURE_2D | GLS_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		glPushMatrix();
		texture.glBind();

		if ( isEnabled() ) {
			glColor4f( 1.0f, 1.0f, 1.0f, ( glowing || inside ? getAlpha() + 0.3f : 1.0f ) );
		} else {
			glColor4f( 0.5f, 0.5f, 0.5f, 0.5f );
		}

		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( getWidth(), 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, getHeight() );
		glTexCoord2i( 1, 1 );
		glVertex2i( getWidth(), getHeight() );
		glEnd();
		glPopMatrix();

		glsDisable( GLS_TEXTURE_2D | GLS_BLEND );
	}

	if ( strlen( label ) ) {
		glPushMatrix();
		int ypos;
		switch ( getLabelPosition() ) {
		case TOP: ypos = 13; break;
		case BOTTOM: ypos = ( y2 - y ) - 2; break;
		default: ypos = ( y2 - y ) / 2 + 5;
		}
		glTranslated( 5, ypos, 0 );
		if ( selected && theme->getButtonSelectionText() ) {
			glColor4f( theme->getButtonSelectionText()->r,
			           theme->getButtonSelectionText()->g,
			           theme->getButtonSelectionText()->b,
			           theme->getButtonSelectionText()->a );
		} else if ( theme->getButtonText() ) {
			glColor4f( theme->getButtonText()->r,
			           theme->getButtonText()->g,
			           theme->getButtonText()->b,
			           theme->getButtonText()->a );
		} else {
			applyColor();
		}
		parent->getScourgeGui()->setFontType( fontType );
		parent->getScourgeGui()->texPrint( 0, 0, label );
		parent->getScourgeGui()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
		glPopMatrix();
	}
}
	
bool Button::handleEvent( Window* parent, SDL_Event* event, int x, int y ) {
	inside = isInside( x, y );
	if ( inside ) parent->setLastWidget( this );

	inverse = pressed;
	// handle it
	switch ( event->type ) {
	case SDL_KEYUP:
		if ( hasFocus() ) {
			if ( event->key.keysym.sym == SDLK_RETURN ) {
				if ( toggle ) selected = !selected;
				return true;
			}
		}
		break;
	case SDL_MOUSEMOTION:
		inverse = ( inside && pressed );
		break;
	case SDL_MOUSEBUTTONUP:
		if ( event->button.button != SDL_BUTTON_LEFT || !pressed ) return false;
		pressed = false;
		if ( inside && toggle ) selected = !selected;
		return inside;
	case SDL_MOUSEBUTTONDOWN:
		if ( event->button.button != SDL_BUTTON_LEFT ) return false;
		pressed = inside;
		inverse = inside;
		break;
	default:
		break;
	}
	return false;
}

void Button::removeEffects() {
	inside = false;
}

