
/***************************************************************************
                          multiplayer.h  -  description
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

#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include "constants.h"
#include "scourge.h"
#include "gui/window.h"
#include "gui/label.h"
#include "gui/button.h"

/**
  *@author Gabor Torok
  */

class Scourge;

class MultiplayerDialog {
private:
  Scourge *scourge;
  Window *mainWin;
  Button *startServer;
  Button *joinServer;
  
public:
  MultiplayerDialog(Scourge *scourge);
  ~MultiplayerDialog();

  inline void show() { mainWin->setVisible(true); }
  inline void hide() { mainWin->setVisible(false); }
  inline bool isVisible() { return mainWin->isVisible(); }

  bool handleEvent(SDL_Event *event);
  bool handleEvent(Widget *widget, SDL_Event *event);
};

#endif
