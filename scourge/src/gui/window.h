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
#include "button.h"
#include "label.h"
#include "checkbox.h"
#include "textfield.h"
#include "guitheme.h"

/**
  *@author Gabor Torok
  */

class SDLHandler;
class Widget;
class Button;
class Label;
class Checkbox;
class TextField;

class Window : public Widget {
 private:
  // the image is 510x270
  static const int TILE_W=510 / 2;
  static const int TILE_H=270 / 2; 

  static const int MAX_WINDOW = 1000;
  static const int MAX_WIDGET = 1000;

  char title[255];
  GLuint texture, texture2;
  SDLHandler *sdlHandler;
  Widget *widget[MAX_WIDGET];
  int tileWidth, tileHeight;
  int widgetCount;
  bool dragging;
  int dragX, dragY;
  int openHeight, currentY;
  GLint lastTick;
  int z;  
  bool modal;
  int type;
  bool locked;
  GuiTheme *theme;

  static Window *window[];
  static int windowCount;  

  static Window *message_dialog;
  static Label *message_label;
  static Window *currentWin;

  Widget *lastWidget;
  int animation;

 public: 

  enum {
    DEFAULT_ANIMATION=0,
    SLIDE_UP
  };
   
   static const char ROLL_OVER_SOUND[80];
   static const char ACTION_SOUND[80];
   static const char DROP_SUCCESS[80];
   static const char DROP_FAILED[80];
   
   static bool windowWasClosed;
   
   Button *closeButton;
   
   enum {
     BASIC_WINDOW=0,
     SIMPLE_WINDOW
   };

  static const int TOP_HEIGHT = 20;
  static const int BOTTOM_HEIGHT = 5;
  static const int SCREEN_GUTTER = 5;

  Window(SDLHandler *sdlHandler, int x, int y, int w, int h, char *title=NULL, 
         GLuint texture=0, bool hasCloseButton=true, int type=BASIC_WINDOW, 
         GLuint texture2=0);

  Window( SDLHandler *sdlHandler, int x, int y, int w, int h, char *title=NULL, 
		  bool hasCloseButton=true, int type=BASIC_WINDOW, const char *themeName=NULL );

  ~Window();

  inline void setAnimation( int a ) { animation = a; }
  inline int getAnimation() { return animation; }

  inline GuiTheme *getTheme() { return theme; }

  inline void setTitle(char *s) { strncpy(title, ( s ? s : "" ), 255); title[254] = '\0'; }

  inline bool isOpening() { 
    return ( openHeight < (h - (TOP_HEIGHT + BOTTOM_HEIGHT)) ||
             currentY != y ); 
  }

  inline void setBackgroundTileWidth(int n) { tileWidth = n; }
  inline void setBackgroundTileHeight(int n) { tileHeight = n; }
  
  inline void setZ(int z) { this->z = z; }
  inline int getZ() { return z; }

  inline void setModal(bool b) { modal = b; }
  inline bool isModal() { return modal; }

  void setVisible(bool b, bool animate=true);
  inline SDLHandler *getSDLHandler() { return sdlHandler; }
  void toTop();
  void toBottom();

  inline void setLocked(bool locked) { this->locked = locked; if(locked) toBottom(); }
  inline bool isLocked() { return locked; }

  // crop view to window area. Don't forget to call glDisable( GL_SCISSOR_TEST ) after!
  void scissorToWindow();
  
  // widget managment functions
  Button    * createButton(int x1, int y1, int x2, int y2, char *label, bool toggle=false);    
  Label     * createLabel(int x1, int x2, char * label, int color=Constants::DEFAULT_COLOR); 
  Checkbox  * createCheckbox(int x1, int y1, int x2, int y2, char *label);  
  TextField * createTextField(int x, int y, int numChars);
  void addWidget(Widget *widget);
  void removeWidget(Widget *widget);
  Widget *handleWindowEvent(SDL_Event *event, int x, int y);
  void setFocus(Widget *w);
  void nextFocus();
  void prevFocus();

  // from Widget
  void drawWidget(Widget *parent);
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);
  bool isInside(int x, int y);
  
  
  // window management
  static void drawVisibleWindows();
  static void addWindow(Window *win);
  static void removeWindow(Window *win);
  static Widget *delegateEvent(SDL_Event *event, int x, int y);
  static void toTop(Window *win);
  static void toBottom(Window *win);
  static void nextWindowToTop();
  static void prevWindowToTop();

  // static message dialog
  static Button *message_button; // so you can check for it in other classes
  static void showMessageDialog(SDLHandler *sdlHandler, 
  int x, int y, int w, int h, 
  char *title, GLuint texture,
  char *message, 
  char *buttonLabel = Constants::messages[Constants::OK_LABEL][0]);

  void move(int x, int y);

  void setLastWidget(Widget *w);
 protected:
  void commonInit(SDLHandler *sdlHandler, int x, int y, int w, int h, char *title, bool hasCloseButton, int type);
};

#endif

