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

/**
  *@author Gabor Torok
  */

class SDLHandler;

class Widget {
 protected:
  int x, y, w, h;
  float red, green, blue, alpha;
  bool visible;

 public: 
  Widget(int x, int y, int w, int h);
  virtual ~Widget();
  void draw(Widget *parent);

  inline int getX() { return x; }
  inline int getY() { return y; }
  inline int getWidth() { return w; }
  inline int getHeight() { return h; }

  inline void move(int x, int y) { this->x = x; this->y = y; }
  inline void resize(int w, int h) { this->w = w; this->h = h; }

  virtual inline void setVisible(bool b) { visible = b; }
  virtual inline bool isVisible() { return visible; }

  virtual void drawWidget(Widget *parent) = 0;
  inline void setColor( float r, float g, float b, float a ) { this->red = r; this->green = g; this->blue = b; this->alpha = a; }

  /**
	 Return true, if the event activated this widget. (For example, button push, etc.)
	 Another way to think about it is that if true, the widget fires an "activated" event
	 to the outside world.
   */
  virtual bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);
  virtual bool isInside(int x, int y);

 protected:
  inline void applyColor() { glColor4f( red, green, blue, alpha ); }
};

#endif

