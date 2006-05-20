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
#include "gui/gui.h"
#include "freetype/FreeType.h"

class Widget;
class GameAdapter;                          
class SDLEventHandler;
class SDLScreenView;
class Preferences;
class Sound;

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

class SDLHandler : public ScourgeGui {
private:
  GameAdapter *gameAdapter;

  /* These are to calculate our fps */
  Sound *sound;
  GLint T0, Frames;
  double fps;
  SDL_Surface *screen;
  SDLEventHandler *eventHandler;
  SDLScreenView *screenView;
  /* Flags to pass to SDL_SetVideoMode */
  int videoFlags;
  bool invertMouse;
  int cursorMode;

  // rotation for test draw view
  GLfloat rtri, rquad;
  int lastWidth, lastHeight;

  SDLEventHandler *eventHandlers[10];
  SDLScreenView *screenViews[10];
  int handlerCount;

  // the last event fired by a widget
  Widget *storedWidget;
  SDL_Event *storedEvent; 
  
  // truetype font  
  freetype_font_data font, uiFont, monoFont, largeFont;
  bool font_initialized;
  int fontType;

  char *debugStr;

  bool attackCursor;

  // only do stencil buffer ops if this is true
  static bool stencilBufferUsed;
  
  // mouse locking
  Widget *mouseLock;
  bool willUnlockMouse;

  bool willBlockEvent;

  Uint32 forbiddenTimer;

  float fadeoutStartAlpha, fadeoutEndAlpha;
  Uint32 fadeoutTimer;
  int fadeoutSteps, fadeoutCurrentStep;

public: 

	static char NORMAL_FONT_NAME[255];
    static char UI_FONT_NAME[255];
	static char FIXED_FONT_NAME[255];
	static char LARGE_FONT_NAME[255];
	static int NORMAL_FONT_SIZE;
    static int UI_FONT_SIZE;
	static int FIXED_FONT_SIZE;
	static int LARGE_FONT_SIZE;

  static bool showDebugInfo;

  bool dontUpdateScreen;

  SDLHandler( GameAdapter *gameAdapter );
  virtual ~SDLHandler();

  void fade( float startAlpha, float endAlpha, int steps = 50 );

  inline void blockEvent() { willBlockEvent = true; }

  inline void lockMouse( Widget *widget ) { mouseLock = widget; }
  inline void unlockMouse() { willUnlockMouse = true; }

  void drawTooltip( float xpos2, float ypos2, float zpos2, 
                    float zrot, float yrot, 
                    char *message,
                    float r=0, float g=0.15f, float b=0.05f );

  inline void setDebugStr(char *s) { debugStr = s; }
  
  Uint16 mouseX, mouseY, lastMouseX, lastMouseY;
  Uint16 mouseFocusX, mouseFocusY;
  Uint8 mouseButton, mouseEvent;
  bool mouseDragging;
  bool mouseIsMovingOverMap;
  Uint32 lastMouseMoveTime;
  Uint32 lastLeftClick;
  bool isDoubleClick;

  // for ScourgeGui
  inline Uint16 getMouseX() { return mouseX; }
  inline Uint16 getMouseY() { return mouseY; }
  void playSound( const char *name );
  inline int getScreenWidth() { return getScreen()->w; }
  inline int getScreenHeight() { return getScreen()->h; }
  GLuint getHighlightTexture();
	GLuint getGuiTexture();
	GLuint getGuiTexture2();
  GLuint loadSystemTexture( char *line );
  void allWindowsClosed();

  void setOrthoView();

  inline Sound *getSound() { return sound; }

  void setCursorMode(int n, bool useTimer=false );
  int getCursorMode() { return cursorMode; }
  void applyMouseOffset(int x, int y, int *newX, int *newY);

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
   
  void setVideoMode(Preferences *uc);
  char ** getVideoModes(int &nbModes);
  void mainLoop();
  void drawScreen();
  bool processEvents( bool *isActive=NULL );
  void processEventsAndRepaint();
  void fireEvent(Widget *widget, SDL_Event *event);
  bool firedEventWaiting();

  inline void setFontType( int fontType ) { this->fontType = fontType; }
  inline int getFontType() { return fontType; }
  void texPrint(GLfloat x, GLfloat y, const char *fmt, ...);
  int textWidth( const char *fmt, ... );
  const freetype_font_data *getCurrentFont();
  void initFonts();

  GLvoid glPrint( const char *fmt, ... );

  static bool intersects( int x, int y, int w, int h,
                          int x2, int y2, int w2, int h2 );
  static bool intersects(SDL_Rect *r1, SDL_Rect *r2);
  static bool sectionIntersects(int a1, int a2, int b1, int b2);


  inline double getFps() { return fps; }

  inline SDL_Surface *getScreen() { return screen; }  

  void testDrawView();

  void quit( int returnCode );

  inline double getFPS() { return fps; }

protected:
    bool popHandlers();
    int resizeWindow( int width, int height );
    int initGL( GLvoid );  
    GLvoid buildFont( GLvoid );
    void drawCursor();
};

#endif

