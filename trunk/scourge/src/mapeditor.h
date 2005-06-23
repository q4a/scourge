/***************************************************************************
                          mapeditor.h  -  description
                             -------------------
    begin                : Tue Jun 18 2005
    copyright            : (C) 2005 by Gabor Torok
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

#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "constants.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "text.h"
#include "partyeditor.h"
#include "gui/window.h"
#include "gui/label.h"
#include "gui/button.h"
#include "gui/scrollinglabel.h"
#include <vector>
#include <map>

/**
  *@author Gabor Torok
  */

class Scourge;
class MapSettings;

using namespace std;

class MapEditor : public SDLEventHandler, SDLScreenView {
private:
  Scourge *scourge;
  MapSettings *mapSettings;
  
  // UI
  Window *mainWin;
  Button *doneButton;
  
  ScrollingList *shapeList;
  char **shapeNames;

  ScrollingList *itemList;
  char **itemNames;

  ScrollingList *creatureList;
  char **creatureNames;
  
public:

  MapEditor( Scourge *scourge );
  ~MapEditor();

  void drawView();
  void drawAfter();
  bool handleEvent(SDL_Event *event);
  bool handleEvent(Widget *widget, SDL_Event *event);

  void show();
  void hide();
  inline bool isVisible() { return mainWin->isVisible(); }

};

#endif

