/***************************************************************************
                          widget.h  -  description
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

#ifndef WIDGET_H
#define WIDGET_H

#include "../constants.h"
#include "../sdlhandler.h"

/**
  *@author Gabor Torok
  */

class SDLHandler;

class Widget {
 protected:
  int x;
  int y;
  float red, green, blue, alpha;

 public: 
  Widget(int x, int y);
  virtual ~Widget();
  void draw(SDLHandler *sdlHandler);

  inline int getX() { return x; }
  inline int getY() { return y; }
  inline void move(int x, int y) { this->x = x; this->y = y; }
  virtual void drawWidget(SDLHandler *sdlHandler) = 0;
  inline void setColor( float r, float g, float b, float a ) { this->red = r; this->green = g; this->blue = b; this->alpha = a; }
  virtual void handleEvent(SDLHandler *sdlHandler, SDL_Event *event, int x, int y);
  virtual bool canHandle(SDLHandler *sdlHandler, SDL_Event *event, int x, int y);

 protected:
  inline void setGlColor() { glColor4f( red, green, blue, alpha ); }
};

#endif

