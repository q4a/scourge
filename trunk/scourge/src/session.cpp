/***************************************************************************
                          session.cpp  -  description
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
#include "session.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "item.h"
#include "creature.h"
#include "sqbinding/sqbinding.h"
#include "test/combattest.h"
#include "shapepalette.h"
#include <iostream>
#include <stdlib.h>
#include <strings.h>

#include "creature.h"
#include "persist.h"
#include "io/file.h"


using namespace std;

/**
 *@author Gabor Torok
 */
Session::Session(GameAdapter *adapter) {
  this->adapter = adapter;
  shapePal = NULL;
  party = NULL;
  map = NULL;
  board = NULL;
#ifdef HAVE_SDL_NET
  server = NULL;
  client = NULL;
#endif
  multiplayerGame = false;
  currentMission = NULL;
  squirrel = NULL;
	strcpy( savegame, "" );
	strcpy( loadgame, "" );
}

Session::~Session() {
  if( squirrel ) delete squirrel;
  deleteCreaturesAndItems();
  if(shapePal) delete shapePal;
  if(party) delete party;
  if(board) delete board;
  if(map) delete map;
#ifdef HAVE_SDL_NET
  if(server) delete server;
  if(client) delete client;
#endif
  delete adapter;
}

void Session::initialize() {
  Constants::initConstants();
  shapePal = new ShapePalette(this);
  adapter->setSession(this);
  adapter->initVideo();
  initData();  
}

void Session::start() {
  adapter->start();
}

void Session::initData() {

  // move all this down to scourge
  adapter->initStart(12, "Loading shapes...");

  shapePal->initialize();

  adapter->initUpdate("Loading characters...");  

  adapter->initUpdate("Loading items...");

	// read the skills, etc.
	Rpg::initRpg();

  // initialize the items
  Item::initItems(getShapePalette());

  adapter->initUpdate("Loading spells...");
  // initialize magic
  MagicSchool::initMagic();

	// init professions
  Character::initCharacters();

  adapter->initUpdate("Loading monsters...");

  // initialize the monsters (they use items, magic)
  Monster::initMonsters();
  shapePal->loadNpcPortraits();

  adapter->initUpdate("Loading missions...");

  // create the mission board
  board = new Board(this);

  adapter->initUpdate("Creating party...");

  // do this before the inventory and optionsdialog (so Z is less than of those)
  party = new Party(this);

  adapter->initUpdate("Starting Squirrel VM...");
  squirrel = new SqBinding( this );

  adapter->initUpdate("Initializing Special Skills...");
  SpecialSkill::initSkills();

  adapter->initUpdate("Initializing map...");
  map = new Map( adapter, adapter->getPreferences(), getShapePalette() );
  
  adapter->initUpdate("Initializing ui...");
  adapter->initUI();

	adapter->initUpdate("finishing initialization...");
  adapter->initEnd();
  
 	adapter->initUpdate("Initialization done.");
}

void Session::quit(int value) {
  // FIXME: if(getSDLHandler()) getSDLHandler()->quit(value);
  exit(value);
}

#ifdef HAVE_SDL_NET
void Session::runServer(int port) {
  GameStateHandler *gsh = new TestGameStateHandler();
  server = new Server(port ? port : DEFAULT_SERVER_PORT);
  server->setGameStateHandler(gsh);
  
  // wait for the server to quit
  int status;
  SDL_WaitThread(server->getThread(), &status);

  delete gsh;
}

