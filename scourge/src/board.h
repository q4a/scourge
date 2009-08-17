
/***************************************************************************
                 board.h  -  Mission/Mission board classes
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
#pragma once

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
struct NpcInfoInfo;
class Mission;
class Board;

/**
  *@author Gabor Torok
  */

class MapPlace {
public:
	enum {
		TYPE_DUNGEON = 0,
		TYPE_CAVE
	};
	
	enum {
		OBJECTIVE_ITEM = 0,
		OBJECTIVE_CREATURE,
		OBJECTIVE_NONE
	};	
	
	char name[80], display_name[80], map_name[300], short_name[10];
	int rx, ry, x, y;
	int type;
	int objective;
	int objective_count;
	int level, depth;
	char ambient[3000];
	char music[300];
	char footsteps[300];
	char description[2000];
	
	// computed
	Mission *mission;
	
	MapPlace();
	~MapPlace();
	Mission *findOrCreateMission( Board *board, MissionInfo *info = NULL );
};

class MapCity {
public:
	char name[80];
	int rx, ry, x, y, w, h; // w,h are in city blocks (a city block=4x16 map units)
	int level;
	
	MapCity() {
	}
	~MapCity() {
	}
};

class RoadWalker;

class Road {
public:
	char name[80], display_name[80];
	int start_rx, start_ry, start_x, start_y;
	int end_rx, end_ry, end_x, end_y;
	bool straight;
	
	Road() {
	}
	~Road() {
	}
	
	void walk( RoadWalker *walker );
};

class RoadWalker {
public:	
	RoadWalker() {}
	virtual ~RoadWalker() {}
	virtual void walk( Road *road, int rx, int ry, int x, int y, bool walksX ) = 0;
};

class CreatureGenerator {
public:
	char monster[255];
	int rx, ry, x, y;
	int count;
	
	// transient
	std::vector<Creature*> creatures;
	
	CreatureGenerator() {
	}
	
	~CreatureGenerator() {
	}
	
	void removeDead();
	void generate( Session *session );
	
};

/// Extra info associated with npc-s on an edited level.

class NpcInfo {
public:
	int x, y, level, type;
	char name[255];
	std::set<int> subtype;
	char subtypeStr[255];

	NpcInfo( int x, int y, char *name, int level, char *type, char *subtype );
	~NpcInfo();

	NpcInfoInfo* save();
	static NpcInfo* load( NpcInfoInfo *info );

	inline int isSubtype( int value ) {
		return( subtype.find( value ) != subtype.end() );
	}
	inline std::set<int> *getSubtype() {
		return &subtype;
	}
};

/// An individual mission, consisting of several maps, objectives etc.
class Mission {
private:
	int chapter;
	Board *board;
	int level;
	int depth;
	char mapName[80];
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
	bool completed;
	bool replayable;
	bool storyLine;
	// the mission's location on the map-grid
	int mapX, mapY;
	char special[80];
	int missionId;
	int locationX, locationY;
	std::string ambientSoundName;
	int regionX, regionY, offsetX, offsetY;
	static std::map<std::string, NpcInfo*> npcInfos;

public:


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
	         char *mapName, char mapType = 'C' );
	~Mission();
	
	inline void setMapPos( int *mapPos ) { regionX = mapPos[0]; regionY = mapPos[1]; offsetX = mapPos[2]; offsetY = mapPos[3]; }
	inline int getMapRegionX() { return regionX; }
	inline int getMapRegionY() { return regionY; }
	inline int getMapOffsetX() { return offsetX; }
	inline int getMapOffsetY() { return offsetY; }
	inline bool compareMapPos( int *mapPos ) { 
		return regionX == mapPos[0] && regionY == mapPos[1] && offsetX == mapPos[2] && offsetY == mapPos[3];
	}

	inline int getLocationX() {
		return locationX;
	}
	inline int getLocationY() {
		return locationY;
	}
	inline int getMissionId() {
		return missionId;
	}
	inline void setMissionId( int n ) {
		this->missionId = n;
	}
	inline char *getIntroDescription() {
		return introDescription;
	}
	inline int getChapter() {
		return chapter;
	}
	inline void setChapter( int n ) {
		chapter = n;
	}
	inline int getMapX() {
		return( locationX > -1 ? locationX : mapX );
	}
	inline int getMapY() {
		return( locationY > -1 ? locationY : mapY );
	}
	inline void setMapXY( int x, int y ) {
		mapX = x; mapY = y;
	}
	inline bool isSpecial() {
		return ( strlen( special ) ? true : false );
	}
	inline char *getSpecial() {
		return special;
	}
	inline void setSpecial( char const* s ) {
		strncpy( special, s, 79 ); special[79] = '\0';
	}
	inline void setAmbientSoundName( std::string& s ) {
		this->ambientSoundName = s;
	}
	std::string& getAmbientSoundName();

	inline void setStoryLine( bool b ) {
		storyLine = b;
	}
	inline bool isStoryLine() {
		return storyLine;
	}

	inline void setLocation( int x, int y ) {
		this->locationX = x; this->locationY = y;
	}

	inline void addCreature( Monster *monster, bool value = false ) {
		creatures[monster] = value;
		creatureList.push_back( monster );
	}

	inline void addItem( RpgItem *item, bool value = false ) {
		items[item] = value;
		itemList.push_back( item );
	}

	void removeMissionItems();

	inline bool isCompleted() {
		return completed;
	}
	
	void setCompleted( bool b );

	inline char *getName() {
		return name;
	}
	inline char *getDisplayName() {
		return displayName;
	}
	inline char *getDescription() {
		return description;
	}
	inline void setDisplayName( char *s ) {
		strcpy( displayName, s );
	}
	inline void setDescription( char *s ) {
		strcpy( description, s );
	}
	inline char *getReplayDisplayName() {
		return replayDisplayName;
	}
	inline char *getReplayDescription() {
		return replayDescription;
	}
	inline char *getMusicTrack() {
		return ( music && music[0] ) ? music : NULL;
	}
	inline char *getSuccess() {
		return success;
	}
	inline char *getFailure() {
		return failure;
	}
	inline int getLevel() {
		return level;
	}
	inline int getDepth() {
		return depth;
	}
	inline char *getMapName() {
		return mapName;
	}
	inline bool isEdited() {
		return edited;
	}
	inline bool isReplayable() {
		return replayable;
	}
	inline bool isReplay() {
		return storyLine && ( chapter == -1 ) && replayable;
	}
	void reset();

	// these return true if the mission has been completed
	bool itemFound( Item *item );
	bool creatureSlain( Creature *creature );

	inline int getItemCount() {
		return static_cast<int>( itemList.size() );
	}
	inline RpgItem *getItem( int index ) {
		return itemList[ index ];
	}
	inline bool getItemHandled( int index ) {
		return items[ itemList[ index ] ];
	}
	inline int getCreatureCount() {
		return static_cast<int>( creatureList.size() );
	}
	inline Monster *getCreature( int index ) {
		return creatureList[ index ];
	}
	inline bool getCreatureHandled( int index ) {
		return creatures[ creatureList[ index ] ];
	}

	MissionInfo *save();
	static Mission *load( Session *session, MissionInfo *info );
	void loadStorylineMission( MissionInfo *info );

	static NpcInfo *addNpcInfo( int x, int y, char *npcName, int level, char *npcType, char *npcSubType );
	static NpcInfo *addNpcInfo( NpcInfo *info );
	static void createTypedNpc( Creature *creature, int level, int fx, int fy );

