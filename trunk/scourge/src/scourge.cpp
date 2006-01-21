/***************************************************************************
                          scourge.cpp  -  description
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

#include "scourge.h"
#include "events/thirsthungerevent.h"
#include "events/reloadevent.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "item.h"
#include "creature.h"
#include "projectile.h"
#include "mapeditor.h"
#include "sound.h"
#include "mapwidget.h"
#include "session.h"
#include "tradedialog.h"
#include "healdialog.h"
#include "donatedialog.h"
#include "traindialog.h"
#include "io/file.h"
#include "texteffect.h"
#include "sqbinding/sqbinding.h"
#include "storable.h"
#include "shapepalette.h"
#include "terraingenerator.h"
#include "cavemaker.h"
#include "dungeongenerator.h"

using namespace std;

//#define CAVE_TEST 1

#define MOUSE_ROT_DELTA 2

#define BATTLES_ENABLED 1

#define DRAG_START_TOLERANCE 5

#define INFO_INTERVAL 3000

//#define DEBUG_KEYS 1

#define SAVE_FILE "savegame.dat"
#define VALUES_FILE "values.dat"

#define HQ_MAP_NAME "hq"
#define RANDOM_MAP_NAME "random"

// 2,3  2,6  3,6*  5,1+  6,3   8,3*

// good for debugging blending
int Scourge::blendA = 2;
int Scourge::blendB = 6;     // 3
int Scourge::blend[] = { 
    GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, 
    GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA_SATURATE
};

void Scourge::setBlendFunc() {
    glBlendFunc(blend[blendA], blend[blendB]);
}

Scourge::Scourge(UserConfiguration *config) : SDLOpenGLAdapter(config) {
  oldStory = currentStory = 0;
  lastTick = 0;
  messageWin = NULL;
  movingX = movingY = movingZ = MAP_WIDTH + 1;
  movingItem = NULL;
  needToCheckInfo = false;
  needToCheckDropLocation = true;
  nextMission = -1;
  teleportFailure = false;
  outlineColor = new Color( 0.3f, 0.3f, 0.5f, 0.5f );
//  cursorMapX = cursorMapY = cursorMapZ = MAP_WIDTH + 1;
  // in HQ map
  inHq = true;
  //showPath = config->getAlwaysShowPath();

  layoutMode = Constants::GUI_LAYOUT_BOTTOM;
  
  isInfoShowing = true; // what is this?
  info_dialog_showing = false;

  // we're not in target selection mode
  targetSelectionFor = NULL;
  
  battleCount = 0;  
  inventory = NULL;
  containerGuiCount = 0;
  changingStory = false;  

  targetWidth = 0.0f;
  targetWidthDelta = 0.05f / DIV;
  lastTargetTick = SDL_GetTicks();

  lastEffectOn = false;
  resetBattles();

  willStartDrag = false;
  willStartDragX = willStartDragY = 0;
  textEffect = NULL;
  textEffectTimer = 0;
  gatepos = NULL;
}

void Scourge::initUI() {

  turnProgress = new Progress(this->getSDLHandler(), 
                              getSession()->getShapePalette()->getProgressTexture(),
                              getSession()->getShapePalette()->getProgressHighlightTexture(),
                              10, false, true, false);
  
  // init UI themes
  GuiTheme::initThemes( getSDLHandler() );

  // for now pass map in
  this->levelMap = session->getMap();
  mapSettings = new GameMapSettings();
  levelMap->setMapSettings( mapSettings );
  miniMap = new MiniMap(this); 

  // create the mission board
  this->board = session->getBoard();

  this->party = session->getParty();  
  createPartyUI();  
  createBoardUI();
  netPlay = new NetPlay(this);
  createUI();

  // show the main menu
  mainMenu = new MainMenu(this);
  mapEditor = new MapEditor( this );
  optionsMenu = new OptionsMenu(this);
  multiplayer = new MultiplayerDialog(this);

  // load character, item sounds
  getSDLHandler()->getSound()->loadSounds( getUserConfiguration() );
}

void Scourge::start() {
  this->quadric = gluNewQuadric();
  bool initMainMenu = true;
  while(true) {

    if(initMainMenu) {
      initMainMenu = false;
      mainMenu->show();    
      getSDLHandler()->getSound()->playMusicMenu();
    }

    getSDLHandler()->setHandlers((SDLEventHandler *)mainMenu, (SDLScreenView *)mainMenu);
    getSDLHandler()->mainLoop();
    session->deleteCreaturesAndItems( false );

    // evaluate results and start a missions
    int value = mainMenu->getValue();
    if(value == NEW_GAME) {
      if(doesSaveGameExist( session )) {
        mainMenu->showNewGameConfirmationDialog();
      } else {
        mainMenu->showPartyEditor();
        //value = NEW_GAME_START;
      }
    }
      
    if(value == NEW_GAME_START ||
       value == MULTIPLAYER_START ||
       value == CONTINUE_GAME ||
       value == EDITOR ) {
      mainMenu->hide();
      getSDLHandler()->getSound()->stopMusicMenu();
      
      initMainMenu = true;
      bool failed = false;

      if( value == EDITOR ) {
        glPushAttrib(GL_ENABLE_BIT);
        
        mapEditor->show();
        getSDLHandler()->setHandlers((SDLEventHandler *)mapEditor, (SDLScreenView *)mapEditor);
        getSDLHandler()->mainLoop();

        glPopAttrib();
      } else {
     
#ifdef HAVE_SDL_NET
        if(value == MULTIPLAYER_START) {
          if(!initMultiplayer()) continue;
        }
#endif  
        
        if(value == CONTINUE_GAME) {
          if(!loadGame( session )) {
            showMessageDialog( "Error loading game!" );
            failed = true;
          }
        }

        if( !failed ) {
          // do this to fix slowness in mainmenu the second time around
          glPushAttrib(GL_ENABLE_BIT);
          startMission();
          glPopAttrib();
        }
      }
    } else if(value == OPTIONS) {
      toggleOptionsWindow();
    } else if(value == MULTIPLAYER) {
      multiplayer->show();
    } else if(value == QUIT) {
      getSDLHandler()->quit(0);
    }
  }
}

Scourge::~Scourge(){
  delete mainMenu;
  delete optionsMenu;
  delete multiplayer;
  delete miniMap;
  delete netPlay;
  delete infoGui;
  delete conversationGui;
  delete tradeDialog;
  delete healDialog;
  delete donateDialog;
  delete trainDialog;
}

void Scourge::startMission() {

#ifdef DEBUG_KEYS
  squirrelWin->setVisible( true );
#endif

  // set up some cross-mission objects
  oldStory = currentStory = 0;
  bool resetParty = true;
  
  // always start in hq
  nextMission = -1;
  inHq = true;

  TerrainGenerator *dg = NULL;
  Mission *lastMission = NULL;
  char result[300];  
  char *scriptName;
  while(true) {

    oldStory = currentStory;    

#ifndef CAVE_TEST
    // add gui
    mainWin->setVisible(true);
    messageWin->setVisible(true);
    if(session->isMultiPlayerGame()) netPlay->getWindow()->setVisible(true);
#endif
    // create the map
    //cerr << "Starting to reset map..." << endl;
    bool fromRandomMap = !( levelMap->isEdited() );
    levelMap->reset();
    //cerr << "\tMap reset is done." << endl;    

    // do this only once
    if(resetParty) {
      board->reset();
      if(session->isMultiPlayerGame()) {
        party->resetMultiplayer(multiplayer->getCreature());
      } else {
        party->reset();
      }
      party->getCalendar()->reset(true); // reset the time

      // re-add party events (hack)
      resetPartyUI();

      Calendar *cal = getSession()->getParty()->getCalendar();
      {
        // Schedule an event to keep reloading scripts if they change on disk
        Date d(0, 5, 0, 0, 0, 0); // (format : sec, min, hours, days, months, years)
        Event *event = new ReloadEvent( cal->getCurrentDate(), 
                                        d, 
                                        Event::INFINITE_EXECUTIONS,
                                        getSession(),
                                        ReloadEvent::MODE_RELOAD_SCRIPTS );
        cal->scheduleEvent( event );
      }

      {
        // Schedule an event to regain MP now and then
        Date d(30, 0, 0, 0, 0, 0); // (format : sec, min, hours, days, months, years)
        Event *event = new ReloadEvent( cal->getCurrentDate(), 
                                        d, 
                                        Event::INFINITE_EXECUTIONS,
                                        getSession(),
                                        ReloadEvent::MODE_REGAIN_POINTS );
        cal->scheduleEvent( event );
      }

      // inventory needs the party
      if(!inventory) {
        inventory = new Inventory(this);
      }

      getSession()->getSquirrel()->startGame();

      resetParty = false;
    }
    //cerr << "Minimap reset" << endl;
    miniMap->reset();

    // ready the party
    //cerr << "Party reset" << endl;    
    party->startPartyOnMission();

    // save the party
    //cerr << "Saving party" << endl;
    if(!session->isMultiPlayerGame()) {
      if(!saveGame(session)) {
        showMessageDialog( "Error saving game!" );
      }
    }

    // position the players
    //cerr << "Calling resetMove" << endl;    
    levelMap->resetMove();
    battleCount = 0;
    containerGuiCount = 0;
    teleporting = false;
    targetSelectionFor = NULL;  
    gatepos = NULL;

    // clear infoMessage
    strcpy( infoMessage, "" );

    if(nextMission == -1) {

      //cerr << "Showing Uzudil's message" << endl;        
      missionWillAwardExpPoints = false;

      // in HQ map
      inHq = true;

      // store mission's outcome (mission may be deleted below)
      if( lastMission ) {
        sprintf( infoMessage, 
                 ( lastMission->isCompleted() ? 
                   lastMission->getSuccess() : 
                   lastMission->getFailure() ) );

        if( lastMission->isCompleted() ) {
          // Add XP points for making it back alive
          char message[1000];
          int exp = (lastMission->getLevel() + 1) * 100;
          sprintf( message, "For returning alive, the party receives %d experience points. ", exp);
          strcat( infoMessage, message );
          
          for(int i = 0; i < getParty()->getPartySize(); i++) {
            int level = getParty()->getParty(i)->getLevel();
            if(!getParty()->getParty(i)->getStateMod(Constants::dead)) {
              int n = getParty()->getParty(i)->addExperience(exp);
              if(n > 0) {
                if( level != getParty()->getParty(i)->getLevel() ) {
                  sprintf(message, "%s gains a level! ", getParty()->getParty(i)->getName());
                  strcat( infoMessage, message );
                }
              }
            }
          }
        }
        
        lastMission = NULL;
      }

      // init the missions board (this deletes completed missions)
      //cerr << "Initializing board" << endl;
      board->initMissions();

      // display the HQ map
      getSession()->setCurrentMission(NULL);
      missionWillAwardExpPoints = false;


#ifdef CAVE_TEST
      dg = new CaveMaker( this, 1, 1, false, false, NULL );
      dg->toMap(levelMap, getSession()->getShapePalette());
      scriptName = RANDOM_MAP_NAME;
#else
      dg = NULL;
      levelMap->loadMap( HQ_MAP_NAME, result, this, currentStory, changingStory );
      scriptName = HQ_MAP_NAME;
#endif

      //cerr << result << endl;

    } else {
      // in HQ map
      inHq = false;

      // Initialize the map with a random dunegeon	
      getSession()->setCurrentMission(board->getMission(nextMission));
      missionWillAwardExpPoints = (!getSession()->getCurrentMission()->isCompleted());
      bool loaded = false;
      if( getSession()->getCurrentMission()->getMapName() &&
          strlen( getSession()->getCurrentMission()->getMapName() ) ) {
        // try to load the edited map
        dg = NULL;
        bool lastLevel = ( currentStory == getSession()->getCurrentMission()->getDepth() - 1 );
        if( lastLevel ) {
          vector< RenderedItem* > items;
          vector< RenderedCreature* > creatures;
          loaded = levelMap->loadMap( getSession()->getCurrentMission()->getMapName(), 
                                      result, 
                                      this, 
                                      currentStory, 
                                      changingStory, 
                                      fromRandomMap,
                                      &items,
                                      &creatures );
          
          // add item/creature instances if last level (this is special handling for edited levels)
          set<int> used;
          for( int i = 0; i < (int)items.size(); i++ ) {
            Item *item = (Item*)( items[i] );
            for( int t = 0; t < (int)getSession()->getCurrentMission()->getItemCount(); t++ ) {
              if( getSession()->getCurrentMission()->getItem( t ) == item->getRpgItem() &&
                  used.find( t ) == used.end() ) {
                getSession()->getCurrentMission()->
                  addItemInstance( item, item->getRpgItem() );
                used.insert( t );
              }
            }
          }
          used.clear();
          for( int i = 0; i < (int)creatures.size(); i++ ) {
            Creature *creature = (Creature*)( creatures[i] );
            if( creature->getMonster() ) {
              for( int t = 0; t < (int)getSession()->getCurrentMission()->getCreatureCount(); t++ ) {
                if( getSession()->getCurrentMission()->getCreature( t ) == creature->getMonster() &&
                    used.find( t ) == used.end() ) {
                  getSession()->getCurrentMission()->
                    addCreatureInstanceMap( creature, creature->getMonster() );
                  used.insert( t );
                }
              }
            }
          }
        } else {
          loaded = levelMap->loadMap( getSession()->getCurrentMission()->getMapName(), 
                                      result, 
                                      this, 
                                      currentStory, 
                                      changingStory, 
                                      fromRandomMap );
        }
        scriptName = getSession()->getCurrentMission()->getMapName();
      } 

      // if no edited map is found, make a random map
      if( !loaded ) {
        /*
        dg = new DungeonGenerator(this, getSession()->getCurrentMission()->getLevel(), currentStory, 
                                  (currentStory < getSession()->getCurrentMission()->getDepth() - 1), 
                                  (currentStory > 0),
                                  getSession()->getCurrentMission());
        */
        dg = new CaveMaker(this, getSession()->getCurrentMission()->getLevel(), currentStory, 
                           (currentStory < getSession()->getCurrentMission()->getDepth() - 1), 
                           (currentStory > 0),
                           getSession()->getCurrentMission());
        dg->toMap(levelMap, getSession()->getShapePalette());
        scriptName = RANDOM_MAP_NAME;
      }
    }

    changingStory = false;
	
    // center map on the player
    levelMap->center(toint(party->getPlayer()->getX()), 
                     toint(party->getPlayer()->getY()),
                     true);

    // set to receive events here
    getSDLHandler()->setHandlers((SDLEventHandler *)this, (SDLScreenView *)this);

    // hack to unfreeze animations, etc.
    party->forceStopRound();

    // show an info dialog if infoMessage not already set with outcome of last mission
    if( !strlen( infoMessage ) ) {
      if(nextMission == -1) {
        sprintf(infoMessage, "Welcome to the S.C.O.U.R.G.E. Head Quarters");
      } else if( teleportFailure ) {
        teleportFailure = false;
        sprintf(infoMessage, "Teleport spell failed!! Entering level %d", ( currentStory + 1 ));
      } else {
        sprintf(infoMessage, "Entering dungeon level %d", ( currentStory + 1 ));
      }
      
      // show infoMessage text
      showMessageDialog(infoMessage);
      info_dialog_showing = true;
    } else {
      
      // start a conversation with Uzudil
      // FIXME: this will not show teleport effect...
        
      // FIXME hack code to find Uzudil.
      Monster *m = Monster::getMonsterByName( "Uzudil the Hand" );
      Creature *uzudil = NULL;
      for( int i = 0; i < session->getCreatureCount(); i++ ) {
        if( session->getCreature( i )->getMonster() == m ) {
          uzudil = session->getCreature( i );
          break;
        }
      }
      
      if( !uzudil ) {
        cerr << "*** Error: can't find Uzudil!" << endl;
      }
      conversationGui->start( uzudil, infoMessage, true );      
    }
#ifndef CAVE_TEST      
    // set the map view
    setUILayout();
#endif
    // start the haunting tunes
    if(inHq) getSDLHandler()->getSound()->playMusicMenu();
    else getSDLHandler()->getSound()->playMusicDungeon();

    // run mission
    getSDLHandler()->mainLoop();

    getSession()->getSquirrel()->endLevel();

    // stop the music
    getSDLHandler()->getSound()->stopMusicDungeon();

    // clean up after the mission
    resetInfos();

    // remove gui
    mainWin->setVisible(false);
    messageWin->setVisible(false);
    closeAllContainerGuis();
    if(inventory->isVisible()) {
      inventory->hide();
      inventoryButton->setSelected( false );
    }
    if(optionsMenu->isVisible()) {
      optionsMenu->hide();
      optionsButton->setSelected( false );
    }
    if(boardWin->isVisible()) boardWin->setVisible(false);
    netPlay->getWindow()->setVisible(false);
    infoGui->getWindow()->setVisible(false);
    conversationGui->hide();
    tradeDialog->getWindow()->setVisible( false );
    healDialog->getWindow()->setVisible( false );
    donateDialog->getWindow()->setVisible( false );
    trainDialog->getWindow()->setVisible( false );

    resetBattles();
    
    // delete active projectiles
    Projectile::resetProjectiles();

    // delete the mission level's item and monster instances
    // This will also delete mission items from inventory. The mission will
    // still succeed.
    if( session->getCurrentMission() ) 
      session->getCurrentMission()->deleteItemMonsterInstances();

    session->deleteCreaturesAndItems(true);

    // remember the last mission
    lastMission = session->getCurrentMission();

    // delete map
    if( dg ) {
      delete dg; 
      dg = NULL;
    }

    //cerr << "Mission end: changingStory=" << changingStory << " inHQ=" << inHq << " teleporting=" << teleporting << " nextMission=" << nextMission << endl;
    if(!changingStory) {
      if(!inHq) {
        if(teleporting) {


          /* 
            When on the lower levels of a named mission, teleport to level 0 
            of the mission. Otherwise go back to HQ when coming from a mission.
          */
          if( getSession()->getCurrentMission() ) {
            cerr << "current mission " << getSession()->getCurrentMission()->getMapName() << endl;
          } else {
            cerr << "no mission" << endl;
          }
          if( getSession()->getCurrentMission() &&
              getSession()->getCurrentMission()->getMapName() &&
              strlen( getSession()->getCurrentMission()->getMapName() ) &&
              currentStory > 0 ) {
            cerr << "\tgoing to level 0" << endl;
            // to 0-th depth in edited map
            oldStory = currentStory;
            currentStory = 0;
            changingStory = true;
      //      gatepos = pos;
          } else {
            cerr << "\tgoing to hq" << endl;
            // to HQ
            oldStory = currentStory = 0;
            nextMission = -1;
          }



//          nextMission = -1;
//          oldStory = currentStory = 0;          
        } else {
          break;
        }
      } else if(nextMission == -1) {
        // if quiting in HQ, exit loop
        break;
      }// otherwise go back to HQ when coming from a mission	
    }
  }