void Session::runClient(char *host, int port, char *userName) {
  CommandInterpreter *ci = new TestCommandInterpreter();
  GameStateHandler *gsh = new TestGameStateHandler();
  client = new Client((char*)host, port, (char*)userName, ci);
  client->setGameStateHandler(gsh);
  if(!client->login()) {
    cerr << Constants::getMessage(Constants::CLIENT_CANT_CONNECT_ERROR) << endl;
    return;
  }

  // connect as a character
//  Party *party = new Party(this);  
  Creature *pc[MAX_PARTY_SIZE];
  int pcCount;
  Party::createHardCodedParty(this, pc, &pcCount);
  cerr << "Sending character: " << pc[0]->getName() << endl;
  getParty()->resetMultiplayer(pc[0]);

  char message[80];
  while(true) {
    cout << "> ";
    int c;
    int n = 0;
    while(n < 79 && (c = getchar()) != '\n') message[n++] = c;
    message[n] = 0;
    client->sendChatTCP(message);
    //client->sendRawTCP(message);
  }  

  delete ci;
  delete gsh;
}

void Session::startServer(GameStateHandler *gsh, int port) {
  server = new Server(port);
  server->setGameStateHandler(gsh);
  multiplayerGame = true;
}

void Session::startClient(GameStateHandler *gsh, CommandInterpreter *ci, char *host, int port, char *username) {
  client = new Client(host, port, username, ci);
  client->setGameStateHandler(gsh);
  multiplayerGame = true;
}

void Session::stopClientServer() {
  if(server) {
    delete server;
    server = NULL;
  }
  if(client) {
    delete client;
    client = NULL;
  }
  multiplayerGame = false;
}

#endif

Item *Session::newItem(RpgItem *rpgItem, int level, Spell *spell, bool loading) {
  // don't randomize special items
  if( rpgItem->isSpecial() ) loading = true;
  int itemLevel = level;
  if( !loading ) {
    itemLevel = level + (int)( 6.0f * rand() / RAND_MAX ) - 3;
  }
  if( itemLevel < 1 ) itemLevel = 1;
  Item *item = new Item(this, rpgItem, itemLevel);
  if(spell) item->setSpell(spell);
  newItems.push_back( item );
  if( rpgItem->isSpecial() ) setSpecialItem( rpgItem, item );
  return item;
}

// creatures created for the mission
Creature *Session::newCreature(Monster *monster, GLShape *shape, bool loaded) {
  Creature *c = new Creature(this, monster, shape, loaded);
  creatures.push_back( c );
  return c;
}

Creature *Session::newCreature( Character *character, char *name, int sex, int model ) {
	Creature *c = new Creature( this, character, name, sex, model );
  creatures.push_back( c );
  return c;
}

bool Session::removeCreatureRef( Creature *creature, int index ) {
	for( vector<Creature*>::iterator i = creatures.begin(); i != creatures.end(); ++i ) {
		Creature *c = *i;
		if( c == creature ) {
			creatures.erase( i );
			return true;
		}
	}
	return false;
}

void Session::addCreatureRef( Creature *creature, int index ) {
	creatures.push_back( creature );
}

void Session::deleteCreaturesAndItems(bool missionItemsOnly) {
  // delete the items and creatures created for this mission
  // (except items in inventory) 
  if(!missionItemsOnly) {
    for(int i = 0; i < (int)newItems.size(); i++) {
      if( newItems[i]->isSpecial() ) {
        // put special item back into play
        special.erase( newItems[i]->getRpgItem() );
      }
      delete newItems[i];
    }
    newItems.clear();
  } else {
    for(int i = 0; i < (int)newItems.size(); i++) {
      bool inInventory = false;
      for(int t = 0; t < getParty()->getPartySize(); t++) {
        if(getParty()->getParty(t)->isItemInInventory(newItems[i])) {
          inInventory = true;
          break;
        }
      }
      if(!inInventory) {
        if( newItems[i]->isSpecial() ) {
          // put special item back into play
          special.erase( newItems[i]->getRpgItem() );
        }
        delete newItems[i];
        for(int t = i; t < (int)newItems.size(); t++) {
          newItems[t] = newItems[t + 1];
        }
        newItems.pop_back();
        i--;
      }
    }
  }
  for(int i = 0; i < (int)creatures.size(); i++) {
    delete creatures[i];
  }
  creatures.clear();

  /*
  cerr << "***************************************" << endl;
    cerr << "After mission: " <<
    " creatureCount=" << creatureCount << 
    " itemCount=" << itemCount << endl;
  cerr << "***************************************" << endl;
  */
	getShapePalette()->debugLoadedModels();
}

