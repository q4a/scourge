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

/**
  *@author Gabor Torok
  */

Button::Button(int x1, int y1, int x2, int y2, char *label) : 
  Widget(x1, y1) {
  this->x2 = x2;
  this->y2 = y2;
  this->label = new Label(0, 0, label);
  alpha = 0.5f;
  alphaInc = 0.05f;
  lastTick = 0;
}

Button::~Button() {
  delete label;
}

void Button::drawWidget(Window *parent) {
  if(!inside) {
	//	glColor4f( 0.7f, 0.65f, 0.2f, 1.0f );
	parent->applyBackgroundColor(true);
	glBegin(GL_QUADS);
	glVertex2d(0, 0);
	glVertex2d(0, y2 - y);
	glVertex2d(x2 - x, y2 - y);
	glVertex2d(x2 - x, 0);
	glEnd();
  } else {
	GLint t = SDL_GetTicks();
	if(lastTick == 0 || t - lastTick > 50) {
	  lastTick = t;
	  alpha += alphaInc;
	  if(alpha >= 0.7f || alpha < 0.4f) alphaInc *= -1.0f;
	}
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	glEnable( GL_BLEND );
	glBegin( GL_QUADS );
	glColor4f( 1, 0, 0, alpha );
	glVertex2d(0, 0);
	glColor4f( 0, 1, 0, alpha );
	glVertex2d(0, y2 - y);
	glColor4f( 0, 0, 1, alpha );
	glVertex2d(x2 - x, y2 - y);
	glColor4f( 1, 0, 1, alpha );
	glVertex2d(x2 - x, 0);
	glEnd();
	glDisable( GL_BLEND );
  }

  parent->applyBorderColor();
  glBegin(GL_LINES);
  glVertex2d(0, 0);
  glVertex2d(0, y2 - y);
  glVertex2d(x2 - x, 0);
  glVertex2d(x2 - x, y2 - y);
  glVertex2d(0, 0);
  glVertex2d(x2 - x, 0);
  glVertex2d(0, y2 - y);
  glVertex2d(x2 - x, y2 - y);
  glEnd();

  glPushMatrix();
  glTranslated( 5, (y2 - y) / 2 + 5, 0);
  label->drawWidget(parent);
  glPopMatrix();
}

bool Button::canHandle(SDLHandler *sdlHandler, SDL_Event *event, int x, int y) {
  inside = (x >= getX() && x <= x2 && 
			y >= getY() && y <= y2);
  return inside;
}

void Button::handleEvent(SDLHandler *sdlHandler, SDL_Event *event, int x, int y) {
  // handle it
  switch( event->type ) {
  case SDL_MOUSEMOTION:
	break;
  case SDL_MOUSEBUTTONUP:
	sdlHandler->fireEvent(this, event);
	break;
  case SDL_MOUSEBUTTONDOWN:
	break;
  default:
	break;
  }
}

