/***************************************************************************
                          window.h  -  description
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

#ifndef WINDOW_H
#define WINDOW_H

#include "../constants.h"
#include "../sdlhandler.h"
#include "widget.h"

/**
  *@author Gabor Torok
  */

class SDLHandler;
class Widget;
  
class Window {
 private:
  // the image is 510x270
  static const int TILE_W=510 / 2;
  static const int TILE_H=270 / 2; 

  static const int MAX_WINDOW = 100;
  static const int MAX_WIDGET = 100;
  static const int TOP_HEIGHT = 20;
  static const int BOTTOM_HEIGHT = 5;

  int x, y, w, h;
  bool visible;
  char *title;
  GLuint texture;
  SDLHandler *sdlHandler;
  Widget *widget[MAX_WIDGET];
  int widgetCount;
  bool dragging;
  int dragX, dragY;

  static Window *window[];
  static int windowCount;

 public: 
  Window(SDLHandler *sdlHandler, int x, int y, int w, int h, const char *title, GLuint texture);
  ~Window();

  inline int getX() { return x; }
  inline int getY() { return y; }
  inline int getWidth() { return w; }
  inline int getHeight() { return h; }
  inline bool isVisible() { return visible; }
  inline void setVisible(bool b) { visible = b; }
  inline void move(int x, int y) { this->x = x; this->y = y; }
  inline void resize(int w, int h) { this->w = w; this->h = h; }
  void applyBorderColor();
  void applyBackgroundColor(bool opaque=false);
  inline SDLHandler *getSDLHandler() { return sdlHandler; }
  void draw();
  
  void addWidget(Widget *widget);
  //  void removeWidget(Widget *widget);
  void handleWindowEvent(SDL_Event *event, int x, int y);
  bool canHandle(SDL_Event *event, int x, int y);
  void handleEvent(SDL_Event *event, int x, int y);

  // window management
  static void drawVisibleWindows();
  static void addWindow(Window *win);
  static void removeWindow(Window *win);
  static void delegateEvent(SDL_Event *event, int x, int y);
  
	
 protected:
};

#endif