/** 
	Return the closest live player within the given radius or null if none can be found.
*/
Creature *Session::getClosestVisibleMonster(int x, int y, int w, int h, int radius) {
  float minDist = 0;
  Creature *p = NULL;
  for(int i = 0; i < getCreatureCount(); i++) {
    if(!getCreature(i)->getStateMod(Constants::dead) && 
       !getCreature(i)->getStateMod(Constants::possessed) && 
       !( getCreature(i)->getMonster() && getCreature(i)->getMonster()->isNpc() ) &&
       map->isLocationVisible(toint(getCreature(i)->getX()), 
                              toint(getCreature(i)->getY())) &&
       map->isLocationInLight(toint(getCreature(i)->getX()), 
                              toint(getCreature(i)->getY()),
                              getCreature(i)->getShape()) &&
       getCreature(i)->isMonster()) {
      float dist = Constants::distance(x, y, w, h,
                                       getCreature(i)->getX(),
                                       getCreature(i)->getY(),
                                       getCreature(i)->getShape()->getWidth(),
                                       getCreature(i)->getShape()->getDepth());
      if(dist <= (float)radius && (!p || dist < minDist)) {
        p = getCreature(i);
        minDist = dist;
      }
    }
  }
  return p;
}

void Session::creatureDeath(Creature *creature) {

  bool result;
  squirrel->callBoolMethod( "creatureDeath", 
                            squirrel->getCreatureRef( creature ), 
                            &result );
  // FIXME: not used currently
  //if( !result ) return;

  if(creature == party->getPlayer()) {
    party->switchToNextLivePartyMember();
  }
  // remove from the map; the object will be cleaned up at the end of the mission
  map->removeCreature(toint(creature->getX()), 
                      toint(creature->getY()), 
                      toint(creature->getZ()));
  // add a container object instead
  //if(battleRound.size() > 0) creature->getShape()->setCurrentAnimation(MD2_DEATH1);
  Item *item = newItem(RpgItem::getItemByName("Corpse"));
  // add creature's inventory to container
  map->setItem(toint(creature->getX()), 
               toint(creature->getY()), 
               toint(creature->getZ()), item);
  int n = creature->getInventoryCount();
  for(int i = 0; i < n; i++) {
    // make it contain all items, no matter what size
    item->addContainedItem(creature->removeInventory(0), true);
  }
  creature->setStateMod(Constants::dead, true);

	// cancel target, otherwise segfaults on resurrection
	creature->cancelTarget();

	if( !( creature->isMonster() ) ) {
    char message[255];
    sprintf( message, "  %s dies!", creature->getName() );
    getGameAdapter()->startTextEffect( message );
  }

#ifdef HAVE_SDL_NET
	bool foundLivePlayer = false;
	for( int i = 0; i < getParty()->getPartySize(); i++ ) {
		if( !getParty()->getParty( i )->getStateMod( Constants::dead ) ) {
			foundLivePlayer = true;
			break;
		}
	}
	if( !foundLivePlayer ) getGameAdapter()->askToUploadScore();
#endif
}

// define below to enable savegame testing
//#define TESTING_SAVEGAME 1

