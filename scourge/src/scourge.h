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
#include "common/constants.h"
#include "sdlhandler.h"
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
class ScourgeView;
class ScourgeHandler;
class ConfirmDialog;
class PcEditor;
class TextDialog;
class SavegameDialog;
class PcUi;
class TextScroller;

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
class Scourge : public SDLOpenGLAdapter,WidgetView,DragAndDropHandler,StatusReport {
 private:
	TextScroller *descriptionScroller;
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
	PcUi *pcui;
  ConfirmDialog *exitConfirmationDialog;
  TextDialog *textDialog;
  InfoGui *infoGui;
  ConversationGui *conversationGui;
  //Label *exitLabel;
  //Button *yesExitConfirm, *noExitConfirm;
  int movingX, movingY, movingZ;
  Item *movingItem;  
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


  // party ui
  bool lastEffectOn;
  int oldX;
  char version[100], min_version[20];
  Window *mainWin, *tbCombatWin;
  Button *inventoryButton;
  Button *endTurnButton;
  Button *optionsButton;
  Button *quitButton;
  Button *roundButton;
	Button *ioButton;
  Button *player1Button;
  Button *player2Button;
  Button *player3Button;
  Button *player4Button;
  Button *groupButton;
  CardContainer *cards;
  Canvas *minPartyInfo;
  Canvas *playerInfo[MAX_PARTY_SIZE], *playerHpMp[MAX_PARTY_SIZE], *playerWeapon[MAX_PARTY_SIZE];
  Canvas *quickSpell[12];
	Button *dismissButton[MAX_PARTY_SIZE];

  // board gui
  ScrollingList *missionList;
  ScrollingLabel *missionDescriptionLabel;
  Button *playMission, *closeBoard;
  Window *boardWin;
  MapWidget *mapWidget;

  Progress *progress;
  bool inBattle;  
  
  TradeDialog *tradeDialog;
  HealDialog *healDialog;
  DonateDialog *donateDialog;
  TrainDialog *trainDialog;

  Location *gatepos;

  Window *squirrelWin;
  ScrollingLabel *squirrelLabel;
  TextField *squirrelText;
  Button *squirrelRun, *squirrelClear;

  std::map<Location*, MagicSchool*> deityLocation;

  ScourgeView *view;
  ScourgeHandler *handler;

	ConfirmDialog *dismissHeroDialog;
	ConfirmDialog *confirmUpload;

  PcEditor *pcEditor;
	SavegameDialog *saveDialog;
	std::set<std::string> visitedMaps;

protected:
  bool getItem(Location *pos);
  // returns new z coordinate
  int dropItem(int x, int y);
  bool useLever( Location *pos, bool showMessage=true );
  bool useSecretDoor(Location *pos);

  void destroyDoor( Sint16 ox, Sint16 oy, Shape *shape );
	void startDoorEffect( int effect, Sint16 ox, Sint16 oy, Shape *shape );
  bool useBoard(Location *pos);
  bool useTeleporter(Location *pos);
  bool useGate(Location *pos);
  bool usePool( Location *pos );
	
	// called from startMission
	void resetGame( bool resetParty );
	void createMissionInfoMessage( Mission *lastMission );
	bool createLevelMap( Mission *lastMission, bool fromRandomMap );
	void showLevelInfo();
	void cleanUpAfterMission();
	bool changeLevel();
	void endGame();
	void getCurrentMapName( char *path, char *dirName=NULL, int depth=-1, char *mapFileName=NULL );
	void getSavedMapName( char *mapName );
	bool loadMap( char *mapName, bool fromRandomMap, bool absolutePath, char *templateMapName=NULL );	

public:
#define TOP_GUI_WIDTH 400
#define TOP_GUI_HEIGHT 100
#define GUI_PLAYER_INFO_W 250
#define GUI_PLAYER_INFO_H 350
#define MINIMAP_WINDOW_WIDTH 200
#define MINIMAP_WINDOW_HEIGHT 150

  static const int PARTY_GUI_WIDTH=486;
  static const int PARTY_GUI_HEIGHT=150;
  static const int PARTY_MIN_GUI_WIDTH=100;
  static const int INVENTORY_WIDTH = 420;
  static const int INVENTORY_HEIGHT = 460;
  
