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

/**
  *@author Gabor Torok
  */

Button::Button(int x1, int y1, int x2, int y2, char *label) : 
  Widget(x1, y1, x2 - x1, y2 - y1) {
  this->x2 = x2;
  this->y2 = y2;
  this->label = new Label(0, 0, label);
  labelPos = CENTER;
  alpha = 0.5f;
  alphaInc = 0.05f;
  lastTick = 0;
  toggle = selected = false;
  inside = false;
}

Button::~Button() {
  delete label;
}

void Button::drawWidget(Widget *parent) {
  if(toggle && selected) {
	applySelectionColor();
  } else {
	applyBackgroundColor(true);
  }
  if(isTranslucent()) {
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	glEnable( GL_BLEND );
  }
  glBegin(GL_QUADS);
  glVertex2d(0, 0);
  glVertex2d(0, y2 - y);
  glVertex2d(x2 - x, y2 - y);
  glVertex2d(x2 - x, 0);
  glEnd();
  if(isTranslucent()) {
	glDisable( GL_BLEND );
  }

  if(inside) {
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

  applyBorderColor();
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
  int ypos;
  switch(getLabelPosition()) {
  case TOP: ypos = 13; break;
  case BOTTOM: ypos = (y2 - y) - 2; break;
  default: ypos = (y2 - y) / 2 + 5;
  }
  glTranslated( 5, ypos, 0);
  label->drawWidget(parent);
  glPopMatrix();
}

bool Button::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  inside = isInside(x, y);
  // handle it
  switch( event->type ) {
  case SDL_MOUSEMOTION:
	break;
  case SDL_MOUSEBUTTONUP:
	if(inside && toggle) selected = (selected ? false : true);
	return inside;
  case SDL_MOUSEBUTTONDOWN:
	break;
  default:
	break;
  }
  return false;
}

void Button::removeEffects(Widget *parent) {
  inside = false;
}

