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
  bool visible;
  Color color;
  Color background;
  Color selColor;
  Color borderColor;
  Color borderColor2;
  Color focusColor;
  bool focus;

 public: 
  Widget(int x, int y, int w, int h);
  virtual ~Widget();
  virtual void draw(Widget *parent);

  virtual inline int getX() { return x; }
  virtual inline int getY() { return y; }
  virtual inline int getWidth() { return w; }
  virtual inline int getHeight() { return h; }

  virtual inline void move(int x, int y) { this->x = x; this->y = y; }
  virtual inline void resize(int w, int h) { this->w = w; this->h = h; }

  virtual inline void setVisible(bool b) { visible = b; }
  virtual inline bool isVisible() { return visible; }

  virtual void drawWidget(Widget *parent) = 0;

  virtual inline bool hasFocus() { return focus; }
  virtual inline void setFocus(bool b) { focus = b; }
  virtual inline bool canGetFocus() { return true; }


  // color
  inline void setColor( float r, float g, float b, float a=1.0f ) { this->color.r = r; this->color.g = g; this->color.b = b; this->color.a = a; }
  inline void setBorderColor( float r, float g, float b, float a=1.0f ) { this->borderColor.r = r; this->borderColor.g = g; this->borderColor.b = b; this->borderColor.a = a; }
  inline void setHighlightedBorderColor( float r, float g, float b, float a=1.0f ) { this->borderColor2.r = r; this->borderColor2.g = g; this->borderColor2.b = b; this->borderColor2.a = a; }
  inline void setBackground(float r, float g, float b, float a=1.0f) { background.r = r; background.g = g; background.b = b; background.a = a; }
  inline void setSelectionColor(float r, float g, float b, float a=1.0f) { selColor.r = r; selColor.g = g; selColor.b = b; selColor.a=a; }
  inline void setColor( Color *c ) { this->color.r = c->r; this->color.g = c->g; this->color.b = c->b; this->color.a = c->a; }
  inline void setBorderColor( Color *c ) { this->borderColor.r = c->r; this->borderColor.g = c->g; this->borderColor.b = c->b; this->borderColor.a = c->a; }
  inline void setBackground( Color *c) { background.r = c->r; background.g = c->g; background.b = c->b; background.a = c->a; }
  inline void setSelectionColor( Color *c ) { selColor.r = c->r; selColor.g = c->g; selColor.b = c->b; selColor.a=c->a; }
  inline void setFocusColor( float r, float g, float b, float a=1.0f ) { this->focusColor.r = r; this->focusColor.g = g; this->focusColor.b = b; this->focusColor.a = a; }
  inline void setFocusColor( Color *c ) { focusColor.r = c->r; focusColor.g = c->g; focusColor.b = c->b; focusColor.a=c->a; }

  inline void applyColor() { glColor4f( color.r, color.g, color.b, color.a ); }
  inline void applyBorderColor() { glColor4f(borderColor.r, borderColor.g, borderColor.b, borderColor.a); }
  inline void applyHighlightedBorderColor() { glColor4f(borderColor2.r, borderColor2.g, borderColor2.b, borderColor2.a); }
  inline void applyBackgroundColor(bool opaque=false) { glColor4f( background.r, background.g, background.b, (opaque ? 1.0f : 0.85f) ); }
  inline void applySelectionColor() { glColor4f( selColor.r, selColor.g, selColor.b, selColor.a ); }
  inline void applyFocusColor() { glColor4f( focusColor.r, focusColor.g, focusColor.b, focusColor.a ); }

  inline Color *getColor() { return &color; }
  inline Color *getBorderColor() { return &borderColor; }
  inline Color *getHighlightedBorderColor() { return &borderColor2; }
  inline Color *getBackgroundColor() { return &background; }
  inline Color * getSelectionColor() { return &selColor; }

  inline bool isTranslucent() { return (background.a < 1.0f); }

  /**
	 Return true, if the event activated this widget. (For example, button push, etc.)
	 Another way to think about it is that if true, the widget fires an "activated" event
	 to the outside world.
   */
  virtual bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);

  /**
   * Called when no events are sent to this widget. (ie. the mouse is elsewhere.)
   */
  virtual void removeEffects(Widget *parent);
  virtual bool isInside(int x, int y);

	inline void setDebug(bool d) { debug = d; }

 protected:
	 bool debug;
};

#endif