#ifdef HAVE_SDL_NET
  session->stopClientServer();
#endif
  session->deleteCreaturesAndItems(false);

  // delete the party (w/o deleting the party ui)
  party->deleteParty();

#ifdef DEBUG_KEYS
  squirrelWin->setVisible( false );
#endif
  getSession()->getSquirrel()->endGame();
}

void Scourge::endMission() {
  for(int i = 0; i < party->getPartySize(); i++) {
    party->getParty(i)->setSelXY(-1, -1);   // stop moving
  }
  movingItem = NULL;          // stop moving items
}

void Scourge::drawView() {

  // move inventory window with party window
  inventory->positionWindow();

  // make a move (player, monsters, etc.)
  playRound();

  updatePartyUI();

  checkForDropTarget();
  checkForInfo();

  levelMap->draw();

  miniMap->drawMap();

  // the boards outside the map
  drawOutsideMap();

  glDisable( GL_DEPTH_TEST );
  glDisable( GL_TEXTURE_2D );

  if(isInfoShowing) {
    levelMap->initMapView();

    // creatures first
    for(int i = 0; i < session->getCreatureCount(); i++) {
      //if(!session->getCreature(i)->getStateMod(Constants::dead) && 
      if( levelMap->isLocationVisible(toint(session->getCreature(i)->getX()), 
                                      toint(session->getCreature(i)->getY())) &&
          levelMap->isLocationInLight(toint(session->getCreature(i)->getX()), 
                                      toint(session->getCreature(i)->getY()))) {
        showCreatureInfo(session->getCreature(i), false, false, false);
      }
    }
    // party next so red target circle shows over gray
    for(int i = 0; i < party->getPartySize(); i++) {
      //if(!party->getParty(i)->getStateMod(Constants::dead)) {

        bool player = party->getPlayer() == party->getParty(i);
        if( getUserConfiguration()->isBattleTurnBased() && 
           party->isRealTimeMode() && 
           battleTurn < (int)battleRound.size()) {
          player = (party->getParty(i) == battleRound[battleTurn]->getCreature());
        }
        showCreatureInfo(party->getParty(i), 
                         player, 
                         (levelMap->getSelectedDropTarget() && 
                          levelMap->getSelectedDropTarget()->creature == party->getParty(i)),
                         !party->isPlayerOnly());
    //}
    }

    drawInfos();

    glDisable( GL_CULL_FACE );
    glDisable( GL_SCISSOR_TEST );
  }

  drawDescriptions(messageList);

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );

  drawBorder();

  // draw the current text effect
  if( textEffect ) {
    if( SDL_GetTicks() - textEffectTimer < 5000 ) {
      textEffect->draw();
    } else {
      delete textEffect;
      textEffect = NULL;
    }
  }

  // Hack: A container window may have been closed by hitting the Esc. button.
  if(Window::windowWasClosed) {
    if(containerGuiCount > 0) {
      for(int i = 0; i < containerGuiCount; i++) {
        if(!containerGui[i]->getWindow()->isVisible()) {
          closeContainerGui(containerGui[i]);
        }
      }
    }
  }
}

void Scourge::drawDescriptions(ScrollingList *list) {
  if( levelMap->didDescriptionsChange()) {
    levelMap->setDescriptionsChanged( false );
    list->setLines( levelMap->getDescriptionCount(), 
                    levelMap->getDesriptions(),
                    levelMap->getDesriptionColors() );
    list->setSelectedLine( levelMap->getDescriptionCount() - 1);
  }
}

void Scourge::drawOutsideMap() {
  // cover the area outside the map
  if(levelMap->getViewWidth() < getSDLHandler()->getScreen()->w || 
     levelMap->getViewHeight() < getSDLHandler()->getScreen()->h) {
    //glPushAttrib( GL_ENABLE_BIT );
    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    glColor3f( 1, 1, 1 );
    glBindTexture( GL_TEXTURE_2D, getSession()->getShapePalette()->getGuiWoodTexture() );
    
    //    float TILE_W = 510 / 2.0f;
    float TILE_H = 270 / 2.0f; 
    
    glLoadIdentity();
    glTranslatef( levelMap->getViewWidth(), 0, 0 );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2i( 0, 0 );
    glTexCoord2f( 0, getSDLHandler()->getScreen()->h / TILE_H );
    glVertex2i( 0, getSDLHandler()->getScreen()->h );
    glTexCoord2f( 1, getSDLHandler()->getScreen()->h / TILE_H );
    glVertex2i( getSDLHandler()->getScreen()->w - levelMap->getViewWidth(), getSDLHandler()->getScreen()->h );
    glTexCoord2f( 1, 0 );
    glVertex2i( getSDLHandler()->getScreen()->w - levelMap->getViewWidth(), 0 );
    glEnd();
    
    glLoadIdentity();
    glTranslatef( 0, levelMap->getViewHeight(), 0 );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2i( 0, 0 );
    glTexCoord2f( 0, levelMap->getViewHeight() / TILE_H );
    glVertex2i( 0, levelMap->getViewHeight() );
    glTexCoord2f( 1, levelMap->getViewHeight() / TILE_H );
    glVertex2i( levelMap->getViewWidth(), levelMap->getViewHeight() );
    glTexCoord2f( 1, 0 );
    glVertex2i( levelMap->getViewWidth(), 0 );
    glEnd();
    
    glPopMatrix();
    //    glPopAttrib();
    glEnable( GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );
    glDisable( GL_TEXTURE_2D );
  }  
}

bool Scourge::inTurnBasedCombatPlayerTurn() {
  return (inTurnBasedCombat() &&
          !battleRound[battleTurn]->getCreature()->isMonster());
}         

void Scourge::drawAfter() {

  drawDraggedItem();

  // draw turn info
  if(inTurnBasedCombat()) {
    //glPushAttrib(GL_ENABLE_BIT);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( 20, 20, 0 );
    Creature *c = battleRound[battleTurn]->getCreature();
    char msg[80];
    sprintf(msg, "%s %d/%d", 
            c->getName(),
            c->getBattle()->getAP(),
            c->getBattle()->getStartingAP());
    turnProgress->updateStatus(msg, false,     
                               c->getBattle()->getAP(), 
                               c->getBattle()->getStartingAP(),
                               c->getPath()->size() );
    glPopMatrix();
    //glPushAttrib(GL_ENABLE_BIT);
  }
}

void Scourge::showCreatureInfo(Creature *creature, bool player, bool selected, bool groupMode) {
  glPushMatrix();
  //showInfoAtMapPos(creature->getX(), creature->getY(), creature->getZ(), creature->getName());

  glEnable( GL_DEPTH_TEST );
  glDepthMask(GL_FALSE);
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glDisable( GL_CULL_FACE );

  // draw circle
  double w = ((double)(creature->getShape()->getWidth()) / 2.0f) / DIV;
  double d = (((double)(creature->getShape()->getWidth()) / 2.0f) + 1.0f) / DIV;
  double s = 0.35f / DIV;

  float xpos2, ypos2, zpos2;

  Uint32 t = SDL_GetTicks();
  if(t - lastTargetTick > 45) {
    // initialize target width
    if(targetWidth == 0.0f) {
      targetWidth = s;
      targetWidthDelta *= -1.0f;
    }
    // targetwidth oscillation
    targetWidth += targetWidthDelta;
    if((targetWidthDelta < 0 && targetWidth < s) || 
       (targetWidthDelta > 0 && targetWidth >= s + (5 * targetWidthDelta)))
      targetWidthDelta *= -1.0f;

    lastTargetTick = t;
  }

  // show path
  if( !creature->getStateMod( Constants::dead ) &&
      player && 
      getUserConfiguration()->isBattleTurnBased() && 
      battleTurn < (int)battleRound.size() ) {
    for( int i = creature->getPathIndex(); 
         i < (int)creature->getPath()->size(); i++) {
      Location pos = (*(creature->getPath()))[i];

      glColor4f(1, 0.4f, 0.0f, 0.5f);
      xpos2 = ((float)(pos.x - levelMap->getX()) / DIV);
      ypos2 = ((float)(pos.y - levelMap->getY()) / DIV);
      zpos2 = 0.0f / DIV;  
      glPushMatrix();
      glTranslatef( xpos2 + w, ypos2 - w, zpos2 + 5);
      gluDisk(quadric, 0, 4, 12, 1);
      glPopMatrix();
    }
  }

  // Yellow for move creature target
  if( !creature->getStateMod( Constants::dead ) &&
      player && creature->getSelX() > -1 && 
      !creature->getTargetCreature() &&
      !(creature->getSelX() == toint(creature->getX()) && 
        creature->getSelY() == toint(creature->getY())) ) {
    // draw target
    glColor4f(1.0f, 0.75f, 0.0f, 0.5f);
    xpos2 = ((float)(creature->getSelX() - levelMap->getX()) / DIV);
    ypos2 = ((float)(creature->getSelY() - levelMap->getY()) / DIV);
    zpos2 = 0.0f / DIV;  
    glPushMatrix();
    //glTranslatef( xpos2 + w, ypos2 - w * 2, zpos2 + 5);
    glTranslatef( xpos2 + w, ypos2 - w, zpos2 + 5);
    gluDisk(quadric, w - targetWidth, w, 12, 1);
    glPopMatrix();
  }

  // red for attack target
  if( !creature->getStateMod( Constants::dead ) && 
      player && 
      creature->getTargetCreature() && 
      !creature->getTargetCreature()->getStateMod( Constants::dead ) ) {
    double tw = ((double)creature->getTargetCreature()->getShape()->getWidth() / 2.0f) / DIV;
    double td = (((double)(creature->getTargetCreature()->getShape()->getWidth()) / 2.0f) + 1.0f) / DIV;
    //double td = ((double)(creature->getTargetCreature()->getShape()->getDepth())) / DIV;
    glColor4f(1.0f, 0.15f, 0.0f, 0.5f);
    xpos2 = ((float)(creature->getTargetCreature()->getX() - levelMap->getX()) / DIV);
    ypos2 = ((float)(creature->getTargetCreature()->getY() - levelMap->getY()) / DIV);
    zpos2 = 0.0f / DIV;  
    glPushMatrix();
    //glTranslatef( xpos2 + tw, ypos2 - tw * 2, zpos2 + 5);
    glTranslatef( xpos2 + tw, ypos2 - td, zpos2 + 5);
    gluDisk(quadric, tw - targetWidth, tw, 12, 1);
    glPopMatrix();
  }

  xpos2 = (creature->getX() - (float)(levelMap->getX())) / DIV;
  ypos2 = (creature->getY() - (float)(levelMap->getY())) / DIV;
  zpos2 = creature->getZ() / DIV;  

  if(creature->getAction() != Constants::ACTION_NO_ACTION) {
    glColor4f(0, 0.7, 1, 0.5f);
  } else if(selected) {
    glColor4f(0, 1, 1, 0.5f);
  } else if(player) {
    glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
  } else if(creature->getMonster() && creature->getMonster()->isNpc()) {
    glColor4f(0.75f, 1.0f, 0.0f, 0.5f);
  } else {
    glColor4f(0.7f, 0.7f, 0.7f, 0.25f);
  }

  // draw state mods
  if( !creature->getStateMod( Constants::dead ) && 
      ( groupMode || player || creature->isMonster() )) {
    glEnable(GL_TEXTURE_2D);
    int n = 16;
    //float x = 0.0f;
    //float y = 0.0f;
    int on = 0;
    for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
      if(creature->getStateMod(i) && i != Constants::dead) {
        on++;
      }
    }
    int count = 0;
    for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
      if(creature->getStateMod(i) && i != Constants::dead) {

        /* DEBUG
        glPushMatrix();
        glTranslatef( xpos2, ypos2 - ( w * 2.0f ) - ( 1.0f / DIV ), zpos2 + 5);
        glColor4f( 1, 1, 1, 1 );
        glBegin( GL_LINES );
        glVertex3f( 0, 0, 0 );
        glVertex3f( 0, w*2.0f, 0 );
        glVertex3f( 0, w*2.0f, 0 );
        glVertex3f( w*2.0f, w*2.0f, 0 );
        glVertex3f( w*2.0f, w*2.0f, 0 );
        glVertex3f( w*2.0f, 0, 0 );
        glVertex3f( w*2.0f, 0, 0 );
        glVertex3f( 0, 0, 0 );
        glVertex3f( 0, 0, 0 );
        glVertex3f( w*2.0f, w*2.0f, 0 );
        glVertex3f( 0, w*2.0f, 0 );
        glVertex3f( w*2.0f, 0, 0 );
        glEnd();
        glPopMatrix();
        */

        glPushMatrix();
        glTranslatef( xpos2 + w, 
                      ypos2 - ( w * 2.0f ) - ( 1.0f / DIV ) + w, 
                      zpos2 + 5);
        float angle = -(count * 30) - (levelMap->getZRot() + 180);
        
        glRotatef( angle, 0, 0, 1 );
        glTranslatef( w + 15, 0, 0 );
        glRotatef( (count * 30) + 180, 0, 0, 1 );
        glTranslatef( -7, -7, 0 );

        GLuint icon = getSession()->getShapePalette()->getStatModIcon(i);
        if(icon) {
          glBindTexture( GL_TEXTURE_2D, icon );
        }
        glBegin( GL_QUADS );
        glNormal3f( 0, 0, 1 );
        if(icon) glTexCoord2f( 0, 0 );
        glVertex3f( 0, 0, 0 );
        if(icon) glTexCoord2f( 0, 1 );
        glVertex3f( 0, n, 0 );
        if(icon) glTexCoord2f( 1, 1 );
        glVertex3f( n, n, 0 );
        if(icon) glTexCoord2f( 1, 0 );
        glVertex3f( n, 0, 0 );
        glEnd();
        glPopMatrix();
        count++;
      }
    }
    glDisable(GL_TEXTURE_2D);
  }

  if( !creature->getStateMod( Constants::dead ) ) {
    glPushMatrix();
    //glTranslatef( xpos2 + w, ypos2 - w * 2, zpos2 + 5);
    glTranslatef( xpos2 + w, ypos2 - d, zpos2 + 5);
    if( groupMode || player || creature->isMonster() ) 
      gluDisk(quadric, w - s, w, 12, 1);
    glPopMatrix();
  }

  // draw recent damages
  glEnable(GL_TEXTURE_2D);
  getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );


  // show recent damages
  glDisable(GL_DEPTH_TEST);
  const float maxPos = 10.0f;
  const Uint32 posSpeed = 70;
  const float posDelta = 0.3f;
  for( int i = 0; i < creature->getRecentDamageCount(); i++ ) {
    DamagePos *dp = creature->getRecentDamage( i );
    xpos2 = ((float)( creature->getX() + 
                      creature->getShape()->getWidth() / 2 - 
                      levelMap->getX()) / DIV);
    ypos2 = ((float)( creature->getY() - 
                      creature->getShape()->getDepth() / 2 - 
                      levelMap->getY()) / DIV);
    zpos2 = ( (float)(creature->getShape()->getHeight() * 1.25f) + dp->pos ) / DIV;  
    glPushMatrix();
    //glTranslatef( xpos2 + w, ypos2 - w * 2, zpos2 + 5);
    glTranslatef( xpos2, ypos2, zpos2 );
    // rotate each particle to face viewer
    glRotatef( -( levelMap->getZRot() ), 0.0f, 0.0f, 1.0f);
    glRotatef( -levelMap->getYRot(), 1.0f, 0.0f, 0.0f);      

    float alpha = (float)( maxPos - dp->pos ) / ( maxPos * 0.75f );
    if( creature->isMonster() ) {
      glColor4f(0.75f, 0.75f, 0.75f, alpha );
    } else {
      glColor4f(1.0f, 1.0f, 0, alpha );
    }
    getSDLHandler()->texPrint( 0, 0, "%d", dp->damage );
    
    glPopMatrix();
    
    Uint32 now = SDL_GetTicks();
    if( now - dp->lastTime >= posSpeed ) {
      dp->pos += posDelta;
      dp->lastTime = now;
      if( dp->pos >= maxPos ) {
        creature->removeRecentDamage( i );
        i--;
      }
    }
  }
  glDisable(GL_TEXTURE_2D);
  getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
  glEnable(GL_DEPTH_TEST);


  glEnable( GL_CULL_FACE );
  glDisable( GL_BLEND );
  glDisable( GL_DEPTH_TEST );
  glDepthMask(GL_TRUE);

  // draw name
  //glTranslatef( 0, 0, 100);
  //getSDLHandler()->texPrint(0, 0, "%s", creature->getName());

  glPopMatrix();
}

