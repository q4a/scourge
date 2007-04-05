/***************************************************************************
                          button.cpp  -  description
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
#include "button.h"
#include "window.h"
#include "guitheme.h"

/**
  *@author Gabor Torok
  */

Button::Button(int x1, int y1, int x2, int y2, GLuint highlight, char *label, GLuint texture) : 
  Widget(x1, y1, x2 - x1, y2 - y1) {
  this->x2 = x2;
  this->y2 = y2;
  setLabel( label );
  labelPos = CENTER;
  alpha = 0.5f;
  alphaInc = 0.05f;
  lastTick = 0;
  toggle = selected = false;
  inside = false;
  this->highlight = highlight;
  this->glowing = false;
  this->inverse = false;
	this->fontType = 0;
	this->texture = texture;
}

Button::~Button() {
}

void Button::drawWidget(Widget *parent) {

  GuiTheme *theme = ((Window*)parent)->getTheme();

  drawButton( parent, 0, 0, x2 - x, y2 - y, toggle, selected, inverse, glowing, inside );

	if( texture ) {
		glDisable( GL_CULL_FACE );
		glEnable( GL_ALPHA_TEST );
		//glAlphaFunc( GL_EQUAL, 0xff );
		glAlphaFunc( GL_NOTEQUAL, 0 );
		glEnable(GL_TEXTURE_2D);
		glPushMatrix();
		glBindTexture( GL_TEXTURE_2D, texture );
		if( isEnabled() ) {
			glColor4f( 1, 1, 1, 1 );
		} else {
			glColor4f( 0.5f, 0.5f, 0.5f, 0.5f );
		}
		
		glBegin( GL_QUADS );
		glNormal3f( 0, 0, 1 );
		glTexCoord2f( 0, 0 );
		glVertex3f( 0, 0, 0 );
		glTexCoord2f( 0, 1 );
		glVertex3f( 0, getHeight(), 0 );
		glTexCoord2f( 1, 1 );
		glVertex3f( getWidth(), getHeight(), 0 );
		glTexCoord2f( 1, 0 );
		glVertex3f( getWidth(), 0, 0 );
		glEnd();
		glPopMatrix();
		
		glDisable( GL_ALPHA_TEST );
		glDisable(GL_TEXTURE_2D);
	}

  if( strlen( label ) ) {
    glPushMatrix();
    int ypos;
    switch(getLabelPosition()) {
    case TOP: ypos = 13; break;
    case BOTTOM: ypos = (y2 - y) - 2; break;
    default: ypos = (y2 - y) / 2 + 5;
    }
    glTranslated( 5, ypos, 0);
    if( selected && theme->getButtonSelectionText() ) {
      glColor4f( theme->getButtonSelectionText()->r,
                 theme->getButtonSelectionText()->g,
                 theme->getButtonSelectionText()->b,
                 theme->getButtonSelectionText()->a );
    } else if( theme->getButtonText() ) {
      glColor4f( theme->getButtonText()->r,
                 theme->getButtonText()->g,
                 theme->getButtonText()->b,
                 theme->getButtonText()->a );
    } else {
      applyColor();      
    }
    ((Window*)parent)->getScourgeGui()->setFontType( fontType );
    ((Window*)parent)->getScourgeGui()->texPrint(0, 0, label);
    ((Window*)parent)->getScourgeGui()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
    glPopMatrix();
  }
}

bool Button::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  inverse = false;
  inside = isInside(x, y);
  if(inside) ((Window*)parent)->setLastWidget(this);
  // handle it
  switch( event->type ) {
  case SDL_KEYUP:
  if(hasFocus()) {
    if(event->key.keysym.sym == SDLK_RETURN) {
      if(toggle) selected = (selected ? false : true);
      return true;
    }
  }
  break;
  case SDL_MOUSEMOTION:
  inverse = ( inside && event->motion.state == SDL_PRESSED );
	break;
  case SDL_MOUSEBUTTONUP:
	if(inside && toggle) selected = (selected ? false : true);
	return inside;
  case SDL_MOUSEBUTTONDOWN:
  inverse = inside;
	break;
  default:
	break;
  }
  return false;
}

void Button::removeEffects(Widget *parent) {
  inside = false;
}

