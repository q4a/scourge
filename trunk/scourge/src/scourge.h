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
#include "calendar.h"
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
#include "rpg/spell.h"
#include "gui/window.h"
#include "gui/button.h"
#include "userconfiguration.h"
#include "effect.h"
#include "containergui.h"
#include "board.h"
#include "battle.h"
#include "party.h"
#include "projectile.h"

using namespace std;

class Creature;
class Calendar;
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
class ContainerGui;
class Board;
class Battle;
class Party;
class Projectile;
class Mission;

/**
  *@author Gabor Torok
  */

#define IMAGES_DIR "images/"
#define RESOURCES_DIR "resources/"
#define DEFAULT_IMAGES_DIR "default/"
#define CREATURES_DIR "creatures/"
#define MAX_BATTLE_COUNT 200

class Scourge : public SDLEventHandler,SDLScreenView {
 private:
  Party *party;
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
  MainMenu *mainMenu;
  OptionsMenu *optionsMenu;
  bool isInfoShowing;
  bool info_dialog_showing;

  Board *board;
  int nextMission;
  bool inHq;
  Mission *currentMission;
  bool missionWillAwardExpPoints;

  char infoMessage[200];

  Inventory *inventory;

  Window *messageWin, *exitConfirmationDialog;
  Label *exitLabel;
  ScrollingList *messageList;

  Button *yesExitConfirm, *noExitConfirm;

  int movingX, movingY, movingZ;
  Item *movingItem;

  Uint16 move;

  GLint lastTick, lastProjectileTick;
  int battleCount;
  Battle *battle[MAX_BATTLE_COUNT];  

  // multi-story levels
  int currentStory, oldStory;
  bool changingStory;

  static const int MAX_CONTAINER_GUI = 100;
  int containerGuiCount;
  ContainerGui *containerGui[MAX_CONTAINER_GUI];

  GLint dragStartTime;
  static const int ACTION_CLICK_TIME = 200;

  int lastMapX, lastMapY, lastMapZ, lastX, lastY;
  // how many pixels to wait between sampling 3d coordinates 
  // when dragging items (the more the faster)
  static const int POSITION_SAMPLE_DELTA = 10; 

  bool teleporting;
  
  vector<Battle *> battleRound;
  int battleTurn;

  bool mouseMoveScreen;
  Creature *targetSelectionFor;

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
  bool useBoard(Location *pos);
  bool useTeleporter(Location *pos);
  bool useGate(Location *pos);

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

  inline Board *getBoard() { return board; }
  inline Mission *getCurrentMission() { return currentMission; }

  inline void setMove(Uint16 n) { move |= n; };  
  inline void removeMove(Uint16 n) { move &= (0xffff - n); }

  inline int getCreatureCount() { return creatureCount; }
  inline Creature *getCreature(int index) { return creatures[index]; }

  /**
	 This method is called after the player decides what to do this round.
	 This method will move monsters, cast spells, organize the battle, etc.
   */
  void playRound();
 
  inline Party *getParty() { return party; }
  inline Map *getMap() { return map; }
  inline MiniMap *getMiniMap() { return miniMap; }

  Item *newItem(RpgItem *rpgItem, Spell *spell=NULL);
  Creature *newCreature(Character *character, char *name);
  Creature *newCreature(Monster *monster);

  // drop an item from inventory
  void setMovingItem(Item *item, int x, int y, int z); 

  inline Item *getMovingItem() { return movingItem; }

  inline MainMenu *getMainMenu() { return mainMenu; }

  inline OptionsMenu *getOptionsMenu() { return optionsMenu; }

  inline Inventory *getInventory() { return inventory; }
  
  void drawView();
  void drawAfter();

  bool handleEvent(SDL_Event *event);
  bool handleEvent(Widget *widget, SDL_Event *event);
   
  void addGameSpeed(int speedFactor);
  
  void startItemDragFromGui(Item *item);
  bool startItemDrag(int x, int y, int z);
  void endItemDrag();
  bool useItem();
  bool useItem(int x, int y, int z);
  
  void startMission();  
  void endMission();

  inline ShapePalette *getShapePalette() { return shapePal; }  

  inline SDLHandler *getSDLHandler() { return sdlHandler; }
  
  inline UserConfiguration * getUserConfiguration() { return userConfiguration; }
  
  void drawTopWindow();

  void openContainerGui(Item *container);

  void closeContainerGui(ContainerGui *gui);

  void closeAllContainerGuis();
  
  void creatureDeath(Creature *creature);

  void showMessageDialog(char *message);

  void setFormation(int formation);
  
  void toggleInventoryWindow();

  void toggleOptionsWindow();

  void showExitConfirmationDialog();

  Window *createWoodWindow(int x, int y, int w, int h, char *title);
  
  void missionCompleted();

  inline void setTargetSelectionFor(Creature *c) { targetSelectionFor = c; sdlHandler->setCursorMode(targetSelectionFor ? SDLHandler::CURSOR_CROSSHAIR : SDLHandler::CURSOR_NORMAL); }
  inline Creature *getTargetSelectionFor() { return targetSelectionFor; }

  /** 
	  Return the closest live player within the given radius or null if none can be found.
  */
  Creature *getClosestVisibleMonster(int x, int y, int w, int h, int radius);

 protected:
  //  void fightBattle(); 

  void decodeName(int name, Uint16* mapx, Uint16* mapy, Uint16* mapz);
  void createUI();
  // change the player's selX,selY values as specified by keyboard movement
  void handleKeyboardMovement();
  // move a creature
  void moveMonster(Creature *monster);

  void refreshContainerGui(Item *container);

};

#endif