void Scourge::drawDraggedItem() {
  if(getMovingItem()) {
    glPushMatrix();
    glLoadIdentity();	
    glTranslatef( getSDLHandler()->mouseX, getSDLHandler()->mouseY, 500);
    glRotatef( getMap()->getXRot(), 0.0f, 1.0f, 0.0f );  
    glRotatef( getMap()->getYRot(), 1.0f, 0.0f, 0.0f );  
    glRotatef( getMap()->getZRot(), 0.0f, 0.0f, 1.0f );

    DrawLater later;
    later.xpos = 0;
    later.ypos = 0;
    later.zpos = 0;
    later.shape = getMovingItem()->getShape();
    later.creature = NULL;
    later.item = getMovingItem();
    later.projectile = NULL;
    later.effect = NULL;
    later.name = 0;
    later.pos = NULL;

    levelMap->doDrawShape( &later );
    glPopMatrix();
  }


  /*
  float xpos2, ypos2, zpos2;  
  Shape *shape = NULL;  

  if(selX >= getX() && selX < getX() + MAP_VIEW_WIDTH &&
	 selY >= getY() && selY < getY() + MAP_VIEW_DEPTH &&
	 selZ < MAP_VIEW_HEIGHT &&
	 scourge->getMovingItem()) {

	shape = scourge->getMovingItem()->getShape();	
	int newz = selZ;
	Location *dropLoc = isBlocked(selX, selY, selZ, -1, -1, -1, shape, &newz);
	selZ = newz;

	// only let drop on other creatures and containers
	// unfortunately I have to call isWallBetween(), so objects aren't dragged behind walls
	// this makes moving items slow
	if(dropLoc || 
	   (oldLocatorSelX < MAP_WIDTH && 
		isWallBetween(selX, selY, selZ, 
					  oldLocatorSelX, 
					  oldLocatorSelY, 
					  oldLocatorSelZ))) {
	  selX = oldLocatorSelX;
	  selY = oldLocatorSelY;
	  selZ = oldLocatorSelZ;
	}
	
	xpos2 = ((float)(selX - getX()) / DIV);
	ypos2 = (((float)(selY - getY() - 1) - (float)shape->getDepth()) / DIV);
	zpos2 = (float)(selZ) / DIV;
	
	doDrawShape(xpos2, ypos2, zpos2, shape, 0);

	oldLocatorSelX = selX;
	oldLocatorSelY = selY;
	oldLocatorSelZ = selZ;
  }
  */
}

void Scourge::drawBorder() {
  if(levelMap->getViewWidth() == getSDLHandler()->getScreen()->w && 
     levelMap->getViewHeight() == getSDLHandler()->getScreen()->h &&
     !getUserConfiguration()->getFrameOnFullScreen()) return;

  glPushMatrix();
  glLoadIdentity();

  // ok change: viewx, viewy always 0
  //glTranslatef(viewX, viewY, 100);
  glTranslatef(0, 0, 100);

  //  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  // draw border
  glColor4f( 1, 1, 1, 1);

  int w = (getMap()->getViewWidth() == getSDLHandler()->getScreen()->w ? 
           getMap()->getViewWidth() : 
           getMap()->getViewWidth() - Window::SCREEN_GUTTER);
  int h = (getMap()->getViewHeight() == getSDLHandler()->getScreen()->h ? 
           getMap()->getViewHeight() : 
           getMap()->getViewHeight() - Window::SCREEN_GUTTER);
  float TILE_W = 20.0f;
  float TILE_H = 120.0f;

  glBindTexture( GL_TEXTURE_2D, getSession()->getShapePalette()->getBorderTexture() );
  glBegin( GL_QUADS );
  // left
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (0, 0);
  glTexCoord2f (0.0f, h/TILE_H);
  glVertex2i (0, h);
  glTexCoord2f (TILE_W/TILE_W, h/TILE_H);
  glVertex2i ((int)TILE_W, h);
  glTexCoord2f (TILE_W/TILE_W, 0.0f);      
  glVertex2i ((int)TILE_W, 0);

  // right
  int gutter = 5;
  glTexCoord2f (TILE_W/TILE_W, 0.0f);
  glVertex2i (w - (int)TILE_W + gutter, 0);
  glTexCoord2f (TILE_W/TILE_W, h/TILE_H);
  glVertex2i (w - (int)TILE_W + gutter, h);
  glTexCoord2f (0.0f, h/TILE_H);
  glVertex2i (w + gutter, h);
  glTexCoord2f (0.0f, 0.0f);      
  glVertex2i (w + gutter, 0);
  glEnd();

  TILE_W = 120.0f;
  TILE_H = 20.0f;
  glBindTexture( GL_TEXTURE_2D, getSession()->getShapePalette()->getBorder2Texture() );
  glBegin( GL_QUADS );
  // top
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (0, 0);
  glTexCoord2f (0.0f, TILE_H/TILE_H);
  glVertex2i (0, (int)TILE_H);
  glTexCoord2f (w/TILE_W, TILE_H/TILE_H);
  glVertex2i (w, (int)TILE_H);
  glTexCoord2f (w/TILE_W, 0.0f);      
  glVertex2i (w, 0);

  // bottom
  glTexCoord2f (w/TILE_W, TILE_H/TILE_H);
  glVertex2i (0, h - (int)TILE_H + gutter);
  glTexCoord2f (w/TILE_W, 0.0f);
  glVertex2i (0, h + gutter);
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (w, h + gutter);
  glTexCoord2f (0.0f, TILE_H/TILE_H);      
  glVertex2i (w, h - (int)TILE_H + gutter);
  glEnd();

  int gw = 220;
  int gh = 64;

  glEnable( GL_ALPHA_TEST );
  glAlphaFunc( GL_GREATER, 0 );
  glBindTexture( GL_TEXTURE_2D, getSession()->getShapePalette()->getGargoyleTexture() );

  glPushMatrix();
  glLoadIdentity();
  //glTranslatef(10, -5, 0);
  //glRotatef(20, 0, 0, 1);
  glBegin( GL_QUADS );
  // top left
  glTexCoord2f (0, 0);
  glVertex2i (0, 0);
  glTexCoord2f (0, 1);
  glVertex2i (0, gh);
  glTexCoord2f ((1.0f / gw) * (gw - 1), 1);
  glVertex2i (gw, gh);
  glTexCoord2f ((1.0f / gw) * (gw - 1), 0);      
  glVertex2i (gw, 0);
  glEnd();
  glPopMatrix();

  // top right
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(w - gw, 0, 0);
  //glRotatef(-20, 0, 0, 1);
  glBegin( GL_QUADS );
  glTexCoord2f ((1.0f / gw) * (gw - 1), 0);
  glVertex2i (0, 0);
  glTexCoord2f ((1.0f / gw) * (gw - 1), 1);
  glVertex2i (0, gh);
  glTexCoord2f (0, 1);
  glVertex2i (gw, gh);
  glTexCoord2f (0, 0);      
  glVertex2i (gw, 0);
  glEnd();

  glPopMatrix();

  //glEnable( GL_TEXTURE_2D );
  glDisable( GL_ALPHA_TEST );
  glEnable(GL_DEPTH_TEST);
  glPopMatrix();
}

bool Scourge::handleEvent(SDL_Event *event) {
  int ea;  

  if(containerGuiCount > 0) {
    for(int i = 0; i < containerGuiCount; i++) {
      if(containerGui[i]->handleEvent(event)) {
        closeContainerGui(containerGui[i]);
      }
    }
  }

  if(inventory->isVisible() && inventory->handleEvent(event) ) {
    return false;
  }

  if(optionsMenu->isVisible()) {
    optionsMenu->handleEvent(event);
    //    return false;
  }

  //if(multiplayer->isVisible()) {
//    multiplayer->handleEvent(event);
  //return false;
  //}

  levelMap->handleEvent( event );

  int mx, my;
  switch(event->type) {
  case SDL_MOUSEMOTION:
    
    if( !levelMap->isMouseRotating() ) {

      mx = event->motion.x;
      my = event->motion.y;

      // start the item drag
      if(willStartDrag && 
         (abs(mx - willStartDragX) > DRAG_START_TOLERANCE ||
          abs(my - willStartDragY) > DRAG_START_TOLERANCE)) {
        // click on an item
        Uint16 mapx = levelMap->getCursorMapX();
        Uint16 mapy = levelMap->getCursorMapY();
        Uint16 mapz = levelMap->getCursorMapZ();
        if(mapx > MAP_WIDTH) {
          levelMap->getMapXYAtScreenXY(willStartDragX, willStartDragY, &mapx, &mapy);
          mapz = 0;
        }
        startItemDrag(mapx, mapy, mapz);
        willStartDrag = false;
      }
    }
    break;
  case SDL_MOUSEBUTTONDOWN:
    if(event->button.button) {
      processGameMouseDown(getSDLHandler()->mouseX, getSDLHandler()->mouseY, event->button.button);
    }
    break;  
  case SDL_MOUSEBUTTONUP:
    if(event->button.button) {
      processGameMouseClick(getSDLHandler()->mouseX, getSDLHandler()->mouseY, event->button.button);
      if(teleporting && !exitConfirmationDialog->isVisible()) {
        exitLabel->setText(Constants::getMessage(Constants::TELEPORT_TO_BASE_LABEL));
        party->toggleRound(true);
        exitConfirmationDialog->setVisible(true);
      } else if(changingStory && !exitConfirmationDialog->isVisible()) {
        exitLabel->setText(Constants::getMessage(Constants::USE_GATE_LABEL));
        party->toggleRound(true);
        exitConfirmationDialog->setVisible(true);
      }
    }
    break;
  }
  switch(event->type) {
  case SDL_KEYDOWN:

    // DEBUG ------------------------------------

#ifdef DEBUG_KEYS
    if(event->key.keysym.sym == SDLK_d) {
      party->getPlayer()->takeDamage( 1000 );
      return false;
    } else if(event->key.keysym.sym == SDLK_l) {
      //cerr << "Lightmap is now=" << getMap()->toggleLightMap() << endl;

      party->getPlayer()->addExperience( 1000 );
      //session->creatureDeath( party->getPlayer() );

      return false;
    } else if(event->key.keysym.sym == SDLK_d) {
      // add a day
      getSession()->getParty()->getCalendar()->addADay();
    } else if(event->key.keysym.sym == SDLK_f) {
      getMap()->useFrustum = ( getMap()->useFrustum ? false : true );
      getMap()->refresh();
    } else if(event->key.keysym.sym == SDLK_r && 
              getSession()->getCurrentMission() && 
              !getSession()->getCurrentMission()->isCompleted()) {
      getSession()->getCurrentMission()->setCompleted( true );
      if( getSession()->getCurrentMission()->isStoryLine() )
        board->storylineMissionCompleted( getSession()->getCurrentMission() );
      missionCompleted();
    } else if( event->key.keysym.sym == SDLK_t ) {
      teleport();
    } else if( event->key.keysym.sym == SDLK_y ) {
      getBoard()->setStorylineIndex( getBoard()->getStorylineIndex() + 1 );
      cerr << "Incremented storyline index to " << getBoard()->getStorylineIndex() << endl;
    } else if( event->key.keysym.sym == SDLK_u ) {
      getBoard()->setStorylineIndex( getBoard()->getStorylineIndex() - 1 );
      cerr << "Decremented storyline index to " << getBoard()->getStorylineIndex() << endl;
    } else if(event->key.keysym.sym == SDLK_p) {
      cerr << "EFFECT!" << endl;
//      party->startEffect( Constants::EFFECT_CAST_SPELL, (Constants::DAMAGE_DURATION * 4));

      party->getPlayer()->startEffect( (int)( (float)Constants::EFFECT_COUNT * rand() / RAND_MAX ), 
                                       (Constants::DAMAGE_DURATION * 4) );
/*                                                                                           */
/*       levelMap->startEffect( toint(party->getPlayer()->getX()),                           */
/*                              toint(party->getPlayer()->getY()), 1,                        */
/*                              (int)( (float)Constants::EFFECT_COUNT * rand() / RAND_MAX ), */
/*                              (Constants::DAMAGE_DURATION * 4),                            */
/*                              12, 12 );                                                    */

    } else if(event->key.keysym.sym == SDLK_m) {
      Map::debugMd2Shapes = ( Map::debugMd2Shapes ? false : true );
      return false;
    } else if(event->key.keysym.sym == SDLK_b) {
      Battle::debugBattle = ( Battle::debugBattle ? false : true );
      return false;
    } else if(event->key.keysym.sym == SDLK_s) {
      squirrelWin->setVisible( squirrelWin->isVisible() ? false : true );
    }
#endif

    // END OF DEBUG ------------------------------------

  case SDL_KEYUP:

    if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_ESCAPE){
      if( inventory->inStoreSpellMode() ) {
        inventory->setStoreSpellMode( false );
        return false;
      } else if( getTargetSelectionFor() ) {
        // cancel target selection ( cross cursor )
        getTargetSelectionFor()->cancelTarget();
        setTargetSelectionFor( NULL );
        return false;
      } else if( exitConfirmationDialog->isVisible() ) {
        exitConfirmationDialog->setVisible(false);
      } else if( !Window::anyFloatingWindowsOpen() ) {
        party->toggleRound(true);
        exitConfirmationDialog->setVisible(true);
      }   
      return false;
    } else if( event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_BACKSPACE ) {
      SDLHandler::showDebugInfo = ( SDLHandler::showDebugInfo ? 0 : 1 );
    }

    // xxx_yyy_stop means : "do xxx_yyy action when the corresponding key is up"
    ea = getUserConfiguration()->getEngineAction(event);    
    if(ea == SWITCH_COMBAT) {
      resetBattles();
      getUserConfiguration()->setBattleTurnBased( getUserConfiguration()->isBattleTurnBased() ? false : true );
      char message[80];
      sprintf( message, "Combat is now %s.", ( getUserConfiguration()->isBattleTurnBased() ?
                                               "Turn-based" :
                                               "Real-time" ) );
      levelMap->addDescription( message, 0, 1, 1 );
    } else if(ea == SET_PLAYER_0){
      setPlayer(0);
    } else if(ea == SET_PLAYER_1){
      setPlayer(1);
    } else if(ea == SET_PLAYER_2){
      setPlayer(2);
    } else if(ea == SET_PLAYER_3){
      setPlayer(3);
    } else if(ea == SET_PLAYER_ONLY && !inTurnBasedCombat()) {
      party->togglePlayerOnly();
    }
    //    else if(ea == BLEND_A){
    else if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_6){
      blendA++; if(blendA >= 11) blendA = 0;
      fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
    }
    //    else if(ea == BLEND_B){    
    else if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_7){    
      blendB++; if(blendB >= 11) blendB = 0;
      fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
    } else if(ea == SHOW_INVENTORY){
      toggleInventoryWindow();
    } else if(ea == SHOW_OPTIONS_MENU){
      toggleOptionsWindow();
    } else if(ea == SET_NEXT_FORMATION_STOP){
      if(party->getFormation() < Creature::FORMATION_COUNT - 1) party->setFormation(party->getFormation() + 1);
      else party->setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
    } else if(ea == MINIMAP_ZOOM_IN || ea == MINIMAP_ZOOM_OUT){
      miniMap->setShowMiniMap( miniMap->isMiniMapShown() ? false : true );
    } else if(ea == TOGGLE_MAP_CENTER){
      bool mc;
      mc = getUserConfiguration()->getAlwaysCenterMap();
      getUserConfiguration()->setAlwaysCenterMap(!mc);
    } else if(ea == INCREASE_GAME_SPEED){
      addGameSpeed(-1);        
    } else if(ea == DECREASE_GAME_SPEED){
      addGameSpeed(1);        
    } else if(ea == START_ROUND) {
      party->toggleRound();
    } else if(ea == LAYOUT_1) {
      setUILayout(Constants::GUI_LAYOUT_ORIGINAL);
    } else if(ea == LAYOUT_2) {
      setUILayout(Constants::GUI_LAYOUT_BOTTOM);
//    } else if(ea == LAYOUT_3) {
//      setUILayout(Constants::GUI_LAYOUT_SIDE);
    } else if(ea == LAYOUT_4) {
      setUILayout(Constants::GUI_LAYOUT_INVENTORY);
    } else if( ea >= QUICK_SPELL_1 && ea <= QUICK_SPELL_12 ) {
      quickSpellAction( ea - QUICK_SPELL_1 );
    }
    break;
  default: break;
  }

  return false;  
}

void Scourge::processGameMouseDown(Uint16 x, Uint16 y, Uint8 button) {
  if(button == SDL_BUTTON_LEFT) {
    // will start to drag when the mouse has moved
    willStartDrag = true;
    willStartDragX = x;
    willStartDragY = y;
  }
}

