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
#include <string>
#include "constants.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "minimap.h"
#include "map.h"
#include "dungeongenerator.h"
#include "creature.h"
#include "mainmenu.h"
#include "optionsmenu.h"
#include "inventory.h"
#include "item.h"
#include "rpg/character.h"
#include "rpg/monster.h"
#include "gui/window.h"
#include "gui/button.h"
#include "userconfiguration.h"
#include "effect.h"

using namespace std;

class Creature;
class MiniMap;
class Map;
class DungeonGenerator;
class ShapePalette;
class Location;
class MainMenu;
class OptionsMenu;
class Inventory;
class UserConfiguration;
class Effect;
class DungeonGenerator;
class Window;

/**
  *@author Gabor Torok
  */

#define BASE_PATH "/home/gabor/sourceforge/woodward/scourge/"
#define IMAGES_DIR "images/"
#define RESOURCES_DIR "resources/"
#define DEFAULT_IMAGES_DIR "default/"
#define CREATURES_DIR "creatures/"
#define MAX_BATTLE_COUNT 200

typedef struct _Battle {
  Creature *creature;
} Battle;

class Scourge : public SDLEventHandler,SDLScreenView {
 private:
  Map *map;
  MiniMap * miniMap;
  UserConfiguration *userConfiguration;  
  DungeonGenerator *dg;
  Scourge *scourge;
  int level;
  Item *items[500];
  Creature *creatures[500];
  int itemCount;
  int creatureCount;
  Creature *player;
  Creature *party[4];
  int formation;
  MainMenu *mainMenu;
  OptionsMenu *optionsMenu;
  bool isInfoShowing;
  Window *mainWin;
  Button *inventoryButton;
  Button *optionsButton;
  Button *quitButton;
  Button *roundButton;

  Button *diamondButton;
  Button *staggeredButton;
  Button *squareButton;
  Button *rowButton;
  Button *scoutButton;
  Button *crossButton;
  Button *player1Button;
  Button *player2Button;
  Button *player3Button;
  Button *player4Button;
  Button *groupButton;

  Inventory *inventory;

  Window *messageWin, *exitConfirmationDialog;
  ScrollingList *messageList;

  Button *yesExitConfirm, *noExitConfirm;

  int movingX, movingY, movingZ;
  Item *movingItem;

  bool player_only;
  Uint16 move;
  bool startRound;

  GLint lastTick;
  int battleCount;
  Battle battle[MAX_BATTLE_COUNT];  
  bool partyDead;

protected:
  SDLHandler *sdlHandler;
  ShapePalette *shapePal;

  void processGameMouseDown(Uint16 x, Uint16 y, Uint8 button);
  void processGameMouseClick(Uint16 x, Uint16 y, Uint8 button);
  void getMapXYZAtScreenXY(Uint16 x, Uint16 y, Uint16 *mapx, Uint16 *mapy, Uint16 *mapz);
  void getMapXYAtScreenXY(Uint16 x, Uint16 y, Uint16 *mapx, Uint16 *mapy);
  void processGameMouseMove(Uint16 x, Uint16 y);

  bool getItem(Location *pos);
  void dropItem(int x, int y);
  bool useDoor(Location *pos);

public:
  #define TOP_GUI_WIDTH 400
  #define TOP_GUI_HEIGHT 100
  #define GUI_PLAYER_INFO_W 250
  #define GUI_PLAYER_INFO_H 350
  
  static int blendA, blendB;
  static int blend[];
  static void setBlendFunc();
  
  Scourge(int argc, char *argv[]);
  ~Scourge();

  inline void setMove(Uint16 n) { move |= n; };  
  inline void removeMove(Uint16 n) { move &= (0xffff - n); }

  /**
	 This method is called after the player decides what to do this round.
	 This method will move monsters, cast spells, organize the battle, etc.
   */
  void playRound();

  inline Map *getMap() { return map; }
  inline MiniMap *getMiniMap() { return miniMap; }

  Item *newItem(RpgItem *rpgItem);
  Creature *newCreature(Character *character, char *name);
  Creature *newCreature(Monster *monster);

  // drop an item from inventory
  void setMovingItem(Item *item, int x, int y, int z); 

  inline Item *getMovingItem() { return movingItem; }

  inline MainMenu *getMainMenu() { return mainMenu; }

  inline OptionsMenu *getOptionsMenu() { return optionsMenu; }

  inline Inventory *getInventory() { return inventory; }
  
  inline Creature *getPlayer() { return player; }

  void drawView(SDL_Surface *screen);
  bool handleEvent(SDL_Event *event);
  bool handleEvent(Widget *widget, SDL_Event *event);

  void setPlayer(int n);
  inline void setPlayer(Creature *c) { player = c; }
  
  void setPartyMotion(int motion);
  
  void setFormation(int formation);
  
  inline int getFormation() { return formation; }
  
  void addGameSpeed(int speedFactor);
  
  Creature *isPartyMember(Location *pos);
  
  bool startItemDrag(int x, int y);
  bool useItem();
  bool useItem(int x, int y);
  
  void startMission();  

  inline ShapePalette *getShapePalette() { return shapePal; }  

  inline SDLHandler *getSDLHandler() { return sdlHandler; }
  
  inline UserConfiguration * getUserConfiguration() { return userConfiguration; }

  inline Creature *getParty(int i) { return party[i]; }  

  void drawTopWindow();

 protected:
  void fightBattle();

  void creatureDeath(Creature *creature);

  void decodeName(int name, Uint16* mapx, Uint16* mapy, Uint16* mapz);
  void createUI();
  // change the player's selX,selY values as specified by keyboard movement
  void handleKeyboardMovement();
  // move the party
  void movePlayers();
  // move a creature
  void moveMonster(Creature *monster);

  void toggleRound();
  void togglePlayerOnly();

	// returns false if the switch could not be made,
	// because the entire party is dead (the mission failed)
	bool switchToNextLivePartyMember();

};

#endif