  static int blendA, blendB;
  static int blend[];
  void setBlendFunc();
  static void setBlendFuncStatic();
  
  Scourge( UserConfiguration *config );
  ~Scourge();

	inline TextScroller *getDescriptionScroller() { return descriptionScroller; }
	inline Window *getTBCombatWin() { return tbCombatWin; }
  //inline Button *getYesExitConfirm() { return yesExitConfirm; }
  //inline Button *getNoExitConfirm() { return noExitConfirm; }
  inline Button *getInventoryButton() { return inventoryButton; }
  inline Button *getEndTurnButton() { return endTurnButton; }
  inline Button *getOptionsButton() { return optionsButton; }
  inline Button *getQuitButton() { return quitButton; }
  inline Button *getRoundButton() { return roundButton; }
	inline Button *getIOButton() { return ioButton; }
  inline Button *getPlayer1Button() { return player1Button; }
  inline Button *getPlayer2Button() { return player2Button; }
  inline Button *getPlayer3Button() { return player3Button; }
  inline Button *getPlayer4Button() { return player4Button; }
  inline Button *getGroupButton() { return groupButton; }
  inline Canvas *getPlayerInfo( int index ) { return playerInfo[ index ]; }
  inline Canvas *getPlayerHpMp( int index ) { return playerHpMp[ index ]; }
  inline Canvas *getPlayerWeapon( int index ) { return playerWeapon[ index ]; }
  inline Canvas *getQuickSpell( int index ) { return quickSpell[ index ]; }
	inline Button *getDismissButton( int index ) { return dismissButton[ index ]; }
	inline SavegameDialog *getSaveDialog() { return saveDialog; }

	virtual void addDescription(char *description, float r=1.0f, float g=1.0f, float b=0.4f);

  void movePartyToGateAndEndMission();

  inline NetPlay *getNetPlay() { return netPlay; }

  inline bool isInHQ() { return inHq; }

  inline Window *getSquirrelConsole() { return squirrelWin; }
  void runSquirrelConsole();
  void clearSquirrelConsole();
  inline TextField *getSquirrelText() { return squirrelText; }
  inline Button *getSquirrelRun() { return squirrelRun; }
  inline Button *getSquirrelClear() { return squirrelClear; }

  inline void addDeityLocation( Location *pos, MagicSchool *ms ) { deityLocation[pos] = ms; }
  char *getDeityLocation( Location *pos );
  inline MagicSchool *getMagicSchoolLocation( Location *pos ) {
    if( deityLocation.find( pos ) != deityLocation.end() ) {
      return deityLocation[ pos ];
    } else {
      return NULL;
    }
  }

  inline bool isInfoDialogShowing() { return info_dialog_showing; }
  inline void setInfoDialogShowing( bool b ) { info_dialog_showing = b; }

  void evalSpecialSkills();

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

	virtual char *getMagicSchoolIndexForLocation( Location *pos );
	virtual void setMagicSchoolIndexForLocation( Location *pos, char *magicSchoolName );

  inline int getCurrentDepth() { return currentStory; }
	void descendDungeon( Location *pos );
	void ascendDungeon( Location *pos );

  //inline Session *getSession() { return session; }

  /**
    @return the Board containing the available missions.
  */
  inline Board *getBoard() { return board; }
  void updateBoard();
  inline Window *getBoardWin() { return boardWin; }
  inline ScrollingList *getMissionList() { return missionList; }
  inline Button *getCloseBoard() { return closeBoard; }
  inline Button *getPlayMission() { return playMission; }

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

	inline PcUi *getPcUi() { return pcui; }
  
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
  void startMission( bool startInHq );  

  /**
    Set up some variables so the mainLoop can quit and control can be transferred 
    back to startMission.
  */
  void endMission();
  
  /**
    Open the container UI for the given container item.
    @param container the container item whose contents to show in the window.
  */
  void openContainerGui(Item *container);

  /**
    Close the specified container gui.
  */
  void closeContainerGui(ContainerGui *gui);

  inline int getContainerGuiCount() { return containerGuiCount; }
  inline ContainerGui *getContainerGui( int i ) { return containerGui[i]; }
  inline void closeContainerGui( int index ) { closeContainerGui( containerGui[index] ); }