void Scourge::processGameMouseClick(Uint16 x, Uint16 y, Uint8 button) {
  // don't drag if you haven't started yet
  willStartDrag = false;

  Uint16 mapx, mapy, mapz;
  //Creature *c = getTargetSelectionFor();
  if(button == SDL_BUTTON_LEFT) {

    mapx = levelMap->getCursorMapX();
    mapy = levelMap->getCursorMapY();
    mapz = levelMap->getCursorMapZ();

    // drop target?
    Location *dropTarget = NULL;
    if(movingItem) {
      if(mapx < MAP_WIDTH) {
        dropTarget = levelMap->getLocation(mapx, mapy, mapz);
        if(!(dropTarget && 
             (dropTarget->creature || 
              (dropTarget->item && 
               ((Item*)(dropTarget->item))->getRpgItem()->getType() == RpgItem::CONTAINER)))) {
          dropTarget = NULL;
        }      
      }
    }
    levelMap->setSelectedDropTarget(dropTarget);

    // clicking on a creature
    if(!movingItem && mapx < MAP_WIDTH) {
      Location *loc = levelMap->getLocation(mapx, mapy, mapz);
      if(loc && loc->creature) {
        if(getTargetSelectionFor()) {
          handleTargetSelectionOfCreature( ((Creature*)loc->creature) );
          return;
        } else if(loc->creature->isMonster()) {
          if( ((Creature*)(loc->creature))->getMonster()->isNpc() ) {
            // start a conversation
            conversationGui->start( ((Creature*)(loc->creature)) );
          } else {
            // follow this creature
            party->setTargetCreature( ((Creature*)(loc->creature)) );
            // show path
            if( inTurnBasedCombatPlayerTurn() ) {
              // start round
              if( getSDLHandler()->isDoubleClick ) {
                party->toggleRound( false );
              }
            }
          }
          return;
        } else {
          // select player
          for(int i = 0; i < party->getPartySize(); i++) {
            if(party->getParty(i) == loc->creature) {
              setPlayer(i);
              return;
            }
          }
        }
      }
    }

    // click on an item
    if(mapx > MAP_WIDTH) {
      mapx = levelMap->getCursorFlatMapX();
      mapy = levelMap->getCursorFlatMapY();
      mapz = 0;
    }

    if( getTargetSelectionFor() ) {
      Location *pos = levelMap->getLocation(mapx, mapy, mapz);
      if(mapx < MAP_WIDTH && pos && pos->item) {
        handleTargetSelectionOfItem( ((Item*)(pos->item)), pos->x, pos->y, pos->z );
        return;
      }
    }

    if(useItem(mapx, mapy, mapz)) return;

    // click on the levelMap
    mapx = levelMap->getCursorFlatMapX();
    mapy = levelMap->getCursorFlatMapY();

    // make sure the selected action can target a location
    if( getTargetSelectionFor() ) {
      handleTargetSelectionOfLocation( mapx, mapy, mapz );
     return;
    }


    
    // Make party move to new location
    if( !party->setSelXY( mapx, mapy ) ) {
      getSDLHandler()->setCursorMode( Constants::CURSOR_FORBIDDEN, true );
    }

    // start round
    if( inTurnBasedCombatPlayerTurn() ) {
      if( getSDLHandler()->isDoubleClick ) {
        party->toggleRound( false );
      }
    }

  } else if(button == SDL_BUTTON_RIGHT) {
    describeLocation(levelMap->getCursorMapX(), levelMap->getCursorMapY(), levelMap->getCursorMapZ());
  }
}

bool Scourge::handleTargetSelectionOfLocation( Uint16 mapx, Uint16 mapy, Uint16 mapz ) {
  bool ret = false;
  char msg[80];
  Creature *c = getTargetSelectionFor();
  if(c->getAction() == Constants::ACTION_CAST_SPELL &&
     c->getActionSpell() &&
     c->getActionSpell()->isLocationTargetAllowed()) {

    // assign this creature
    c->setTargetLocation(mapx, mapy, 0);
    char msg[80];
    sprintf(msg, "%s selected a target", c->getName());
    levelMap->addDescription(msg);       
    ret = true;
  } else {
    sprintf(msg, "%s cancelled a pending action.", c->getName());
    levelMap->addDescription(msg);
  }
  // turn off selection mode
  setTargetSelectionFor(NULL);
  return ret;
}

bool Scourge::handleTargetSelectionOfCreature( Creature *potentialTarget ) {
  bool ret = false;
  char msg[80];
  Creature *c = getTargetSelectionFor();
  // make sure the selected action can target a creature
  if(c->getAction() == Constants::ACTION_CAST_SPELL &&
     c->getActionSpell() &&
     c->getActionSpell()->isCreatureTargetAllowed()) {
    
    // assign this creature
    c->setTargetCreature( potentialTarget );
    char msg[80];
    sprintf(msg, "%s will target %s", c->getName(), c->getTargetCreature()->getName());
    levelMap->addDescription(msg);       
    ret = true;
  } else {
    sprintf(msg, "%s cancelled a pending action.", c->getName());
    levelMap->addDescription(msg);
  }
  // turn off selection mode
  setTargetSelectionFor(NULL);
  return ret;
}

bool Scourge::handleTargetSelectionOfItem( Item *item, int x, int y, int z ) {
  bool ret = false;
  char msg[80];  
  // make sure the selected action can target an item
  Creature *c = getTargetSelectionFor();
  if(c->getAction() == Constants::ACTION_CAST_SPELL &&
     c->getActionSpell() &&
     c->getActionSpell()->isItemTargetAllowed()) {
    
    // assign this creature
    c->setTargetItem( x, y, z, item );
    sprintf( msg, "%s targeted %s.", c->getName(), item->getRpgItem()->getName() );
    levelMap->addDescription( msg );
    ret = true;
  } else {
    sprintf( msg, "%s cancelled a pending action.", c->getName() );
    levelMap->addDescription( msg );
  }
  // turn off selection mode
  setTargetSelectionFor( NULL );
  return ret;
}

void Scourge::describeLocation(int mapx, int mapy, int mapz) {
  char s[300];
  if(mapx < MAP_WIDTH) {
    //fprintf(stderr, "\tclicked map coordinates: x=%u y=%u z=%u\n", mapx, mapy, mapz);
    Location *loc = levelMap->getPosition(mapx, mapy, mapz);
    if(loc) {
      char *description = NULL;
      Creature *creature = ((Creature*)(loc->creature));
      //fprintf(stderr, "\tcreature?%s\n", (creature ? "yes" : "no"));
      if(creature) {
        creature->getDetailedDescription(s);
        description = s;
      } else {
        Item *item = ((Item*)(loc->item));
        //fprintf(stderr, "\titem?%s\n", (item ? "yes" : "no"));
        if( item ) {
          //item->getDetailedDescription(s, false);
          //description = s;
          infoGui->setItem( item, getParty()->getPlayer()->getSkill(Constants::IDENTIFY_ITEM_SKILL) );
          if(!infoGui->getWindow()->isVisible()) infoGui->getWindow()->setVisible( true );
        } else {
          Shape *shape = loc->shape;
          //fprintf(stderr, "\tshape?%s\n", (shape ? "yes" : "no"));
          if(shape) {
            description = session->getShapePalette()->getRandomDescription(shape->getDescriptionGroup());
          }        
        }
      }
      if(description) {
        levelMap->addDescription(description);
      }
    }
  }
} 

void Scourge::startItemDragFromGui(Item *item) {
  movingX = -1;
  movingY = -1;
  movingZ = -1;
  movingItem = item;
}

bool Scourge::startItemDrag(int x, int y, int z) {
  if(movingItem) return false;
  Location *pos = levelMap->getPosition(x, y, z);
  if(pos) {
	if(getItem(pos)) {  
	  dragStartTime = SDL_GetTicks();
	  return true;
	}
  }
  return false;
}

void Scourge::endItemDrag() {
  // item move is over
  movingItem = NULL;
  movingX = movingY = movingZ = MAP_WIDTH + 1;  
}

bool Scourge::useItem(int x, int y, int z) {
  if (movingItem) {
    dropItem(levelMap->getCursorFlatMapX(), levelMap->getCursorFlatMapY());
    return true;
  }

  Location *pos = levelMap->getPosition(x, y, z);
  if (pos) {
    Shape *shape = (pos->item ? pos->item->getShape() : pos->shape);
    if (levelMap->isWallBetweenShapes(toint(party->getPlayer()->getX()), 
                                 toint(party->getPlayer()->getY()), 
                                 toint(party->getPlayer()->getZ()), 
                                 party->getPlayer()->getShape(),
                                 x, y, z,
                                 shape)) {
      levelMap->addDescription(Constants::getMessage(Constants::ITEM_OUT_OF_REACH));
      return true;
    } else {
      if (useLever(pos)) {
        return true;
      } else if (useDoor(pos)) {
        return true;
      } else if (useGate(pos)) {
        return true;
      } else if (useBoard(pos)) {
        return true;
      } else if (useTeleporter(pos)) {
        return true;
      } else if( usePool( pos ) ) {
        return true;
      } else if(pos && pos->item && ((Item*)(pos->item))->getRpgItem()->getType() == RpgItem::CONTAINER) {
        openContainerGui(((Item*)(pos->item)));      
        return true;
      }
    }
  }
  return false;
}

bool Scourge::getItem(Location *pos) {
    if(pos->item) {
      if(levelMap->isWallBetween(pos->x, pos->y, pos->z, 
                            toint(party->getPlayer()->getX()),
                            toint(party->getPlayer()->getY()),
                            0)) {
		levelMap->addDescription(Constants::getMessage(Constants::ITEM_OUT_OF_REACH));
	  } else {
        movingX = pos->x;
        movingY = pos->y;
        movingZ = pos->z;
        movingItem = ((Item*)(pos->item));		
		int x = pos->x;
		int y = pos->y;
		int z = pos->z;
        levelMap->removeItem(pos->x, pos->y, pos->z);
		levelMap->dropItemsAbove(x, y, z, movingItem);
		// draw the item as 'selected'
		levelMap->setSelectedDropTarget(NULL);
		//levelMap->handleMouseMove(movingX, movingY, movingZ);
	  }
	  return true;
    }
    return false;
}

// drop an item from the inventory
void Scourge::setMovingItem(Item *item, int x, int y, int z) {
  movingX = x;
  movingY = y;
  movingZ = z;
  movingItem = item;
}

int Scourge::dropItem(int x, int y) {
  int z=-1;
  bool replace = false;
  if(levelMap->getSelectedDropTarget()) {
    char message[120];
    Creature *c = ((Creature*)(levelMap->getSelectedDropTarget()->creature));
    if(c) {
      if(c->addInventory(movingItem)) {
        sprintf(message, "%s picks up %s.", 
                c->getName(), 
                movingItem->getItemName());
        levelMap->addDescription(message);
        // if the inventory is open, update it
        if(inventory->isVisible()) inventory->refresh();
      } else {
        showMessageDialog("The item won't fit in that container!");
        replace = true;
      }
    } else if(levelMap->getSelectedDropTarget()->item && 
              ((Item*)(levelMap->getSelectedDropTarget()->item))->getRpgItem()->getType() == RpgItem::CONTAINER) {
      if(!((Item*)(levelMap->getSelectedDropTarget()->item))->addContainedItem(movingItem)) {
        showMessageDialog("The item won't fit in that container!");
        replace = true;
      } else {
        sprintf(message, "%s is placed in %s.", 
                movingItem->getItemName(), 
                levelMap->getSelectedDropTarget()->item->getItemName());
        levelMap->addDescription(message);
        // if this container's gui is open, update it
        refreshContainerGui(((Item*)(levelMap->getSelectedDropTarget()->item)));
      }
    } else {
      replace = true;
    }
    levelMap->setSelectedDropTarget(NULL);
  } else {
    // see if it's blocked and get the value of z (stacking items)
    Location *pos = levelMap->isBlocked(x, y, 0,
                                   movingX, movingY, movingZ,
                                   movingItem->getShape(), &z);
    if(!pos && 
       !levelMap->isWallBetween(toint(party->getPlayer()->getX()), 
                           toint(party->getPlayer()->getY()), 
                           toint(party->getPlayer()->getZ()), 
                           x, y, z)) {
      levelMap->setItem(x, y, z, movingItem);
    } else {
      replace = true;
    }
  }

  // failed to drop item; put it back to where we got it from
  if(replace) {
    if(movingX <= -1 || movingX >= MAP_WIDTH) {
      // the item drag originated from the gui... what to do?
      // for now don't end the drag
      return z;
    } else {
      levelMap->isBlocked(movingX, movingY, movingZ,
                     -1, -1, -1,
                     movingItem->getShape(), &z);
      levelMap->setItem(movingX, movingY, z, movingItem);
    }
  }
  endItemDrag();
  getSDLHandler()->getSound()->playSound(Window::DROP_SUCCESS);
  return z;
}

bool Scourge::useGate(Location *pos) {
  for (int i = 0; i < party->getPartySize(); i++) {
    if (!party->getParty(i)->getStateMod(Constants::dead)) {
      if (pos->shape == getSession()->getShapePalette()->findShapeByName("GATE_UP")) {
        oldStory = currentStory;
        currentStory--;
        changingStory = true;
        gatepos = pos;
        return true;
      } else if (pos->shape == getSession()->getShapePalette()->findShapeByName("GATE_DOWN")) {
        oldStory = currentStory;
        currentStory++;
        changingStory = true;
        gatepos = pos;
        return true;
      }
    }
  }
  return false;
}

bool Scourge::useBoard(Location *pos) {
  if(pos->shape == getSession()->getShapePalette()->findShapeByName("BOARD")) {
    boardWin->setVisible(true);
    return true;
  }
  return false;
}

bool Scourge::usePool( Location *pos ) {
  if(pos->shape == getSession()->getShapePalette()->findShapeByName("POOL")) {
    session->getSquirrel()->callMapPosMethod( "usePool", pos->x, pos->y, pos->z );
    return true;
  }
  return false;
}

bool Scourge::useTeleporter(Location *pos) {
  if(pos->shape == getSession()->getShapePalette()->findShapeByName("TELEPORTER") ||
     pos->shape == getSession()->getShapePalette()->findShapeByName("TELEPORTER_BASE")) {
    if(levelMap->isLocked(pos->x, pos->y, pos->z)) {
      levelMap->addDescription(Constants::getMessage(Constants::TELEPORTER_OFFLINE));
      return true;
    } else {
      // able to teleport if any party member is alive
      for(int i = 0; i < 4; i++) {
        if(!party->getParty(i)->getStateMod(Constants::dead)) {
          teleporting = true;
          return true;
        }
      }
    }
  }
  return false;
}

bool Scourge::useLever(Location *pos) {
  Shape *newShape = NULL;
  if(pos->shape == getSession()->getShapePalette()->findShapeByName("SWITCH_OFF")) {
    newShape = getSession()->getShapePalette()->findShapeByName("SWITCH_ON");
  } else if(pos->shape == getSession()->getShapePalette()->findShapeByName("SWITCH_ON")) {
    newShape = getSession()->getShapePalette()->findShapeByName("SWITCH_OFF");
  }
  if(newShape) {
    int keyX = pos->x;
    int keyY = pos->y;
    int keyZ = pos->z;
    int doorX, doorY, doorZ;
    levelMap->getDoorLocation(keyX, keyY, keyZ,
                         &doorX, &doorY, &doorZ);
    // flip the switch
    levelMap->setPosition(keyX, keyY, keyZ, newShape);
    // unlock the door
    levelMap->setLocked(doorX, doorY, doorZ, (levelMap->isLocked(doorX, doorY, doorZ) ? false : true));
    // show message, depending on distance from key to door
    float d = Constants::distance(keyX,  keyY, 1, 1, doorX, doorY, 1, 1);
    if(d < 20.0f) {
      levelMap->addDescription(Constants::getMessage(Constants::DOOR_OPENED_CLOSE));
    } else if(d >= 20.0f && d < 100.0f) {
      levelMap->addDescription(Constants::getMessage(Constants::DOOR_OPENED));
    } else {
      levelMap->addDescription(Constants::getMessage(Constants::DOOR_OPENED_FAR));
    }
    return true;
  }
  return false;
}

bool Scourge::useDoor(Location *pos) {
  Shape *newDoorShape = NULL;
  Shape *oldDoorShape = pos->shape;
  if(oldDoorShape == getSession()->getShapePalette()->findShapeByName("EW_DOOR")) {
	newDoorShape = getSession()->getShapePalette()->findShapeByName("NS_DOOR");
  } else if(oldDoorShape == getSession()->getShapePalette()->findShapeByName("NS_DOOR")) {
	newDoorShape = getSession()->getShapePalette()->findShapeByName("EW_DOOR");
  }
  if(newDoorShape) {
	int doorX = pos->x;
	int doorY = pos->y;
	int doorZ = pos->z;
    
	// see if the door is open or closed. This is done by checking the shape above the
	// door. If there's something there and the orientation (NS vs. EW) matches, the
	// door is closed. I know it's a hack.
	Location *above = levelMap->getLocation(doorX, 
											doorY, 
											doorZ + pos->shape->getHeight());
	//if(above && above->shape) cerr << "ABOVE: shape=" << above->shape->getName() << endl;
	//else cerr << "Nothing above!" << endl;
	bool closed = ((pos->shape == getSession()->getShapePalette()->findShapeByName("EW_DOOR") &&
					above && above->shape == getSession()->getShapePalette()->findShapeByName("EW_DOOR_TOP")) ||
				   (pos->shape == getSession()->getShapePalette()->findShapeByName("NS_DOOR") &&
					above && above->shape == getSession()->getShapePalette()->findShapeByName("NS_DOOR_TOP")) ?
				   true : false);
	//cerr << "DOOR is closed? " << closed << endl;
	if(closed && levelMap->isLocked(doorX, doorY, doorZ)) {
	  levelMap->addDescription(Constants::getMessage(Constants::DOOR_LOCKED));
	  return true;
	}
	
	// switch door
	Sint16 ox = pos->x;
	Sint16 oy = pos->y;
	Sint16 nx = pos->x;
	Sint16 ny = (pos->y - pos->shape->getDepth()) + newDoorShape->getDepth();
	
	Shape *oldDoorShape = levelMap->removePosition(ox, oy, toint(party->getPlayer()->getZ()));
	Location *blocker = levelMap->isBlocked(nx, ny, toint(party->getPlayer()->getZ()),
											ox, oy, toint(party->getPlayer()->getZ()),
											newDoorShape);
	if( !blocker ) {

	  // there is a chance that the door will be destroyed
	  if( 0 == (int)( 20.0f * rand()/RAND_MAX ) ) {
		destroyDoor( ox, oy, oldDoorShape );
		levelMap->updateLightMap();
	  } else {
		levelMap->setPosition(nx, ny, toint(party->getPlayer()->getZ()), newDoorShape);
		levelMap->updateLightMap();          
		levelMap->updateDoorLocation(doorX, doorY, doorZ,
									 nx, ny, toint(party->getPlayer()->getZ()));
	  }
	  return true;
	} else if( blocker->creature && !( blocker->creature->isMonster() ) ) {
	  // rollback if blocked by a player
	  levelMap->setPosition(ox, oy, toint(party->getPlayer()->getZ()), oldDoorShape);
	  levelMap->addDescription(Constants::getMessage(Constants::DOOR_BLOCKED));
	  return true;
	} else {
	  // Deeestroy!
	  destroyDoor( ox, oy, oldDoorShape );
	  levelMap->updateLightMap();
	  return true;
	}
  }
  return false;
}

