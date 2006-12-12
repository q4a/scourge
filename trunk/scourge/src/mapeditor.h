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
#include "common/constants.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "text.h"
#include "gui/window.h"
#include "gui/label.h"
#include "gui/button.h"
#include "gui/scrollinglabel.h"
#include "gui/scrollinglist.h"
#include "minimap.h"
#include <vector>
#include <map>

/**
  *@author Gabor Torok
  */

class Scourge;
class MapSettings;
class Creature;
class MapWidget;
class Item;
class Monster;
class GLShape;
class MiniMap;

class MapEditor : public SDLEventHandler, SDLScreenView {
private:
  Scourge *scourge;
  MapSettings *mapSettings;

  // map settings
  int level, depth;

  
  // UI
  Window *mainWin;
  Button *doneButton;
  Button *wallButton;
  Button *doorButton;
  Button *eraseButton;
  Button *shapeButton;
  Button *itemButton;
  Button *creatureButton;
  Button *startPosButton;
  Button *floorType;
  Button *mapButton;
  std::vector<Button *> toggleButtonList;
                                                                                             
  TextField *nameText;
  Button *loadButton, *saveButton;
  Button *newButton;

  // new map gui
  Window *newMapWin;
  Button *cancelButton;
  Button *okButton;
  Button *applyButton;
  TextField *levelText, *depthText;
  ScrollingList *themeList;
  char **themeNames;
  MapWidget *mapWidget;
  
  // lists
  ScrollingList *shapeList, *itemList, *creatureList;
  char **shapeNames, **itemNames, **creatureNames;

  std::map<Monster*,GLShape*> creatureOutlines;

  MiniMap * miniMap;
  
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

protected:
  void processMouseMotion( Uint8 button, int editorZ );
  bool getShape( GLShape **shape,
                 Item **item = NULL,
                 Creature **creature = NULL );

  void addWall( Sint16 mapx, Sint16 mapy, int dir );
  void addDoor( Sint16 mapx, Sint16 mapy, int dir );
  void removeWall( Sint16 mapx, Sint16 mapy, int dir );

  void addEWDoor( Sint16 mapx, Sint16 mapy, int dir );
  void addNSDoor( Sint16 mapx, Sint16 mapy, int dir );
  void addEWWall( Sint16 mapx, Sint16 mapy, int dir );
  void addNSWall( Sint16 mapx, Sint16 mapy, int dir );
  void removeEWWall( Sint16 mapx, Sint16 mapy, int dir );
  void removeNSWall( Sint16 mapx, Sint16 mapy, int dir );
  void addFloor( Sint16 mapx, Sint16 mapy );
  void removeFloor( Sint16 mapx, Sint16 mapy );
  void blendCorners( Sint16 mapx, Sint16 mapy );
  bool isShape( Sint16 mapx, Sint16 mapy, Sint16 mapz, const char *name );
};

#endif
