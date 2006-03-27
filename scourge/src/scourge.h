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
#include <map>
#include "constants.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "calendar.h"
#include "minimap.h"
#include "mainmenu.h"
#include "optionsmenu.h"
#include "inventory.h"
#include "gui/window.h"
#include "gui/button.h"
#include "userconfiguration.h"
#include "containergui.h"
#include "board.h"
#include "battle.h"
#include "party.h"
#include "multiplayer.h"
#include "net/server.h"
#include "net/client.h"
#include "net/gamestatehandler.h"
#include "netplay.h"
#include "gui/progress.h"
#include "gameadapter.h"
#include "session.h"
#include "infogui.h"
#include "conversationgui.h"
#include "gui/guitheme.h"
#include "gui/scrollinglabel.h"

class Item;
class Creature;
class Calendar;
class MiniMap;
class Map;
class DungeonGenerator;
class ShapePalette;
class Location;
class MainMenu;
class MapEditor;
class MapSettings;
class OptionsMenu;
class Inventory;
class UserConfiguration;
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
class GameAdapter;
class InfoGui;
class ConversationGui;
class GuiTheme;
class ScrollingLabel;
class MapWidget;
class TradeDialog;
class HealDialog;
class DonateDialog;
class TextEffect;
class TrainDialog;
class MagicSchool;

#define IMAGES_DIR "images/"
#define RESOURCES_DIR "resources/"
#define DEFAULT_IMAGES_DIR "default/"
#define CREATURES_DIR "creatures/"
#define MAX_BATTLE_COUNT 200

class InfoMessage {
public:
  char message[300];
  void *obj;
  int x, y, z;

  InfoMessage( char *s, void *obj, int x, int y, int z ) {
    strcpy( this->message, s );
    this->obj = obj;
    this->x = x;
    this->y = y;
    this->z = z;
  }

  ~InfoMessage() {
  }
};

/** 
  This is the main class of the game. It is a central place to put
  references to other objects, like the party, minimap, etc.
  
  @author Gabor Torok
*/ 
class Scourge : public SDLOpenGLAdapter,SDLEventHandler,SDLScreenView,WidgetView,DragAndDropHandler,StatusReport {
 private:
  Party *party;
  Map *levelMap;
  MapSettings *mapSettings;
  MiniMap * miniMap;
  int level;
  MapEditor *mapEditor;
  MainMenu *mainMenu;
  OptionsMenu *optionsMenu;
  MultiplayerDialog *multiplayer;
  bool isInfoShowing;
  bool info_dialog_showing;
  Board *board;
  int nextMission;
  bool teleportFailure;
  bool inHq;
  bool missionWillAwardExpPoints;
  char infoMessage[2000];
  Inventory *inventory;
  Window *messageWin, *exitConfirmationDialog;
  InfoGui *infoGui;
  ConversationGui *conversationGui;
  Label *exitLabel;
  ScrollingList *messageList;
  Button *yesExitConfirm, *noExitConfirm;
  int movingX, movingY, movingZ;
  //Uint16 cursorMapX, cursorMapY, cursorMapZ;
  Item *movingItem;
  bool needToCheckDropLocation;
  GLint lastTick;
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

  // how many pixels to wait between sampling 3d coordinates 
  // when dragging items (the more the faster)
  static const int POSITION_SAMPLE_DELTA = 10; 

  bool teleporting;
  
  std::vector<Battle *> battleRound;
  int battleTurn, rtStartTurn;

  Creature *targetSelectionFor;

  int layoutMode;
  NetPlay *netPlay;

  float targetWidth, targetWidthDelta;
  Uint32 lastTargetTick;

  // party ui
  bool lastEffectOn;
  int oldX;
  char version[100], min_version[20];
  Window *mainWin;
  Button *inventoryButton;
  Button *endTurnButton;
  Button *optionsButton;
  Button *quitButton;
  Button *roundButton;
  Button *player1Button;
  Button *player2Button;
  Button *player3Button;
  Button *player4Button;
  Button *groupButton;
  CardContainer *cards;
  Canvas *minPartyInfo;
  Canvas *playerInfo[MAX_PARTY_SIZE], *playerHpMp[MAX_PARTY_SIZE], *playerWeapon[MAX_PARTY_SIZE];
  Canvas *quickSpell[12];

  // board gui
  ScrollingList *missionList;
  ScrollingLabel *missionDescriptionLabel;
  Button *playMission, *closeBoard;
  Window *boardWin;
  MapWidget *mapWidget;

  Progress *progress;
  bool inBattle;
  Progress *turnProgress;

  bool willStartDrag;
  int willStartDragX, willStartDragY;
  GLUquadric *quadric;

