/***************************************************************************
                          sdlhandler.h  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#ifndef SDLHANDLER_H
#define SDLHANDLER_H

#include <iostream>
#include <stdlib.h>
#include <stdarg.h>
#include "constants.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "shapepalette.h"
#include "text.h"
#include "gui/window.h"
#include "userconfiguration.h"
#include "glut/glutpart.h"

class TexturedText;

/**
  *@author Gabor Torok
  */

/**
  How many measurements to take to average the fps.
*/
#define MAX_TICK_COUNT 20

/* screen width, height, and bit depth */
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP     16

/* Define our booleans */
#define TRUE  1
#define FALSE 0

class ShapePalette;                          
class SDLEventHandler;
class UserConfiguration;

class SDLHandler {
private:
  /* These are to calculate our fps */
  GLint T0, Frames;
  double fps;
  SDL_Surface *screen;
  SDLEventHandler *eventHandler;
  SDLScreenView *screenView;
  /* Flags to pass to SDL_SetVideoMode */
  int videoFlags;
  ShapePalette *shapePal;
  bool invertMouse;
  int cursorMode;

  // rotation for test draw view
  GLfloat rtri, rquad;
  int lastWidth, lastHeight;

  TexturedText *text;

  SDLEventHandler *eventHandlers[10];
  SDLScreenView *screenViews[10];
  int handlerCount;

  // the last event fired by a widget
  Widget *storedWidget;
  SDL_Event *storedEvent; 

 public: 

  enum {
	CURSOR_NORMAL=0,
	CURSOR_CROSSHAIR
  };  

  SDLHandler();
  ~SDLHandler();
  
  Uint16 mouseX, mouseY;
  Uint8 mouseButton, mouseEvent;
  bool mouseDragging;

  void setOrthoView();

  void setCursorMode(int n) { cursorMode = n; }
  int getCursorMode() { return cursorMode; }
  inline void applyMouseOffset(int x, int y, int *newX, int *newY) {
	if(cursorMode == CURSOR_NORMAL) {
	  *newX = x;
	  *newY = y;
	} else {
	  *newX = x + 24;
	  *newY = y + 24;
	}
  }

  /**
   * Add a new set of handlers and push the old ones on the stack.
   * When the eventHandler returns true, the stack will be popped.
   * If the stack is empty the game will quit.
   */                                                            
  void pushHandlers(SDLEventHandler *eventHandler, SDLScreenView *screenView);

  /**
   * Replace the current handlers with the new ones
   */
  void setHandlers(SDLEventHandler *eventHandler, SDLScreenView *screenView);

  /**
	 Get the current event handler.
  */
  inline SDLEventHandler *getEventHandler() { return eventHandler; }
   
  void setVideoMode(UserConfiguration *uc);
  char ** getVideoModes(int &nbModes);
  void mainLoop();
  void fireEvent(Widget *widget, SDL_Event *event);
  bool firedEventWaiting();

  void texPrint(GLfloat x, GLfloat y, const char *fmt, ...);

  GLvoid glPrint( const char *fmt, ... );

  static bool intersects(SDL_Rect *r1, SDL_Rect *r2);
  static bool sectionIntersects(int a1, int a2, int b1, int b2);

  inline double getFps() { return fps; }

  inline SDL_Surface *getScreen() { return screen; }  

  void testDrawView();

  inline ShapePalette *getShapePalette() { return shapePal; }
 
  void quit( int returnCode );

  inline double getFPS() { return fps; }

protected:
    bool popHandlers();
    int resizeWindow( int width, int height );
    int initGL( GLvoid );  
    GLvoid buildFont( GLvoid );
};

#endif
