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
  shapePal = new ShapePalette();
  adapter->initVideo(shapePal);
  initData();
  adapter->initUI(this);
}

void Session::start() {
  adapter->start();
}

void Session::initData() {
  shapePal->initialize();

  map = new Map(this);

  // init characters first. Items use it for acl
  Character::initCharacters();
  // initialize the items
  Item::initItems(getShapePalette());
  // initialize magic
  MagicSchool::initMagic();
  // initialize the monsters (they use items, magic)
  Monster::initMonsters();

  // create the mission board
//  board = new Board(this);

  // do this before the inventory and optionsdialog (so Z is less than of those)
  party = new Party(this);  
}

void Session::quit(int value) {
  // FIXME: if(getSDLHandler()) getSDLHandler()->quit(value);
  exit(value);
}

#ifdef HAVE_SDL_NET
void Session::runServer(int port) {
  /*
  GameStateHandler *gsh = new TestGameStateHandler();
  server = new Server(port ? port : DEFAULT_SERVER_PORT);
  server->setGameStateHandler(gsh);
  
  // wait for the server to quit
  int status;
  SDL_WaitThread(server->getThread(), &status);

  delete gsh;
  */
}

void Session::runClient(char *host, int port, char *userName) {
  /*
  CommandInterpreter *ci = new TestCommandInterpreter();
  GameStateHandler *gsh = new TestGameStateHandler();
  client = new Client((char*)host, port, (char*)userName, ci);
  client->setGameStateHandler(gsh);
  if(!client->login()) {
    cerr << Constants::getMessage(Constants::CLIENT_CANT_CONNECT_ERROR) << endl;
    return;
  }

  // connect as a character
  Party *party = new Party(this);  
  Creature **pc;
  int pcCount;
  Party::createHardCodedParty(this, &pc, &pcCount);
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
  */
}

void Session::startServer(GameStateHandler *gsh, int port) {
  server = new Server(port);
  server->setGameStateHandler(gsh);
}

void Session::startClient(GameStateHandler *gsh, CommandInterpreter *ci, char *host, int port, char *username) {
  client = new Client(host, port, username, ci);
  client->setGameStateHandler(gsh);
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
}

#endif

Item *Session::newItem(RpgItem *rpgItem, Spell *spell) {
  items[itemCount] = new Item(rpgItem);
  if(spell) items[itemCount]->setSpell(spell);
  itemCount++;
  return items[itemCount - 1];
}

// creatures created for the mission
Creature *Session::newCreature(Character *character, char *name) {
  creatures[creatureCount++] = new Creature(this, character, name);
  return creatures[creatureCount - 1];
}

// creatures created for the mission
Creature *Session::newCreature(Monster *monster) {
  creatures[creatureCount++] = new Creature(this, monster);
  return creatures[creatureCount - 1];
}

void Session::deleteCreaturesAndItems(bool missionItemsOnly) {
  // delete the items and creatures created for this mission
  // (except items in inventory) 
  for(int i = 0; i < itemCount; i++) {
    if(!missionItemsOnly) {
      delete items[i];
      itemCount--;
      continue;
    }
    bool inInventory = false;
    for(int t = 0; t < getParty()->getPartySize(); t++) {
      if(getParty()->getParty(t)->isItemInInventory(items[i])) {
        inInventory = true;
        break;
      }
    }
    if(!inInventory) {
      delete items[i];
      itemCount--;
      for(int t = i; t < itemCount; t++) {
        items[t] = items[t + 1];
      }
      i--;
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