private:
	//static void addWanderingHeroes( GameAdapter *adapter );
	static void loadMapDataFile( GameAdapter *adapter, const std::string& filename, bool generalOnly = false );
	static std::string getMapConfigFile( const std::string& filename );
	static void initNpcs( ConfigLang *config, GameAdapter *adapter );
	static NpcInfo *getNpcInfo( int x, int y );
	static std::string getNpcInfoKey( int x, int y );

	void checkMissionCompleted();
};


/// Manages the list of missions, and the advancement of the story line.
class Board : public RoadWalker {
private:
	Session *session;
//	std::vector<MissionTemplate *> templates;
	std::vector<Mission*> storylineMissions;
	int storylineIndex;

	std::map<std::string, std::vector<CreatureGenerator*>*> generators;
	std::map<std::string, std::vector<MapCity*>*> cities;
	std::map<std::string, std::vector<MapPlace*>*> places;
	std::map<std::string, MapPlace*> placesByShortName;
	std::map<std::string, std::set<Road*>*> roads;
	std::set<Road*> allRoads;

public:
	void walk( Road *road, int rx, int ry, int x, int y, bool walksX );
	
	enum {
		EVENT_HANDLED = 0,
		EVENT_PLAY_MISSION = 1,
		MSNTXT_SIZE = 120
	};

	Board( Session *session );
	virtual ~Board();
	
	inline std::vector<MapPlace*> *getPlacesForRegion( int rx, int ry ) {
		char tmp[80];
		sprintf( tmp, "%d,%d", rx, ry );
		std::string key = tmp;
		if( places.find( key ) == places.end() ) {
			return NULL;
		} else {
			return places[key];
		}
	}
	
	inline std::vector<MapCity*> *getCitiesForRegion( int rx, int ry ) {
		char tmp[80];
		sprintf( tmp, "%d,%d", rx, ry );
		std::string key = tmp;
		if( cities.find( key ) == cities.end() ) {
			return NULL;
		} else {
			return cities[key];
		}
	}
	
	inline std::set<Road*> *getRoadsForRegion( int rx, int ry ) {
		char tmp[80];
		sprintf( tmp, "%d,%d", rx, ry );
		std::string key = tmp;
		if( roads.find( key ) == roads.end() ) {
			return NULL;
		} else {
			return roads[key];
		}
	}
	
	std::vector<CreatureGenerator*> *getGeneratorsForRegion( int rx, int ry );

	inline Session *getSession() {
		return session;
	}

	void initMissions();
	void initLocations();
	void reset();

	inline Mission *getCurrentStorylineMission() {
		return storylineMissions[storylineIndex];
	}
	inline char *getStorylineTitle() {
		return storylineMissions[storylineIndex]->getDisplayName();
	}
	inline int getStorylineIndex() {
		return storylineIndex;
	}
	void setStorylineIndex( int n );
	void storylineMissionCompleted( Mission *mission );

	inline MapPlace *getMapPlaceByShortName( char *short_name ) {
		std::string s = short_name;
		if( placesByShortName.find( s ) == placesByShortName.end() ) {
			return NULL;
		} else {
			return placesByShortName[ s ];
		}
	}
};

#endif

