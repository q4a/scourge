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
#include <string>

using namespace std;

class RpgItem;
class Monster;
class Creature;
class Item;
class Session;
class Board;

/**
  *@author Gabor Torok
  */
	
#define BOARD_GUI_WIDTH 400
#define BOARD_GUI_HEIGHT 400

class NpcConversation {
  public:

    NpcConversation() {
    }

    ~NpcConversation() {
    }

    vector<string> npc_intros;
    vector<string> npc_unknownPhrases;
    map<string, string> npc_conversations;    
};

class Mission {
private:
  Board *board;
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

#define INTRO_PHRASE "_INTRO_"
#define UNKNOWN_PHRASE "_UNKNOWN_"

  static vector<string> intros;
  static vector<string> unknownPhrases;
  static map<string, string> conversations;
  static map<Monster*,NpcConversation*> npcConversations;

  static char *getIntro();
  static char *getAnswer( char *keyphrase );
  static char *getIntro( Monster *npc );
  static char *getAnswer( Monster *npc, char *keyphrase );


  Mission( Board *board, int level, int depth, 
		   char *name, char *description, 
		   char *success, char *failure );
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

  inline bool isMissionItem( Item *item ) {
    return ( itemInstanceMap.find( item ) != itemInstanceMap.end() );
  }

  inline bool isMissionCreature( Creature *creature ) {
    return ( monsterInstanceMap.find( creature ) != monsterInstanceMap.end() );
  }

  void deleteItemMonsterInstances();

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
  Board *board;
  char name[80];
  char description[2000];
public:
  MissionTemplate( Board *board, char *name, char *description );
  ~MissionTemplate();
  Mission *createMission( Session *session, int level, int depth );
private:
  void parseText( Session *session, int level, int depth,
                  char *text, char *parsedText,
                  map<string, RpgItem*> *items, 
                  map<string, Monster*> *creatures );
};




class Board	{								
 private:
  Session *session;
  vector<MissionTemplate *> templates;
  vector<Mission*> storylineMissions;
  int storylineIndex;

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

  inline int getStorylineIndex() { return storylineIndex; }
  void setStorylineIndex( int n );
  void storylineMissionCompleted( Mission *mission );
  
  inline int getMissionCount() { return availableMissions.size(); }
  inline Mission *getMission(int index) { return availableMissions[index]; }

 private:
  void freeListText();
  int readConversationLine( FILE *fp, char *line,
                            char *keyphrase, char *answer,
                            int n );  
};

#endif

