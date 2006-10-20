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
#include "sqbinding/sqbinding.h"
#include "storable.h"
#include "shapepalette.h"
#include "terraingenerator.h"
#include "cavemaker.h"
#include "dungeongenerator.h"
#include "mondrian.h"
#include "debug.h"
#include "scourgeview.h"
#include "scourgehandler.h"
#include "gui/confirmdialog.h"
#include "gui/textdialog.h"
#include "pceditor.h"

using namespace std;

#define MOUSE_ROT_DELTA 2

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

void Scourge::setBlendFuncStatic() {
  glBlendFunc(blend[blendA], blend[blendB]);
}

Scourge::Scourge(UserConfiguration *config) : SDLOpenGLAdapter(config) {
  oldStory = currentStory = 0;
  lastTick = 0;
  messageWin = NULL;
  textDialog = NULL;
  movingX = movingY = movingZ = MAP_WIDTH + 1;
  movingItem = NULL;
  nextMission = -1;
  teleportFailure = false;

  // in HQ map
  inHq = true;

  layoutMode = Constants::GUI_LAYOUT_BOTTOM;

  isInfoShowing = true; // what is this?
  info_dialog_showing = false;

  // we're not in target selection mode
  targetSelectionFor = NULL;

  battleCount = 0;
  inventory = NULL;
  containerGuiCount = 0;
  changingStory = false;

  lastEffectOn = false;
  resetBattles();

  gatepos = NULL;

  view = new ScourgeView( this );
  handler = new ScourgeHandler( this );
}

void Scourge::initUI() {

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
  createBoardUI();
  netPlay = new NetPlay(this);
  createUI();
  createPartyUI();

  // show the main menu
  mainMenu = new MainMenu(this);
  mapEditor = new MapEditor( this );
  optionsMenu = new OptionsMenu(this);
  multiplayer = new MultiplayerDialog(this);

	hireHeroDialog = new ConfirmDialog( getSDLHandler(), "Hire a Wandering Hero" );
	dismissHeroDialog = new ConfirmDialog( getSDLHandler(), "Dismiss Party Member" );
	textDialog = new TextDialog( getSDLHandler() );

  // load character, item sounds
  getSDLHandler()->getSound()->loadSounds( getUserConfiguration() );

  view->initUI();
}