void Scourge::destroyDoor( Sint16 ox, Sint16 oy, Shape *shape ) {
  levelMap->addDescription( "The door splinters into many, tiny pieces!", 0, 1, 1 );
  for( int i = 0; i < 8; i++ ) {
	int x = ox + (int)( (float)( shape->getWidth() ) * rand() / RAND_MAX );
	int y = oy - (int)( (float)( shape->getDepth() ) * rand() / RAND_MAX );
	int z = 2 + (int)(( ( (float)( shape->getHeight() ) / 2.0f ) - 2.0f ) * rand() / RAND_MAX );
//	cerr << "starting effects at: " << x << "," << y << "," << z << endl;
	levelMap->startEffect( x, y, z, Constants::EFFECT_DUST, 
						   (GLuint)( (float)Constants::DAMAGE_DURATION / 2.0f ), 2, 2,
						   (GLuint)((float)(i) / 4.0f * (float)Constants::DAMAGE_DURATION) );
  }
}

void Scourge::toggleInventoryWindow() {
  if(inventory->isVisible()) {
    //if(!inventory->getWindow()->isLocked()) inventory->hide();
    inventory->hide();
  } else {
    inventory->show();
  }
  inventoryButton->setSelected( inventory->isVisible() );
}

void Scourge::toggleOptionsWindow() {
  if(optionsMenu->isVisible()) {
    optionsMenu->hide();
  } else {
    //	party->toggleRound(true);
	optionsMenu->show();
  }
  optionsButton->setSelected( optionsMenu->isVisible() );
}

void Scourge::showExitConfirmationDialog() {
  party->toggleRound(true);
  exitConfirmationDialog->setVisible(true);
}

bool Scourge::handleEvent(Widget *widget, SDL_Event *event) {

  if(widget == Window::message_button && info_dialog_showing) {
    party->toggleRound(false);
    info_dialog_showing = false;
    getSession()->getSquirrel()->startLevel();
    // re-eval the special skills
    cerr << "Evaluating special skills at level's start" << endl;
    Uint32 t = SDL_GetTicks();
    for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
      session->getParty()->getParty(i)->evalSpecialSkills();
    }
    for( int i = 0; i < session->getCreatureCount(); i++ ) {
      session->getCreature(i)->evalSpecialSkills();
    }
    cerr << "\tIt took: " << 
      ( ((float)( SDL_GetTicks() - t ) / 1000.0f ) ) << 
      " seconds." << endl;
    party->startEffect(Constants::EFFECT_TELEPORT, (Constants::DAMAGE_DURATION * 4));
  }
  
  if(containerGuiCount > 0) {
    for(int i = 0; i < containerGuiCount; i++) {
      if(containerGui[i]->handleEvent(widget, event)) {
        closeContainerGui(containerGui[i]);
      }
    }
    //	return false;
  }
  
  if(inventory->isVisible()) {
    inventory->handleEvent(widget, event);
    //	return false;
  }
  
  if(optionsMenu->isVisible()) {
    optionsMenu->handleEvent(widget, event);
    //	return false;
  }
  
  if(netPlay->getWindow()->isVisible()) {
    netPlay->handleEvent(widget, event);
  }
  
  //if(multiplayer->isVisible()) {
  //    multiplayer->handleEvent(widget, event);
  //return false;
  //}
  
  if(infoGui->getWindow()->isVisible()) {
    infoGui->handleEvent(widget, event);
  }

  if( conversationGui->getWindow()->isVisible() ) {
    conversationGui->handleEvent( widget, event );
  }

  if( tradeDialog->getWindow()->isVisible() ) {
    tradeDialog->handleEvent( widget, event );
  }

  if( healDialog->getWindow()->isVisible() ) {
    healDialog->handleEvent( widget, event );
  }

  if( donateDialog->getWindow()->isVisible() ) {
    donateDialog->handleEvent( widget, event );
  }

  if( trainDialog->getWindow()->isVisible() ) {
    trainDialog->handleEvent( widget, event );
  }

  // FIXME: this is hacky...
  if(handlePartyEvent(widget, event)) return true;
  int n = handleBoardEvent(widget, event);
  if(n == Board::EVENT_HANDLED) return false;
  else if(n == Board::EVENT_PLAY_MISSION) {
    int selected = missionList->getSelectedLine();
    if(selected != -1 && selected < board->getMissionCount()) {
      nextMission = selected;
      oldStory = currentStory = 0;
      endMission();
      return true;
    }
  }
  
  if(widget == yesExitConfirm) {
    exitLabel->setText(Constants::getMessage(Constants::EXIT_MISSION_LABEL));
    exitConfirmationDialog->setVisible(false);
    endMission();
    // move the creature to the gate so it will be near it on the next level
    if( gatepos ) {
      for (int i = 0; i < party->getPartySize(); i++) {
        if (!party->getParty(i)->getStateMod(Constants::dead)) {
          party->getParty(i)->moveTo( gatepos->x, gatepos->y, gatepos->z );
        }
      }
      gatepos = NULL;
    }
    return true;
  } else if(widget == noExitConfirm) {
    gatepos = NULL;
    teleporting = false;
    changingStory = false;
    currentStory = oldStory;
    exitLabel->setText(Constants::getMessage(Constants::EXIT_MISSION_LABEL));
    exitConfirmationDialog->setVisible(false);
    return false;
  } else if( widget == squirrelRun ||
             widget == squirrelText ) {
    squirrelLabel->appendText( "> " );
    squirrelLabel->appendText( squirrelText->getText() );
    squirrelLabel->appendText( "|" );
    getSession()->getSquirrel()->compileBuffer( squirrelText->getText() );
    squirrelText->clearText();
    squirrelLabel->appendText( "|" );
  } else if( widget == squirrelClear ) {
    squirrelLabel->setText( "" );
  }
  return false;
}

// create the ui
void Scourge::createUI() {

  infoGui = new InfoGui( this );

  conversationGui = new ConversationGui( this );
  tradeDialog = new TradeDialog( this );
  healDialog = new HealDialog( this );
  donateDialog = new DonateDialog( this );
  trainDialog = new TrainDialog( this );

  int width = 
    getSDLHandler()->getScreen()->w - 
    (PARTY_GUI_WIDTH + (Window::SCREEN_GUTTER * 2));
  messageWin = new Window( getSDLHandler(),
                           0, 0, width, PARTY_GUI_HEIGHT, 
                           "Messages", 
                           getSession()->getShapePalette()->getGuiTexture(), false,
                           Window::BASIC_WINDOW,
                           getSession()->getShapePalette()->getGuiTexture2() );
  messageWin->setBackground(0, 0, 0);
  messageList = new ScrollingList(0, 0, width, PARTY_GUI_HEIGHT - 25, getSession()->getShapePalette()->getHighlightTexture());
  messageList->setSelectionColor( 0.15f, 0.15f, 0.3f );
  messageList->setCanGetFocus( false );
  messageWin->addWidget(messageList);
// this has to be after addWidget
  messageList->setBackground( 1, 0.75f, 0.45f );
  messageList->setSelectionColor( 0.25f, 0.25f, 0.25f );

  // FIXME: try to encapsulate this in a class...
  //  exitConfirmationDialog = NULL;
  int w = 400;
  int h = 120;
  exitConfirmationDialog = new Window(getSDLHandler(),
                                      (getSDLHandler()->getScreen()->w/2) - (w/2), 
                                      (getSDLHandler()->getScreen()->h/2) - (h/2), 
                                      w, h,
                                      "Leave level?", 
                                      getSession()->getShapePalette()->getGuiTexture(), false,
                                      Window::BASIC_WINDOW,
                                      getSession()->getShapePalette()->getGuiTexture2());
  int mx = w / 2;
  yesExitConfirm = new Button( mx - 80, 50, mx - 10, 80, getSession()->getShapePalette()->getHighlightTexture(), "Yes" );
  exitConfirmationDialog->addWidget((Widget*)yesExitConfirm);
  noExitConfirm = new Button( mx + 10, 50, mx + 80, 80, getSession()->getShapePalette()->getHighlightTexture(), "No" );
  exitConfirmationDialog->addWidget((Widget*)noExitConfirm);
  exitLabel = new Label(20, 20, Constants::getMessage(Constants::EXIT_MISSION_LABEL));
  exitConfirmationDialog->addWidget((Widget*)exitLabel);

  squirrelWin = new Window( getSDLHandler(), 5, 0, getSDLHandler()->getScreen()->w - 10, 200, "Squirrel Console", 
                            getSession()->getShapePalette()->getGuiTexture(), true,
                            Window::BASIC_WINDOW, getSession()->getShapePalette()->getGuiTexture2() );
  squirrelLabel = new ScrollingLabel( 5, 0, getSDLHandler()->getScreen()->w - 20, 145, "" );
  squirrelLabel->setCanGetFocus( false );
  squirrelWin->addWidget( squirrelLabel );
  squirrelText = new TextField( 5, 150, 100 );
  squirrelWin->addWidget( squirrelText );
  squirrelRun = squirrelWin->createButton( getSDLHandler()->getScreen()->w - 100, 150, getSDLHandler()->getScreen()->w - 10, 170, "Run" );
  squirrelClear = squirrelWin->createButton( getSDLHandler()->getScreen()->w - 200, 150, getSDLHandler()->getScreen()->w - 110, 170, "Clear" );
}

void Scourge::setUILayout(int mode) {
  layoutMode = mode;
  setUILayout();
}

void Scourge::setUILayout() {

  // reshape the levelMap
  int mapX = 0;
  int mapY = 0;
  int mapWidth = getSDLHandler()->getScreen()->w;
  int mapHeight = getSDLHandler()->getScreen()->h;

  // move the message gui
  int width = 
  getSDLHandler()->getScreen()->w -                 
  (PARTY_GUI_WIDTH + (Window::SCREEN_GUTTER * 2));

  mainWin->setVisible( false );
  messageWin->setVisible(false);
  switch(layoutMode) {
  case Constants::GUI_LAYOUT_ORIGINAL:
    messageList->resize(width, PARTY_GUI_HEIGHT - 25);
    messageWin->resize(width, PARTY_GUI_HEIGHT);
    messageWin->move(getSDLHandler()->getScreen()->w - width, 0);
    messageWin->setLocked(false);
    mainWin->setLocked(false);
//  if(inventory->getWindow()->isLocked()) {
    inventory->hide();
//    inventory->getWindow()->setLocked(false);
//  }
    netPlay->getWindow()->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER);
    netPlay->getWindow()->setLocked(false);
    break;

  case Constants::GUI_LAYOUT_BOTTOM:
    messageList->resize(width, PARTY_GUI_HEIGHT - 25);
    messageWin->resize(width, PARTY_GUI_HEIGHT);
    messageWin->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER);
    mapHeight = getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER;
    messageWin->setLocked(true);
    mainWin->setLocked(true);
//  if(inventory->getWindow()->isLocked()) {
    //inventory->getWindow()->setVisible(false);
    inventory->hide();
//    inventory->getWindow()->setLocked(false);
//  }
    netPlay->getWindow()->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT * 2 - Window::SCREEN_GUTTER);
    netPlay->getWindow()->setLocked(true);
    break;

  case Constants::GUI_LAYOUT_SIDE:
    messageList->resize(PARTY_GUI_WIDTH, getSDLHandler()->getScreen()->h - (PARTY_GUI_HEIGHT + Window::SCREEN_GUTTER * 2 + MINIMAP_WINDOW_HEIGHT + 25));
    messageWin->resize(PARTY_GUI_WIDTH, 
                       getSDLHandler()->getScreen()->h - (PARTY_GUI_HEIGHT + Window::SCREEN_GUTTER * 2 + MINIMAP_WINDOW_HEIGHT));
    messageWin->move(getSDLHandler()->getScreen()->w - PARTY_GUI_WIDTH,  MINIMAP_WINDOW_HEIGHT + Window::SCREEN_GUTTER);
    //mapX = 400;
    mapX = 0;
    mapWidth = getSDLHandler()->getScreen()->w - PARTY_GUI_WIDTH;
    mapHeight = getSDLHandler()->getScreen()->h;
    messageWin->setLocked(true);
    mainWin->setLocked(true);
//  if(inventory->getWindow()->isLocked()) {
    inventory->hide();
//    inventory->getWindow()->setLocked(false);
//  }
    netPlay->getWindow()->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT);
    netPlay->getWindow()->setLocked(true);
    break;

  case Constants::GUI_LAYOUT_INVENTORY:
    messageList->resize(width, PARTY_GUI_HEIGHT - 25);
    messageWin->resize(width, PARTY_GUI_HEIGHT);
    messageWin->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER);
    mapHeight = getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER;
    messageWin->setLocked(true);
    mainWin->setLocked(true);
    inventory->hide();
    //inventory->getWindow()->move(getSDLHandler()->getScreen()->w - INVENTORY_WIDTH, 
                                 //getSDLHandler()->getScreen()->h - (PARTY_GUI_HEIGHT + INVENTORY_HEIGHT + Window::SCREEN_GUTTER));
//  inventory->getWindow()->setLocked(true);
    inventory->show(false);
    //mapX = INVENTORY_WIDTH;
    mapX = 0;
    mapWidth = getSDLHandler()->getScreen()->w - INVENTORY_WIDTH;
    mapHeight = getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER;
    netPlay->getWindow()->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT * 2 - Window::SCREEN_GUTTER);
    netPlay->getWindow()->setLocked(true);
    break;
  }

  inventoryButton->setSelected( inventory->isVisible() );
  optionsButton->setSelected( optionsMenu->isVisible() );

  messageWin->setVisible(true, false);

  mainWin->move(getSDLHandler()->getScreen()->w - PARTY_GUI_WIDTH,
                getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT);
  mainWin->setVisible( true, false );
  //inventory->positionWindow();

  // FIXME: resize levelMap drawing area to remainder of screen.
  levelMap->setViewArea(mapX, mapY, mapWidth, mapHeight);
  if(getParty()->getPlayer()) {
    getMap()->center(toint(getParty()->getPlayer()->getX()),
                     toint(getParty()->getPlayer()->getY()), 
                     true);
  }
}

void Scourge::playRound() {                           
  if(targetSelectionFor) return;
 
  // is the game not paused?
  if(party->isRealTimeMode()) {
    
    Projectile::moveProjectiles(getSession());

    // fight battle turns
    bool fromBattle = false;
    if(battleRound.size() > 0) {
      fromBattle = fightCurrentBattleTurn();
    } 

    // create a new battle round
    if(battleRound.size() == 0 && !createBattleTurns()) {
      // not in battle
      if(fromBattle) {
        // go back to real-time, group-mode
        resetUIAfterBattle();
      }
      moveCreatures();
    }
  }
}

// fight a turn of the battle
bool Scourge::fightCurrentBattleTurn() {
  if(battleRound.size() > 0) {

    // end of battle if party has no-one to attack
    bool roundOver = false;
    int c = 0;
    for( int i = 0; i < party->getPartySize(); i++ ) {
      if( party->getParty(i)->getAction() == Constants::ACTION_NO_ACTION && 
          !party->getParty(i)->getBattle()->getAvailableTarget() ) {
        c++;
      }
    }
    if( c == party->getPartySize() ) {
      for( int i = 0; i < party->getPartySize(); i++ ) {
        if( Battle::debugBattle ) cerr << "*** Reset in scourge!" << endl;
        party->getParty(i)->getBattle()->reset();
      }
      roundOver = true;
    }

    if( !roundOver ) {
      if( getUserConfiguration()->isBattleTurnBased() ) {
        // TB: fight the current battle turn only
        Battle *battle = battleRound[battleTurn];
        resetNonParticipantAnimation( battle );

        if( battle->fightTurn() ) {
          battleTurn++;
        }
        roundOver = ( battleTurn >= (int)battleRound.size() );
      } else {
        // RT: fight every battle turn
        for( int i = 0; i < (int)battleRound.size(); i++) {
          Battle *battle = battleRound[i];
          battle->fightTurn();
        }
        roundOver = true;
      }
    }

    if( roundOver ) {
      rtStartTurn = battleTurn = 0;
      if(battleRound.size()) battleRound.erase(battleRound.begin(), battleRound.end());
      
      if(Battle::debugBattle) cerr << "ROUND ENDS" << endl;
      if(Battle::debugBattle) cerr << "----------------------------------" << endl;
      return true;
    }
  }
  return false;
}

void Scourge::resetNonParticipantAnimation( Battle *battle ) {
  // in TB battle reset animations of non-participants
  for( int i = 0; i < session->getCreatureCount(); i++ ) {
    bool active = ( session->getCreature( i ) == battle->getCreature() ||
                    session->getCreature( i ) == battle->getCreature()->getTargetCreature() );
    ((MD2Shape*)session->getCreature(i)->getShape())->setPauseAnimation( !active );
  }
  for( int i = 0; i < getParty()->getPartySize(); i++ ) {
    bool active = ( getParty()->getParty( i ) == battle->getCreature() ||
                    getParty()->getParty( i ) == battle->getCreature()->getTargetCreature() );
    ((MD2Shape*)getParty()->getParty( i )->getShape())->setPauseAnimation( !active );
  }
}

