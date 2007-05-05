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

#include "common/constants.h"
#include "persist.h"
#include <map>
#include <vector>
#include <string>
#include <set>

class RpgItem;
class Monster;
class Creature;
class RenderedCreature;
class Item;
class Session;
class Board;
class GameAdapter;
class ConfigLang;

/**
  *@author Gabor Torok
  */
	
class NpcConversation {
  public:

    NpcConversation() {
    }

    ~NpcConversation() {
    }

    std::vector<std::string> npc_intros;
    std::vector<std::string> npc_unknownPhrases;
    std::map<std::string, int> npc_conversations;    
    std::vector<std::string> npc_answers;    
};

/**
 * Extra info associated with npc-s on an edited level.
 */
class NpcInfo {
public:
  int x, y, level, type;
  char *name;
  std::set<int> subtype;
  char *subtypeStr;

  NpcInfo( int x, int y, char *name, int level, char *type, char *subtype );
  ~NpcInfo();

  inline int isSubtype( int value ) { return( subtype.find( value ) != subtype.end() ); }
  inline std::set<int> *getSubtype() { return &subtype; }
};

class Mission {
private:
	int chapter;
  Board *board;
  int level;
  int depth;
  char mapName[80];
	char savedMapName[300];
  bool edited;
  char name[80];
  char displayName[255];
  char music[255];
  char description[2000];
  char success[2000];
  char failure[2000];
  std::map<RpgItem*, bool> items;
  std::vector<RpgItem*> itemList;
  std::map<Monster*, bool> creatures;
  std::vector<Monster*> creatureList;
  std::map<Item*,RpgItem*> itemInstanceMap;
  std::map<Creature*,Monster*> monsterInstanceMap;
  bool completed;
  bool storyLine;
  // the mission's location on the map-grid
  int mapX, mapY;
  char special[80];
	char templateName[80];
public:

#define INTRO_PHRASE "_INTRO_"
#define UNKNOWN_PHRASE "_UNKNOWN_"

  static std::vector<std::string> intros;
  static std::vector<std::string> unknownPhrases;
  static std::map<std::string, int> conversations;
  static std::vector<std::string> answers;
  static std::map<std::string,NpcConversation*> npcConversations;
  static std::map<std::string, NpcInfo*> npcInfos;

  static char *getIntro();
  static char *getAnswer( char *keyphrase );
  static char *getIntro( char *npc );
	static bool setIntro( Creature *creature, char *keyphrase );
  static char *getAnswer( char *npc, char *keyphrase );
  /**
   * Load extra data from text file alongside an edited map.
   * "fileName" in this case is the name of the .map binary file 
   * that was loaded.
   */
  static void loadMapData( GameAdapter *adapter, const char *fileName );

  /**
   * Append extra data (npc info, etc.) to a text file alongside an edited map.
   * "fileName" in this case is the name of the .map binary file 
   * that was loaded. A file will only be created if one doesn't already
   * exist.
   */
  static void saveMapData( GameAdapter *adapter, const char *fileName );

  Mission( Board *board, int level, int depth, 
           char *name, char *displayName, char *description, 
           char *music,
           char *success, char *failure,
           char *mapName, char mapType='C' );
  ~Mission();

	inline int getChapter() { return chapter; }
	inline void setChapter( int n ) { chapter = n; }
  inline int getMapX() { return mapX; }
  inline int getMapY() { return mapY; }
  inline void setMapXY( int x, int y ) { mapX = x; mapY = y; }
  inline bool isSpecial() { return ( strlen( special ) ? true : false ); }
  inline char *getSpecial() { return special; }
  inline void setSpecial( char *s ) { strncpy( special, s, 79 ); special[79]='\0'; }

  inline void setStoryLine( bool b ) { storyLine = b; }
  inline bool isStoryLine() { return storyLine; }

  inline void addCreature( Monster *monster, bool value=false ) {
    creatures[monster] = value;
    creatureList.push_back( monster );
  }

