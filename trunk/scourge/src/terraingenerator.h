/***************************************************************************
                          terraingenerator.h  -  description
                             -------------------
    begin                : Thu Jan 15 2006
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
#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include "common/constants.h"

// forward decl.
class Map;
class Creature;
class Mission;
class Progress;
class Item;
class Scourge;
class Shape;
class RpgItem;
class Monster;
class Spell;
class GLShape;
class ShapePalette;
class DisplayInfo;
class Location;
class MapRenderHelper;

#define LOCKED_DOOR_RAND 8

// raise the magic item level by 1 for every 3 levels (magic item level [0,1,2,3])
#define MAGIC_ITEM_MUL 3

class TerrainGenerator {
protected:

  // directions
  const static int DIR_N = 0;
  const static int DIR_E = 1;
  const static int DIR_S = 2;
  const static int DIR_W = 3;
  const static int DIR_COUNT = 4;

  const static bool debug = false;

  const static Sint16 unitOffset = MAP_UNIT_OFFSET;
  const static Sint16 unitSide = MAP_UNIT;
  const static Sint16 wallHeight = MAP_WALL_HEIGHT;   

  const static Sint16 offset = MAP_OFFSET;


  Sint16 *ff;
  int ffCount;

  struct Room {
    int x, y, w, h;
    int valueBonus;
  };
  Room room[200];


  Scourge *scourge;
  int level;
  int depth;
  int maxDepth;
  bool stairsDown, stairsUp;
  Mission *mission;
  Progress *progress;
  Uint32 start;
  int stairsUpX, stairsUpY, stairsDownX, stairsDownY;

  const static int MAX_DOOR_COUNT = 500;
  int doorCount;
  int door[MAX_DOOR_COUNT][2];
  
  std::vector<Item*> containers;
  std::vector<int> containerX;
  std::vector<int> containerY;
  std::vector<int> teleporterX;
  std::vector<int> teleporterY;

  int roomCount;
  int roomMaxWidth;
  int roomMaxHeight;
  int objectCount;
  int monsters;

public:
	static TerrainGenerator *getGenerator( Scourge *scourge, int depth );

  TerrainGenerator( Scourge *scourge, 
                    int level, 
                    int depth, 
										int maxDepth,
                    bool stairsDown, 
                    bool stairsUp, 
                    Mission *mission, 
                    int progressSteps );
  virtual ~TerrainGenerator();

  bool toMap( Map *map, ShapePalette *shapePal, bool goingUp, bool goingDown );

	// for debugging
  virtual void printMaze()=0;
	virtual void getName(char *s)=0;

  

protected:

  void updateStatus(const char *statusMessage);

  bool drawNodesOnMap(Map *map, ShapePalette *shapePal, bool goingUp, bool goingDown);

  // generate the map in memory
  virtual void generate( Map *map, ShapePalette *shapePal ) = 0;

  // transfer the in-memory map to the displayed map
  virtual bool drawNodes( Map *map, ShapePalette *shapePal ) = 0;

  virtual MapRenderHelper *getMapRenderHelper() = 0;

  
	virtual void addShapes(Map *map, ShapePalette *shapePal);
  virtual void addContainers(Map *map, ShapePalette *shapePal);
  virtual bool addStairs(Map *map, ShapePalette *shapePal);
  virtual void addItems(Map *map, ShapePalette *shapePal);
  virtual void addMissionObjectives(Map *map, ShapePalette *shapePal);
  virtual void addMonsters(Map *map, ShapePalette *shapePal);
	virtual void addHarmlessCreatures(Map *map, ShapePalette *shapePal);
  virtual void addFurniture(Map *map, ShapePalette *shapePal);
	virtual void addMagicPools( Map *map, ShapePalette *shapePal );
  virtual bool addTeleporters(Map *map, ShapePalette *shapePal);
  virtual bool addParty(Map *map, ShapePalette *shapePal, bool goingUp, bool goingDown );
  virtual void lockDoors(Map *map, ShapePalette *shapePal);
  virtual void lockLocation(Map *map, int mapx, int mapy);
  virtual void createFreeSpaceMap(Map *map, ShapePalette *shapePal);
  virtual void deleteFreeSpaceMap(Map *map, ShapePalette *shapePal);
  virtual void calculateRoomValues(Map *map, ShapePalette *shapePal);
	virtual void addRugs( Map *map, ShapePalette *shapePal );
  virtual void addTraps( Map *map, ShapePalette *shapePal );

  // ===========================================
  // Utilities
  /**
   * Find a random location on the map. This method doesn't remove the found location from the pool of possible locations.
   */
  void getRandomLocationSimple( Map *map, Shape *shape, int *xpos, int *ypos );

  /**
   * Find a random location on the map. This method removes the found location from the pool of possible locations.
   */
  void getRandomLocation(Map *map, Shape *shape, int *x, int *y, bool accessible=false, int fromX=0, int fromY=0);

  bool getLocationInRoom(Map *map, int roomIndex, Shape *shape, 
                         int *xpos, int *ypos, bool startMiddle=false);
  
  static const int MAX_STEPS = 10000;
  bool isAccessible(Map *map, int x, int y, int fromX, int fromY, int stepsTaken=0, int dir=DIR_N);
  
  void addItem(Map *map, Creature *creature, Item *item, Shape *shape, int x, int y, int z = 0, DisplayInfo *di=NULL);
  
  
  void addItemsInEveryRoom(RpgItem *rpgItem, int n);
  void addItemsInRoom(RpgItem *rpgItem, int n, int room );
  
  bool addShapeInARoom( Shape *shape );
  Location *addShapeInRoom( Shape *shape, int room, DisplayInfo *di=NULL );
  
  void getRandomDeadEndLocation(int *x, int *y, GLShape *shape, Map *map);
  
  int getRoomIndex(int x, int y);

  Item *createRandomItem();

	/**
	 * Return a multiplier for figuring out the total monster level per room.
	 * For example, in order to make caves easier, it returns 0.5f.
	 * Harder rooms increase the chance of finding better treasure.
	 */
	virtual inline float getMonsterLevelMod() { return 1.0f; }

	/**
	 * Return a bool indicating whether or not extra-tough monsters should be
	 * used. Harder rooms increase the chance of finding better treasure.
	 */
	virtual inline bool getUseBadassMonsters() { return true; }

	/**
	 * Get the base level of the monsters used on this map. This value will
	 * be adjusted by depth, and some other random settings.
	 */
	virtual inline int getBaseMonsterLevel() { return level; }
	
	virtual void addContainersInRooms( Map *map, ShapePalette *shapePal );
  
};

#endif

