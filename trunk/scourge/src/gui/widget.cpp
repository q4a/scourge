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

Widget::Widget(int x, int y) {
  this->x = x;
  this->y = y;
  red = green = blue = alpha = 1.0f;
}
Widget::~Widget() {
}

void Widget::draw(SDLHandler *sdlHandler) {
  glTranslated( x, y, 0 );
  drawWidget(sdlHandler);
}

void Widget::handleEvent(SDLHandler *sdlHandler, SDL_Event *event, int x, int y) {
  // do nothing by default
}

bool Widget::canHandle(SDLHandler *sdlHandler, SDL_Event *event, int x, int y) {
  return false;
}
