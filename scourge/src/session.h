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

#include <vector>
#include "common/constants.h"
#include "preferences.h"
//#include "board.h"
#include "party.h"
#include "net/server.h"
#include "net/client.h"
#include "net/gamestatehandler.h"
#include "net/commands.h"
#include "gameadapter.h"

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
class Preferences;
class RpgItem;
class Spell;
class Character;
class Monster;
class GLShape;
class SqBinding;
class ShapePalette;

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
  std::vector<Item*> newItems;
  std::vector<Creature*> creatures;
  SqBinding *squirrel;
  std::map<RpgItem*, Item*> special;

  // private constructor: call startGame instead.
  Session(GameAdapter *adapter);

public:


  /**
   * The main method for a project to run a game.
   * Pass in a GameAdapter implementation (eg.: Scourge class)
   * @return the value when the game exits. You can return this
   * from your main() function back to the os.
   */
  static int runGame( GameAdapter *adapter, int argc, char *argv[] );


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

  inline bool isMultiPlayerGame() { return multiplayerGame; }
  inline void setMultiPlayerGame(bool b) { multiplayerGame = b; }
  inline GameAdapter *getGameAdapter() { return adapter; }
  inline void playSound(const char *sound) { getGameAdapter()->playSound(sound); }

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
  virtual Creature *newCreature( Monster *monster, GLShape *shape, bool loaded=false );
	virtual Creature *newCreature( Character *character, char *name, int model );
  inline int getCreatureCount() { return creatures.size(); }
  inline Creature *getCreature(int index) { return creatures[index]; }
  inline int getItemCount() { return newItems.size(); }
  inline Item *getItem(int index) { return newItems[index]; }
  virtual void deleteCreaturesAndItems(bool missionItemsOnly=false);

  inline ShapePalette *getShapePalette() { return shapePal; }
  inline Map *getMap() { return map; }
  inline Board *getBoard() { return board; }
  inline Party *getParty() { return party; }
  inline Preferences *getPreferences() { return getGameAdapter()->getPreferences(); }
  inline Mission *getCurrentMission() { return currentMission; }
  inline void setCurrentMission(Mission *mission) { currentMission = mission; }

  virtual Creature *getClosestVisibleMonster(int x, int y, int w, int h, int radius);
  virtual void creatureDeath(Creature *creature);

  inline SqBinding *getSquirrel() { return squirrel; }

  inline void setSpecialItem( RpgItem *rpgItem, Item *item ) { special[ rpgItem ] = item; }
  inline Item *getSpecialItem( RpgItem *rpgItem ) {
    if( special.find( rpgItem ) == special.end() ) return NULL;
    else return special[ rpgItem ];
  }

protected:
  virtual void initData();

};

#endif

