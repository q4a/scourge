/***************************************************************************
                          board.h  -  description
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

#ifndef BOARD_H
#define BOARD_H

#include "constants.h"
#include <map>
#include <vector>

using namespace std;

class RpgItem;
class Monster;
class Creature;
class Item;
class Session;

/**
  *@author Gabor Torok
  */
	
#define BOARD_GUI_WIDTH 400
#define BOARD_GUI_HEIGHT 400


class Mission {
private:
  int level;
  int depth;
  char name[80];
  char description[2000];
  char success[2000];
  char failure[2000];
  map<RpgItem*, bool> items;
  vector<RpgItem*> itemList;
  map<Monster*, bool> creatures;
  vector<Monster*> creatureList;
  map<Item*,RpgItem*> itemInstanceMap;
  map<Creature*,Monster*> monsterInstanceMap;
  bool completed;
  bool storyLine;
public:
  Mission( int level, int depth, char *name, char *description, char *success, char *failure );
  ~Mission();

  inline void setStoryLine( bool b ) { storyLine = b; }
  inline bool isStoryLine() { return storyLine; }

  inline void addCreature( Monster *monster ) {
    creatures[monster] = false;
    creatureList.push_back( monster );
  }

  inline void addItem( RpgItem *item ) {
    items[item] = false;
    itemList.push_back( item );
  }

  inline void addItemInstance( Item *item, RpgItem *rpgItem ) {
    itemInstanceMap[ item ] = rpgItem;
  }

  inline void addCreatureInstanceMap( Creature *creature, Monster *monster ) {
    monsterInstanceMap[ creature ] = monster;
  }

  inline bool isCompleted() { return completed; }
  inline void setCompleted( bool b ) { completed = b; }
  inline char *getName() { return name; }
  inline char *getDescription() { return description; }
  inline char *getSuccess() { return success; }
  inline char *getFailure() { return failure; }
  inline int getLevel() { return level; }
  inline int getDepth() { return depth; }  
  void reset();

  // these return true if the mission has been completed
  bool itemFound(Item *item);
  bool creatureSlain(Creature *creature);

  int getItemCount() { return (int)itemList.size(); }
  RpgItem *getItem( int index ) { return itemList[ index ]; }
  bool getItemHandled( int index ) { return items[ itemList[ index ] ]; }
  int getCreatureCount() { return (int)creatureList.size(); }
  Monster *getCreature( int index ) { return creatureList[ index ]; }
  bool getCreatureHandled( int index ) { return creatures[ creatureList[ index ] ]; }
 private:
  void checkMissionCompleted();

};


class MissionTemplate {
private:
  char name[80];
  char description[2000];
public:
  MissionTemplate( char *name, char *description );
  ~MissionTemplate();
  Mission *createMission( Session *session, int level, int depth );
private:
  void parseText( Session *session, int level, 
                  char *text, char *parsedText,
                  map<string, RpgItem*> *items, 
                  map<string, Monster*> *creatures );
};




class Board	{								
 private:
  Session *session;
  vector<MissionTemplate *> templates;
  vector<Mission*> storylineMissions;


  vector<Mission*> availableMissions;

  char **missionText;
  Color *missionColor;

public:

  static const int EVENT_HANDLED = 0;
  static const int EVENT_PLAY_MISSION = 1;

  Board(Session *session);
  virtual ~Board();
  
  void initMissions();
  void reset();
  
  inline int getMissionCount() { return availableMissions.size(); }
  inline Mission *getMission(int index) { return availableMissions[index]; }

 private:
  void freeListText();
  
};

#endif