  /**
    Close all open container guis.
  */
  void closeAllContainerGuis();

  void removeClosedContainerGuis();
  
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

  void closeExitConfirmationDialog();

  inline ConfirmDialog *getExitConfirmationDialog() { return exitConfirmationDialog; }

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
	int getCursorWidth();
	int getCursorHeight();

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
	bool handleTargetSelectionOfDoor( Uint16 mapx, Uint16 mapy, Uint16 mapz );
	void cancelTargetSelection();

  void teleport( bool toHQ=true );

  inline Window *getPartyWindow() { return mainWin; }

  virtual void unlockMouse() { getSDLHandler()->unlockMouse(); }
  virtual void lockMouse( Widget *widget ) { getSDLHandler()->lockMouse( widget ); }

  GLuint getHighlightTexture();
	GLuint getGuiTexture();
	GLuint getGuiTexture2();

  GLuint loadSystemTexture( char *line );

  inline TradeDialog *getTradeDialog() { return tradeDialog; }
  inline HealDialog *getHealDialog() { return healDialog; }
  inline DonateDialog *getDonateDialog() { return donateDialog; }
  inline TrainDialog *getTrainDialog() { return trainDialog; }

  bool startTextEffect( char *message );

  static bool testLoadGame(Session *session);

  void startConversation( RenderedCreature *creature, char *message=NULL );
	void endConversation();

  void drawItemIcon( Item *item, int size=25 );

  virtual void completeCurrentMission();

  Battle *getCurrentBattle();
  void endCurrentBattle();

  void updatePartyUI();

  void resetBattles();

  bool playSelectedMission();

  void selectDropTarget( Uint16 mapx, Uint16 mapy, Uint16 mapz );

  void executeQuickSpell( Spell *spell );
  void executeSpecialSkill( SpecialSkill *skill );
	/**
	 * @return true if the item cannot be used anymore
	 */
  bool executeItem( Item *item );

  void describeLocation(int mapx, int mapy, int mapz);

  void mouseClickWhileExiting();

  bool saveGame( Session *session, char *dirName, char *title );  
  bool loadGame( Session *session, char *dirName, char *error );	

	RenderedCreature *createWanderingHero( int level );

	ConfirmDialog *getDismissHeroDialog() { return dismissHeroDialog; }
	ConfirmDialog *getConfirmUpload() { return confirmUpload; }
	void uploadScore();
	TextDialog *getTextDialog() { return textDialog; }
  PcEditor *getPcEditor() { return pcEditor; }

	void handleWanderingHeroClick( Creature *creature );
	void handleDismiss( int index );
	
	void showTextMessage( char *message );
	void askToUploadScore();

	bool saveCurrentMap( char *dirName=NULL );

	bool useDoor( Location *pos, bool openLocked=false );

	void drawPortrait( Creature *p, int width, int height, int offs_x=0, int offs_y=0 );

	bool getStateModIcon( GLuint *icon, char *name, Color *color, Creature *p, int stateMod, bool protect=false );

  void describeAttacks( Creature *p, int x, int y, bool currentOnly=false );
	void describeDefense( Creature *p, int x, int y );

	bool enchantItem( Creature *creature, Item *item );
	bool transcribeItem( Creature *creature, Item *item );
	bool useItem( Creature *creature, Item *item );

protected:

	bool describeWeapon( Creature *p, Item *item, int x, int y, int inventoryLocation, bool handleNull );

  char *getAPRDescription( Creature *p, Item *item, char *buff );

	bool doLoadGame( Session *session, char *dirName, char *error );

	void drawPortrait( Widget *w, Creature *p=NULL );

	int initMultiplayer();

  void createUI();

  void decideMonsterAction(Creature *monster);

  void refreshContainerGui( Item *container=NULL );

  void createPartyUI();

  void moveProjectiles();
  bool fightCurrentBattleTurn();
  void resetNonParticipantAnimation( Battle *battle );
  bool createBattleTurns();
  void resetUIAfterBattle();
  void moveCreatures( bool allCreatures=true );  

	void addWanderingHeroes();

	bool saveScoreid( char *dirName, char *p );
	bool loadScoreid( char *dirName, char *p );
};

#endif