void testSaveGame( Session *session ) {
	cerr << "Loading savegame." << endl;  	
	Uint32 storylineIndex;
	Uint32 partySize;	
	Creature *pc[MAX_PARTY_SIZE];	
		
	{
	FILE *fp = fopen( "/home/gabor/.scourge/savegame.dat", "rb" );
	File *file = new File( fp );
	Uint32 n = PERSIST_VERSION;
	file->read( &n );	
	file->read( &storylineIndex );
	file->read( &partySize );
	cerr << "LOADING: " << endl;
	for(int i = 0; i < (int)partySize; i++) {
		CreatureInfo *info = Persist::loadCreature( file );
		pc[i] = Creature::load( session, info );
		if( i == 0 ) {
			for( int t = 0; t < Skill::SKILL_COUNT; t++ ) {
				cerr << "\tinfo=" << info->skills[t] <<
				" infoMOD=" << info->skillMod[t] <<  
				" SK=" << pc[i]->getSkill( t, false ) << 
				" MOD=" << pc[i]->getSkillMod( t ) <<  
				" BON=" << pc[i]->getSkillBonus( t ) <<  				
				endl;
			}
		}
		Persist::deleteCreatureInfo( info );		
	}
	delete file;
	}

	cerr << "Saving savegame." << endl;
	{	
	FILE *fp = fopen( "/home/gabor/.scourge/out.dat", "wb" );
  if( !fp ) {
    cerr << "Error creating savegame file!" << endl;
    return;
  }
  File *file = new File( fp );
  Uint32 n = PERSIST_VERSION;
  file->write( &n );
  file->write( &storylineIndex );
  file->write( &partySize );
  for(int i = 0; i < (int)partySize; i++) {
    CreatureInfo *info = pc[i]->save();
    Persist::saveCreature( file, info );
    Persist::deleteCreatureInfo( info );
  }
  delete file;
  }
  
 	cerr << "AFTER SAVING: " << endl;
	for( int t = 0; t < Skill::SKILL_COUNT; t++ ) {
		cerr << "\tSK=" << pc[0]->getSkill( t, false ) << endl;
	}
  
	cerr << "Done." << endl;  
}

int Session::runGame( GameAdapter *adapter, int argc, char *argv[] ) {

	int err = Constants::initRootDir( argc, argv );
	if( err ) return err;

#ifdef TESTING_SAVEGAME
	adapter = new GameAdapter( adapter->getPreferences() );
#endif
	
  Session *session = new Session( adapter );
  session->initialize();
  if(argc >= 2 && !strcmp(argv[1], "--run-tests")) {
    char *path = ( argc >= 3 ? 
                   argv[2] : 
                   (char*)"/home/gabor/sourceforge/scourge/api/tests" );
    if( CombatTest::executeTests( session, path ) ) {
      cout << "Tests were succesfully written to: " << path << endl;
      return 0;
    } else {
      cerr << "Error while running tests." << endl;
      return 1;
    }
    return 0;
  }
#ifndef TESTING_SAVEGAME  
  session->start();
#else  
	testSaveGame( session );
#endif

  return EXIT_SUCCESS;
}

int Session::getCountForDate( char *key, bool withinLastHour ) {
	int count = 0;
	char *value = getSquirrel()->getValue( key );
	if( value != NULL ) {
		char s[255];
		strcpy( s, value );
		char *p = strtok( s, "+" );		
		if( p != NULL ) {
			char *q = strtok( NULL, "+" );
			Date *lastUsed = new Date( p );
			Date now = getParty()->getCalendar()->getCurrentDate();

			bool withinDate = ( withinLastHour && now.isAnHourLater( *lastUsed ) ||
													!withinLastHour && now.isADayLater( *lastUsed ) );

			// did specified amount of time pass?
			if( !withinDate ) {				
				if( q ) {
					count = atoi( q );
				}
			}
			delete lastUsed;
		}
	}
	return count;
}

void Session::setCountForDate( char *key, int value ) {
	char s[255];
	sprintf( s, "%s+%d", 
					 getParty()->getCalendar()->getCurrentDate().getShortString(), 
					 value );
	getSquirrel()->setValue( key, s );
}

void Session::setSavegameName( char *s ) {
	strcpy( savegame, s );
}

Creature *Session::getCreatureByName( char *name ) {
	for( unsigned int i = 0; i < creatures.size(); i++ ) {
		if( !strcmp( creatures[i]->getName(), name ) ) return creatures[i];
	}
	return NULL;
}
