/***************************************************************************
                          widget.cpp  -  description
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

#include "widget.h"

/**
  *@author Gabor Torok
  */

Widget::Widget(int x, int y, int w, int h) {
  move(x, y);
  this->w = w;
  this->h = h;
  setColor( 0.05f, 0.05f, 0.05f, 1 );
  setBackground( 1, 0.75f, 0.45f );
  //  setSelectionColor( 1, 0.5f, 0.45f );
  setSelectionColor( 0.75f, 0.75f, 0.8f );
  setBorderColor( 0.8f, 0.5f, 0.2f );
  setHighlightedBorderColor( 1.0f, 0.8f, 0.4f );
  visible = true;
  debug = false;
}

Widget::~Widget() {
}

void Widget::draw(Widget *parent) {
  glTranslated( x, y, 0 );
  drawWidget(parent);
}

bool Widget::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  // do nothing by default
  return false;
}

void Widget::removeEffects(Widget *parent) {
}

bool Widget::isInside(int x, int y) {
  return(x >= getX() && x < getX() + w &&
  		  y >= getY() && y < getY() + h);
}
