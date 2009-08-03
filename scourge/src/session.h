/***************************************************************************
                     session.h  -  Game session manager
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
#pragma once

#include <vector>
#include <set>
#include "preferences.h"
#include "party.h"
#include "net/server.h"
#include "net/client.h"
#include "net/gamestatehandler.h"
#include "net/commands.h"
#include "gameadapter.h"
#include "terraingenerator.h"
#include "render/texture.h"
#include "render/weather.h"

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
class Characters;
class Character;
class Monster;
class GLShape;
class SqBinding;
class ShapePalette;
class Sound;
class Cutscene;
class TerrainGenerator;
class Weather;
class CreatureGenerator;

/**
 *@author Gabor Torok
 */

/// Manages a game session (basically the whole "real" ingame part).
class Session {
private:
	Sound *sound;
	ShapePalette *shapePal;
	GameAdapter *adapter;
	Party *party;
	Map *map;
	Board *board;
	Cutscene *cutscene;
	Weather *weather;
#ifdef HAVE_SDL_NET
	Server *server;
	Client *client;
#endif
	bool multiplayerGame;
	std::vector<Mission*> missions;
	int currentMission;
	TextureData chapterImage;
	Texture chapterImageTexture;
	int chapterImageWidth, chapterImageHeight;
	bool showChapterIntro;
	std::vector<Item*> newItems;
	std::vector<Creature*> creatures;
	std::set<Creature*> nonVisibleCreatures;
	std::set<Uint16> visitedRegions;
	std::vector<CreatureGenerator*> generators;

	SqBinding *squirrel;
	std::map<RpgItem*, Item*> special;
	std::string savegame;
	std::string loadgame;
	std::string savetitle;
	std::string loadtitle;
	char scoreid[40];
	bool autosave;

	char interruptFunction[255];
	Characters* characters;
	bool exiting;

	// private constructor: call startGame instead.
	Session( GameAdapter *adapter );

	int dataInitialized;
	TerrainGenerator *terrainGenerator;

public:

	enum {
		NOT_INITIALIZED = 0,
		INIT_STARTED,
		INIT_DONE
	};
	static Session *instance;

	/**
	  * The main method for a project to run a game.
	  * Pass in a GameAdapter implementation (eg.: Scourge class)
	  * @return the value when the game exits. You can return this
	  * from your main() function back to the os.
	  */
	static int runGame( GameAdapter *adapter, int argc, char *argv[] );


	virtual ~Session();

	void initialize();
	
	inline void addGenerator( CreatureGenerator *generator ) { generators.push_back( generator ); }
	inline void clearGenerators() { generators.clear(); }
	void runGenerators();
	int getGeneratorCount( int region_x, int region_y );
	GeneratorInfo *getGeneratorInfo( int region_x, int region_y, int index );
	void loadGenerator( GeneratorInfo *info );
	void assignCreaturesToGenerators();
	
	void registerWithSquirrel( Creature *creature );
	
	inline void setExiting( bool b ) { exiting = b; }
	inline bool isExiting() { return exiting; }

	virtual void start();
	virtual void quit( int value );
#ifdef HAVE_SDL_NET
	virtual void runClient( char const* host, int port, char const* userName );
	virtual void runServer( int port );
	virtual inline Server *getServer() {
		return server;
	}
	virtual inline Client *getClient() {
		return client;
	}
	virtual void startServer( GameStateHandler *gsh, int port );
	virtual void startClient( GameStateHandler *gsh, CommandInterpreter *ci, char const* host, int port, char const* username );
	virtual void stopClientServer();
#endif

	inline bool isMultiPlayerGame() {
		return multiplayerGame;
	}
	inline void setMultiPlayerGame( bool b ) {
		multiplayerGame = b;
	}
	inline GameAdapter *getGameAdapter() {
		return adapter;
	}
	void playSound( const std::string& sound, int panning );
	std::string& getAmbientSoundName();

	inline void setInterruptFunction( char *s ) {
		strcpy( this->interruptFunction, s );
	}
	inline char *getInterruptFunction() {
		return this->interruptFunction;
	}

	/**
	  Creat a new item for use on this story. Calling this method instead of new Item()
	  directly ensures that the item will be cleaned up properly when the story is
	  exited. Only items in a party member's backpack are not deleted.

	  @param rpgItem if not NULL, the RpgItem template for the item to create.
	  @param spell if not NULL, the spell to associate with the created scroll.
	  @return the item created.
	*/
	virtual Item *newItem( RpgItem *rpgItem, int level = 1, Spell *spell = NULL, bool loading = false );

