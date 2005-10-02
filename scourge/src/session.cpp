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
#include <iostream>
#include <stdlib.h>
#include <strings.h>

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
}

Session::~Session() {
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
  adapter->initStart(7, "Loading shapes...");

  shapePal->initialize();

  adapter->initUpdate("Loading characters...");

  // init characters first. Items use it for acl
  Character::initCharacters();

  adapter->initUpdate("Loading items...");

  // initialize the items
  Item::initItems(getShapePalette());

  adapter->initUpdate("Loading spells...");
  // initialize magic
  MagicSchool::initMagic();

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

  adapter->initUpdate("Initializing...");

  
  
  
  
  
  map = new Map( adapter, adapter->getPreferences(), getShapePalette() );
  
  adapter->initUI();

  adapter->initEnd();

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
  int itemLevel = level;
  if( !loading ) {
    itemLevel = level + (int)( 6.0f * rand() / RAND_MAX ) - 3;
  }
  if( itemLevel < 1 ) itemLevel = 1;
  Item *item = new Item(this, rpgItem, itemLevel, loading);
  if(spell) item->setSpell(spell);
  newItems.push_back( item );
  return item;
}

// creatures created for the mission
Creature *Session::newCreature(Monster *monster, GLShape *shape, bool loaded) {
  Creature *c = new Creature(this, monster, shape, loaded);
  creatures.push_back( c );
  return c;
}

void Session::deleteCreaturesAndItems(bool missionItemsOnly) {
  // delete the items and creatures created for this mission
  // (except items in inventory) 
  if(!missionItemsOnly) {
    for(int i = 0; i < (int)newItems.size(); i++) {
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
       !getCreature(i)->getMonster()->isNpc() &&
       map->isLocationVisible(toint(getCreature(i)->getX()), 
                              toint(getCreature(i)->getY())) &&
       map->isLocationInLight(toint(getCreature(i)->getX()), 
                              toint(getCreature(i)->getY())) &&
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

  if( !( creature->isMonster() ) ) {
    char message[255];
    sprintf( message, "  %s dies!", creature->getName() );
    getGameAdapter()->startTextEffect( message );
  }
}                 

int Session::runGame( GameAdapter *adapter, int argc, char *argv[] ) {

  // Only allocate once. (May leak some mem. but saves a lot of headaches.)
  rootDir = (char*)malloc( 300 * sizeof( char ) );
  
  // init the rootdir via binreloc
#ifdef WIN32
  // for windows (binreloc doesn't compile in windows)
  strcpy( rootDir, "data" ); 
#else
#ifdef ENABLE_BINRELOC
//  rootDir = (char*)BR_DATADIR( "/data" );
  strcpy( root, (char*)BR_DATADIR( "/data" ) );
#else
  strcpy( rootDir, DATA_DIR );
#endif
#endif  

  // FIXME: for windows, if this doesn't work, try using DATA_DIR
  // which is made by autoconf

  // Check to see if there's a local version of the data dir
  // (ie. we're running in the build folder and not in a distribution)
  char dir[300];
  dir[0] = '\0';
  cerr << "app path: " << argv[0] << endl;
  findLocalResources(argv[0], dir);
  if(strlen(dir)) {
    cerr << "*** Using local data dir. Not running a distribution. dir=" << dir << endl;
    sprintf(rootDir, "%sdata", dir);
  }
  
  // config check
  if(argc >= 2 && !strcmp(argv[1], "--test-config")) {
    cerr << "Configuration:" << endl;
    char dir[300];
    char file[500];
    int dir_res = get_config_dir_name( dir, 300 );
    int file_res = get_config_file_name( file, 500 );
    cerr << "starting app: " << argv[0] << endl;
    cerr << "rootDir=" << rootDir << 
      "\nconfigDir=" << configDir << 
      "\nconfigFile=" << CONFIG_FILE << 
      "\ndir=" << dir << " dir_res=" << dir_res <<
      "\nfile=" << file << " file_res=" << file_res <<	endl;
    return 0;
  }
  
  // do a final sanity check before running the game
  if(!checkFile(rootDir, "/cursor.bmp")) {
    cerr << "ERROR: check for files failed in data dir: " << rootDir << endl;
    cerr << "Either install the data files at the above location, or rebuild with ./configure --with-data-dir=<new location> or run the game from the source distribution's main directory (the one that contains src,data,etc.)" << endl;
    return 1;
  }
  
  cerr << "Starting session, using rootDir=" << rootDir << endl;

  Session *session = new Session( adapter );
  session->initialize();
  session->start();
  
  return EXIT_SUCCESS;
}

bool Session::checkFile(const char *dir, const char *file) {
  char path[300];
  strcpy(path, dir);
  strcat(path, file);
  //fprintf(stderr, "\tchecking path: %s\n", path);
  bool ret = true;
  FILE *fp = fopen(path, "rb");
  if(!fp || ferror(fp)) ret = false;
  if(fp) fclose(fp);
  return ret;
}

// this function is used to be able to run scourge while developing
void Session::findLocalResources(const char *appPath, char *dir) {
  // Where are we running from?
  strcpy(dir, appPath);	 
  // Look in this and the parent dir for a 'data' folder
  // ('i' has to count to at least 4 for OS X)
  for(int i = 0; i < 10; i++) {
    char *pp = strrchr(dir, '/');
    char *p = strrchr(dir, SEPARATOR);
    if(!p && !pp) {
      dir[0] = '\0';
      cerr << "*** Can't find local version of data dir. You're running a distribution." << endl;
      return;
    }
    // Take whichever comes first. This is to solve a problem when running in
    // mingw or cygwin. It may cause problems if the actual path has a \ or / in it.
    if(pp > p) p = pp;
    *(p + 1) = 0;
    //	fprintf(stderr, "*** Looking at: dir=%s\n", dir);
    if(checkFile(dir, "data/cursor.bmp")) return;
    // remove the last separator
    *(p) = 0;
  }
  dir[0] = '\0';
}

