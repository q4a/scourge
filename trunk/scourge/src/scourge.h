/***************************************************************************
                          scourge.h  -  description
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

#ifndef SCOURGE_H
#define SCOURGE_H

#include <iostream>
#include "constants.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "map.h"
#include "dungeongenerator.h"
#include "creature.h"
#include "mainmenu.h"
#include "gui.h"
#include "inventory.h"
#include "item.h"

using namespace std;

class Creature;
class Map;
class ShapePalette;
class Location;
class MainMenu;
class Gui;
class Inventory;

/**
  *@author Gabor Torok
  */

#define BASE_PATH "/home/gabor/sourceforge/woodward/scourge/"
#define IMAGES_DIR "images/"
#define RESOURCES_DIR "resources/"
#define DEFAULT_IMAGES_DIR "default/"
#define CREATURES_DIR "creatures/"

class Scourge : public SDLEventHandler,SDLScreenView {
private:
	Creature *player;
	Creature *party[4];
	int formation;
  MainMenu *mainMenu;
  Gui *gui;
  bool isInfoShowing;
  int topWin;
  Inventory *inventory;

  int movingX, movingY, movingZ;
  Item *movingItem;

  bool player_only;
  
protected:
  SDLHandler *sdlHandler;
  Map *map;
  ShapePalette *shapePal;

  void processGameMouseClick(Uint16 x, Uint16 y, Uint8 button);
  void getMapXYZAtScreenXY(Uint16 x, Uint16 y, Uint16 *mapx, Uint16 *mapy, Uint16 *mapz);
  void getMapXYAtScreenXY(Uint16 x, Uint16 y, Uint16 *mapx, Uint16 *mapy);
  void processGameMouseMove(Uint16 x, Uint16 y);

  bool Scourge::getItem(Location *pos);
  void Scourge::dropItem(int x, int y);
  bool Scourge::useDoor(Location *pos);

public:
  #define SCOURGE_VERSION 0.01
  #define GUI_HEIGHT 100
  #define TOP_GUI_WIDTH 400
  #define TOP_GUI_HEIGHT 100
  #define GUI_PLAYER_INFO_W 250
  #define GUI_PLAYER_INFO_H 350
  
  static int blendA, blendB;
  static int blend[];
  static void setBlendFunc();
  
  Scourge(int argc, char *argv[]);
  ~Scourge();

  inline Item *getMovingItem() { return movingItem; }

  inline MainMenu *getMainMenu() { return mainMenu; }

  inline Gui *getGui() { return gui; }

  inline Inventory *getInventory() { return inventory; }

	inline Creature *getPlayer() { return player; }

  void drawView(SDL_Surface *screen);
  bool handleEvent(SDL_Event *event);

  void setPlayer(int n);

  void moveParty();
  
  void setPartyMotion(int motion);
  
  void setFormation(int formation);
  
  inline int getFormation() { return formation; }
  
  Creature *isPartyMember(Location *pos);
  
  bool useItem();
  bool useItem(int x, int y);
  
  void startMission();  

  inline Map *getMap() { return map; }

  inline ShapePalette *getShapePalette() { return shapePal; }  

  inline SDLHandler *getSDLHandler() { return sdlHandler; }

  inline Creature *getParty(int i) { return party[i]; }  

  void drawTopWindow();

protected:
    void decodeName(int name, Uint16* mapx, Uint16* mapy, Uint16* mapz);
};

#endif