void Scourge::start() {  
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

    if(value == NEW_GAME_START ||
       value == MULTIPLAYER_START ||
       value == CONTINUE_GAME ||
       value == EDITOR ) {

      // fade away
      getSDLHandler()->getSound()->stopMusic();
      getSDLHandler()->fade( 0, 1, 20 );
      mainMenu->hide();

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
					char error[255];
          if(!loadGame( session, error )) {
            showMessageDialog( error );
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
  delete pcEditor;
  delete view;
  delete handler;
}

void Scourge::startMission() {

#if DEBUG_SQUIRREL
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

    // add gui
    messageWin->setVisible(true);
    mainWin->setVisible(true);
    if(session->isMultiPlayerGame()) netPlay->getWindow()->setVisible(true);

    // create the map
    //cerr << "Starting to reset map..." << endl;
    bool fromRandomMap = !( levelMap->isEdited() );
    levelMap->reset();
    //cerr << "\tMap reset is done." << endl;

    // do this only once
    if(resetParty) {
      // clear the board
      board->reset();
      // clear the current mission (otherwise weird crashes later)
      getSession()->setCurrentMission(NULL);
      // reset the party
      if(session->isMultiPlayerGame()) {
        party->resetMultiplayer(multiplayer->getCreature());
      } else {
        party->reset();
      }
      // reset the calendar
      party->getCalendar()->reset(true); // reset the time

      // re-add party events (hack)
      resetPartyUI();

      Calendar *cal = getSession()->getParty()->getCalendar();
      {
        // Schedule an event to keep reloading scripts if they change on disk
        Date d(0, 0, 1, 0, 0, 0); // (format : sec, min, hours, days, months, years)
        Event *event = new ReloadEvent( cal->getCurrentDate(),
                                        d,
                                        Event::INFINITE_EXECUTIONS,
                                        getSession(),
                                        ReloadEvent::MODE_RELOAD_SCRIPTS );
        cal->scheduleEvent( event );
      }

      {
        // Schedule an event to regain MP now and then
        Date d(0, 10, 0, 0, 0, 0); // (format : sec, min, hours, days, months, years)
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

    bool mapCreated = true;
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
          sprintf( message, " For returning alive, the party receives %d experience points. ", exp);
          strcat( infoMessage, message );

          for(int i = 0; i < getParty()->getPartySize(); i++) {
            int level = getParty()->getParty(i)->getLevel();
            if(!getParty()->getParty(i)->getStateMod(Constants::dead)) {
              int n = getParty()->getParty(i)->addExperience(exp);
              if(n > 0) {
                if( level != getParty()->getParty(i)->getLevel() ) {
                  sprintf(message, " %s gains a level! ", getParty()->getParty(i)->getName());
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
      dg = new CaveMaker( this, CAVE_TEST_LEVEL, 1, 1, false, false, NULL );
      mapCreated = dg->toMap( levelMap, getSession()->getShapePalette() );
      scriptName = RANDOM_MAP_NAME;
#else
      dg = NULL;
      levelMap->loadMap( HQ_MAP_NAME, result, this, 1, currentStory, changingStory );
      scriptName = HQ_MAP_NAME;
#endif

      //cerr << result << endl;

    } else {
      // in HQ map
      inHq = false;

      // Initialize the map with a random dungeon
      getSession()->setCurrentMission(board->getMission(nextMission));
      missionWillAwardExpPoints = (!getSession()->getCurrentMission()->isCompleted());
      bool loaded = false;
      if( getSession()->getCurrentMission()->isEdited() ) {
				//cerr << "EDITED" << endl;
        // try to load the edited map
        dg = NULL;
        bool lastLevel = ( currentStory == getSession()->getCurrentMission()->getDepth() - 1 );
				//cerr << "lastLevel=" << lastLevel << " currentStory=" << currentStory << " depth=" << getSession()->getCurrentMission()->getDepth() << endl;
        
				
				if( lastLevel ) {

					/**
					 * If it's the last edited level, add mission items and monsters.
					 * 
					 * FIXME: this is very easy to break code. Relying on the last level
					 * is bad because:
					 * 1. the algorithm might change (it has for random-levels)
					 * 2. if edited level 2 contains the mission monster, but missions.txt claims
					 *    there are 5 levels, the creature won't be marked as such. (e.g. Myco.)
					 */

          vector< RenderedItem* > items;
          vector< RenderedCreature* > creatures;
          loaded = levelMap->loadMap( getSession()->getCurrentMission()->getMapName(),
                                      result,
                                      this,
																			getSession()->getCurrentMission()->getLevel(),
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
							//cerr << "\tLooking for item: " << getSession()->getCurrentMission()->getItem( t )->getName() << endl;
              if( getSession()->getCurrentMission()->getItem( t ) == item->getRpgItem() &&
                  used.find( t ) == used.end() ) {
								//cerr << "\t\tAdding item " << item->getName() << endl;
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
								//cerr << "\tLooking for creature: " << getSession()->getCurrentMission()->getCreature( t )->getType() << endl;
                if( getSession()->getCurrentMission()->getCreature( t ) == creature->getMonster() &&
                    used.find( t ) == used.end() ) {
									//cerr << "\t\tAdding creature " << creature->getName() << endl;
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
																			getSession()->getCurrentMission()->getLevel(),
                                      currentStory,
                                      changingStory,
                                      fromRandomMap );
        }
        scriptName = getSession()->getCurrentMission()->getMapName();
      }

      // if no edited map is found, make a random map
      if( !loaded ) {
				dg = TerrainGenerator::getGenerator( this, currentStory );
        mapCreated = dg->toMap(levelMap, getSession()->getShapePalette());
        scriptName = RANDOM_MAP_NAME;
      }
    }
    if( mapCreated ) {
      changingStory = false;

			if( inHq ) addWanderingHeroes();

      // center map on the player
      levelMap->center(toint(party->getPlayer()->getX()),
                       toint(party->getPlayer()->getY()),
                       true);

      // set to receive events here
      getSDLHandler()->setHandlers( handler, view );

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
      // set the map view
      setUILayout();
      // start the haunting tunes
      getSDLHandler()->getSound()->selectMusic( getPreferences() );
      if(inHq) getSDLHandler()->getSound()->playMusicHQ();
      else getSDLHandler()->getSound()->playMusicMission();
      getSDLHandler()->fade( 1, 0, 20 );

      // run mission
      getSDLHandler()->mainLoop();

      getSession()->getSquirrel()->endLevel();

      // stop the music
      getSDLHandler()->getSound()->stopMusic();
      getSDLHandler()->fade( 0, 1, 20 );
    } else {
      showMessageDialog( "Error #666: Failed to create map." );
    }
    // clean up after the mission
    resetInfos();

    // remove gui
		hireHeroDialog->setVisible( false );
		dismissHeroDialog->setVisible( false );
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
    pcEditor->getWindow()->setVisible( false );

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
              getSession()->getCurrentMission()->isEdited() &&
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

	//cerr << "Saving party" << endl;
	if(!session->isMultiPlayerGame()) {
		if(!saveGame(session)) {
			showMessageDialog( "Error saving game!" );
		}
	}

#ifdef HAVE_SDL_NET
  session->stopClientServer();
#endif
  session->deleteCreaturesAndItems(false);

  // delete the party (w/o deleting the party ui)
  party->deleteParty();

#if DEBUG_SQUIRREL
  squirrelWin->setVisible( false );
#endif
  getSession()->getSquirrel()->endGame();
}

void Scourge::addWanderingHeroes() {

	if( !hasParty() ) return;
	
	int level = getSession()->getParty()->getAverageLevel();
	int count = (int)( 5.0f * rand() / RAND_MAX ) + 5;
	for( int i = 0; i < count; i++ ) {
		// find a place for it near another creature
		// note: this must be done before creating the new creature...
		int n = (int)( (float)getSession()->getCreatureCount() * rand() / RAND_MAX );
		int cx = toint( getSession()->getCreature( n )->getX() );
		int cy = toint( getSession()->getCreature( n )->getY() );	
		if( cx == 0 || cy == 0 ) {
			cerr << "*** Error 0,0 coordinates for " << getSession()->getCreature( n )->getName() << endl;
		}
		assert( cx && cy );
		RenderedCreature *creature = createWanderingHero( level );
		creature->findPlace( cx, cy );
	}
}

void Scourge::endMission() {
  for(int i = 0; i < party->getPartySize(); i++) {
    party->getParty(i)->setSelXY(-1, -1);   // stop moving
  }
  movingItem = NULL;          // stop moving items
}

bool Scourge::inTurnBasedCombatPlayerTurn() {
  return (inTurnBasedCombat() &&
          !battleRound[battleTurn]->getCreature()->isMonster());
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
		// cancel target selection ( cross cursor )
		getTargetSelectionFor()->cancelTarget();
		getTargetSelectionFor()->getBattle()->reset();
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
		// cancel target selection ( cross cursor )
		getTargetSelectionFor()->cancelTarget();
		getTargetSelectionFor()->getBattle()->reset();
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
		// cancel target selection ( cross cursor )
		getTargetSelectionFor()->cancelTarget();
		getTargetSelectionFor()->getBattle()->reset();
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
		if( !loc ) loc = levelMap->getItemLocation( mapx, mapy );
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
          infoGui->setItem( item );
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
	if( !pos || !pos->item ) pos = levelMap->getItemLocation( x, y );
  if(pos && getItem(pos) ) {
		dragStartTime = SDL_GetTicks();
		return true;
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
	if( !pos ) pos = levelMap->getItemLocation( x, y );
  if (pos) {
    Shape *shape = (pos->item ? pos->item->getShape() : pos->shape);
    if (levelMap->isWallBetweenShapes(toint(party->getPlayer()->getX()),
                                 toint(party->getPlayer()->getY()),
                                 toint(party->getPlayer()->getZ()),
                                 party->getPlayer()->getShape(),
                                 x, y, z,
                                 shape)) {
      levelMap->addDescription(Constants::getMessage(Constants::ITEM_OUT_OF_REACH));
			getParty()->setSelXY( x, y, false ); // get as close as possible to location
      return true;
    } else {
      if (useLever(pos)) {
        return true;
      } else if (useDoor(pos)) {
        return true;
      } else if (useSecretDoor(pos)) {
        return true;
      } else if (useGate(pos)) {
        return true;
      } else if (useBoard(pos)) {
        return true;
      } else if (useTeleporter(pos)) {
        return true;
      } else if( usePool( pos ) ) {
        return true;
      } else if( pos && pos->item && 
								 ((Item*)(pos->item))->getRpgItem()->getType() == RpgItem::CONTAINER ) {
				if( SDL_GetModState() & KMOD_CTRL ) {
					openContainerGui(((Item*)(pos->item)));
				} else {
					getParty()->setSelXY( x, y, false ); // get as close as possible to location
				}
				return true;
      } else if( session->getSquirrel()->
								 callMapPosMethod( "useShape", 
																	 pos->x, 
																	 pos->y, 
																	 pos->z ) ) {
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
		Location *pos = levelMap->isBlocked( x, y, 0,
																				 movingX, movingY, movingZ,
																				 movingItem->getShape(), &z, 
																				 true );
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
			levelMap->isBlocked( movingX, movingY, movingZ,
													 -1, -1, -1,
													 movingItem->getShape(), &z,
													 true );
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
        ascendDungeon( pos );
        return true;
      } else if (pos->shape == getSession()->getShapePalette()->findShapeByName("GATE_DOWN")) {
				descendDungeon( pos );
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
	Location *p = getMap()->getLocation( pos->x, pos->y, 6 );
  if( p && p->shape && 
			p->shape == getSession()->getShapePalette()->findShapeByName("TELEPORTER") ) {
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

// FIXME: smooth movement, 
// fix disapearing post bug
bool Scourge::useSecretDoor(Location *pos) {
  bool ret = false;
  if( levelMap->isSecretDoor( pos ) ) {
    Shape *post = getShapePalette()->findShapeByName( "SECRET_DOOR_POST" );
    
    // Clicked a strut
    if( pos->z == 0 && pos->shape == post ) {
      pos = levelMap->getLocation( pos->x, pos->y, pos->shape->getHeight() );
    }

    GLShape *wall = (GLShape*)(pos->shape);    
    int s1 = ((GLShape*)( getShapePalette()->findShapeByName( "EW_WALL" ) ) )->getShapePalIndex();
    int s2 = ((GLShape*)( getShapePalette()->findShapeByName( "SECRET_EW_WALL" ) ) )->getShapePalIndex();
    ret = true;
    if( pos->z == 0 ) {

      // try to detect the secret door
      if( !levelMap->isSecretDoorDetected( pos ) &&
          !getParty()->getPlayer()->rollSecretDoor( pos ) ) {
        return false;
      }
      levelMap->setSecretDoorDetected( pos );
      
      levelMap->removePosition( pos->x, pos->y, pos->z );      
      Shape *shape = getShapePalette()->getShape( wall->getShapePalIndex() - s1 + s2 );
      levelMap->setPosition( pos->x, pos->y, post->getHeight(), shape );
      
      levelMap->setPosition( pos->x, pos->y, 0, post );
      if( wall->getWidth() > wall->getDepth() ) {
        levelMap->setPosition( pos->x + wall->getWidth() - post->getWidth(), pos->y, 0, post );
      } else {
        levelMap->setPosition( pos->x, pos->y - wall->getDepth() + post->getDepth(), 0, post );
      }
    } else {

      // remove struts
      levelMap->removePosition( pos->x, pos->y, 0 );
      if( wall->getWidth() > wall->getDepth() ) {
        levelMap->removePosition( pos->x + wall->getWidth() - post->getWidth(), pos->y, 0 );
      } else {
        levelMap->removePosition( pos->x, pos->y - wall->getDepth() + post->getDepth(), 0 );
      }

      // blocked?
      if( levelMap->isBlocked( pos->x, pos->y, 0, pos->x, pos->y, pos->z, wall ) ) {
        // put struts back
        levelMap->setPosition( pos->x, pos->y, 0, post );
        if( wall->getWidth() > wall->getDepth() ) {
          levelMap->setPosition( pos->x + wall->getWidth() - post->getWidth(), pos->y, 0, post );
        } else {
          levelMap->setPosition( pos->x, pos->y - wall->getDepth() + post->getDepth(), 0, post );
        }
        levelMap->addDescription( "Something is blocking the door from closing." );
        return false;
      }

      // move the door down
      levelMap->removePosition( pos->x, pos->y, pos->z );
      Shape *shape = getShapePalette()->getShape( wall->getShapePalIndex() - s2 + s1 );
      levelMap->setPosition( pos->x, pos->y, 0, shape );
    }
    levelMap->updateLightMap();
  }
  return ret;
}

bool Scourge::useDoor(Location *pos) {
  Shape *newDoorShape = NULL;
  Shape *oldDoorShape = pos->shape;
  if (oldDoorShape == getSession()->getShapePalette()->findShapeByName("EW_DOOR")) {
    newDoorShape = getSession()->getShapePalette()->findShapeByName("NS_DOOR");
  } else if (oldDoorShape == getSession()->getShapePalette()->findShapeByName("NS_DOOR")) {
    newDoorShape = getSession()->getShapePalette()->findShapeByName("EW_DOOR");
  }
  if (newDoorShape) {
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
    if (closed && levelMap->isLocked(doorX, doorY, doorZ)) {
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
    if ( !blocker ) {

      // there is a chance that the door will be destroyed
      if ( 0 == (int)( 20.0f * rand()/RAND_MAX ) ) {
        destroyDoor( ox, oy, oldDoorShape );
        levelMap->updateLightMap();
      } else {
        levelMap->setPosition(nx, ny, toint(party->getPlayer()->getZ()), newDoorShape);
        levelMap->updateLightMap();
        levelMap->updateDoorLocation(doorX, doorY, doorZ,
                                     nx, ny, toint(party->getPlayer()->getZ()));
      }
      return true;
    } else if ( blocker->creature && !( blocker->creature->isMonster() ) ) {
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

// create the ui
void Scourge::createUI() {

  infoGui = new InfoGui( this );

  conversationGui = new ConversationGui( this );
  tradeDialog = new TradeDialog( this );
  healDialog = new HealDialog( this );
  donateDialog = new DonateDialog( this );
  trainDialog = new TrainDialog( this );
  pcEditor = new PcEditor( this );

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
  messageList = new ScrollingList(8, 0, 
																	width - 100, 
																	PARTY_GUI_HEIGHT - 100, 
																	getSession()->getShapePalette()->getHighlightTexture());
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
  yesExitConfirm = new Button( mx - 80, 55, mx - 10, 75, getSession()->getShapePalette()->getHighlightTexture(), "Yes" );
  exitConfirmationDialog->addWidget((Widget*)yesExitConfirm);
  noExitConfirm = new Button( mx + 10, 55, mx + 80, 75, getSession()->getShapePalette()->getHighlightTexture(), "No" );
  exitConfirmationDialog->addWidget((Widget*)noExitConfirm);
  exitLabel = new Label(20, 25, Constants::getMessage(Constants::EXIT_MISSION_LABEL));
  exitConfirmationDialog->addWidget((Widget*)exitLabel);

  squirrelWin = new Window( getSDLHandler(), 5, 0, getSDLHandler()->getScreen()->w - 10, 200, "Squirrel Console",
                            getSession()->getShapePalette()->getGuiTexture(), true,
                            Window::BASIC_WINDOW, getSession()->getShapePalette()->getGuiTexture2() );
  squirrelLabel = new ScrollingLabel( 5, 0, getSDLHandler()->getScreen()->w - 30, 145, "" );
  squirrelLabel->setCanGetFocus( false );
  squirrelWin->addWidget( squirrelLabel );
  squirrelText = new TextField( 8, 150, 100 );
  squirrelWin->addWidget( squirrelText );
  squirrelRun = squirrelWin->createButton( getSDLHandler()->getScreen()->w - 110, 150, getSDLHandler()->getScreen()->w - 30, 170, "Run" );
  squirrelClear = squirrelWin->createButton( getSDLHandler()->getScreen()->w - 200, 150, getSDLHandler()->getScreen()->w - 120, 170, "Clear" );
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

  messageWin->setVisible(false);
  mainWin->setVisible( false );
  switch(layoutMode) {
  case Constants::GUI_LAYOUT_ORIGINAL:
    messageList->resize( width - 18, PARTY_GUI_HEIGHT - 30 );
    messageWin->resize(width, PARTY_GUI_HEIGHT);
    messageWin->move(getSDLHandler()->getScreen()->w - width, 0);
    messageWin->setLocked(false);
    mainWin->setLocked(false);
//  if(inventory->getWindow()->isLocked()) {
    inventory->hide();
    inventory->getWindow()->setLocked( false );
		inventory->getWindow()->setAnimation( Window::DEFAULT_ANIMATION );
//  }
    netPlay->getWindow()->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER);
    netPlay->getWindow()->setLocked(false);
    break;

  case Constants::GUI_LAYOUT_BOTTOM:
    messageList->resize( width - 18, PARTY_GUI_HEIGHT - 30 );
    messageWin->resize(width, PARTY_GUI_HEIGHT);
    messageWin->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER);
    mapHeight = getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER;
    messageWin->setLocked(true);
    mainWin->setLocked(true);
//  if(inventory->getWindow()->isLocked()) {
    //inventory->getWindow()->setVisible(false);
    inventory->hide();
		inventory->getWindow()->setLocked(true);
		inventory->getWindow()->setAnimation( Window::SLIDE_UP );
//    inventory->getWindow()->setLocked(false);
//  }
    netPlay->getWindow()->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT * 2 - Window::SCREEN_GUTTER);
    netPlay->getWindow()->setLocked(true);
    break;

  case Constants::GUI_LAYOUT_SIDE:
    messageList->resize( PARTY_GUI_WIDTH - 18, 
												 getSDLHandler()->getScreen()->h - ( PARTY_GUI_HEIGHT + 30 ) );
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
		inventory->getWindow()->setLocked(true);
		inventory->getWindow()->setAnimation( Window::SLIDE_UP );
//    inventory->getWindow()->setLocked(false);
//  }
    netPlay->getWindow()->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT);
    netPlay->getWindow()->setLocked(true);
    break;

  case Constants::GUI_LAYOUT_INVENTORY:
    messageList->resize( width - 18, PARTY_GUI_HEIGHT - 30 );
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
		inventory->getWindow()->setLocked(true);
		inventory->getWindow()->setAnimation( Window::SLIDE_UP );
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
  messageWin->toBottom();

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
    if( battleRound.size() == 0 && !createBattleTurns() ) {
      // not in battle
      if( fromBattle ) {
        // go back to real-time, group-mode
        resetUIAfterBattle();
      }
      // move all creatures
      moveCreatures();
    } else {
    	// move creatures not in combat
    	moveCreatures( false );
    }
  }
}

// fight a turn of the battle
bool Scourge::fightCurrentBattleTurn() {
  if(battleRound.size() > 0) {

    // end of battle if party has no-one to attack
    bool roundOver = false;

    // Cancel combat when all party members have nothing to attack.
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
        /* cancel a target in case it's too far away
          (getAvailableTarget only looks 20 paces away)
          the alternative is to scan the entire map...
        */
        party->getParty(i)->cancelTarget();
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

      getMap()->getMapRenderHelper()->hideDeadParty();

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
    ((AnimatedShape*)session->getCreature(i)->getShape())->setPauseAnimation( !active );
  }
  for( int i = 0; i < getParty()->getPartySize(); i++ ) {
    bool active = ( getParty()->getParty( i ) == battle->getCreature() ||
                    getParty()->getParty( i ) == battle->getCreature()->getTargetCreature() );
    ((AnimatedShape*)getParty()->getParty( i )->getShape())->setPauseAnimation( !active );
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
//      if(party->getParty(i)->getTargetCreature() &&
//         party->getParty(i)->getStateMod(Constants::possessed)) {
      if( party->getParty(i)->getStateMod( Constants::possessed ) ) {
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
                                                   toint(party->getParty(i)->getY())) );
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
                                    toint(session->getCreature(i)->getY()),
                                    session->getCreature(i)->getShape())) {

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
  // and add party to the battle round. (Monsters are added when attacked,
	// they fight back, see Battle::dealDamage())
  if (battleCount > 0) {

    // add other movement
    for (int i = 0; i < party->getPartySize(); i++) {
      bool visible = ( levelMap->isLocationVisible(toint(party->getParty(i)->getX()),
                                              toint(party->getParty(i)->getY())) );
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

    party->savePlayerSettings();

    // order the battle turns by initiative
    Battle::setupBattles(getSession(), battle, battleCount, &battleRound);
    rtStartTurn = battleTurn = 0;
    if(Battle::debugBattle) cerr << "++++++++++++++++++++++++++++++++++" << endl;
    if(Battle::debugBattle) cerr << "ROUND STARTS" << endl;

    //if(getUserConfiguration()->isBattleTurnBased()) groupButton->setVisible(false);
		if(getUserConfiguration()->isBattleTurnBased()) {
			tbCombatWin->move( getScreenWidth() - tbCombatWin->getWidth(), 0 );
			tbCombatWin->setVisible( true, false );
		}
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
//  groupButton->setVisible(true);
	tbCombatWin->setVisible( false );
  for(int i = 0; i < party->getPartySize(); i++) {
    party->getParty(i)->cancelTarget();
    ((AnimatedShape*)party->getParty(i)->getShape())->setPauseAnimation( false );
    if(party->getParty(i)->anyMovesLeft()) {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_RUN, true);
    } else {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_STAND, true);
    }
  }
  // animate monsters again after TB combat (see resetNonParticipantAnimation() )
  for(int i = 0; i < session->getCreatureCount(); i++) {
		if( !session->getCreature(i)->getStateMod( Constants::dead ) ) {    
		/*
		if( !session->getCreature(i)->getStateMod( Constants::dead ) &&
        !( session->getCreature(i)->getMonster() &&
					 session->getCreature(i)->getMonster()->isNpc() ) ) {
			*/
      session->getCreature(i)->setMotion( Constants::MOTION_LOITER );
      ((AnimatedShape*)session->getCreature(i)->getShape())->setPauseAnimation( false );
    }
  }
}

// if allCreatures == false, only creatures not in a battle turn are moved
void Scourge::moveCreatures( bool allCreatures ) {
  // change animation if needed
  for(int i = 0; i < party->getPartySize(); i++) {
    if(((AnimatedShape*)(party->getParty(i)->getShape()))->getAttackEffect()) {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_ATTACK);
      ((AnimatedShape*)(party->getParty(i)->getShape()))->setAngle(party->getParty(i)->getTargetAngle());
    } else if( party->getParty(i)->isMoving() ) {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_RUN);
			party->getParty(i)->setMoving( false );
    } else {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_STAND);
    }
  }

  // move the party members
  // if allCreatures is false, they're moved in Battle::moveCreature()
	if( allCreatures ) party->movePlayers();

  // move visible monsters
  for( int i = 0; i < session->getCreatureCount(); i++ ) {
  	Creature *c = session->getCreature( i );
    if( !c->getStateMod( Constants::dead ) &&
				levelMap->isLocationVisible( toint( c->getX() ),
					toint( c->getY() ) ) ) {
			// move if allCreatures is true, or if monster has no target.
			// otherwise move is in Battle::moveCreature()
			if( allCreatures || 
					!( c->hasTarget() && c->isTargetValid() ) ) {
	      moveMonster( c );
	  	}
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
  if(((AnimatedShape*)(monster->getShape()))->getAttackEffect()) {
		if( monster->getCharacter() ) cerr << "111" << endl;
    //monster->getShape()->setCurrentAnimation((int)MD2_ATTACK);
    //((AnimatedShape*)(monster->getShape()))->setAngle(monster->getTargetAngle());
    // don't move when attacking
    return;
  } else {
    monster->getShape()->setCurrentAnimation( monster->getMotion() == Constants::MOTION_LOITER ||
																							monster->getMotion() == Constants::MOTION_MOVE_TOWARDS ?
                                              (int)MD2_RUN :
                                              (int)MD2_STAND );
  }

	if( monster->getMotion() == Constants::MOTION_MOVE_AWAY ) {
		monster->moveToLocator();
	} else if( monster->getCharacter() || 
						 ( monster->isNpc() && monster->getMotion() == Constants::MOTION_MOVE_TOWARDS ) ||
						 monster->getMotion() == Constants::MOTION_LOITER ) {
    // attack the closest player
    if( BATTLES_ENABLED &&
        (int)(20.0f * rand()/RAND_MAX) == 0) {
      monster->decideMonsterAction();
    } else {
      // random (non-attack) monster movement
			monster->move(monster->getDir());
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

void Scourge::refreshContainerGui( Item *container ) {
  for(int i = 0; i < containerGuiCount; i++) {
		if( !container || containerGui[i]->getContainer() == container ) {
			containerGui[i]->refresh();
		}
  }
}

void Scourge::removeClosedContainerGuis() {
  if(containerGuiCount > 0) {
    for(int i = 0; i < containerGuiCount; i++) {
      if(!containerGui[i]->getWindow()->isVisible()) {
        closeContainerGui(containerGui[i]);
      }
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
  //win->setBorderColor( 0.0f, 1.0f, 0.1f );
  win->setColor( 0.8f, 0.8f, 0.7f, 1 );
  win->setBackground( 0.65, 0.30f, 0.20f, 0.15f );
  win->setSelectionColor(  0.25f, 0.35f, 0.6f );
//  win->setSelectionColor(  1.0f, 0.0f, 0.0f );
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
			getParty()->getParty(i)->addExperienceWithMessage( exp );
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
  Progress *progress = new Progress(this->getSDLHandler(), 12, getSession()->getShapePalette()->getProgressTexture(), getSession()->getShapePalette()->getProgressHighlightTexture() );
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

	tbCombatWin = new Window( getSDLHandler(),
														0, 0, 80 + 9, 52, "Combat",
														false, Window::BASIC_WINDOW, "default" );
	endTurnButton = tbCombatWin->createButton( 8, 0, 80, 20, "End Turn", 0 );
	tbCombatWin->setVisible( false );
	tbCombatWin->setLocked( true );

  sprintf(version, "S.C.O.U.R.G.E. v%s", SCOURGE_VERSION);
  sprintf(min_version, "S.C.O.U.R.G.E.");
  mainWin = new Window( getSDLHandler(),
                        getSDLHandler()->getScreen()->w - Scourge::PARTY_GUI_WIDTH,
                        getSDLHandler()->getScreen()->h - Scourge::PARTY_GUI_HEIGHT,
                        Scourge::PARTY_GUI_WIDTH,
                        Scourge::PARTY_GUI_HEIGHT,
                        version, false, Window::BASIC_WINDOW,
                        "default" );
  cards = new CardContainer(mainWin);

	int offsetX = 64;

	int xstart = 8;
	//int buttonHeight = 19;
	//int ystart = 0;


	roundButton = 
		cards->createButton( 8, 0,  
												 offsetX, offsetX - 2, 
												 "", 0, false );
	roundButton->setTooltip( "Pause game" );
	offsetX+=4;

	int quickButtonWidth = 24;
	xstart = Scourge::PARTY_GUI_WIDTH - 10 - quickButtonWidth;
	quitButton = 
		cards->createButton( xstart, 0,  
												 xstart + quickButtonWidth, quickButtonWidth, 
												 "", 0, false, 
												 getShapePalette()->getExitTexture() );
	quitButton->setTooltip( "Exit game" );
	xstart = Scourge::PARTY_GUI_WIDTH - 10 - quickButtonWidth * 2;
	optionsButton = 
		cards->createButton( xstart, 0,  
												 xstart + quickButtonWidth, quickButtonWidth, 
												 "", 0, false, 
												 getShapePalette()->getOptionsTexture() );
	optionsButton->setTooltip( "Game options" );
	xstart = Scourge::PARTY_GUI_WIDTH - 10 - quickButtonWidth * 3;
	groupButton = 
		cards->createButton( xstart, 0,  
												 xstart + quickButtonWidth, quickButtonWidth, 
												 "", 0, true, 
												 getShapePalette()->getGroupTexture() );
	groupButton->setTooltip( "Move as a group" );

  groupButton->setToggle(true);
  groupButton->setSelected(true);
  roundButton->setToggle(true);
  roundButton->setSelected(true);
  optionsButton->setToggle(true);
  optionsButton->setSelected(false);
  
  int playerButtonWidth = (Scourge::PARTY_GUI_WIDTH - 8 - offsetX) / 4;
  //int playerButtonHeight = 20;
  int playerInfoHeight = 95;
  //int playerButtonY = playerInfoHeight;

	
	int yy = quickButtonWidth + 2;
  for(int i = 0; i < 4; i++) {
    playerInfo[i] = new Canvas( offsetX + playerButtonWidth * i, yy,
                                offsetX + playerButtonWidth * (i + 1) - 25, yy + playerInfoHeight,
                                this, this );
    cards->addWidget( playerInfo[i], 0 );
		if( i == 0 ) {
			playerHpMp[i] = new Canvas( offsetX + playerButtonWidth * (i + 1) - 25, yy,
																	offsetX + playerButtonWidth * (i + 1), yy + playerInfoHeight - 25,
																	this, NULL, true );
			cards->addWidget( playerHpMp[i], 0 );
		} else {
			playerHpMp[i] = new Canvas( offsetX + playerButtonWidth * (i + 1) - 25, yy,
																	offsetX + playerButtonWidth * (i + 1), 
																	yy + playerInfoHeight - 50,
																	this, NULL, true );
			cards->addWidget( playerHpMp[i], 0 );
			dismissButton[i] = cards->createButton( offsetX + playerButtonWidth * (i + 1) - 25, 
																							yy + playerInfoHeight - 50,
																							offsetX + playerButtonWidth * (i + 1), 
																							yy + playerInfoHeight - 25,
																							"", 																							
																							0,
																							false,
																							getShapePalette()->getDismissTexture() );
			dismissButton[i]->setTooltip( "Dismiss this character" );
		}
    playerWeapon[i] = new Canvas( offsetX + playerButtonWidth * (i + 1) - 25, 
																	yy + playerInfoHeight - 25,
                                  offsetX + playerButtonWidth * (i + 1), 
																	yy + playerInfoHeight,
                                  this, NULL, true );
    cards->addWidget( playerWeapon[i], 0 );
  }
  //int quickButtonWidth = (int)((float)(Scourge::PARTY_GUI_WIDTH - offsetX - 20) / 12.0f);	

	int inventoryButtonWidth = 2 * quickButtonWidth;
	inventoryButton = 
		cards->createButton( offsetX, 0,  
												 offsetX + inventoryButtonWidth, quickButtonWidth, 
												 "", 0, true, 
												 getShapePalette()->getInventoryTexture() );
  inventoryButton->setToggle(true);
  inventoryButton->setSelected(false);
	inventoryButton->setTooltip( "Inventory and party info" );


	int gap = 0;
  for( int i = 0; i < 12; i++ ) {
    int xx = inventoryButtonWidth + offsetX + quickButtonWidth * i + ( i / 4 ) * gap;
    quickSpell[i] = new Canvas( xx, 0, xx + quickButtonWidth, quickButtonWidth,
                                this, NULL, true );
    quickSpell[i]->setTooltip( "Click to assign a spell, capability or magic item." );
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
      char msg[80];
      sprintf( msg, "HP:%d(%d) MP:%d(%d)",
               p->getHp(), p->getMaxHp(),
               p->getMp(), p->getMaxMp() );
      w->setTooltip( msg );
      Util::drawBar( 10, 5, ( i == 0 ? 60 : 35 ),
                     (float)p->getHp(), (float)p->getMaxHp(),
                     -1, -1, -1, true,
                     NULL,
                     //mainWin->getTheme(),
                     Util::VERTICAL_LAYOUT );
      Util::drawBar( 17, 5, ( i == 0 ? 60 : 35 ),
                     (float)p->getMp(), (float)p->getMaxMp(),
                     0, 0, 1, false,
                     NULL,
                     //mainWin->getTheme(),
                     Util::VERTICAL_LAYOUT );
      return;
    } else if( playerWeapon[i] == w ) {
      // draw the current weapon
      char msg[80];
      if( party->getParty( i )->getPreferredWeapon() == -1 ) {
        w->setTooltip( "Current Weapon: Bare Hands" );
      } else {
        Item *item = party->getParty( i )->getItemAtLocation( party->getParty( i )->getPreferredWeapon() );
        if(item &&
           item->getRpgItem()->isWeapon() ) {
          sprintf( msg, "Current Weapon: %s", item->getRpgItem()->getName() );
          w->setTooltip( msg );
          drawItemIcon( item );
        }
      }
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
            if( storable->getStorableType() == Storable::ITEM_STORABLE ) {
              glBindTexture( GL_TEXTURE_2D, getSession()->getShapePalette()->tilesTex[ storable->getIconTileX() ][ storable->getIconTileY() ] );
            } else {
              glBindTexture( GL_TEXTURE_2D, getSession()->getShapePalette()->spellsTex[ storable->getIconTileX() ][ storable->getIconTileY() ] );
            }
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

  //cerr << "Warning: Unknown widget in Party::drawWidget." << endl;
  return;
}

void Scourge::drawItemIcon( Item *item, int n ) {
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_ALPHA_TEST );
  glAlphaFunc( GL_EQUAL, 0xff );
  glBindTexture( GL_TEXTURE_2D,
                 getShapePalette()->tilesTex[ item->getRpgItem()->getIconTileX() ][ item->getRpgItem()->getIconTileY() ] );
  glColor4f(1, 1, 1, 1);
  glPushMatrix();
  //glTranslatef( 0, 5, 0 );
  glBegin( GL_QUADS );
  glNormal3f( 0, 0, 1 );
  glTexCoord2f( 0, 0 );
  glVertex3f( 0, 0, 0 );
  glTexCoord2f( 0, 1 );
  glVertex3f( 0, n, 0 );
  glTexCoord2f( 1, 1 );
  glVertex3f( n, n, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( n, 0, 0 );
  glEnd();
  glDisable( GL_ALPHA_TEST );
  glDisable( GL_TEXTURE_2D );
  glPopMatrix();
}

void Scourge::drawPortrait( Widget *w, Creature *p ) {
  glPushMatrix();
  glEnable( GL_TEXTURE_2D );
  glColor4f( 1, 1, 1, 1 );
  if( p->getStateMod( Constants::dead ) ) {
    glBindTexture( GL_TEXTURE_2D, getSession()->getShapePalette()->getDeathPortraitTexture() );
  } else {
    glBindTexture( GL_TEXTURE_2D, getSession()->getShapePalette()->getPortraitTexture( p->getSex(), p->getPortraitTextureIndex() ) );
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
  bool darker = false;
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
      darker = true;
    }
  }

  if( p->getStateMod( Constants::possessed ) ) {
    glColor4f( ( darker ? 0.5f : 1.0f ), 0, 0, 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::invisible ) ) {
    glColor4f( 0, ( darker ? 0.375f : 0.75f ), ( darker ? 0.5f : 1.0f ), 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::poisoned ) ) {
    glColor4f( ( darker ? 0.5f : 1.0f ), ( darker ? 0.375f : 0.75f ), 0, 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::blinded ) ) {
    glColor4f( ( darker ? 0.5f : 1.0f ), ( darker ? 0.5f : 1.0f ), ( darker ? 0.5f : 1.0f ), 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::cursed ) ) {
    glColor4f( ( darker ? 0.375f : 0.75f ), 0, ( darker ? 0.375f : 0.75f ), 0.5f );
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

	char *message = NULL;

	// can train?
	if( p->getCharacter()->getChildCount() > 0 &&
			p->getCharacter()->getChild( 0 )->getMinLevelReq() <= p->getLevel() ) {
		message = Constants::getMessage( Constants::TRAINING_AVAILABLE );
	} else if( p->getAvailableSkillMod() > 0 ) {
		message = Constants::getMessage( Constants::SKILL_POINTS_AVAILABLE );
	}

	if( message ) {
		glColor4f( 0, 0, 0, 0.4f );
		glPushMatrix();
		glTranslatef( 3, 14, 0 );
		glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glBegin( GL_QUADS );
		glVertex2i( 0, 0 );
    glVertex2i( 40, 0 );
    glVertex2i( 40, 13 );
    glVertex2i( 0, 13 );
		glEnd();
		glDisable( GL_BLEND );
		glPopMatrix();
		glColor3f( 1, 0.75f, 0 );
		getSDLHandler()->texPrint( 5, 24, message );
	}

  // show stat mods
  glEnable(GL_TEXTURE_2D);
  glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
  int xp = 0;
  int yp = 1;
  float n = 12;
  int row = ( w->getWidth() / (int)n );
  for(int i = 0; i < Constants::STATE_MOD_COUNT + 2; i++) {
		GLuint icon = 255;

		if( i == Constants::STATE_MOD_COUNT && 
				p->getThirst() <= 5 ) {
			icon = getSession()->getShapePalette()->getThirstIcon();
			if( p->getThirst() <= 3 ) {
				glColor4f( 1.0f, 0.2f, 0.2f, 0.5f );
			} else {
				glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
			}
		} else if( i == Constants::STATE_MOD_COUNT + 1 &&
							 p->getHunger() <= 5 ) {
			icon = getSession()->getShapePalette()->getHungerIcon();
			if( p->getHunger() <= 3 ) {
				glColor4f( 1.0f, 0.2f, 0.2f, 0.5f );
			} else {
				glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
			}
		} else if(p->getStateMod(i)) {
      icon = getSession()->getShapePalette()->getStatModIcon(i);
			glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
		}

		if( icon < 255 ) {
			glBindTexture( GL_TEXTURE_2D, icon );
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
    if( theme->getSelectedCharacterBorder() ) {
      glColor4f( theme->getSelectedCharacterBorder()->color.r,
                 theme->getSelectedCharacterBorder()->color.g,
                 theme->getSelectedCharacterBorder()->color.b,
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
  Date d(0, 0, 10, 0, 0, 0); // 2 hours (format : sec, min, hours, days, months, years)
  for(int i = 0; i < party->getPartySize() ; i++){
    e = new ThirstHungerEvent(party->getCalendar()->getCurrentDate(), d, party->getParty(i), this, Event::INFINITE_EXECUTIONS);
    party->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!
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

void Scourge::executeItem( Item *item ) {
  Creature *creature = getParty()->getPlayer();
  creature->setAction(Constants::ACTION_CAST_SPELL,
                      item,
                      item->getSpell());
  if(!item->getSpell()->isPartyTargetAllowed()) {
    setTargetSelectionFor(creature);
  } else {
    creature->setTargetCreature(creature);
  }
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
	if( getTradeDialog()->getWindow()->isVisible() ) 
		getTradeDialog()->updateUI();
	refreshContainerGui();
}

void Scourge::refreshInventoryUI() {
  getInventory()->refresh();
	if( getTradeDialog()->getWindow()->isVisible() ) 
		getTradeDialog()->updateUI();
	refreshContainerGui();
}

void Scourge::updatePartyUI() {

  // FIXME: for now, just print the date.
  // Expect a spanky new date ui soon. (complete with moon phases, etc.)
  if( getParty()->getCalendar()->update( getUserConfiguration()->getGameSpeedLevel() ) ){
    sprintf( version, "S.C.O.U.R.G.E. v%s %s", SCOURGE_VERSION,
             getParty()->getCalendar()->getCurrentDate().getDateString() );
    mainWin->setTitle( version );
  }

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
      //roundButton->setLabel("Begin Turn");
			roundButton->setTexture( getShapePalette()->getStartTexture() );
      roundButton->setGlowing(true);
     	roundButton->setTooltip( "Begin Turn" );
    } else {
      //roundButton->setLabel("...in Turn...");
			roundButton->setTexture( getShapePalette()->getWaitTexture() );
      roundButton->setGlowing(false);
     	roundButton->setTooltip( "...In Turn..." );
    }
  } else {
    if(startRound) {
			//roundButton->setLabel("Real-Time      ");
			roundButton->setTexture( getShapePalette()->getRealTimeTexture() );
			roundButton->setTooltip( "Pause Game" );
		} else { 
			//roundButton->setLabel("Paused");
			roundButton->setTexture( getShapePalette()->getPausedTexture() );
			roundButton->setTooltip( "Unpause game." );
		}
    roundButton->setGlowing(false);
  }
  roundButton->setSelected(startRound);
}

void Scourge::setFormationUI(int formation, bool playerOnly) {
  groupButton->setSelected(playerOnly);
  roundButton->setSelected(true);
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
                          12, false, true);
	progress->setStatus( 12 );
}

#define BOARD_GUI_WIDTH 600
#define BOARD_GUI_HEIGHT 300

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
	int colWidth = (int)( BOARD_GUI_WIDTH * 0.6f );
	int colHeight = BOARD_GUI_HEIGHT / 2 - 30;
  missionList = new ScrollingList(5, 30, colWidth, colHeight, getSession()->getShapePalette()->getHighlightTexture());
  boardWin->addWidget(missionList);
  boardWin->createLabel( colWidth + 5, 25, "Drag map to look around." );
  mapWidget = new MapWidget( this, boardWin, 
														 colWidth + 10, 30,
                             BOARD_GUI_WIDTH - 10,
                             BOARD_GUI_HEIGHT - 30,
                             false );
  boardWin->addWidget( mapWidget );
  //missionDescriptionLabel = new Label(5, 210, "", 67);
  missionDescriptionLabel = new ScrollingLabel( 5, 30 + colHeight + 5,
                                                colWidth,
                                                colHeight - 5, "" );
  boardWin->addWidget(missionDescriptionLabel);
  playMission = new Button(5, 5, 125, 25, getSession()->getShapePalette()->getHighlightTexture(), Constants::getMessage(Constants::PLAY_MISSION_LABEL));
  boardWin->addWidget(playMission);
  closeBoard = new Button(130, 5, 250, 25, getSession()->getShapePalette()->getHighlightTexture(), Constants::getMessage(Constants::CLOSE_LABEL));
  boardWin->addWidget(closeBoard);
}

void Scourge::updateBoardUI(int count, const char **missionText, Color *missionColor) {
  missionList->setLines(count, missionText, missionColor);
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

void Scourge::showItemInfoUI(Item *item, int level) {
  infoGui->setItem( item );
  if(!infoGui->getWindow()->isVisible()) infoGui->getWindow()->setVisible( true );
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

void Scourge::loadCharacterSounds( char *type ) {
  getSDLHandler()->getSound()->loadCharacterSounds( type );
}

void Scourge::unloadCharacterSounds( char *type ) {
  getSDLHandler()->getSound()->unloadCharacterSounds( type );
}

void Scourge::playCharacterSound( char *type, int soundType ) {
  getSDLHandler()->getSound()->playCharacterSound( type, soundType );
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

GLuint Scourge::getGuiTexture() { 
	return getShapePalette()->getGuiTexture(); 
}

GLuint Scourge::getGuiTexture2() { 
	return getShapePalette()->getGuiTexture2(); 
}

GLuint Scourge::loadSystemTexture( char *line ) {
  return getShapePalette()->loadSystemTexture( line );
}

// check for interactive items.
Color *Scourge::getOutlineColor( Location *pos ) {
  return view->getOutlineColor( pos );
}

bool Scourge::saveGame(Session *session) {
  session->getPreferences()->createConfigDir();

  {
    char path[300];
    get_file_name( path, 300, session->getSavegameName() );
		cerr << "Saving: " << path << endl;
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
    get_file_name( path, 300, getSession()->getValuefile() );
		cerr << "Saving: " << path << endl;
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

bool Scourge::loadGame( Session *session, char *error ) {
	char path[300];
	strcpy( error, "" );
	get_file_name( path, 300, session->getSavegameName() );
	cerr << "Loading: " << path << endl;
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
		strcpy( error, "Error: Saved game version is too old." );
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
		get_file_name( path, 300, getSession()->getValuefile() );
		cerr << "Loading: " << path << endl;
		FILE *fp = fopen( path, "rb" );
		if( fp ) {
			File *file = new File( fp );
			getSession()->getSquirrel()->loadValues( file );
			delete file;
		} else {
			cerr << "*** Warning: can't find values file." << endl;
		}
	}
  return true;
}

bool Scourge::testLoadGame( Session *session ) {
  char path[300];
  get_file_name( path, 300, session->getSavegameName() );
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
  return view->startTextEffect( message );
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

void Scourge::completeCurrentMission() {
  getSession()->getCurrentMission()->setCompleted( true );
  if( getSession()->getCurrentMission()->isStoryLine() )
    board->storylineMissionCompleted( getSession()->getCurrentMission() );
  missionCompleted();
}

Battle *Scourge::getCurrentBattle() {
  return battleRound[battleTurn];
}

void Scourge::endCurrentBattle() {
  battleRound[battleTurn]->endTurn();
}

void Scourge::resetInfos() {
  view->resetInfos();
}

void Scourge::evalSpecialSkills() {
  // re-eval the special skills
  //cerr << "Evaluating special skills at level's start" << endl;
  //Uint32 t = SDL_GetTicks();
  for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
    session->getParty()->getParty(i)->evalSpecialSkills();
  }
  for( int i = 0; i < session->getCreatureCount(); i++ ) {
    session->getCreature(i)->evalSpecialSkills();
  }
  //    cerr << "\tIt took: " <<
  //      ( ((float)( SDL_GetTicks() - t ) / 1000.0f ) ) <<
  //      " seconds." << endl;
}

bool Scourge::playSelectedMission() {
  int selected = missionList->getSelectedLine();
  if(selected != -1 && selected < board->getMissionCount()) {
    nextMission = selected;
    oldStory = currentStory = 0;
    endMission();
    return true;
  }
  return false;
}

void Scourge::movePartyToGateAndEndMission() {
  exitLabel->setText(Constants::getMessage(Constants::EXIT_MISSION_LABEL));
  exitConfirmationDialog->setVisible(false);
  endMission();
  // move the creature to the gate so it will be near it on the next level
  if( gatepos ) {
    for (int i = 0; i < getParty()->getPartySize(); i++) {
      if (!getParty()->getParty(i)->getStateMod(Constants::dead)) {
        getParty()->getParty(i)->moveTo( gatepos->x, gatepos->y, gatepos->z );
      }
    }
    gatepos = NULL;
  }
}                                   

void Scourge::showExitConfirmationDialog() {
  party->toggleRound(true);
  exitConfirmationDialog->setVisible(true);
}

void Scourge::closeExitConfirmationDialog() {
  gatepos = NULL;
  teleporting = false;
  changingStory = false;
  currentStory = oldStory;
  exitLabel->setText(Constants::getMessage(Constants::EXIT_MISSION_LABEL));
  exitConfirmationDialog->setVisible(false);
}

void Scourge::runSquirrelConsole() {
  squirrelLabel->appendText( "> " );
  squirrelLabel->appendText( squirrelText->getText() );
  squirrelLabel->appendText( "|" );
  getSession()->getSquirrel()->compileBuffer( squirrelText->getText() );
  squirrelText->clearText();
  squirrelLabel->appendText( "|" );
}

void Scourge::clearSquirrelConsole() {
  squirrelLabel->setText( "" );
}

void Scourge::selectDropTarget( Uint16 mapx, Uint16 mapy, Uint16 mapz ) {
  // drop target?
  Location *dropTarget = NULL;
  if(movingItem) {
    if(mapx < MAP_WIDTH) {
      dropTarget = getMap()->getLocation(mapx, mapy, mapz);
      if(!(dropTarget &&
           (dropTarget->creature ||
            (dropTarget->item &&
             ((Item*)(dropTarget->item))->getRpgItem()->getType() == RpgItem::CONTAINER)))) {
        dropTarget = NULL;
      }
    }
  }
  getMap()->setSelectedDropTarget(dropTarget);
}

void Scourge::updateBoard() {
  int selected = missionList->getSelectedLine();
  if( selected != -1 && selected < board->getMissionCount()) {
    Mission *mission = board->getMission(selected);
    missionDescriptionLabel->setText((char*)(mission->getDescription()));
    mapWidget->setSelection( mission->getMapX(), mission->getMapY() );
  }
}

// I have no recollection about what this is for...
void Scourge::mouseClickWhileExiting() {
  if( teleporting && !exitConfirmationDialog->isVisible() ) {
    exitLabel->setText(Constants::getMessage(Constants::TELEPORT_TO_BASE_LABEL));
    party->toggleRound(true);
    exitConfirmationDialog->setVisible(true);
  } else if( changingStory && !exitConfirmationDialog->isVisible() ) {
    exitLabel->setText(Constants::getMessage(Constants::USE_GATE_LABEL));
    party->toggleRound(true);
    exitConfirmationDialog->setVisible(true);
  }
}

RenderedCreature *Scourge::createWanderingHero( int level ) {
	return mainMenu->createWanderingHero( level );
}

void Scourge::handleWanderingHeroClick( Creature *creature ) {
	if( getSession()->getParty()->getPartySize() == MAX_PARTY_SIZE ) {
		showMessageDialog( "You cannot hire more party members." );
	} else {

    pcEditor->setCreature( creature, false );
    pcEditor->getWindow()->setVisible( true );

    /*
		char msg[300];
		sprintf( msg, "Would you like %s the %s to join your party?", 
						 creature->getName(),
						 creature->getCharacter()->getName() );
		hireHeroDialog->setText( msg );
		hireHeroDialog->setObject( creature );
		hireHeroDialog->setVisible( true );
    */
	}
}

void Scourge::handleDismiss( int index ) {
	char msg[300];
	sprintf( msg, "Would you like to dismiss %s the %s?", 
					 getParty()->getParty( index )->getName(),
					 getParty()->getParty( index )->getCharacter()->getName() );
	dismissHeroDialog->setText( msg );
	dismissHeroDialog->setMode( index );
	dismissHeroDialog->setVisible( true );	
}

void Scourge::descendDungeon( Location *pos ) {
	oldStory = currentStory;
	currentStory++;
	changingStory = true;
	gatepos = pos;
}

void Scourge::ascendDungeon( Location *pos ) {
	oldStory = currentStory;
	currentStory--;
	changingStory = true;
	gatepos = pos;
}

void Scourge::showTextMessage( char *message ) {
	textDialog->setText( message );
	textDialog->setVisible( true );
}