  inline void addItem( RpgItem *item, bool value=false ) {
    items[item] = value;
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

  inline bool isMissionCreature( RenderedCreature *creature ) {
    return ( monsterInstanceMap.find( (Creature*)creature ) != monsterInstanceMap.end() );
  }

  void deleteItemMonsterInstances();

  inline bool isCompleted() { return completed; }
  inline void setCompleted( bool b ) { completed = b; }
  inline char *getName() { return name; }
  inline char *getDisplayName() { return displayName; }
  inline char *getDescription() { return description; }
  inline char *getMusicTrack() { return (music && music[0]) ? music : NULL; }
  inline char *getSuccess() { return success; }
  inline char *getFailure() { return failure; }
  inline int getLevel() { return level; }
  inline int getDepth() { return depth; } 
  inline char *getMapName() { return mapName; } 
  inline bool isEdited() { return edited; }
  void reset();

  // these return true if the mission has been completed
  bool itemFound(Item *item);
  bool creatureSlain(Creature *creature);

  inline int getItemCount() { return (int)itemList.size(); }
  inline RpgItem *getItem( int index ) { return itemList[ index ]; }
  inline bool getItemHandled( int index ) { return items[ itemList[ index ] ]; }
  inline int getCreatureCount() { return (int)creatureList.size(); }
  inline Monster *getCreature( int index ) { return creatureList[ index ]; }
  inline bool getCreatureHandled( int index ) { return creatures[ creatureList[ index ] ]; }
  
	inline char *getTemplateName() { return templateName; }
	inline void setTemplateName( char *s ) { strcpy( templateName, s ); }

	inline void setSavedMapName( char *s ) { strcpy( savedMapName, s ); }
	inline char *getSavedMapName() { return savedMapName; }

	MissionInfo *save();
	static Mission *load( Session *session, MissionInfo *info );

private:
	//static void addWanderingHeroes( GameAdapter *adapter );
  static void loadMapDataFile( GameAdapter *adapter, const char *filename, bool generalOnly=false );
	static void getMapConfigFile( const char *filename, const char *out );
	static void initConversations( ConfigLang *config, GameAdapter *adapter, bool generalOnly );
	static void initNpcs( ConfigLang *config, GameAdapter *adapter );
	static void setGeneralConversationLine( std::string keyphrase, std::string answer );
	static void setConversationLine( std::string npc, std::string keyphrase, std::string answer );
	static void storeConversationLine( std::string keyphrase, 
																		 std::string answer,
																		 std::vector<std::string> *intros,
																		 std::vector<std::string> *unknownPhrases,
																		 std::map<std::string, int> *conversations,
																		 std::vector<std::string> *answers );
  static NpcInfo *getNpcInfo( int x, int y );
  static std::string getNpcInfoKey( int x, int y );
  
  void checkMissionCompleted();
  static int readConversationLine( FILE *fp, char *line, int n,
                                   std::vector<std::string> *intros,
                                   std::vector<std::string> *unknownPhrases,
                                   std::map<std::string, int> *conversations,
                                   std::vector<std::string> *answers );
  
};                                  


class MissionTemplate {
private:
  Board *board;
  char name[80];
  char displayName[255];
  char mapType;
  char description[2000];
  char music[255];
  char success[2000];
  char failure[2000];
public:
  MissionTemplate( Board *board, char *name, char *displayName, char mapType, char *description, char *music, char *success, char *failure );
  ~MissionTemplate();
  Mission *createMission( Session *session, int level, int depth, MissionInfo *info=NULL );
	inline char *getName() { return name; }
  inline char *getDisplayName() { return displayName; }
private:
  void parseText( Session *session, int level, int depth,
                  char *text, char *parsedText,
                  std::map<std::string, RpgItem*> *items, 
                  std::map<std::string, Monster*> *creatures,
									MissionInfo *info=NULL );
};




class Board	{								
 private:
  Session *session;
  std::vector<MissionTemplate *> templates;
  std::vector<Mission*> storylineMissions;
  int storylineIndex;

  std::vector<Mission*> availableMissions;

	int missionListCount;
  char **missionText;
  Color *missionColor;

public:

  static const int EVENT_HANDLED = 0;
  static const int EVENT_PLAY_MISSION = 1;

  Board(Session *session);
  virtual ~Board();

  inline Session *getSession() { return session; }

  void initMissions();
  void reset();

	inline char *getStorylineTitle() { return storylineMissions[storylineIndex]->getDisplayName(); }
  inline int getStorylineIndex() { return storylineIndex; }
  void setStorylineIndex( int n );
  void storylineMissionCompleted( Mission *mission );
  
  inline int getMissionCount() { return availableMissions.size(); }
  inline Mission *getMission(int index) { return availableMissions[index]; }
	inline void addMission( Mission *mission ) { availableMissions.push_back( mission ); }

	inline MissionTemplate *findTemplateByName( char *name ) {
		for( int i = 0; i < (int)templates.size(); i++ ) {
			if( !strcmp( templates[i]->getName(), name ) ) return templates[i];
		}
		std::cerr << "*** Error: can't find template: " << name << std::endl;
		return NULL;
	}

 private:
  void freeListText();
};

#endif

