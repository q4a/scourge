/***************************************************************************
                          slider.cpp  -  description
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
#include "slider.h"
#include "window.h"

/**
  *@author Gabor Torok
  */

#define BUTTON_SIZE 15

Slider::Slider(int x1, int y1, int x2, int y2, int minValue, int maxValue, char *label) : 
  Widget(x1, y1, x2 - x1, y2 - y1) {
  this->x2 = x2;
  this->y2 = y2;
  this->minValue = minValue;
  this->maxValue = maxValue;
  this->label = new Label(0, 0, label);
  this->pos = 0;
  this->inside = false;
  this->dragging = false;
}

Slider::~Slider() {
}

void Slider::drawWidget(Widget *parent) {
  applyBorderColor();

  glBegin(GL_LINES);
  glVertex2d( 0, 5 );
  glVertex2d( x2 - x, 5 );
  glEnd();

  label->drawWidget(parent);

  // draw the drag-button
  glPushMatrix();
  glTranslatef( pos, 0, 0 );
  glBegin( GL_QUADS );
  glVertex2d( 0, 0 );
  glVertex2d( 0, BUTTON_SIZE );
  glVertex2d( BUTTON_SIZE, BUTTON_SIZE );
  glVertex2d( BUTTON_SIZE, 0 );
  glEnd();
  glPopMatrix();
}

bool Slider::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  inside = isInside(x, y);
  if(inside) ((Window*)parent)->setLastWidget(this);
  // handle it
  switch( event->type ) {
  case SDL_KEYUP:
  if(hasFocus()) {
    if(event->key.keysym.sym == SDLK_LEFT) {
      if(pos) pos -= getStep();
      return true;
    } else if(event->key.keysym.sym == SDLK_RIGHT) {
      if(pos < getWidth() - BUTTON_SIZE) pos += getStep();
      return true;
    }
  }
  break;
  case SDL_MOUSEMOTION:
    if(inside && dragging && x - getX() < getWidth() - BUTTON_SIZE) {
      pos = x - getX();
    }
	break;
  case SDL_MOUSEBUTTONUP:
    if(dragging) dragging = false;
    else if(inside) pos = x - getX();
	return inside;
  case SDL_MOUSEBUTTONDOWN:
    if(x - getX() >= pos && x - getX() < pos + BUTTON_SIZE) {
      dragging = true;
    }
	break;
  default:
	break;
  }
  return false;
}

void Slider::removeEffects(Widget *parent) {
  inside = false;
}

