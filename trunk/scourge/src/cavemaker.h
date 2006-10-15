/***************************************************************************
                          cavemaker.h  -  description
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
#ifndef CAVEMAKER_H
#define CAVEMAKER_H

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

#define CELL_GROWTH_CYCLES 3

#define CLEAR_WALL_RATIO 0.45f

#define MAX_ROOM_COUNT 100

class CaveMaker : public TerrainGenerator {
private:
  int w, h;
  int phase;

  typedef struct _NodePoint {
    bool wall, island;
    int room;
    bool seen;
  } NodePoint;
  NodePoint **node;

  typedef struct _Room {
    int size;
    int x,y;
  } Room;
  int roomCounter, biggestRoom;
  Room room[ MAX_ROOM_COUNT ];

public:

  CaveMaker( Scourge *scourge, int level, int depth, int maxDepth, 
             bool stairsDown, bool stairsUp, 
             Mission *mission);
  virtual ~CaveMaker();

  

protected:
  void randomize();
  void growCells();
  void setSeen( bool b );
  bool canReach( int sx, int sy, int ex, int ey );
  void findRooms();
  void connectPoints( int sx, int sy, int ex, int ey, bool isBiggestRoom );
  void connectRooms();
  void removeSingles();
  void generate();
  void print();
  void addIslands();
  void growCellsIsland();
  void addIslandLand();

  virtual bool drawNodes( Map *map, ShapePalette *shapePal );

  virtual void generate( Map *map, ShapePalette *shapePal );

  virtual void addFurniture(Map *map, ShapePalette *shapePal);

  virtual MapRenderHelper *getMapRenderHelper();

	// Make cave rooms easier.
	virtual inline float getMonsterLevelMod() { return 0.5f; }
	virtual bool getUseBadassMonsters() { return false; }

};

#endif