	virtual Item *addItemFromScript( char *name, int x, int y, int z, bool isContainer = false, int level = 1, int depth = 1 );

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
	virtual Creature *newCreature( Monster *monster, GLShape *shape, bool loaded = false );
	virtual Creature *newCreature( Character *character, char const* name, int sex, int model );
	virtual Creature *replaceCreature( Creature *creature, char *newCreatureType );
	virtual Creature *addCreatureFromScript( char *creatureType, int cx, int cy, int *fx = NULL, int *fy = NULL, int r = 0 );
	virtual Creature *addWanderingHeroFromScript( int level, int cx, int cy, int cz );
	virtual bool removeCreatureRef( Creature *creature, int index );
	virtual bool removeCreature( Creature *creature );
	virtual void addCreatureRef( Creature *creature, int index );
	void setVisible( Creature *creature, bool b );
	bool isVisible( Creature *creature );

	inline int getCreatureCount() {
		return creatures.size();
	}
	inline Creature *getCreature( int index ) {
		return creatures[index];
	}
	Creature *getCreatureByName( char const* name );
	inline int getItemCount() {
		return newItems.size();
	}
	inline Item *getItem( int index ) {
		return newItems[index];
	}
	virtual void deleteCreaturesAndItems( bool missionItemsOnly = false );

	inline Sound *getSound() {
		return sound;
	}
	inline ShapePalette *getShapePalette() {
		return shapePal;
	}
	inline Map *getMap() {
		return map;
	}
	inline Board *getBoard() {
		return board;
	}
	inline Party *getParty() {
		return party;
	}
	inline Preferences *getPreferences() {
		return getGameAdapter()->getPreferences();
	}
	
	inline Cutscene *getCutscene() {
		return cutscene;
	}
	inline Weather *getWeather() {
		return weather;
	}
	
	void setCurrentMission( Mission *mission );
	void debugMissions( char *message );
	inline Mission *getCurrentMission() {
		return currentMission < 0 || currentMission >= (int)missions.size() ? NULL : missions[ currentMission ];
	}
	inline int getMissionCount() { return missions.size(); }
	inline Mission *getMission( int index ) { return missions[ index ]; }
	
	void reset();
	
	
	void setChapterImage( char *image );
	inline TextureData const& getChapterImage() {
		return chapterImage;
	}
	inline Texture getChapterImageTexture() {
		return chapterImageTexture;
	}
	inline int getChapterImageWidth() {
		return chapterImageWidth;
	}
	inline int getChapterImageHeight() {
		return chapterImageHeight;
	}
	inline void setShowChapterIntro( bool b ) {
		this->showChapterIntro = b;
	}
	inline bool isShowingChapterIntro() {
		return showChapterIntro;
	}

	virtual Creature *getClosestMonster( int x, int y, int w, int h, int radius );
	virtual Creature *getClosestGoodGuy( int x, int y, int w, int h, int radius );
	virtual Creature *getRandomNearbyMonster( int x, int y, int w, int h, int radius );
	virtual Creature *getRandomNearbyGoodGuy( int x, int y, int w, int h, int radius );
	virtual void creatureDeath( Creature *creature );

	inline SqBinding *getSquirrel() {
		return squirrel;
	}

	inline void setSpecialItem( RpgItem *rpgItem, Item *item ) {
		special[ rpgItem ] = item;
	}
	inline Item *getSpecialItem( RpgItem *rpgItem ) {
		if ( special.find( rpgItem ) == special.end() ) return NULL;
		else return special[ rpgItem ];
	}

	/**
	 * How many times did this 'key' occur in the last hour?
	 * if withinLastHour is false, a day is used.
	 */
	int getCountForDate( char *key, bool withinLastHour = true );

	/**
	 * Store a key with data+value
	 */
	void setCountForDate( char *key, int value );

	void setSavegameName( std::string& s );
	inline std::string& getSavegameName() {
		return savegame;
	}
	inline void setLoadgameName( const std::string& s ) {
		loadgame = s;
	}
	inline std::string& getLoadgameName() {
		return loadgame;
	}
	inline void setLoadAutosave( bool b ) {
		autosave = b;
	}
	inline bool getLoadAutosave() {
		return autosave;
	}
	void setSavegameTitle( std::string& s );
	inline std::string& getSavegameTitle() {
		return savetitle;
	}
	inline void setLoadgameTitle( const std::string& s ) {
		loadtitle = s;
	}
	inline std::string& getLoadgameTitle() {
		return loadtitle;
	}
	inline bool willLoadGame() {
		return( loadgame.length()  ? true : false );
	}

	inline char *getScoreid() {
		return scoreid;
	}

	virtual void initData();
	virtual void doInitData();
	inline bool isDataInitialized() {
		return dataInitialized == INIT_DONE;
	}

	inline void setTerrainGenerator( TerrainGenerator *tg ) {
		this->terrainGenerator = tg;
	}
	inline TerrainGenerator *getTerrainGenerator() {
		return this->terrainGenerator;
	}
	
	void addVisitedRegion( int regionX, int regionY );
	bool isRegionVisited( int regionX, int regionY );
	inline std::set<Uint16> *getVisitedRegions() {
		return &visitedRegions;
	}
	inline void addVisitedRegionByIndex( Uint16 index ) {
		visitedRegions.insert( index );
	}
	
private:
	void clearMissions();
};

#endif

