
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
	std::map<std::string, std::string> npc_firstKeyPhrase;
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
	std::string savedMapName;
  bool edited;
  char name[80];
  char displayName[255];
  char music[255];
	char introDescription[2000];
  char description[2000];
  char success[2000];
  char failure[2000];
  char replayDisplayName[255];
  char replayDescription[2000];
  std::map<RpgItem*, bool> items;
  std::vector<RpgItem*> itemList;
  std::map<Monster*, bool> creatures;
  std::vector<Monster*> creatureList;
  std::map<Creature*,Monster*> monsterInstanceMap;
  bool completed;
  bool replayable;
  bool storyLine;
  // the mission's location on the map-grid
  int mapX, mapY;
  char special[80];
  char templateName[80];
  int missionId;
  int locationX, locationY;
	std::string ambientSoundName;
public:

#define INTRO_PHRASE "_INTRO_"
#define UNKNOWN_PHRASE "_UNKNOWN_"

  static std::vector<std::string> intros;
  static std::vector<std::string> unknownPhrases;
  static std::map<std::string, int> conversations;
  static std::map<std::string, std::string> firstKeyPhrase;
  static std::vector<std::string> answers;
  static std::map<std::string,NpcConversation*> npcConversations;
  static std::map<std::string, NpcInfo*> npcInfos;

  static char const* getIntro();
  static char const* getAnswer( char const* keyphrase );
  static char const* getFirstKeyPhrase( char const* keyphrase );
  static char const* getIntro( char const* npc );
  static bool setIntro( Creature *creature, char const* keyphrase );
  static char const* getAnswer( char const* npc, char const* keyphrase );
  static char const* getFirstKeyPhrase( char const* npc, char const* keyphrase );
  /**
   * Load extra data from text file alongside an edited map.
   * "fileName" in this case is the name of the .map binary file 
   * that was loaded.
   */
  static void loadMapData( GameAdapter *adapter, const std::string& fileName );
	static void loadMapConfig( GameAdapter *adapter, const std::string& filename );

  /**
   * Append extra data (npc info, etc.) to a text file alongside an edited map.
   * "fileName" in this case is the name of the .map binary file 
   * that was loaded. A file will only be created if one doesn't already
   * exist.
   */
  static void saveMapData( GameAdapter *adapter, const std::string& fileName );

  Mission( Board *board, int level, int depth, bool replayable,
           char *name, char *displayName, char *description, char *replayDisplayName, char *replayDescription, char *introDescription,
           char *music,
           char *success, char *failure,
           char *mapName, char mapType='C' );
  ~Mission();
	
  inline int getLocationX() { return locationX; }
  inline int getLocationY() { return locationY; }
  inline int getMissionId() { return missionId; }
  inline void setMissionId( int n ) { this->missionId = n; }
  inline char *getIntroDescription() { return introDescription; }
  inline int getChapter() { return chapter; }
  inline void setChapter( int n ) { chapter = n; }
  inline int getMapX() { return( locationX > -1 ? locationX : mapX ); }
  inline int getMapY() { return( locationY > -1 ? locationY : mapY ); }
  inline void setMapXY( int x, int y ) { mapX = x; mapY = y; }
  inline bool isSpecial() { return ( strlen( special ) ? true : false ); }
  inline char *getSpecial() { return special; }
  inline void setSpecial( char const* s ) { strncpy( special, s, 79 ); special[79]='\0'; }
	inline void setAmbientSoundName( std::string& s ) { this->ambientSoundName = s; }
	std::string& getAmbientSoundName();

  inline void setStoryLine( bool b ) { storyLine = b; }
  inline bool isStoryLine() { return storyLine; }
	
  inline void setLocation( int x, int y ) { this->locationX = x; this->locationY = y; }

  inline void addCreature( Monster *monster, bool value=false ) {
    creatures[monster] = value;
    creatureList.push_back( monster );
  }

  inline void addItem( RpgItem *item, bool value=false ) {
    items[item] = value;
    itemList.push_back( item );
  }

  inline void addCreatureInstanceMap( Creature *creature, Monster *monster ) {
    monsterInstanceMap[ creature ] = monster;
  }

  inline bool isMissionCreature( RenderedCreature *creature ) {
    return ( monsterInstanceMap.find( (Creature*)creature ) != monsterInstanceMap.end() );
  }

  void deleteMonsterInstances();
  void removeMissionItems();

  inline bool isCompleted() { return completed; }
  inline void setCompleted( bool b ) { completed = b; }
  inline char *getName() { return name; }
  inline char *getDisplayName() { return displayName; }
  inline char *getDescription() { return description; }
  inline void setDisplayName( char *s ) { strcpy( displayName, s ); }
  inline void setDescription( char *s ) { strcpy( description, s ); }
  inline char *getReplayDisplayName() { return replayDisplayName; }
  inline char *getReplayDescription() { return replayDescription; }
  inline char *getMusicTrack() { return (music && music[0]) ? music : NULL; }
  inline char *getSuccess() { return success; }
  inline char *getFailure() { return failure; }
  inline int getLevel() { return level; }
  inline int getDepth() { return depth; } 
  inline char *getMapName() { return mapName; } 
  inline bool isEdited() { return edited; }
  inline bool isReplayable() { return replayable; }
  inline bool isReplay() { return storyLine && completed && replayable; }
  void reset();

  // these return true if the mission has been completed
  bool itemFound(Item *item);
  bool creatureSlain(Creature *creature);

  inline int getItemCount() { return static_cast<int>(itemList.size()); }
  inline RpgItem *getItem( int index ) { return itemList[ index ]; }
  inline bool getItemHandled( int index ) { return items[ itemList[ index ] ]; }
  inline int getCreatureCount() { return static_cast<int>(creatureList.size()); }
  inline Monster *getCreature( int index ) { return creatureList[ index ]; }
  inline bool getCreatureHandled( int index ) { return creatures[ creatureList[ index ] ]; }
  
	inline char *getTemplateName() { return templateName; }
	inline void setTemplateName( char *s ) { strcpy( templateName, s ); }

	inline void setSavedMapName( const std::string s ) { savedMapName = s; }
	inline std::string getSavedMapName() { return savedMapName; }

	MissionInfo *save();
	static Mission *load( Session *session, MissionInfo *info );
	void loadStorylineMission( MissionInfo *info );

