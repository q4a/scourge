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
#include "session.h"
#include "rpg/rpgitem.h"
#include "rpg/monster.h"
#include <map>

using namespace std;

/**
  *@author Gabor Torok
  */
	
#define BOARD_GUI_WIDTH 400
#define BOARD_GUI_HEIGHT 400

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

class Board	{								
 private:
  Session *session;
  vector<Mission*> availableMissions;
  map<int, vector<Mission*>* > missions;

  static char objectiveName[OBJECTIVE_COUNT][80];

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