bool Scourge::createBattleTurns() {
  if(!BATTLES_ENABLED) return false;

  // set up battles
  battleCount = 0;

  // anybody doing anything?
  for( int i = 0; i < party->getPartySize(); i++ ) {
    if( !party->getParty(i)->getStateMod(Constants::dead) ) {
      // possessed creature attacks fellows...
      if(party->getParty(i)->getTargetCreature() && 
         party->getParty(i)->getStateMod(Constants::possessed)) {
        Creature *target = session->getParty()->getClosestPlayer(toint(party->getParty(i)->getX()), 
                                                                 toint(party->getParty(i)->getY()), 
                                                                 party->getParty(i)->getShape()->getWidth(),
                                                                 party->getParty(i)->getShape()->getDepth(),
                                                                 20);
        if (target) {
          party->getParty(i)->setTargetCreature(target);
        }
      }
      bool hasTarget = (party->getParty(i)->hasTarget() || 
                        party->getParty(i)->getAction() > -1);
      bool visible = ( levelMap->isLocationVisible(toint(party->getParty(i)->getX()), 
                                                   toint(party->getParty(i)->getY())) &&
                       levelMap->isLocationInLight(toint(party->getParty(i)->getX()), 
                                                   toint(party->getParty(i)->getY())));
      if( hasTarget ) {
        if( party->getParty(i)->isTargetValid() && visible ) {
          if( Battle::debugBattle ) cerr << "*** init party target" << endl;
          battle[battleCount++] = party->getParty(i)->getBattle();
        } else {
          party->getParty(i)->cancelTarget();
        }
      }
    }
  }
  for (int i = 0; i < session->getCreatureCount(); i++) {
    if (!session->getCreature(i)->getStateMod(Constants::dead) &&
        session->getCreature(i)->getMonster() &&
        !session->getCreature(i)->getMonster()->isNpc() &&
        levelMap->isLocationVisible(toint(session->getCreature(i)->getX()), 
                                    toint(session->getCreature(i)->getY())) &&
        levelMap->isLocationInLight(toint(session->getCreature(i)->getX()), 
                                    toint(session->getCreature(i)->getY()))) {

      bool hasTarget = (session->getCreature(i)->getTargetCreature() ||
                        session->getCreature(i)->getAction() > -1);
      // Don't start a round if this creature is unreachable by party. Otherwise
      // this causes a lock-up.
      bool possible = ( session->getCreature(i)->getBattle()->getAvailablePartyTarget() != NULL );
      if( hasTarget ) {
        if( session->getCreature(i)->isTargetValid() ) {
          if( !possible ) {
            if( Battle::debugBattle ) 
              cerr << "*** not starting combat: possible is false." << endl;
            session->getCreature(i)->cancelTarget();
          } else battle[battleCount++] = session->getCreature(i)->getBattle();
        } else {
          session->getCreature(i)->cancelTarget();
        }
      }
    }
  }

  // if somebody is attacking (or casting a spell), enter battle mode
  // and add everyone present to the battle round.
  if (battleCount > 0) {

    // add other movement
    for (int i = 0; i < party->getPartySize(); i++) {
      bool visible = ( levelMap->isLocationVisible(toint(party->getParty(i)->getX()), 
                                              toint(party->getParty(i)->getY())) &&
                       levelMap->isLocationInLight(toint(party->getParty(i)->getX()), 
                                              toint(party->getParty(i)->getY())));
      if( visible && !party->getParty(i)->getStateMod(Constants::dead) ) {
        bool found = false;
        for( int t = 0; t < battleCount; t++ ) {
          if( battle[t] == party->getParty(i)->getBattle() ) {
            found = true;
            break;
          }
        }
        if( !found ) battle[battleCount++] = party->getParty(i)->getBattle();
      }
    }
    for (int i = 0; i < session->getCreatureCount(); i++) {
      if (!session->getCreature(i)->getStateMod(Constants::dead) &&
          session->getCreature(i)->getMonster() &&
          !session->getCreature(i)->getMonster()->isNpc() &&
          levelMap->isLocationVisible(toint(session->getCreature(i)->getX()), 
                                 toint(session->getCreature(i)->getY())) &&
          levelMap->isLocationInLight(toint(session->getCreature(i)->getX()), 
                                 toint(session->getCreature(i)->getY()))) {
        bool hasTarget = (session->getCreature(i)->getTargetCreature() ||
                          session->getCreature(i)->getAction() > -1);
        if (!hasTarget || (hasTarget && !session->getCreature(i)->isTargetValid())) {
          bool found = false;
          for( int t = 0; t < battleCount; t++ ) {
            if( battle[t] == session->getCreature(i)->getBattle() ) {
              found = true;
              break;
            }
          }
          if( !found ) battle[battleCount++] = session->getCreature(i)->getBattle();
        }
      }
    }

    party->savePlayerSettings();

    // order the battle turns by initiative
    Battle::setupBattles(getSession(), battle, battleCount, &battleRound);
    rtStartTurn = battleTurn = 0;
    if(Battle::debugBattle) cerr << "++++++++++++++++++++++++++++++++++" << endl;
    if(Battle::debugBattle) cerr << "ROUND STARTS" << endl;

    if(getUserConfiguration()->isBattleTurnBased()) groupButton->setVisible(false);
    return true;
  } else {
    return false;
  }
}

void Scourge::resetUIAfterBattle() {
  toggleRoundUI(party->isRealTimeMode());
  //party->setFirstLivePlayer();
  party->restorePlayerSettings();
  if(getUserConfiguration()->isBattleTurnBased() && 
     party->isPlayerOnly()) party->togglePlayerOnly();
  groupButton->setVisible(true);
  for(int i = 0; i < party->getPartySize(); i++) {
    party->getParty(i)->cancelTarget();
    ((MD2Shape*)party->getParty(i)->getShape())->setPauseAnimation( false );
    if(party->getParty(i)->anyMovesLeft()) {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_RUN, true);
    } else {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_STAND, true);
    }
  }
  // animate monsters again after TB combat (see resetNonParticipantAnimation() )
  for(int i = 0; i < session->getCreatureCount(); i++) {
    if( !session->getCreature(i)->getStateMod( Constants::dead ) &&
        !session->getCreature(i)->getMonster()->isNpc() ) {
      session->getCreature(i)->setMotion( Constants::MOTION_LOITER );
      ((MD2Shape*)session->getCreature(i)->getShape())->setPauseAnimation( false );
    }
  }
}

void Scourge::moveCreatures() {
  // change animation if needed
  for(int i = 0; i < party->getPartySize(); i++) {                            
    if(((MD2Shape*)(party->getParty(i)->getShape()))->getAttackEffect()) {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_ATTACK);	  
      ((MD2Shape*)(party->getParty(i)->getShape()))->setAngle(party->getParty(i)->getTargetAngle());
    } else if( party->getParty(i)->anyMovesLeft() && 
               ( !party->getPlayerMoved() || 
                 party->getParty(i) == party->getPlayer() ) ) {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_RUN);
    } else {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_STAND);
    }
  }

  // move the party members
  party->movePlayers();

  // move visible monsters
  for(int i = 0; i < session->getCreatureCount(); i++) {
    if(!session->getCreature(i)->getStateMod(Constants::dead) && 
       levelMap->isLocationVisible(toint(session->getCreature(i)->getX()), 
                                   toint(session->getCreature(i)->getY()))) {
      moveMonster(session->getCreature(i));
    }
  }
}

void Scourge::addGameSpeed(int speedFactor){
  char msg[80];
  getUserConfiguration()->setGameSpeedLevel(getUserConfiguration()->getGameSpeedLevel() + speedFactor);
  sprintf(msg, "Speed set to %d\n", getUserConfiguration()->getGameSpeedTicks());
  levelMap->addDescription(msg);
}

//#define MONSTER_FLEE_IF_LOW_HP

void Scourge::moveMonster(Creature *monster) {
  // set running animation (currently move or attack)
  if(((MD2Shape*)(monster->getShape()))->getAttackEffect()) {
    //monster->getShape()->setCurrentAnimation((int)MD2_ATTACK);
    //((MD2Shape*)(monster->getShape()))->setAngle(monster->getTargetAngle());
    // don't move when attacking
    return;
  } else {
    monster->getShape()->setCurrentAnimation( monster->getMotion() == Constants::MOTION_LOITER ? 
                                              (int)MD2_RUN :
                                              (int)MD2_STAND );
  }

  if(monster->getMotion() == Constants::MOTION_LOITER) {
    // attack the closest player
    if( BATTLES_ENABLED && 
        (int)(20.0f * rand()/RAND_MAX) == 0) {
      monster->decideMonsterAction();
    } else {
      // random (non-attack) monster movement
      monster->move(monster->getDir(), levelMap);
    }
  } else if(monster->getMotion() == Constants::MOTION_STAND) {
    if( (int)(40.0f * rand()/RAND_MAX) == 0) {
      monster->setMotion(Constants::MOTION_LOITER);
    } else {
      monster->decideMonsterAction();
    }
  } else if(monster->hasTarget()) {
#ifdef MONSTER_FLEE_IF_LOW_HP
    // monster gives up when low on hp or bored
    // FIXME: when low on hp, it should run away not loiter
    if(monster->getAction() == Constants::ACTION_NO_ACTION &&
       monster->getHp() < (int)((float)monster->getStartingHp() * 0.2f)) {
      monster->setMotion(Constants::MOTION_LOITER);
      monster->cancelTarget();
    } else {
      monster->moveToLocator(levelMap);
    }
#endif
    // see if there's another target that's closer
    if(monster->getAction() == Constants::ACTION_NO_ACTION) {
      monster->decideMonsterAction();
    }
  }
}

void Scourge::openContainerGui(Item *container) {
  // is it already open?
  for(int i = 0; i < containerGuiCount; i++) {
    if(containerGui[i]->getContainer() == container) {
      containerGui[i]->getWindow()->toTop();
      return;
    }
  }
  // open new window
  if(containerGuiCount < MAX_CONTAINER_GUI) {
    containerGui[containerGuiCount++] = new ContainerGui(this, container, 
                                                         10 + containerGuiCount * 15, 
                                                         10 + containerGuiCount * 15);
  }
}

void Scourge::closeContainerGui(ContainerGui *gui) {
  if(containerGuiCount <= 0) return;
  for(int i = 0; i < containerGuiCount; i++) {
	if(containerGui[i] == gui) {
	  for(int t = i; t < containerGuiCount - 1; t++) {
		containerGui[t] = containerGui[t + 1];
	  }
	  containerGuiCount--;
	  delete gui;
	  return;
	}
  }
}

void Scourge::closeAllContainerGuis() {
  for(int i = 0; i < containerGuiCount; i++) {
	delete containerGui[i];
  }
  containerGuiCount = 0;
}

void Scourge::refreshContainerGui(Item *container) {
  for(int i = 0; i < containerGuiCount; i++) {
	if(containerGui[i]->getContainer() == container) {
	  containerGui[i]->refresh();
	}
  }
}

void Scourge::showMessageDialog(char *message) {
  if( party && party->getPartySize() ) party->toggleRound(true);
  Window::showMessageDialog(getSDLHandler(), 
							getSDLHandler()->getScreen()->w / 2 - 200,
							getSDLHandler()->getScreen()->h / 2 - 55,
							400, 110, Constants::messages[Constants::SCOURGE_DIALOG][0],
							getSession()->getShapePalette()->getGuiTexture(),
							message);
}

Window *Scourge::createWoodWindow(int x, int y, int w, int h, char *title) {
  Window *win = new Window( getSDLHandler(), x, y, w, h, title, 
							getSession()->getShapePalette()->getGuiWoodTexture(),
							true, Window::SIMPLE_WINDOW );
  win->setBackgroundTileHeight(96);
  win->setBorderColor( 0.5f, 0.2f, 0.1f );
  win->setColor( 0.8f, 0.8f, 0.7f, 1 );
  win->setBackground( 0.65, 0.30f, 0.20f, 0.15f );
  win->setSelectionColor(  0.25f, 0.35f, 0.6f );
  return win;
}

Window *Scourge::createWindow(int x, int y, int w, int h, char *title) {
  Window *win = new Window( getSDLHandler(), x, y, w, h, title, 
                            getSession()->getShapePalette()->getGuiTexture(), 
                            true, Window::BASIC_WINDOW,
                            getSession()->getShapePalette()->getGuiTexture2() );
  return win;
}

void Scourge::missionCompleted() {
  showMessageDialog("Congratulations, mission accomplished!");
  
  // Award exp. points for completing the mission
  if(getSession()->getCurrentMission() && missionWillAwardExpPoints && 
     getSession()->getCurrentMission()->isCompleted()) {
    
    // only do this once
    missionWillAwardExpPoints = false;
    
    // how many points?
    int exp = (getSession()->getCurrentMission()->getLevel() + 1) * 100;
    levelMap->addDescription("For completing the mission", 0, 1, 1);
    char message[200];
    sprintf(message, "The party receives %d points.", exp);
    levelMap->addDescription(message, 0, 1, 1);
    
    for(int i = 0; i < getParty()->getPartySize(); i++) {
      int level = getParty()->getParty(i)->getLevel();
      if(!getParty()->getParty(i)->getStateMod(Constants::dead)) {
        int n = getParty()->getParty(i)->addExperience(exp);
        if(n > 0) {
          // sprintf(message, "%s gains %d experience points.", getParty()->getParty(i)->getName(), n);
          // getMap()->addDescription(message);
          if( level != getParty()->getParty(i)->getLevel() ) {
            sprintf(message, "%s gains a level!", getParty()->getParty(i)->getName());
            getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);
          }
        }
      }
    }
  }
}

#ifdef HAVE_SDL_NET
int Scourge::initMultiplayer() {
  int serverPort = 6666; // FIXME: make this more dynamic
  if(multiplayer->getValue() == MultiplayerDialog::START_SERVER) {
    session->startServer((GameStateHandler*)netPlay, serverPort);
    session->startClient((GameStateHandler*)netPlay, (CommandInterpreter*)netPlay, 
                         (char*)Constants::localhost, 
                         serverPort, 
                         (char*)Constants::adminUserName);
  } else {
    session->startClient((GameStateHandler*)netPlay, (CommandInterpreter*)netPlay,
                         multiplayer->getServerName(),
                         atoi(multiplayer->getServerPort()),
                         multiplayer->getUserName());
  }
  Progress *progress = new Progress(this->getSDLHandler(), 10, getSession()->getShapePalette()->getProgressTexture(), getSession()->getShapePalette()->getProgressHighlightTexture() );
  progress->updateStatus("Connecting to server");
  if(!session->getClient()->login()) {
    cerr << Constants::getMessage(Constants::CLIENT_CANT_CONNECT_ERROR) << endl;
    showMessageDialog(Constants::getMessage(Constants::CLIENT_CANT_CONNECT_ERROR));
    delete progress;
    return 0;
  }
  progress->updateStatus("Connected!");
  SDL_Delay(3000);

  delete progress;

  return 1;
} 


#endif

void Scourge::fightProjectileHitTurn(Projectile *proj, RenderedCreature *creature) {
  Battle::projectileHitTurn(getSession(), proj, (Creature*)creature);
}

void Scourge::fightProjectileHitTurn(Projectile *proj, int x, int y) {
  Battle::projectileHitTurn(getSession(), proj, x, y);
}

void Scourge::createPartyUI() {
  sprintf(version, "S.C.O.U.R.G.E. version %s", SCOURGE_VERSION);
  sprintf(min_version, "S.C.O.U.R.G.E.");
  mainWin = new Window( getSDLHandler(),
                        getSDLHandler()->getScreen()->w - Scourge::PARTY_GUI_WIDTH, 
                        getSDLHandler()->getScreen()->h - Scourge::PARTY_GUI_HEIGHT, 
                        Scourge::PARTY_GUI_WIDTH, 
                        Scourge::PARTY_GUI_HEIGHT, 
                        version, false, Window::BASIC_WINDOW,
                        "default" );
  cards = new CardContainer(mainWin);  

  roundButton = cards->createButton( 5, 0, 90, 20, "Real-Time", 0 );
  endTurnButton = cards->createButton( 5, 20, 90, 40, "End Turn", 0 );
  groupButton = cards->createButton( 5, 40,  90, 60, "Group Mode", 0 );
  inventoryButton = cards->createButton( 5, 60, 90, 80, "Party Info", 0 );  
  optionsButton = cards->createButton( 5, 80,  90, 100, "Options", 0 );
  quitButton = cards->createButton( 5, 100,  90, 120, "Quit", 0 );
  groupButton->setToggle(true);
  groupButton->setSelected(true);
  roundButton->setToggle(true);
  roundButton->setSelected(true);
  inventoryButton->setToggle(true);
  inventoryButton->setSelected(false);
  optionsButton->setToggle(true);
  optionsButton->setSelected(false);

  int offsetX = 90;
  int playerButtonWidth = (Scourge::PARTY_GUI_WIDTH - offsetX) / 4;
  //int playerButtonHeight = 20;  
  int playerInfoHeight = 100;
  //int playerButtonY = playerInfoHeight;

  for(int i = 0; i < 4; i++) {
    playerInfo[i] = new Canvas( offsetX + playerButtonWidth * i, 20,  
                                offsetX + playerButtonWidth * (i + 1) - 25, 20 + playerInfoHeight, 
                                this, this );
    cards->addWidget( playerInfo[i], 0 );
    playerHpMp[i] = new Canvas( offsetX + playerButtonWidth * (i + 1) - 25, 20,  
                                offsetX + playerButtonWidth * (i + 1), 20 + playerInfoHeight, 
                                this, NULL, true );
    cards->addWidget( playerHpMp[i], 0 );
  }
  int quickButtonWidth = (int)((float)(Scourge::PARTY_GUI_WIDTH - offsetX - 20) / 12.0f);
  for( int i = 0; i < 12; i++ ) {
    int xx = offsetX + quickButtonWidth * i + ( i / 4 ) * 10;
    quickSpell[i] = new Canvas( xx, 0, xx + quickButtonWidth, 20, 
                                this, NULL, true );
    cards->addWidget( quickSpell[i], 0 );
  }    
  
  cards->setActiveCard( 0 );   
}