  bool needToCheckInfo;
  std::map<InfoMessage *, Uint32> infos;
  
  TradeDialog *tradeDialog;
  HealDialog *healDialog;
  DonateDialog *donateDialog;
  TrainDialog *trainDialog;

  Color *outlineColor;
  TextEffect *textEffect;
  GLint textEffectTimer;

  Location *gatepos;

  Window *squirrelWin;
  ScrollingLabel *squirrelLabel;
  TextField *squirrelText;
  Button *squirrelRun, *squirrelClear;

  std::map<Location*, MagicSchool*> deityLocation;

protected:
  void processGameMouseDown(Uint16 x, Uint16 y, Uint8 button);
  void processGameMouseClick(Uint16 x, Uint16 y, Uint8 button);
  void describeLocation(int mapx, int mapy, int mapz);


  bool getItem(Location *pos);
  // returns new z coordinate
  int dropItem(int x, int y);
  bool useLever(Location *pos);
  bool useSecretDoor(Location *pos);
  bool useDoor(Location *pos);
  void destroyDoor( Sint16 ox, Sint16 oy, Shape *shape );
  bool useBoard(Location *pos);
  bool useTeleporter(Location *pos);
  bool useGate(Location *pos);
  bool usePool( Location *pos );

public:
#define TOP_GUI_WIDTH 400
#define TOP_GUI_HEIGHT 100
#define GUI_PLAYER_INFO_W 250
#define GUI_PLAYER_INFO_H 350
#define MINIMAP_WINDOW_WIDTH 200
#define MINIMAP_WINDOW_HEIGHT 150

  static const int PARTY_GUI_WIDTH=500;
  static const int PARTY_GUI_HEIGHT=145;
  static const int PARTY_MIN_GUI_WIDTH=100;
  static const int INVENTORY_WIDTH = 420;
  static const int INVENTORY_HEIGHT = 460;
  
  static int blendA, blendB;
  static int blend[];
  void setBlendFunc();
  static void setBlendFuncStatic();
  
  Scourge( UserConfiguration *config );
  ~Scourge();

  inline bool isInHQ() { return inHq; }

  inline Window *getSquirrelConsole() { return squirrelWin; }

  inline void addDeityLocation( Location *pos, MagicSchool *ms ) { deityLocation[pos] = ms; }
  char *getDeityLocation( Location *pos );
  inline MagicSchool *getMagicSchoolLocation( Location *pos ) {
    if( deityLocation.find( pos ) != deityLocation.end() ) {
      return deityLocation[ pos ];
    } else {
      return NULL;
    }
  }

  bool isLevelShaded();

  void printToConsole( const char *s );

  void updateStatus( int status, int maxStatus, const char *message=NULL );

  /**
    The widget received a dragged item
  */
  void receive(Widget *widget);

  /**
	 The widget initiated a drag
   * return true if there's something to drag at x,y
   */
  inline bool startDrag(Widget *widget, int x=0, int y=0) {
    return false;
  }

  /**
   * If the current location is interactive, return the outline color. 
   * Return NULL otherwise.
   */
  Color *getOutlineColor( Location *pos );

  inline int getCurrentDepth() { return currentStory; }

  //inline Session *getSession() { return session; }

  /**
    @return the Board containing the available missions.
  */
  inline Board *getBoard() { return board; }
  
  /**
	  This method is called by the main loop to play a round. A round may consist of 
    a battle with multiple participants, someone drinking a potion, casting a spell, etc.
  */
  void playRound();

  Battle *getBattle(Creature *creature);
 
  /**
    @return the party object.
  */
  inline Party *getParty() { return party; }
  
  /**
    @return the map.
  */
  inline Map *getMap() { return levelMap; }

  inline MapSettings *getMapSettings() { return mapSettings; }
  
  /**
    @return the MiniMap.
  */
  inline MiniMap *getMiniMap() { return miniMap; }

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
    @return the map editor
  */
  inline MapEditor *getMapEditor() { return mapEditor; }

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

  void drawDraggedItem();

  void drawBorder();
  
  void drawOutsideMap();

  void showCreatureInfo(Creature *creature, bool player, bool selected, bool groupMode);

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

  void togglePlayerOnly();
  
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
    A helper method to create a window with the default look.
    @param x the window's position on screen
    @param y the window's position on screen
    @param w the window's width
    @param h the window's height
    @param title the window's title
  */
  Window *createWindow(int x, int y, int w, int h, char *title);
  
  /**
    Called when the mission was completed (monster killed, item bagged, etc.). This
    method awards experience points.
  */
  void missionCompleted();

