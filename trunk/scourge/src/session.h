/***************************************************************************
                          session.h  -  description
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

#ifndef SESSION_H
#define SESSION_H

#include "constants.h"
#include "userconfiguration.h"
#include "map.h"
#include "board.h"
#include "party.h"
#include "item.h"
#include "creature.h"
#include "shape.h"
#include "rpg/character.h"
#include "rpg/monster.h"
#include "rpg/rpgitem.h"
#include "rpg/spell.h"
#include "net/server.h"
#include "net/client.h"
#include "net/gamestatehandler.h"
#include "net/commands.h"
#include "gameadapter.h"

using namespace std;

#ifdef HAVE_SDL_NET
class Server;
class Client;
#endif
class Mission;
class Board;
class Party;
class Map;
class Item;
class Creature;
class GameAdapter;

/**
 *@author Gabor Torok
 */

class Session {
private:
  ShapePalette *shapePal;
  GameAdapter *adapter;
  Party *party;
  Map *map;
  Board *board;
#ifdef HAVE_SDL_NET
  Server *server;
  Client *client;
#endif
  bool multiplayerGame;
  Mission *currentMission;
  Item *items[500];
  Creature *creatures[500];
  int itemCount;
  int creatureCount;

public:
  Session(GameAdapter *adapter);
  virtual ~Session();

  void initialize();

  virtual void start();
  virtual void quit(int value);
#ifdef HAVE_SDL_NET
  virtual void runClient(char *host, int port, char *userName);
  virtual void runServer(int port);
  virtual inline Server *getServer() { return server; }
  virtual inline Client *getClient() { return client; }
  virtual void startServer(GameStateHandler *gsh, int port);
  virtual void startClient(GameStateHandler *gsh, CommandInterpreter *ci, char *host, int port, char *username);
  virtual void stopClientServer();
#endif
  virtual Item *newItem(RpgItem *rpgItem, Spell *spell=NULL);
  virtual Creature *newCreature(Character *character, char *name);
  virtual Creature *newCreature(Monster *monster);
  virtual void deleteCreaturesAndItems(bool missionItemsOnly=false);
  inline bool isMultiPlayerGame() { return multiplayerGame; }
  inline void setMultiPlayerGame(bool b) { multiplayerGame = b; }
  inline ShapePalette *getShapePalette() { return shapePal; }

  inline Map *getMap() { return map; }
  inline Board *getBoard() { return board; }
  inline Party *getParty() { return party; }

protected:
  virtual void initData();
};

#endif

