/***************************************************************************
                          options.h  -  description
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

#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include "constants.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"

/**
  *@author Gabor Torok
  */

class Scourge;

class OptionsMenu : public SDLEventHandler,SDLScreenView {
private:
  Scourge *scourge;
  int win;
  bool showDebug;
  
public:
  OptionsMenu(Scourge *scourge);
  ~OptionsMenu();

  void drawView(SDL_Surface *screen);
  bool handleEvent(SDL_Event *event);
  void drawMenu(int x, int y);

  void show();

 protected:
};

#endif