private:
	//static void addWanderingHeroes( GameAdapter *adapter );
	static void clearConversations();
  static void loadMapDataFile( GameAdapter *adapter, const std::string& filename, bool generalOnly=false );
	static std::string getMapConfigFile( const std::string& filename );
	static void initConversations( ConfigLang *config, GameAdapter *adapter, bool generalOnly );
	static void initNpcs( ConfigLang *config, GameAdapter *adapter );
	static void setGeneralConversationLine( std::string keyphrase, std::string answer );
	static void setConversationLine( std::string npc, std::string keyphrase, std::string answer );
	static void storeConversationLine( std::string keyphrase, 
									   std::string answer,
									   std::vector<std::string> *intros,
									   std::vector<std::string> *unknownPhrases,
									   std::map<std::string, int> *conversations,
									   std::map<std::string, std::string> *firstKeyPhrase,
									   std::vector<std::string> *answers );
  static NpcInfo *getNpcInfo( int x, int y );
  static std::string getNpcInfoKey( int x, int y );
  
  void checkMissionCompleted();
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
	std::string ambientSoundName;
public:
  MissionTemplate( Board *board, char *name, char *displayName, char mapType, char *description, char *music, char *success, char *failure, std::string& ambientSoundName );
  ~MissionTemplate();
  Mission *createMission( Session *session, int level, int depth, MissionInfo *info=NULL );
	inline char *getName() { return name; }
  inline char *getDisplayName() { return displayName; }
	inline std::string& getAmbientSoundName() { return ambientSoundName; }
	inline char getMapType() { return mapType; }
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
	std::string* missionText;
  Color *missionColor;

public:

	enum { 
		EVENT_HANDLED = 0,
		EVENT_PLAY_MISSION = 1,
		MSNTXT_SIZE = 120
	};

  Board(Session *session);
  virtual ~Board();

  inline Session *getSession() { return session; }

  void initMissions();
  void reset();

	void removeCompletedMissionsAndItems();

	inline Mission *getCurrentStorylineMission() { return storylineMissions[storylineIndex]; }
	inline char *getStorylineTitle() { return storylineMissions[storylineIndex]->getDisplayName(); }
  inline int getStorylineIndex() { return storylineIndex; }
  void setStorylineIndex( int n );
  void storylineMissionCompleted( Mission *mission );
  
  inline int getMissionCount() { return availableMissions.size(); }
  inline Mission *getMission(int index) { return availableMissions[index]; }
	inline void addMission( Mission *mission ) { availableMissions.push_back( mission ); }

	inline MissionTemplate *findTemplateByName( char *name ) {
		for( int i = 0; i < static_cast<int>(templates.size()); i++ ) {
			if( !strcmp( templates[i]->getName(), name ) ) return templates[i];
		}
		std::cerr << "*** Error: can't find template: " << name << std::endl;
		return NULL;
	}

 private:
  void freeListText();
};

#endif

