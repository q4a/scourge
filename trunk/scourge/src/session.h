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
//#include "board.h"
#include "party.h"
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
class UserConfiguration;
class RpgItem;
class Spell;
class Monster;
class GLShape;

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
  Item *newItems[500];
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

  /**
    Creat a new item for use on this story. Calling this method instead of new Item()
    directly ensures that the item will be cleaned up properly when the story is
    exited. Only items in a party member's inventory are not deleted.

    @param rpgItem if not NULL, the RpgItem template for the item to create.
    @param spell if not NULL, the spell to associate with the created scroll.
    @return the item created.
  */
  virtual Item *newItem(RpgItem *rpgItem, int level=1, Spell *spell=NULL, bool loading=false);

  /**
    Create a new creature for use on this story. Calling this method instead of new Creature()
    directly ensures that the creature will be cleaned up properly when the story is
    exited. 

    @param character the character class to use for the new creature.
    @param name the name of the new creature
    @return the creature created.
  */
  //virtual Creature *newCreature(Character *character, char *name);

  /**
    Create a new creature for use on this story. Calling this method instead of new Creature()
    directly ensures that the creature will be cleaned up properly when the story is
    exited. 

    @param monster the monster template to use for the new creature.
    @return the creature created.
  */
  virtual Creature *newCreature(Monster *monster, GLShape *shape);
  virtual void deleteCreaturesAndItems(bool missionItemsOnly=false);
  inline bool isMultiPlayerGame() { return multiplayerGame; }
  inline void setMultiPlayerGame(bool b) { multiplayerGame = b; }

  inline GameAdapter *getGameAdapter() { return adapter; }
  inline ShapePalette *getShapePalette() { return shapePal; }
  inline Map *getMap() { return map; }
  inline Board *getBoard() { return board; }
  inline Party *getParty() { return party; }
  inline UserConfiguration *getUserConfiguration() { return getGameAdapter()->getUserConfiguration(); }
  inline int getCreatureCount() { return creatureCount; }
  inline Creature *getCreature(int index) { return creatures[index]; }
  inline int getItemCount() { return itemCount; }
  inline Item *getItem(int index) { return newItems[index]; }
  inline Mission *getCurrentMission() { return currentMission; }
  inline void setCurrentMission(Mission *mission) { currentMission = mission; }
  inline void playSound(const char *sound) { getGameAdapter()->playSound(sound); }

  virtual Creature *getClosestVisibleMonster(int x, int y, int w, int h, int radius);
  virtual void creatureDeath(Creature *creature);

protected:
  virtual void initData();
};

#endif

