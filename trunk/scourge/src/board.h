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

class Item;
class Creature;
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
  map<Item*, bool> items;
  vector<Item*> itemList;
  map<Creature*, bool> creatures;
  vector<Creature*> creatureList;
  bool completed;
public:
  Mission( int level, int depth, char *name, char *description, char *success, char *failure );
  ~Mission();

  inline void addCreature( Creature *creature ) {
    creatures[creature] = false;
    creatureList.push_back( creature );
  }

  inline void addItem( Item *item ) {
    items[item] = false;
    itemList.push_back( item );
  }

  inline bool isCompleted() { return completed; }
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
  Item *getItem( int index ) { return itemList[ index ]; }
  bool getItemHandled( int index ) { return items[ itemList[ index ] ]; }
  int getCreatureCount() { return (int)creatureList.size(); }
  Creature *getCreature( int index ) { return creatureList[ index ]; }
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
                  map<string, Item*> *items, 
                  map<string, Creature*> *creatures );
};









/*
// mission objectives
enum {
  FIND_OBJECT = 0,
  KILL_MONSTER,
  
  // must be last
  OBJECTIVE_COUNT
};

#define MAX_OBJECTIVE_PARAM_COUNT 10
typedef struct _MissionObjective {
  int index;
  char name[80];
  int paramCount;
  char param[MAX_OBJECTIVE_PARAM_COUNT][80];
  int itemCount;
  RpgItem *item[MAX_OBJECTIVE_PARAM_COUNT];
  bool itemHandled[MAX_OBJECTIVE_PARAM_COUNT];
  int monsterCount;  
  Monster *monster[MAX_OBJECTIVE_PARAM_COUNT];
  bool monsterHandled[MAX_OBJECTIVE_PARAM_COUNT];
} MissionObjective;

class Mission {
 private:
  char name[80]; // name of mission
  int level; // what level dungeon
  int dungeonStoryCount; // how many stories
  bool completed;
  char story[2000]; // the background story of the level
  MissionObjective *objective;

 public:
  Mission(char *name, int level, int dungeonStoryCount);
  virtual ~Mission();

  void reset();

  inline bool isCompleted() { return completed; }
  inline char *getName() { return name; }
  inline char *getStory() { return story; }
  inline int getLevel() { return level; }
  inline int getDungeonStoryCount() { return dungeonStoryCount; }
  inline MissionObjective *getObjective() { return objective; }
  inline void addToStory(char *s) { if(strlen(story)) strcat(story, " "); strcat(story, s); }  
  void setObjective(MissionObjective *o) { objective = o; }

  // these return true if the mission has been completed
  bool itemFound(RpgItem *item);
  bool monsterSlain(Monster *monster);
 private:
  void checkMissionCompleted();
};
*/










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

