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
  
class Window : public Widget {
 private:
  // the image is 510x270
  static const int TILE_W=510 / 2;
  static const int TILE_H=270 / 2; 

  static const int MAX_WINDOW = 100;
  static const int MAX_WIDGET = 100;

  char *title;
  GLuint texture;
  SDLHandler *sdlHandler;
  Widget *widget[MAX_WIDGET];
  int widgetCount;
  bool dragging;
  int dragX, dragY;
  int openHeight;
  GLint lastTick;
  int z;

  static Window *window[];
  static int windowCount;

 public: 

  static const int TOP_HEIGHT = 20;
  static const int BOTTOM_HEIGHT = 5;

  Window(SDLHandler *sdlHandler, int x, int y, int w, int h, const char *title, GLuint texture);
  ~Window();

  inline void setZ(int z) { this->z = z; }
  inline int getZ() { return z; }

  void setVisible(bool b);
  inline SDLHandler *getSDLHandler() { return sdlHandler; }

  // crop view to window area. Don't forget to call glDisable( GL_SCISSOR_TEST ) after!
  void scissorToWindow();
  
  void addWidget(Widget *widget);
  void removeWidget(Widget *widget);
  Widget *handleWindowEvent(SDL_Event *event, int x, int y);

  // from Widget
  void drawWidget(Widget *parent);
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);
  bool isInside(int x, int y);

  // window management
  static void drawVisibleWindows();
  static void addWindow(Window *win);
  static void removeWindow(Window *win);
  static Widget *delegateEvent(SDL_Event *event, int x, int y);
  
	
 protected:
};

#endif