  /**
    Enable "target" mode for a given creature. The cursor changes to a cross-hair.
  */
  inline void setTargetSelectionFor(Creature *c) { targetSelectionFor = c; sdlHandler->setCursorMode(targetSelectionFor ? Constants::CURSOR_CROSSHAIR : Constants::CURSOR_NORMAL); }
  
  /**
    @return who the current target mode is activated for. (ie. the spellcaster)
  */
  inline Creature *getTargetSelectionFor() { return targetSelectionFor; }

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

  // initialization events
  void initStart(int statusCount, char *message);
  void initUpdate(char *message);
  void initEnd();

  void initUI();
  void start();

  ShapePalette *getShapePalette();

  GLuint getCursorTexture( int cursorMode );

  void loadMonsterSounds( char *type, std::map<int, std::vector<std::string>*> *soundMap );
  void unloadMonsterSounds( char *type, std::map<int, std::vector<std::string>*> *soundMap );
  void loadCharacterSounds( char *type );
  void unloadCharacterSounds( char *type );
  void playCharacterSound( char *type, int soundType );

  void fightProjectileHitTurn(Projectile *proj, RenderedCreature *creature);

  void fightProjectileHitTurn(Projectile *proj, int x, int y);

  void drawWidgetContents(Widget *w);

  void resetPartyUI();

  void refreshInventoryUI(int playerIndex);

  void refreshInventoryUI();

  void toggleRoundUI(bool startRound);

  void setFormationUI(int formation, bool playerOnly);

  void togglePlayerOnlyUI(bool playerOnly);

  void setPlayerUI(int index);

  void setPlayer(int n);

  void createBoardUI();

  void updateBoardUI(int count, const char **missionText, Color *missionColor);

  int handleBoardEvent(Widget *widget, SDL_Event *event);

  void setMissionDescriptionUI(char *s, int mapx, int mapy);

  // move a creature
  void moveMonster(Creature *monster);

  void removeBattle(Battle *battle);

  inline bool inTurnBasedCombat() {
    return (battleTurn < (int)battleRound.size() && 
            getUserConfiguration()->isBattleTurnBased());
  }

  inline UserConfiguration *getUserConfiguration() { return (UserConfiguration*)getPreferences(); }

  bool inTurnBasedCombatPlayerTurn();

  inline InfoGui *getInfoGui() { return infoGui; }

  inline ConversationGui *getConversationGui() { return conversationGui; }

  void showItemInfoUI(Item *item, int level);

  void resetInfos();

  void createParty( Creature **pc, int *partySize );

  bool handleTargetSelectionOfCreature( Creature *potentialTarget );
  bool handleTargetSelectionOfItem( Item *item, int x=0, int y=0, int z=0 );
  bool handleTargetSelectionOfLocation( Uint16 mapx, Uint16 mapy, Uint16 mapz );

  void teleport( bool toHQ=true );

  inline Window *getPartyWindow() { return mainWin; }

  virtual void unlockMouse() { getSDLHandler()->unlockMouse(); }
  virtual void lockMouse( Widget *widget ) { getSDLHandler()->lockMouse( widget ); }

  GLuint getHighlightTexture();

  GLuint loadSystemTexture( char *line );

  inline TradeDialog *getTradeDialog() { return tradeDialog; }
  inline HealDialog *getHealDialog() { return healDialog; }
  inline DonateDialog *getDonateDialog() { return donateDialog; }
  inline TrainDialog *getTrainDialog() { return trainDialog; }

  bool startTextEffect( char *message );

  static bool testLoadGame(Session *session);

  void startConversation( RenderedCreature *creature );

  void drawItemIcon( Item *item, int size=25 );

  virtual void completeCurrentMission();

 protected:

   void drawPortrait( Widget *w, Creature *p );

  void resetBattles();

   int initMultiplayer();

  void createUI();
  // change the player's selX,selY values as specified by keyboard movement
  void handleKeyboardMovement();  

  void decideMonsterAction(Creature *monster);

  void refreshContainerGui(Item *container);

  void createPartyUI();

  bool handlePartyEvent(Widget *widget, SDL_Event *event);

  void updatePartyUI();

  void moveProjectiles();
  bool fightCurrentBattleTurn();
  void resetNonParticipantAnimation( Battle *battle );
  bool createBattleTurns();
  void resetUIAfterBattle();
  void moveCreatures();

  void checkForDropTarget();
  void checkForInfo();
  void drawInfos();

  void quickSpellAction( int index, int button=SDL_BUTTON_LEFT );
  void executeQuickSpell( Spell *spell );
  void executeSpecialSkill( SpecialSkill *skill );
  void executeItem( Item *item );

  void drawDescriptions(ScrollingList *list);

  bool doesSaveGameExist(Session *session);
  bool saveGame(Session *session);  
  bool loadGame(Session *session);
};

#endif
