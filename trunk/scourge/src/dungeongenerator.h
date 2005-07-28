/***************************************************************************
                          dungeongenerator.h  -  description
                             -------------------
    begin                : Thu May 15 2003
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

#ifndef DUNGEONGENERATOR_H
#define DUNGEONGENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include "constants.h"
#include "shapepalette.h"
#include "board.h"
#include "gui/progress.h"

using namespace std;

// forward decl.
class Map;
class Creature;
class Mission;
class Progress;
class Item;
class Scourge;

#define DRAW_UNVISITED 0

// a pre-rendered map location
typedef struct _ShapePosition {
  char name[80];
  int x, y, z;
} ShapePosition;

typedef struct _MapLocation {
  int x, y, w, h;
  int start[4][2]; // party starting coordinates
  bool monsters;
  int roomCount;
  Uint16 roomDimension[20][5];
  char *map[MAP_WIDTH];
  int shapeCount;            // how many shapes
  ShapePosition shapePosition[100]; // shapes: shape_index, x, y, z (x,y,z in map coordinates)
} MapLocation;

/**
  *@author Gabor Torok
  */

class DungeonGenerator {
private:

  static const char MESSAGE[];

  typedef struct _Room {
    int x, y, w, h;
    int valueBonus;
  } Room;
  Room room[20];

  enum { dgWIDTH = 0, dgHEIGHT, dgCURVYNESS, dgSPARSENESS,
         dgLOOPYNESS, dgROOMCOUNT, dgROOMMAXWIDTH, dgROOMMAXHEIGHT,
         dgOBJECTCOUNT };
  static const int levels[][9];

  int level;
  int depth;
  int width;
  int height;
  int curvyness; // the lower this number the more random the maze
  int sparseness; // the higher the more sparse (more empty space)
  Uint16 **nodes;
  int notVisitedCount, visitedCount;
  int *notVisited, *visited;
  int loopyness; // 0-100 % value of whether or not to make a dead-end into a loop
  int roomCount;
  int roomMaxWidth;
  int roomMaxHeight;
  int objectCount;
  int monsters;
  bool stairsDown, stairsUp;
  Mission *mission;
  Progress *progress;

  Sint16 *ff;
  int ffCount;

  // directions
  const static int DIR_N = 0;
  const static int DIR_E = 1;
  const static int DIR_S = 2;
  const static int DIR_W = 3;
  const static int DIR_COUNT = 4;

  // coridors
  const static Uint16 UNVISITED = 0x0000;
  const static Uint16 N_PASS = 0x0001;
  const static Uint16 S_PASS = 0x0002;
  const static Uint16 W_PASS = 0x0004;
  const static Uint16 E_PASS = 0x0008;

  const static Uint16 N_DOOR = 0x0010;
  const static Uint16 S_DOOR = 0x0020;
  const static Uint16 E_DOOR = 0x0040;
  const static Uint16 W_DOOR = 0x0080;
  
  const static Uint16 PASSAGE = 0x0100;
  const static Uint16 ROOM = 0x0200;
	const static Uint16 ROOM2 = 0x0400;

  const static Uint16 EMPTY_ROOM = ROOM + N_PASS + S_PASS + E_PASS + W_PASS;

  const static Sint16 offset = MAP_OFFSET;

  const static Sint16 torches = 25; // % of time there's a torch
  const static Sint16 randomDoors = 20; // % chance of a random door

  int dirCount;
  int dirs[DIR_COUNT];

  // shapes used to draw in the map
  const static Uint8 VERT_SHORT = 1;
  const static Uint8 VERT_LONG = 2;  
  const static Uint8 HORIZ_SHORT = 3;
  const static Uint8 HORIZ_LONG = 4;  
  const static Uint8 HORIZ_MEDIUM = 5;
  const static Uint8 HORIZ_SHORT2 = 6;
  const static Uint8 EMPTY_SHAPE = 7;
  
  const static bool debug = false;

  const static Sint16 unitOffset = MAP_UNIT_OFFSET;
  const static Sint16 unitSide = MAP_UNIT;
  const static Sint16 wallHeight = MAP_WALL_HEIGHT;   

  Scourge *scourge;

  static const MapLocation location[];

  const static int MAX_DOOR_COUNT = 500;
  int doorCount;
  int door[MAX_DOOR_COUNT][2];

  vector<Item*> containers;
  vector<int> containerX;
  vector<int> containerY;
  vector<int> teleporterX;
  vector<int> teleporterY;

  Uint32 start;

public: 

  // pre-rendered location index
  enum {
	HQ_LOCATION = 1,

	// must be the last one
	LOCATION_COUNT
  };
  
  DungeonGenerator(Scourge *scourge, int level, int depth, bool stairsDown, bool stairsUp, Mission *mission = NULL);
  ~DungeonGenerator();

  void toMap(Map *map, ShapePalette *shapePal, int location=0);

protected:

  // used by toMap
  void drawBasics(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void removeColumns(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void addContainers(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  bool addStairs(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void addPregeneratedShapes(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void addItems(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void addMissionObjectives(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void addMonsters(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void addFurniture(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  bool addTeleporters(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void addParty(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void lockDoors(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void lockLocation(Map *map, int mapx, int mapy);
  void createFreeSpaceMap(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void deleteFreeSpaceMap(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);
  void calculateRoomValues(Map *map, ShapePalette *shapePal, bool preGenerated, int locationIndex);

  void initByLevel();
  void generateMaze();
  void makeSparse();
  void makeLoops();
  void makeRooms();

  // construct a pre-rendered location
  void constructMaze(int location);

  bool drawNodesOnMap(Map *map, ShapePalette *shapePal, 
                      bool preGenerated, int locationIndex);

  /**
    Return a random location in the maze that has not been visited yet.
    returns -1 for x, y if there are no more such locations
  */
  void nextNotVisited(int *x, int *y);

  void nextVisited(int *x, int *y);

  bool isVisited(int x, int y);

  void markVisited(int x, int y);

  int initDirections();

  int nextDirection();

  void printMaze();

  void generatePassage(const int x, const int y, const bool stopAtVisited);

  int getScore(int x, int y, int rw, int rh);                        

  void getRandomLocation(Map *map, Shape *shape, int *x, int *y, bool accessible=false, int fromX=0, int fromY=0);

  bool getLocationInRoom(Map *map, int roomIndex, Shape *shape, 
						 int *xpos, int *ypos, bool startMiddle=false);
  
  bool coversDoor(Map *map, ShapePalette *shapePal, Shape *shape, int x, int y);

  static const int MAX_STEPS = 10000;
  bool isAccessible(Map *map, int x, int y, int fromX, int fromY, int stepsTaken=0, int dir=DIR_N);

  void addItem(Map *map, Creature *creature, Item *item, Shape *shape, int x, int y, int z = 0);

  void drawDoor(Map *map, ShapePalette *shapePal, 
				Sint16 mapx, Sint16 mapy, int doorType);
  
  void addItemsInRoom(RpgItem *rpgItem, int n, bool preGenerated, int locationIndex);
  
  bool addShapeInARoom(int shapeIndex);

  void getRandomDeadEndLocation(int *x, int *y, GLShape *shape, Map *map);

  int getRoomIndex(int x, int y);

  void updateStatus(const char *statusMessage);

};

#endif




