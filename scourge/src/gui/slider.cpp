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

#define BUTTON_SIZE 20

Slider::Slider(int x1, int y1, int x2, GLuint highlight, int minValue, int maxValue, char *label) : 
  Widget(x1, y1, x2 - x1, 30) {
  this->x2 = x2;
  this->minValue = minValue;
  this->maxValue = maxValue;
  this->label = new Label(0, 0, label);
  this->pos = 0;
  this->inside = false;
  this->dragging = false;
  alpha = 0.5f;
  alphaInc = 0.05f;
  lastTick = 0;

}

Slider::~Slider() {
}

void Slider::drawWidget(Widget *parent) {
  GuiTheme *theme = ((Window*)parent)->getTheme();

  GLint t = SDL_GetTicks();
  if(lastTick == 0 || t - lastTick > 50) {
    lastTick = t;
    alpha += alphaInc;
    if(alpha >= 0.7f || alpha < 0.4f) alphaInc *= -1.0f;
  }

  glPushMatrix();
  glTranslatef( 0, 10, 0 );
  label->drawWidget(parent);
  glPopMatrix();

  // draw the drag-button
  if( theme->getButtonBorder() ) {
    glColor4f( theme->getButtonBorder()->color.r,
               theme->getButtonBorder()->color.g,
               theme->getButtonBorder()->color.b,
               theme->getButtonBorder()->color.a );
  } else {
    applyBorderColor();
  }
  glPushMatrix();
  
  glBegin(GL_LINES);
  glVertex2d( 0, 20 );
  glVertex2d( x2 - x, 20 );
  glEnd();



  drawButton( parent, pos, 12, pos + BUTTON_SIZE / 2, 12 + BUTTON_SIZE,
              false, false, false, false, inside );

  /*
  glTranslatef( pos, 12, 0 );

  applyBackgroundColor();
  glBegin( GL_QUADS );
  glVertex2d(0, 0);
  glVertex2d(0, BUTTON_SIZE);
  glVertex2d(BUTTON_SIZE / 2, BUTTON_SIZE);
  glVertex2d(BUTTON_SIZE / 2, 0);
  glEnd();

  if(inside) {
    glEnable( GL_TEXTURE_2D );
    glColor4f( 0.75, 0.75, 1, alpha );
    glBindTexture( GL_TEXTURE_2D, highlight );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
    glDisable( GL_CULL_FACE );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );glDisable( GL_CULL_FACE );
    glVertex2d(0, 0);
    glTexCoord2f( 0, 1 );
    glVertex2d(0, BUTTON_SIZE);
    glTexCoord2f( 1, 1 );
    glVertex2d(BUTTON_SIZE / 2, BUTTON_SIZE);
    glTexCoord2f( 1, 0 );
    glVertex2d(BUTTON_SIZE / 2, 0);
    glEnd();
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
    glEnable( GL_CULL_FACE );
  }
  applyBorderColor();
  glBegin( GL_LINES );
  glVertex2d( 0, 0 );
  glVertex2d( 0, BUTTON_SIZE );
  glVertex2d( 0, BUTTON_SIZE );
  glVertex2d( BUTTON_SIZE / 2, BUTTON_SIZE );
  glVertex2d( BUTTON_SIZE / 2, BUTTON_SIZE );
  glVertex2d( BUTTON_SIZE / 2, 0 );
  glVertex2d( BUTTON_SIZE / 2, 0 );
  glVertex2d( 0, 0 );
  */

  glEnd();
  glPopMatrix();
}

bool Slider::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  inside = isInside(x, y);
  if(inside) ((Window*)parent)->setLastWidget(this);
  else dragging = false;
  // handle it
  switch( event->type ) {
  case SDL_KEYUP:
  if(hasFocus()) {
    if(event->key.keysym.sym == SDLK_LEFT) {
      if(pos) pos -= 10 * getStep();
      if(pos < 0) pos = 0;
      return true;
    } else if(event->key.keysym.sym == SDLK_RIGHT) {
      if(pos < getWidth() - BUTTON_SIZE / 2) pos += 10 * getStep();
      if(pos >= getWidth() - BUTTON_SIZE / 2) pos = getWidth() - BUTTON_SIZE / 2;
      return true;
    }
  }
  break;
  case SDL_MOUSEMOTION:
    if(inside && dragging) {
      pos = x - getX();
      if(pos >= getWidth() - BUTTON_SIZE / 2) pos = getWidth() - BUTTON_SIZE / 2;
      if(pos < 0) pos = 0;
      return true;
    }
	break;
  case SDL_MOUSEBUTTONUP:
  if(inside) {
    dragging = false;
    pos = x - getX();
    if(pos >= getWidth() - BUTTON_SIZE / 2) pos = getWidth() - BUTTON_SIZE / 2;
    if(pos < 0) pos = 0;
    return true;
  }
  case SDL_MOUSEBUTTONDOWN:
  if(inside) {
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

void Slider::setValue(int n) {
  pos = (int)((float)(n * getWidth()) / (float)(maxValue - minValue));
  if(pos >= getWidth() - BUTTON_SIZE / 2) pos = getWidth() - BUTTON_SIZE / 2;
  if(pos < 0) pos = 0;
}

