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

//#include "../scourge.h"

/**
  *@author Gabor Torok
  */

Button::Button(int x1, int y1, int x2, int y2, GLuint highlight, char *label) : 
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
  this->highlight = highlight;
  this->glowing = false;
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

  GLint t = SDL_GetTicks();
  if(lastTick == 0 || t - lastTick > 50) {
    lastTick = t;
    alpha += alphaInc;
    if(alpha >= 0.7f || alpha < 0.4f) alphaInc *= -1.0f;
  }
  if(glowing) {
    glEnable( GL_TEXTURE_2D );
    glColor4f( 1, 0.15, 0.15, alpha );
    glBindTexture( GL_TEXTURE_2D, highlight );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2d(0, 0);
    glTexCoord2f( 0, 1 );
    glVertex2d(0, y2 - y);
    glTexCoord2f( 1, 1 );
    glVertex2d(x2 - x, y2 - y);
    glTexCoord2f( 1, 0 );
    glVertex2d(x2 - x, 0);
    glEnd();
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
  }
  if(inside) {
    glEnable( GL_TEXTURE_2D );
    glColor4f( 0.75, 0.75, 1, alpha );
    glBindTexture( GL_TEXTURE_2D, highlight );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2d(0, 0);
    glTexCoord2f( 0, 1 );
    glVertex2d(0, y2 - y);
    glTexCoord2f( 1, 1 );
    glVertex2d(x2 - x, y2 - y);
    glTexCoord2f( 1, 0 );
    glVertex2d(x2 - x, 0);
    glEnd();
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
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

