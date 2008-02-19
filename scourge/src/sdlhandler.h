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

class Widget;
class GameAdapter;
class SDLEventHandler;
class SDLScreenView;
class Preferences;
class FontMgr;

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

	bool running;
	bool cursorVisible;

public: 

  struct FontInfo {
    std::string path;
    int size;
    int style;
    int yoffset;
		int shadowX, shadowY;

    TTF_Font *font;
    FontMgr *fontMgr;
  };
  static std::vector<FontInfo*> fontInfos;

  static bool showDebugInfo;

  bool dontUpdateScreen;

  SDLHandler( GameAdapter *gameAdapter );
  virtual ~SDLHandler();

	virtual void playSound( const std::string& file );

	inline void setCursorVisible( bool b ) { cursorVisible = b; }
	inline void endMainLoop() { running = false; }

  void fade( float startAlpha, float endAlpha, int steps = 50 );

  inline void blockEvent() { willBlockEvent = true; }

  inline void lockMouse( Widget *widget ) { mouseLock = widget; }
  inline void unlockMouse() { willUnlockMouse = true; }

  void drawTooltip( float xpos2, float ypos2, float zpos2, 
                    float zrot, float yrot, 
                    char *message,
                    float r=0, float g=0.15f, float b=0.05f,
										float zoom=1.0f );

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
  inline int getScreenWidth() { return getScreen()->w; }
  inline int getScreenHeight() { return getScreen()->h; }
  GLuint getHighlightTexture();
	GLuint getGuiTexture();
	GLuint getGuiTexture2();
  GLuint loadSystemTexture( char *line );
  void allWindowsClosed();

  void setOrthoView();

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
  TTF_Font *getCurrentTTFFont();
	FontMgr *getCurrentFontManager();
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

	void saveScreen( std::string& path );
	void saveScreenNow( std::string& path );

	void setUpdate( char *message, int n=-1, int total=-1 );

protected:	
	bool popHandlers();
	int resizeWindow( int width, int height );
	int initGL();  
	void drawCursor();

	void drawScreenInternal();
	void saveScreenInternal( std::string& path );
	void calculateFps();
	void drawDebugInfo();
	void drawFadeout();

};

#endif

