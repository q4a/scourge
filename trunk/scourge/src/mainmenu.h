/***************************************************************************
                          mainmenu.h  -  description
                             -------------------
    begin                : Tue Aug 12 2003
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

#ifndef MAINMENU_H
#define MAINMENU_H

#include "constants.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "text.h"

#include "gui/window.h"
#include "gui/label.h"
#include "gui/button.h"

/**
  *@author Gabor Torok
  */

class Scourge;

class MainMenu : public SDLEventHandler,SDLScreenView {
private:
  Scourge *scourge;
  int value;

  typedef struct _Cloud {
	int x, y, w, h, speed;
  } Cloud;
  Cloud cloud[100];
  int cloudCount;

  Window *mainWin;
  Button *newGameButton;
  Button *continueButton;
  Button *optionsButton;
  Button *aboutButton;
  Button *quitButton;
  
public:
#define NEW_GAME 1
#define CONTINUE_GAME 2
#define OPTIONS 3
#define ABOUT 4
#define QUIT 5

  MainMenu(Scourge *scourge);
  ~MainMenu();

  void drawView(SDL_Surface *screen);
  bool handleEvent(SDL_Event *event);
  bool handleEvent(Widget *widget, SDL_Event *event);
  int getValue();

  inline void show() { mainWin->setVisible(true); }
  inline void hide() { mainWin->setVisible(false); }
  inline bool isVisible() { return mainWin->isVisible(); }

 protected:
  void drawClouds(bool moveClouds, bool flipped);
  void drawWater();
};

#endif