void Scourge::receive( Widget *widget ) {
  if( getMovingItem() ) {
    int selected = -1;
    for(int i = 0; i < getParty()->getPartySize(); i++) {
      if( widget == playerInfo[i] ) {
        selected = i;
        break;
      }
    }
    if( selected == -1 ) return;

    // in TB combat only drop on current player
    if( getParty()->getPlayer() != getParty()->getParty( selected ) &&
        inTurnBasedCombat() ) return;

    getParty()->setPlayer( selected );
    inventory->receive( widget );
  }
}     

void Scourge::drawWidgetContents(Widget *w) {
  for(int i = 0; i < party->getPartySize(); i++) {
    if(playerInfo[i] == w) {
      drawPortrait( w, party->getParty( i ) );
      return;
    } else if( playerHpMp[i] == w ) {
      Creature *p = party->getParty( i );
      Util::drawBar( 10, 5, 90,
                     (float)p->getHp(), (float)p->getMaxHp(), 
                     -1, -1, -1, true, 
                     NULL,
                     //mainWin->getTheme(), 
                     Util::VERTICAL_LAYOUT );
      Util::drawBar( 17, 5, 90,
                     (float)p->getMp(), (float)p->getMaxMp(), 
                     0, 0, 1, false, 
                     NULL,
                     //mainWin->getTheme(), 
                     Util::VERTICAL_LAYOUT );
      return;
    }
  }
  for( int t = 0; t < 12; t++ ) {
    if( quickSpell[t] == w ) {
      quickSpell[t]->setGlowing( inventory->inStoreSpellMode() );
      for(int i = 0; i < party->getPartySize(); i++) {
        if( party->getParty( i ) == getParty()->getPlayer() ) {
          if( getParty()->getPlayer()->getQuickSpell( t ) ) {
            Storable *storable = getParty()->getPlayer()->getQuickSpell( t );
            w->setTooltip( (char*)(storable->getName()) );
            glEnable( GL_ALPHA_TEST );
            glAlphaFunc( GL_EQUAL, 0xff );
            glEnable(GL_TEXTURE_2D);
            glPushMatrix();
            //    glTranslatef( x, y, 0 );
            glBindTexture( GL_TEXTURE_2D, getSession()->getShapePalette()->spellsTex[ storable->getIconTileX() ][ storable->getIconTileY() ] );
            glColor4f(1, 1, 1, 1);
            
            glBegin( GL_QUADS );
            glNormal3f( 0, 0, 1 );
            glTexCoord2f( 0, 0 );
            glVertex3f( 0, 0, 0 );
            glTexCoord2f( 0, 1 );
            glVertex3f( 0, w->getHeight(), 0 );
            glTexCoord2f( 1, 1 );
            glVertex3f( w->getWidth(), w->getHeight(), 0 );
            glTexCoord2f( 1, 0 );
            glVertex3f( w->getWidth(), 0, 0 );
            glEnd();
            glPopMatrix();
            
            glDisable( GL_ALPHA_TEST );
            glDisable(GL_TEXTURE_2D);
          }
          return;
        }
      }
    }
  }
  

  cerr << "Warning: Unknown widget in Party::drawWidget." << endl;
  return;
}

void Scourge::drawPortrait( Widget *w, Creature *p ) {
  glPushMatrix();
  glEnable( GL_TEXTURE_2D );
  glColor4f( 1, 1, 1, 1 );
  if( p->getStateMod( Constants::dead ) ) {
    glBindTexture( GL_TEXTURE_2D, getSession()->getShapePalette()->getDeathPortraitTexture() );
  } else {
    glBindTexture( GL_TEXTURE_2D, getSession()->getShapePalette()->getPortraitTexture( p->getPortraitTextureIndex() ) );
  }
  int portraitSize = ((Scourge::PARTY_GUI_WIDTH - 90) / 4);
  int offs = 15;
  glBegin( GL_QUADS );
  glNormal3f( 0, 0, 1 );
  glTexCoord2f( 0, 0 );
  glVertex2i( -offs, 0 );
  glTexCoord2f( 1, 0 );
  glVertex2i( portraitSize - offs, 0 );
  glTexCoord2f( 1, 1 );
  glVertex2i( portraitSize - offs, portraitSize );
  glTexCoord2f( 0, 1 );
  glVertex2i( -offs, portraitSize );
  glEnd();
  glDisable( GL_TEXTURE_2D );

  bool shade = false;
  if( inTurnBasedCombat() ) {
    bool found = false;
    for( int i = battleTurn; i < (int)battleRound.size(); i++ ) {
      if( battleRound[i]->getCreature() == p ) {
        found = true;
        break;
      }
    }
    // already had a turn in battle
    if( !found ) {
      glColor4f( 0, 0, 0, 0.75f );
      shade = true;
    }
  } else if( p->getStateMod( Constants::possessed ) ) {
    glColor4f( 1.0f, 0, 0, 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::invisible ) ) {
    glColor4f( 0, 0.75f, 1.0f, 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::poisoned ) ) {
    glColor4f( 1, 0.75f, 0, 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::blinded ) ) {
    glColor4f( 1, 1, 1, 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::cursed ) ) {
    glColor4f( 0.75, 0, 0.75f, 0.5f );
    shade = true;
  }
  if( shade ) {
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glBegin( GL_QUADS );
    glVertex2i( 0, 0 );
    glVertex2i( portraitSize, 0 );
    glVertex2i( portraitSize, portraitSize );
    glVertex2i( 0, portraitSize );
    glEnd();
    glDisable( GL_BLEND );
  }

  glPopMatrix();
  
  glColor3f( 1, 1, 1 );
  getSDLHandler()->texPrint( 5, 12, "%s", p->getName() );

  // show if has available points
  if( p->getAvailableSkillPoints() > 0 ) {
    glColor3f( 1, 1, 0 );
    getSDLHandler()->texPrint( 5, 22, "PTS" );
  }

  // show stat mods
  glEnable(GL_TEXTURE_2D);
  glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
  int xp = 0;
  int yp = 1;
  float n = 12;
  int row = ( w->getWidth() / (int)n );
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    if(p->getStateMod(i)) {
      GLuint icon = getSession()->getShapePalette()->getStatModIcon(i);
      if(icon) {
        glBindTexture( GL_TEXTURE_2D, icon );
      }

      glPushMatrix();
      glTranslatef( 5 + xp * (n + 1), 
                    w->getHeight() - (yp * (n + 1)), 
                    0 );
      glBegin( GL_QUADS );
      glNormal3f( 0, 0, 1 );
      if(icon) glTexCoord2f( 0, 0 );
      glVertex3f( 0, 0, 0 );
      if(icon) glTexCoord2f( 0, 1 );
      glVertex3f( 0, n, 0 );
      if(icon) glTexCoord2f( 1, 1 );
      glVertex3f( n, n, 0 );
      if(icon) glTexCoord2f( 1, 0 );
      glVertex3f( n, 0, 0 );
      glEnd();
      glPopMatrix();

      xp++;
      if(xp >= row) {
        xp = 0;
        yp++;
      }
    }
  }
  glDisable(GL_TEXTURE_2D);

  // draw selection border
  if( p == getParty()->getPlayer() ) {
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    float lineWidth = 5.0f;
    glLineWidth( 5.0f );
    GuiTheme *theme = mainWin->getTheme();
    if( theme->getSelectionBackground() ) {
      glColor4f( theme->getSelectionBackground()->color.r,
                 theme->getSelectionBackground()->color.g,
                 theme->getSelectionBackground()->color.b,
                 0.5f );
    } else {
      mainWin->applySelectionColor();
    }
    glBegin( GL_LINE_LOOP );
    glVertex2f( lineWidth / 2.0f, lineWidth / 2.0f );
    glVertex2f( lineWidth / 2.0f, w->getHeight() - lineWidth / 2.0f );
    glVertex2f( w->getWidth() - lineWidth / 2.0f, w->getHeight() - lineWidth / 2.0f );
    glVertex2f( w->getWidth() - lineWidth / 2.0f, lineWidth / 2.0f );
    glEnd();
    glLineWidth( 1.0f );
    glDisable( GL_BLEND );
  }
}

void Scourge::resetPartyUI() {
  Event *e;  
  Date d(0, 0, 1, 0, 0, 0); // 2 hours (format : sec, min, hours, days, months, years)
  for(int i = 0; i < party->getPartySize() ; i++){
    e = new ThirstHungerEvent(party->getCalendar()->getCurrentDate(), d, party->getParty(i), this, Event::INFINITE_EXECUTIONS);
    party->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!
  }
}

bool Scourge::handlePartyEvent(Widget *widget, SDL_Event *event) {
  if(widget == inventoryButton) {
    toggleInventoryWindow();
  } else if(widget == endTurnButton &&
            inTurnBasedCombatPlayerTurn()) {
    battleRound[battleTurn]->endTurn();
  } else if(widget == optionsButton) {
    toggleOptionsWindow();
  } else if(widget == quitButton) {
    showExitConfirmationDialog();
    /*
  } else if(widget == diamondButton) {
    party->setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == staggeredButton) {
    party->setFormation(Constants::STAGGERED_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == squareButton) {
    party->setFormation(Constants::SQUARE_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == rowButton) {
    party->setFormation(Constants::ROW_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == scoutButton) {
    party->setFormation(Constants::SCOUT_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == crossButton) {
    party->setFormation(Constants::CROSS_FORMATION - Constants::DIAMOND_FORMATION);
    */
  } else if(widget == playerInfo[0] ) {
    if( getTargetSelectionFor() ) {
      handleTargetSelectionOfCreature( getParty()->getParty( 0 ) );
    } else {
      setPlayer(Constants::PLAYER_1 - Constants::PLAYER_1);
    }
  } else if(widget == playerInfo[1] ) {
    if( getTargetSelectionFor() ) {
      handleTargetSelectionOfCreature( getParty()->getParty( 1 ) );
    } else {
      setPlayer(Constants::PLAYER_2 - Constants::PLAYER_1);
    }
  } else if(widget == playerInfo[2] ) {
    if( getTargetSelectionFor() ) {
      handleTargetSelectionOfCreature( getParty()->getParty( 2 ) );
    } else {
      setPlayer(Constants::PLAYER_3 - Constants::PLAYER_1);
    }
  } else if(widget == playerInfo[3] ) {
    if( getTargetSelectionFor() ) {
      handleTargetSelectionOfCreature( getParty()->getParty( 3 ) );
    } else {
      setPlayer(Constants::PLAYER_4 - Constants::PLAYER_1);
    }
  } else if(widget == groupButton && !inTurnBasedCombat()) {
    party->togglePlayerOnly();
  } else if(widget == roundButton) {
    party->toggleRound();
  } else {
    for( int t = 0; t < 4; t++ ) {
      if( widget == playerHpMp[t] ) {
        inventory->showSkills();
        if( getParty()->getPlayer() != getParty()->getParty( t ) ) {
          getParty()->setPlayer( t );
          if( !inventory->isVisible() ) toggleInventoryWindow();
        } else {
          toggleInventoryWindow();
        }
      }
    }
    for( int t = 0; t < 12; t++ ) {
      if( widget == quickSpell[t] ) {
        quickSpellAction( t );
      }
    }
  }
  return false;
}

void Scourge::quickSpellAction( int index ) {
  if( inventory->inStoreSpellMode() ) {    
    getParty()->getPlayer()->setQuickSpell( index, inventory->getStorable() );
    inventory->setStoreSpellMode( false );
    if( inventory->isVisible() ) toggleInventoryWindow();
  } else {
    Creature *creature = getParty()->getPlayer();
    Storable *storable = creature->getQuickSpell( index );
    if( storable ) {
      if( storable->getStorableType() == Storable::SPELL_STORABLE ) {
        executeQuickSpell( (Spell*)storable );
      } else if( storable->getStorableType() == Storable::SPECIAL_STORABLE ) {
        executeSpecialSkill( (SpecialSkill*)storable );
      } else {
        cerr << "*** Error: unknown storable type: " << storable->getStorableType() << endl;
      }
    } else {
      inventory->showSpells();
      if( !inventory->isVisible() ) toggleInventoryWindow();
    }
  }
}

void Scourge::executeSpecialSkill( SpecialSkill *skill ) {
  Creature *creature = getSession()->getParty()->getPlayer();
  creature->
    setAction( Constants::ACTION_SPECIAL, 
               NULL,
               NULL,
               skill );
  creature->setTargetCreature(creature);
  /*
  char *err = 
    creature->useSpecialSkill( (SpecialSkill*)storable, true );
  if( err ) {
    showMessageDialog( err );
  }
  */
}

void Scourge::executeQuickSpell( Spell *spell ) {
  Creature *creature = getParty()->getPlayer();
  if(spell->getMp() > creature->getMp()) {
    showMessageDialog("Not enough Magic Points to cast this spell!");
  } else {
    creature->setAction(Constants::ACTION_CAST_SPELL, 
                        NULL,
                        spell);
    if(!spell->isPartyTargetAllowed()) {
      setTargetSelectionFor(creature);
    } else {
      creature->setTargetCreature(creature);
    }
  }
}

void Scourge::refreshInventoryUI(int playerIndex) {
  getInventory()->refresh(playerIndex);
}

void Scourge::refreshInventoryUI() {
  getInventory()->refresh();
}

void Scourge::updatePartyUI() {
  // update current date variables and see if scheduled events have occured  
  /*
  if(party->getCalendar()->update(getUserConfiguration()->getGameSpeedLevel())){
    calendarButton->setLabel(party->getCalendar()->getCurrentDate().getDateString());        
  }
  */
  // refresh levelMap if any party member's effect is on
  bool effectOn = false;
  for(int i = 0; i < party->getPartySize(); i++) {
	if(!party->getParty(i)->getStateMod(Constants::dead) && party->getParty(i)->isEffectOn()) {
	  effectOn = true;
	  break;
	}
  }
  if(effectOn != lastEffectOn) {
	lastEffectOn = effectOn;
	getMap()->refresh();
  }
}

void Scourge::setPlayer(int n) {
  // don't change player in TB combat
  if(battleTurn < (int)battleRound.size() &&
     getUserConfiguration()->isBattleTurnBased()) return;
  party->setPlayer(n);
}

void Scourge::setPlayerUI(int index) {
  if( tradeDialog->getWindow()->isVisible() ) tradeDialog->updateUI();
  if( healDialog->getWindow()->isVisible() ) healDialog->updateUI();
  if( donateDialog->getWindow()->isVisible() ) donateDialog->updateUI();
  if( trainDialog->getWindow()->isVisible() ) trainDialog->updateUI();
}

void Scourge::toggleRoundUI(bool startRound) {
  if(battleTurn < (int)battleRound.size() &&
     getUserConfiguration()->isBattleTurnBased()) {
    if(!startRound && 
       !battleRound[battleTurn]->getCreature()->isMonster()) {
      roundButton->setLabel("Begin Turn");
      roundButton->setGlowing(true);
    } else {
      roundButton->setLabel("...in Turn...");
      roundButton->setGlowing(false);
    }
  } else {
    if(startRound) roundButton->setLabel("Real-Time      ");
    else roundButton->setLabel("Paused");
    roundButton->setGlowing(false);
  }
  roundButton->setSelected(startRound);
}

void Scourge::setFormationUI(int formation, bool playerOnly) {
  groupButton->setSelected(playerOnly);
  roundButton->setSelected(true);
  /*
  diamondButton->setSelected(false);
  staggeredButton->setSelected(false);
  squareButton->setSelected(false);
  rowButton->setSelected(false);
  scoutButton->setSelected(false);
  crossButton->setSelected(false);
  switch(formation + Constants::DIAMOND_FORMATION) {
  case Constants::DIAMOND_FORMATION:
    diamondButton->setSelected(true); break;
  case Constants::STAGGERED_FORMATION:
    staggeredButton->setSelected(true); break;
  case Constants::SQUARE_FORMATION:
    squareButton->setSelected(true); break;
  case Constants::ROW_FORMATION:
    rowButton->setSelected(true); break;
  case Constants::SCOUT_FORMATION:
    scoutButton->setSelected(true); break;
  case Constants::CROSS_FORMATION:
    crossButton->setSelected(true); break;
  }
  */
}

void Scourge::togglePlayerOnlyUI(bool playerOnly) {
  groupButton->setSelected(playerOnly);
}

  // initialization events
void Scourge::initStart(int statusCount, char *message) {
  getSession()->getShapePalette()->preInitialize();
  progress = new Progress(this->getSDLHandler(), 
                          getSession()->getShapePalette()->getProgressTexture(), 
                          getSession()->getShapePalette()->getProgressHighlightTexture(),
                          statusCount, 
                          true, true);
  // Don't print text during startup. On windows this causes font corruption.
//  progress->updateStatus(message);
  progress->updateStatus(NULL);
}

void Scourge::initUpdate(char *message) {
  // Don't print text during startup. On windows this causes font corruption.  
//  progress->updateStatus(message);
  progress->updateStatus(NULL);  
}

void Scourge::initEnd() {
  delete progress;
  // re-create progress bar for map loading (recreate with different options)
  progress = new Progress(this->getSDLHandler(), 
                          getSession()->getShapePalette()->getProgressTexture(), 
                          getSession()->getShapePalette()->getProgressHighlightTexture(),
                          20, false, true);
}

