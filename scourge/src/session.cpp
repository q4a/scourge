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
  // new item and creature references
  itemCount = creatureCount = 0;
  multiplayerGame = false;
  currentMission = NULL;
}

Session::~Session() {
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
  adapter->initVideo(shapePal);
  initData();  
}

void Session::start() {
  adapter->start();
}

void Session::initData() {

  adapter->initStart(7, "Loading shapes...");

  shapePal->initialize();

  map = new Map(this);

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

  adapter->initUpdate("Loading missions...");

  // create the mission board
  board = new Board(this);

  adapter->initUpdate("Creating party...");

  // do this before the inventory and optionsdialog (so Z is less than of those)
  party = new Party(this);

  adapter->initUpdate("Initializing...");

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

Item *Session::newItem(RpgItem *rpgItem, Spell *spell) {
  newItems[itemCount] = new Item(rpgItem);
  if(spell) newItems[itemCount]->setSpell(spell);
  itemCount++;
  return newItems[itemCount - 1];
}

// creatures created for the mission
Creature *Session::newCreature(Character *character, char *name) {
  creatures[creatureCount++] = new Creature(this, character, name);
  return creatures[creatureCount - 1];
}

// creatures created for the mission
Creature *Session::newCreature(Monster *monster, GLShape *shape) {
  creatures[creatureCount++] = new Creature(this, monster, shape);
  return creatures[creatureCount - 1];
}

void Session::deleteCreaturesAndItems(bool missionItemsOnly) {
  // delete the items and creatures created for this mission
  // (except items in inventory) 
  if(!missionItemsOnly) {
    for(int i = 0; i < itemCount; i++) {
      delete newItems[i];
    }
    itemCount = 0;
  } else {
    for(int i = 0; i < itemCount; i++) {
      bool inInventory = false;
      for(int t = 0; t < getParty()->getPartySize(); t++) {
        if(getParty()->getParty(t)->isItemInInventory(newItems[i])) {
          inInventory = true;
          break;
        }
      }
      if(!inInventory) {
        delete newItems[i];
        itemCount--;
        for(int t = i; t < itemCount; t++) {
          newItems[t] = newItems[t + 1];
        }
        i--;
      }
    }
  }
  for(int i = 0; i < creatureCount; i++) {
    delete creatures[i];
  }
  creatureCount = 0;
  /*
    cerr << "After mission: " <<
    " creatureCount=" << creatureCount << 
    " itemCount=" << itemCount << endl;
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
}                 

