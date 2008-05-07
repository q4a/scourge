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
#include "common/constants.h"
#include "terraingenerator.h"

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

#define DRAW_UNVISITED 0

/**
  *@author Gabor Torok
  */

class DungeonGenerator : public TerrainGenerator {
private:

  enum { dgWIDTH = 0, dgHEIGHT, dgCURVYNESS, dgSPARSENESS,
         dgLOOPYNESS, dgROOMCOUNT, dgROOMMAXWIDTH, dgROOMMAXHEIGHT,
         dgOBJECTCOUNT };
  static const int levels[][9];

  int width;
  int height;
  int curvyness; // the lower this number the more random the maze
  int sparseness; // the higher the more sparse (more empty space)
  Uint16 **nodes;
  int notVisitedCount, visitedCount;
  int *notVisited, *visited;
  int loopyness; // 0-100 % value of whether or not to make a dead-end into a loop

  int dirCount;
  int dirs[DIR_COUNT];

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

  const static Sint16 torches = 25; // % of time there's a torch
  const static Sint16 randomDoors = 20; // % chance of a random door

  // shapes used to draw in the map
  const static Uint8 VERT_SHORT = 1;
  const static Uint8 VERT_LONG = 2;  
  const static Uint8 HORIZ_SHORT = 3;
  const static Uint8 HORIZ_LONG = 4;  
  const static Uint8 HORIZ_MEDIUM = 5;
  const static Uint8 HORIZ_SHORT2 = 6;
  const static Uint8 EMPTY_SHAPE = 7;
  
public: 

  DungeonGenerator(Scourge *scourge, int level, int depth, int maxDepth, bool stairsDown, bool stairsUp, Mission *mission = NULL);
  virtual ~DungeonGenerator();

  virtual void generate( Map *map, ShapePalette *shapePal );

	void printMaze();
	inline void getName(char *s) { strcpy( s, "dungeon" ); }

protected:

  // used by toMap
  void drawBasics(Map *map, ShapePalette *shapePal);
  void removeColumns(Map *map, ShapePalette *shapePal);	

  void initByLevel();
  void generateMaze();
  void makeSparse();
  void makeLoops();
  void makeRooms();

  bool drawNodes(Map *map, ShapePalette *shapePal);

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

  void generatePassage(const int x, const int y, const bool stopAtVisited);

  int getScore(int x, int y, int rw, int rh);                        

  void drawDoor(Map *map, ShapePalette *shapePal, 
      Sint16 mapx, Sint16 mapy, int doorType, bool secret=false);
  void drawEastDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret );
  void drawWestDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret );
  void drawSouthDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret );
  void drawNorthDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret );

  virtual void addFurniture(Map *map, ShapePalette *shapePal);

  virtual void addContainers(Map *map, ShapePalette *shapePal);

  virtual MapRenderHelper *getMapRenderHelper();
  
};

#endif