void Scourge::createBoardUI() {
  // init gui
  /*
  boardWin = createWoodWindow((getSDLHandler()->getScreen()->w - BOARD_GUI_WIDTH) / 2, 
									   (getSDLHandler()->getScreen()->h - BOARD_GUI_HEIGHT) / 2, 
									   BOARD_GUI_WIDTH, BOARD_GUI_HEIGHT, 
									   "Available Missions");
*/                     
  boardWin = new Window( getSDLHandler(),
                        (getSDLHandler()->getScreen()->w - BOARD_GUI_WIDTH) / 2, 
                        (getSDLHandler()->getScreen()->h - BOARD_GUI_HEIGHT) / 2, 
                        BOARD_GUI_WIDTH, BOARD_GUI_HEIGHT, 
                        "Available Missions", true, Window::SIMPLE_WINDOW,
                        "wood" );
  missionList = new ScrollingList(5, 40, BOARD_GUI_WIDTH - 260, 150, getSession()->getShapePalette()->getHighlightTexture());
  boardWin->addWidget(missionList);
  boardWin->createLabel( BOARD_GUI_WIDTH - 250, 35, "Drag map to look around." );
  mapWidget = new MapWidget( this, boardWin, BOARD_GUI_WIDTH - 250, 40,
                             BOARD_GUI_WIDTH - 10, 
                             BOARD_GUI_HEIGHT - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 10, 
                             false );
  boardWin->addWidget( mapWidget );
  //missionDescriptionLabel = new Label(5, 210, "", 67);
  missionDescriptionLabel = new ScrollingLabel( 5, 210, 
                                                BOARD_GUI_WIDTH - 260, 
                                                BOARD_GUI_HEIGHT - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 210 - 10, "" );
  boardWin->addWidget(missionDescriptionLabel);
  playMission = new Button(5, 5, 105, 35, getSession()->getShapePalette()->getHighlightTexture(), Constants::getMessage(Constants::PLAY_MISSION_LABEL));
  boardWin->addWidget(playMission);
  closeBoard = new Button(110, 5, 210, 35, getSession()->getShapePalette()->getHighlightTexture(), Constants::getMessage(Constants::CLOSE_LABEL));
  boardWin->addWidget(closeBoard);
}

void Scourge::updateBoardUI(int count, const char **missionText, Color *missionColor) {
  missionList->setLines(count, missionText, missionColor);
}

int Scourge::handleBoardEvent(Widget *widget, SDL_Event *event) {
  if(widget == boardWin->closeButton || 
     widget == closeBoard) {
    boardWin->setVisible(false);
    return Board::EVENT_HANDLED;
  } else if(widget == missionList) {
    int selected = missionList->getSelectedLine();
    if(selected != -1 && selected < board->getMissionCount()) {
      Mission *mission = board->getMission(selected);
      missionDescriptionLabel->setText((char*)(mission->getDescription()));
      mapWidget->setSelection( mission->getMapX(), mission->getMapY() );
    }
    return Board::EVENT_HANDLED;
  } else if(widget == playMission) {
    int selected = missionList->getSelectedLine();
    if(selected != -1 && selected < board->getMissionCount()) {
      return Board::EVENT_PLAY_MISSION;
    }
    return Board::EVENT_HANDLED;
  }
  return -1;
}

void Scourge::setMissionDescriptionUI(char *s, int mapx, int mapy) {
  missionDescriptionLabel->setText(s);
  mapWidget->setSelection( mapx, mapy );
}

void Scourge::removeBattle(Battle *b) {
  for(int i = 0; i < (int)battleRound.size(); i++) {
    Battle *battle = battleRound[i];
    if(battle == b) {
      delete battle;
      if(battleTurn > i) battleTurn--;
      for(int t = i; t < (int)battleRound.size() - 1; t++) {
        battle[t] = battle[t + 1];
      }
      return;
    }
  }
}

void Scourge::resetBattles() {
  // delete battle references
  if(battleRound.size()) {
    for( int i = 0; i < (int)battleRound.size(); i++ ) {
      battleRound[i]->reset();
    }
    battleRound.erase(battleRound.begin(), battleRound.end());
  }
  for(int i = 0; i < MAX_BATTLE_COUNT; i++) {
    battle[i] = NULL;
  }
  battleCount = 0;
  battleTurn = 0;
  inBattle = false;  
}  

void Scourge::checkForDropTarget() {
  // find the drop target
  if(movingItem) {

    // is the mouse moving?
    if(!getSDLHandler()->mouseIsMovingOverMap) {
      if(needToCheckDropLocation) {
        needToCheckDropLocation = false;

        // check location
        Location *dropTarget = NULL;
        Uint16 mapx = levelMap->getCursorMapX();
        Uint16 mapy = levelMap->getCursorMapY();
        Uint16 mapz = levelMap->getCursorMapZ();
        if(mapx < MAP_WIDTH) {
          dropTarget = levelMap->getLocation(mapx, mapy, mapz);
          if(!(dropTarget && 
               (dropTarget->creature || 
                (dropTarget->item && 
                 ((Item*)(dropTarget->item))->getRpgItem()->getType() == RpgItem::CONTAINER)))) {
            dropTarget = NULL;
          }      
        }
        levelMap->setSelectedDropTarget(dropTarget);  
      }
    } else {
      needToCheckDropLocation = true;
    }
  }  
}

void Scourge::showItemInfoUI(Item *item, int level) {
  infoGui->setItem( item, level );
  if(!infoGui->getWindow()->isVisible()) infoGui->getWindow()->setVisible( true );    
}

void Scourge::checkForInfo() {
  Uint16 mapx, mapy, mapz;

  // change cursor when over a hostile creature  
  if( getSDLHandler()->getCursorMode() == Constants::CURSOR_NORMAL || 
      getSDLHandler()->getCursorMode() == Constants::CURSOR_ATTACK ||
      getSDLHandler()->getCursorMode() == Constants::CURSOR_RANGED ||
      getSDLHandler()->getCursorMode() == Constants::CURSOR_MOVE ||
      getSDLHandler()->getCursorMode() == Constants::CURSOR_TALK ||
      getSDLHandler()->getCursorMode() == Constants::CURSOR_USE ) {
    if( getSDLHandler()->mouseIsMovingOverMap ) {
      bool handled = false;
      mapx = levelMap->getCursorMapX();
      mapy = levelMap->getCursorMapY();
      mapz = levelMap->getCursorMapZ();
      if( mapx < MAP_WIDTH) {
        Location *pos = levelMap->getLocation(mapx, mapy, mapz);    
        if( pos ) {
          int cursor;
          if( pos->creature && 
              party->getPlayer()->canAttack( pos->creature, &cursor ) ) {
            getSDLHandler()->setCursorMode( ((Creature*)(pos->creature))->isMonster() && ((Creature*)(pos->creature))->getMonster()->isNpc() ?
                                            Constants::CURSOR_TALK :
                                            cursor );
                                            //Constants::CURSOR_ATTACK );
            handled = true;
          } else if( getOutlineColor( pos ) ) {
            getSDLHandler()->setCursorMode( Constants::CURSOR_USE );
            handled = true;
          }
        }
      }
      if( !handled ) getSDLHandler()->setCursorMode( Constants::CURSOR_NORMAL );
    }  
  }

  if( getUserConfiguration()->getTooltipEnabled() &&
      SDL_GetTicks() - getSDLHandler()->lastMouseMoveTime > 
      (Uint32)( getUserConfiguration()->getTooltipInterval() * 10) ) {
    if(needToCheckInfo) {
      needToCheckInfo = false;
      
      // check location
      Uint16 mapx = levelMap->getCursorMapX();
      Uint16 mapy = levelMap->getCursorMapY();
      Uint16 mapz = levelMap->getCursorMapZ();
      if(mapx < MAP_WIDTH) {
        Location *pos = levelMap->getLocation(mapx, mapy, mapz);
        if( pos ) {
          char s[300];
          void *obj = NULL;
          if( pos->creature ) {
            obj = pos->creature;
            ((Creature*)(pos->creature))->getDetailedDescription(s);
          } else if( pos->item ) {
            obj = pos->item;
            ((Item*)(pos->item))->getDetailedDescription(s);
          } else if( pos->shape ) {
            obj = pos->shape;
            strcpy( s, session->getShapePalette()->getRandomDescription( pos->shape->getDescriptionGroup() ) );
          }
          if( obj ) {
            bool found = false;
            // Don't show info about the same object twice
            // FIXME: use lookup table
            for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {
              InfoMessage *message = i->first;
              if( message->obj == obj || 
                  ( message->x == pos->x &&
                    message->y == pos->y &&
                    message->z == pos->z ) ) {
                found = true;
                break;
              }
            }
            if( !found ) {
              InfoMessage *message = 
                new InfoMessage( s, obj, pos->x, pos->y, 
                                 pos->z + pos->shape->getHeight() );
              infos[ message ] = SDL_GetTicks();
            }
          }
        }
      }
    } else {
      needToCheckInfo = true;
    }
  }
  // timeout descriptions
  Uint32 now = SDL_GetTicks();
  for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {
    InfoMessage *message = i->first;
    Uint32 time = i->second;
    if( now - time > INFO_INTERVAL ) {
      delete message;
      infos.erase( i );
    }
  }
}

void Scourge::resetInfos() {
  for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {
    InfoMessage *message = i->first;
    delete message;
    infos.erase( i );
  }
}

void Scourge::drawInfos() {
  float xpos2, ypos2, zpos2;
  for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {

    InfoMessage *message = i->first;
    xpos2 = ((float)(message->x - levelMap->getX()) / DIV);
    ypos2 = ((float)(message->y - levelMap->getY()) / DIV);
    zpos2 = ((float)(message->z) / DIV);

    getSDLHandler()->drawTooltip( xpos2, ypos2, zpos2, 
                                  -( levelMap->getZRot() ),
                                  -( levelMap->getYRot() ),
                                  message->message );
  }
}

void Scourge::createParty( Creature **pc, int *partySize ) { 
  mainMenu->createParty( pc, partySize ); 
}

void Scourge::teleport( bool toHQ ) {
  if( inHq || !session->getCurrentMission() ) {
    this->showMessageDialog( "This spell has no effect here..." );
  } else if( toHQ ) {
    //oldStory = currentStory = 0;
    teleporting = true;
    exitLabel->setText(Constants::getMessage(Constants::TELEPORT_TO_BASE_LABEL));
    party->toggleRound(true);
    exitConfirmationDialog->setVisible(true);
  } else {
    // teleport to a random depth within the same mission
    teleportFailure = true;

		oldStory = currentStory;
		currentStory = (int)( (float)( session->getCurrentMission()->getDepth() ) * rand() / RAND_MAX );
		changingStory = true;

    exitLabel->setText(Constants::getMessage(Constants::TELEPORT_TO_BASE_LABEL));
    party->toggleRound(true);
    exitConfirmationDialog->setVisible(true);
  }
}

void Scourge::loadMonsterSounds( char *type, map<int, vector<string>*> *soundMap ) {
  getSDLHandler()->getSound()->loadMonsterSounds( type, soundMap, getUserConfiguration() );
}

void Scourge::unloadMonsterSounds( char *type, map<int, vector<string>*> *soundMap ) {
  getSDLHandler()->getSound()->unloadMonsterSounds( type, soundMap );
}

ShapePalette *Scourge::getShapePalette() {
  return getSession()->getShapePalette();
}

GLuint Scourge::getCursorTexture( int cursorMode ) {
  return session->getShapePalette()->getCursorTexture( cursorMode );
}

GLuint Scourge::getHighlightTexture() { 
  return getShapePalette()->getHighlightTexture(); 
}

GLuint Scourge::loadSystemTexture( char *line ) { 
  return getShapePalette()->loadSystemTexture( line ); 
}

// check for interactive items.
Color *Scourge::getOutlineColor( Location *pos ) {
  return( pos->item || pos->shape->isInteractive() ? outlineColor : NULL );
}

bool Scourge::doesSaveGameExist(Session *session) {
  char path[300];
  get_file_name( path, 300, SAVE_FILE );
  FILE *fp = fopen( path, "rb" );
  bool ret = false;
  if(fp) {
    ret = true;
    fclose( fp );
  }
  return ret;
}

bool Scourge::saveGame(Session *session) {
  session->getPreferences()->createConfigDir();
  
  {
    char path[300];
    get_file_name( path, 300, SAVE_FILE );
    FILE *fp = fopen( path, "wb" );
    if(!fp) {
      cerr << "Error creating savegame file! path=" << path << endl;
      return false;
    }
    File *file = new File( fp );
    Uint32 n = PERSIST_VERSION;
    file->write( &n );
    n = session->getBoard()->getStorylineIndex();
    file->write( &n );
    n = session->getParty()->getPartySize();
    file->write( &n );
    for(int i = 0; i < session->getParty()->getPartySize(); i++) {
      CreatureInfo *info = session->getParty()->getParty(i)->save();
      Persist::saveCreature( file, info );
      Persist::deleteCreatureInfo( info );
    }
    delete file;
  }

  {
    char path[300];
    get_file_name( path, 300, VALUES_FILE );
    FILE *fp = fopen( path, "wb" );
    if(!fp) {
      cerr << "Error creating values file! path=" << path << endl;
      return false;
    }
    File *file = new File( fp );
    getSession()->getSquirrel()->saveValues( file );
    delete file;
  }

  return true;
}

bool Scourge::loadGame(Session *session) {
  {
    char path[300];
    get_file_name( path, 300, SAVE_FILE );
    FILE *fp = fopen( path, "rb" );
    if(!fp) {
      return false;
    }
    File *file = new File( fp );
    Uint32 n = PERSIST_VERSION;
    file->read( &n );
    if( n < OLDEST_HANDLED_VERSION ) {
      cerr << "*** Error: Savegame file is too old (v" << n << 
        " vs. current v" << PERSIST_VERSION << 
        ", vs. last handled v" << OLDEST_HANDLED_VERSION << 
        "): ignoring data in file." << endl;
      delete file;
      return false;
    } else {
      if( n < PERSIST_VERSION ) {
        cerr << "*** Warning: loading older savegame file: v" << n << 
          " vs. v" << PERSIST_VERSION << ". Will try to convert it." << endl;
      }
      Uint32 storylineIndex;
      file->read( &storylineIndex );
      file->read( &n );
      Creature *pc[MAX_PARTY_SIZE];
      for(int i = 0; i < (int)n; i++) {
        CreatureInfo *info = Persist::loadCreature( file );
        pc[i] = session->getParty()->getParty(i)->load( session, info );
        Persist::deleteCreatureInfo( info );
      }
      
      // set the new party
      session->getParty()->setParty( n, pc, storylineIndex );

      delete file;
    }

    {
      char path[300];
      get_file_name( path, 300, VALUES_FILE );
      FILE *fp = fopen( path, "rb" );
      if( fp ) {
        File *file = new File( fp );
        getSession()->getSquirrel()->loadValues( file );
        delete file;
      } else {
        cerr << "*** Warning: can't find values file." << endl;
      }
    }
  }
  return true;
}

bool Scourge::testLoadGame( Session *session ) {
  char path[300];
  get_file_name( path, 300, SAVE_FILE );
  FILE *fp = fopen( path, "rb" );
  if(!fp) {
    return false;
  }
  File *file = new File( fp );
  Uint32 n = PERSIST_VERSION;
  file->read( &n );
  if( n < OLDEST_HANDLED_VERSION ) {
    cerr << "*** Error: Savegame file is too old (v" << n << 
      " vs. current v" << PERSIST_VERSION << 
      ", vs. last handled v" << OLDEST_HANDLED_VERSION << 
      "): ignoring data in file." << endl;
    delete file;
    return false;
  } else {
    if( n < PERSIST_VERSION ) {
      cerr << "*** Warning: loading older savegame file: v" << n << 
        " vs. v" << PERSIST_VERSION << ". Will try to convert it." << endl;
    }
    Uint32 storylineIndex;
    file->read( &storylineIndex );
    file->read( &n );
    Creature *pc[MAX_PARTY_SIZE];
    for(int i = 0; i < (int)n; i++) {
      CreatureInfo *info = Persist::loadCreature( file );
      pc[i] = session->getParty()->getParty(i)->load( session, info );
      Persist::deleteCreatureInfo( info );
    }
    
    // set the new party
    //session->getParty()->setParty( n, pc, storylineIndex );

    cerr << "Loaded party:" << endl;
    for( int i = 0; i < (int)n; i++ ) {
      cerr << "\t" << pc[i]->getName() << endl;
      for( int t = 0; t < pc[i]->getInventoryCount(); t++ ) {
        cerr << "\t\t" << pc[i]->getInventory(t)->getRpgItem()->getName() << endl;
      }
    }

    delete file;
  }
  return true;
}

bool Scourge::startTextEffect( char *message ) {
  if( !textEffect ) {
    int x = getScreenWidth() / 2 - ( strlen( message ) / 2 * 18 );
    int y = getScreenHeight() / 2 - 50;
    //cerr << "x=" << x << " y=" << y << endl;
    textEffect = new TextEffect( this, x, y, message );
    textEffect->setActive( true );
    textEffectTimer = SDL_GetTicks();
    return true;
  } else {
    return false;
  }
}

void Scourge::updateStatus( int status, int maxStatus, const char *message ) {
  progress->updateStatus( message, true, status, maxStatus );
}

bool Scourge::isLevelShaded() {
  // don't shade the first depth of and edited map (incl. hq)
  return( !( getSession()->getMap()->isEdited() && oldStory == 0 ) && 
          getSession()->getPreferences()->isOvalCutoutShown() );
}                                                               

void Scourge::printToConsole( const char *s ) {
  if( squirrelLabel ) {
    // replace eol with a | (pipe). This renders as an eol in ScrollingLabel.
    char *p = strpbrk( s, "\n\r" );
    while( p ) {
      *p = '|';
      if( !*(p + 1) ) break;
      p = strpbrk( p + 1, "\n\r" );
    }
    squirrelLabel->appendText( s );
  } else {
    cerr << s << endl;
  }
}

char *Scourge::getDeityLocation( Location *pos ) {
  MagicSchool *ms = getMagicSchoolLocation( pos );
  return( ms ? ms->getDeity() : NULL );
}

void Scourge::startConversation( RenderedCreature *creature ) {
  conversationGui->start( (Creature*)creature );
}

