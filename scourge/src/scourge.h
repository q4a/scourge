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
#include "multiplayer.h"
#include "net/server.h"
#include "net/client.h"
#include "net/gamestatehandler.h"
#include "netplay.h"
#include "gui/progress.h"

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
class MultiplayerDialog;
#ifdef HAVE_SDL_NET
class Server;
class Client;
#endif
class GameStateHandler;
class NetPlay;
class Progress;

#define IMAGES_DIR "images/"
#define RESOURCES_DIR "resources/"
#define DEFAULT_IMAGES_DIR "default/"
#define CREATURES_DIR "creatures/"
#define MAX_BATTLE_COUNT 200

/** 
  This is the main class of the game. It is a central place to put
  references to other objects, like the party, minimap, etc.
  
  @author Gabor Torok
*/ 
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
  MultiplayerDialog *multiplayer;
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

  int layoutMode;
#ifdef HAVE_SDL_NET
  Server *server;
  Client *client;
#endif
  NetPlay *netPlay;
  bool multiplayerGame;

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
#define MINIMAP_WINDOW_WIDTH 200
#define MINIMAP_WINDOW_HEIGHT 150

  static const int PARTY_GUI_WIDTH=500;
  static const int PARTY_GUI_HEIGHT=165;
  static const int PARTY_MIN_GUI_WIDTH=100;
  static const int INVENTORY_WIDTH = 420;
  static const int INVENTORY_HEIGHT = 505;
  
  static int blendA, blendB;
  static int blend[];
  static void setBlendFunc();
  
  Scourge(int argc, char *argv[]);
  ~Scourge();

  /**
    @return the Board containing the available missions.
  */
  inline Board *getBoard() { return board; }
  
  /**
    @return the current mission.
  */
  inline Mission *getCurrentMission() { return currentMission; }

  /**
    Set which direction to move in.
    @param n is a bitfield. See constants for direction values.
  */
  inline void setMove(Uint16 n) { move |= n; };  
  
  /**
    Stop moving in the given direction(s).
    @param n is a bitfield. See constants for directions values.
  */
  inline void removeMove(Uint16 n) { move &= (0xffff - n); }

  /**
    @return the number of creatures on this story.
  */
  inline int getCreatureCount() { return creatureCount; }
  
  /**
    @return the creature a creature on this story.
  */
  inline Creature *getCreature(int index) { return creatures[index]; }

  /**
	  This method is called by the main loop to play a round. A round may consist of 
    a battle with multiple participants, someone drinking a potion, casting a spell, etc.
  */
  void playRound();
 
  /**
    @return the party object.
  */
  inline Party *getParty() { return party; }
  
  /**
    @return the map.
  */
  inline Map *getMap() { return map; }
  
  /**
    @return the MiniMap.
  */
  inline MiniMap *getMiniMap() { return miniMap; }

  /**
    Creat a new item for use on this story. Calling this method instead of new Item()
    directly ensures that the item will be cleaned up properly when the story is
    exited. Only items in a party member's inventory are not deleted.
    
    @param rpgItem if not NULL, the RpgItem template for the item to create.
    @param spell if not NULL, the spell to associate with the created scroll.
    @return the item created.
  */
  Item *newItem(RpgItem *rpgItem, Spell *spell=NULL);
  
  /**
    Create a new creature for use on this story. Calling this method instead of new Creature()
    directly ensures that the creature will be cleaned up properly when the story is
    exited. 
    
    @param character the character class to use for the new creature.
    @param name the name of the new creature
    @return the creature created.
  */
  Creature *newCreature(Character *character, char *name);
  
  /**
    Create a new creature for use on this story. Calling this method instead of new Creature()
    directly ensures that the creature will be cleaned up properly when the story is
    exited. 
    
    @param monster the character template to use for the new creature.
    @return the creature created.
  */
  Creature *newCreature(Monster *monster);

  /** 
    When dropping an item from the inventory this method sets up the parameters so
    the cursor can drag it around the screen and eventually deposit it at a location
    or in a container.
    
    @param item The item to drop
    @param x where to drop the item on the map
    @param y where to drop the item on the map
    @param z where to drop the item on the map
  */
  void setMovingItem(Item *item, int x, int y, int z); 

  /**
    @return the item currently being dragged by the cursor.
  */
  inline Item *getMovingItem() { return movingItem; }

  /**
    @return the main menu
  */
  inline MainMenu *getMainMenu() { return mainMenu; }

  /**
    @return the options menu
  */
  inline OptionsMenu *getOptionsMenu() { return optionsMenu; }

  /**                                                             
    @return the multiplayer dialog window
  */
  inline MultiplayerDialog *getMultiplayerDialog() { return multiplayer; }

  /**
    @return the inventory
  */
  inline Inventory *getInventory() { return inventory; }
  
  /**
    The main app loop calls this method to repaint the screen. In this implementation the 
    following happens: the round is played, the map is drawn, the map overlay (circles around
    the good guys, names of creatues, etc.) is painted and some extra updates to other components
    (like the minimap, message ui, etc.) occur.    
  */
  void drawView();
  
  /**
    The main app loop calls this after the drawView and the UI (windows) have been drawn.
    In this implementation, the dragged item is drawn over the map.
  */
  void drawAfter();

  /**
    Respond to keyboard and mouse events in this method.
    @param event the actual SDL_Event structure as captured by the main app loop.
    @return true to exit from the current screen, false otherwise
  */
  bool handleEvent(SDL_Event *event);
  
  /**
    Respond to UI (windowing) events in this method.
    @param widget The widget which fired the event (e.g.: button, list, etc.)
    @param event the actual SDL_Event structure as captured by the main app loop.
    @return true to exit from the current screen, false otherwise
  */
  bool handleEvent(Widget *widget, SDL_Event *event);
   
  /**
    Increase the game speed.
    @param speedFactor add this number to the current game speed.
  */
  void addGameSpeed(int speedFactor);
  
  /**
    Start to drag an item from a container gui.
    @param item the item to drag.
  */
  void startItemDragFromGui(Item *item);
  
  /**
    Start to drag an item from the map at a specific location.
    @param x the position of the item to drag.
    @param y the position of the item to drag.
    @param z the position of the item to drag.
  */
  bool startItemDrag(int x, int y, int z);
  
  /**
    Stop dragging an item.
  */
  void endItemDrag();
  
  /**
    Use the first item in a 2 tile radius around the current party leader.
  */
  bool useItem();
  
  /**
    Use the item at the given location. This method also selects targets for pending spells.
    Depending on the type of the item, further methods are called. 
    (e.g.: useDoor, useTeleporter, etc.)
    
    @param x the location of the item on the map
    @param y the location of the item on the map
    @param z the location of the item on the map
  */
  bool useItem(int x, int y, int z);
  
  /**
    Set up a new mission or story of a mission and call sdlHandler->mainLoop() to play it. 
    Several things happen here: a new map is created via the DungeonGenerator, the party is
    placed on it, the UI is set up, etc.
    After the mission is over (when mainLoop returns) items and creatures created for this
    story are deleted, the UI is hidden, etc.
  */
  void startMission();  
  
  /**
    Set up some variables so the mainLoop can quit and control can be transferred 
    back to startMission.
  */
  void endMission();

  /**
    @return the ShapePalette.
  */
  inline ShapePalette *getShapePalette() { return shapePal; }  

  /**
    @return the SDLHandler.
  */
  inline SDLHandler *getSDLHandler() { return sdlHandler; }
  
  /**
    @return the UserConfiguration.
  */
  inline UserConfiguration * getUserConfiguration() { return userConfiguration; }
  
  
  //void drawTopWindow();

  /**
    Open the container UI for the given container item.
    @param container the container item whose contents to show in the window.
  */
  void openContainerGui(Item *container);

  /**
    Close the specified container gui.
  */
  void closeContainerGui(ContainerGui *gui);

  /**
    Close all open container guis.
  */
  void closeAllContainerGuis();
  
  /**
    A creature has died, mark it dead (via state_mod), and create a "skull and bones"
    container for it containing the items from the dead creature's inventory.
    Additionally, if the creature was a party member perform some other things like
    transferring control to another (still living) player.
    @param creature the ex-creature
  */
  void creatureDeath(Creature *creature);

  /**
    A helper method to show a message in a modal dialog.
  */
  void showMessageDialog(char *message);

  /**
    Set the group's walking formation.
    @param formation One of the formation constants.
  */
  void setFormation(int formation);
  
  void toggleInventoryWindow();

  void toggleOptionsWindow();

  /**
    Show the modal yes/no dialog asking the user if the story should be exited.
  */
  void showExitConfirmationDialog();

  /**
    A helper method to create a window with the "wood" look. (e.g. a container ui)
    @param x the window's position on screen
    @param y the window's position on screen
    @param w the window's width
    @param h the window's height
    @param title the window's title
  */
  Window *createWoodWindow(int x, int y, int w, int h, char *title);
  
  /**
    Called when the mission was completed (monster killed, item bagged, etc.). This
    method awards experience points.
  */
  void missionCompleted();

  /**
    Enable "target" mode for a given creature. The cursor changes to a cross-hair.
  */
  inline void setTargetSelectionFor(Creature *c) { targetSelectionFor = c; sdlHandler->setCursorMode(targetSelectionFor ? SDLHandler::CURSOR_CROSSHAIR : SDLHandler::CURSOR_NORMAL); }
  
  /**
    @return who the current target mode is activated for. (ie. the spellcaster)
  */
  inline Creature *getTargetSelectionFor() { return targetSelectionFor; }

  /** 
	  @return the closest live monster within the given radius or NULL if none can be found.
  */
  Creature *getClosestVisibleMonster(int x, int y, int w, int h, int radius);

  
  /**
    Refresh (redraw) the current UI layout.
  */
  void setUILayout();
  
  /**  
    Set the UI layout to one of the values specified in constants. A UI layout is either
    free-floating (where windows can be dragged anywhere on the screen) or old-school where
    some windows are "locked". These locked windows are always visible and cannot be moved.
    The default screen layout is the free-floating mode.
  */
  void setUILayout(int mode);
  
  /**
    @return the current UI layout mode.
  */
  int getLayoutMode() { return layoutMode; }

#ifdef HAVE_SDL_NET
  void runClient(char *host, int port, char *userName);
  void runServer(int port);
  /**
    @return the server.
  */
  inline Server *getServer() { return server; }

  /**
    @return the client.
  */
  inline Client *getClient() { return client; }
#endif

 protected:
  //  void fightBattle(); 

   int initMultiplayer();

  void decodeName(int name, Uint16* mapx, Uint16* mapy, Uint16* mapz);
  void createUI();
  // change the player's selX,selY values as specified by keyboard movement
  void handleKeyboardMovement();
  // move a creature
  void moveMonster(Creature *monster);

  void decideMonsterAction(Creature *monster);

  void refreshContainerGui(Item *container);

};

#endif
