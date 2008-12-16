/***************************************************************************
                          game.h  -  description
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
#ifndef GAME_H
#define GAME_H 
 
#include <constants.h>
#include <render/renderlib.h>
#include <preferences.h>

class Game {
private:
  Shapes *shapes;
  Map *levelMap;

public:
  Game( Preferences *preferences, MapAdapter *adapter );
  ~Game();

  // Methods called from mainloop
  void drawView();
  void handleEvent( SDL_Event *event );

protected:
  void createMap();
};

#endif
