/***************************************************************************
                          outdoorgenerator.h  -  description
                             -------------------
    begin                : Sat May 12 2007
    copyright            : (C) 2007 by Gabor Torok
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
#ifndef OUTDOOR_GENERATOR_H
#define OUTDOOR_GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include "common/constants.h"
#include "terraingenerator.h"

class Map;
class ShapePalette;
class Scourge;
class Mission;
class GLShape;


#define CELL_GROWTH_CYCLES 3

#define CLEAR_WALL_RATIO 0.45f

#define MAX_ROOM_COUNT 100

#define OUTDOOR_NODE_SIZE 6
//#define WIDTH_IN_NODES ( MAP_WIDTH - ( MAP_OFFSET * 2 ) / OUTDOOR_NODE_SIZE )
//#define DEPTH_IN_NODES ( MAP_DEPTH - ( MAP_OFFSET * 2 ) / OUTDOOR_NODE_SIZE )
#define WIDTH_IN_NODES 50
#define DEPTH_IN_NODES 50

class OutdoorGenerator : public TerrainGenerator {
private:
  int w, h;
  int phase;
	float ground[MAP_WIDTH][MAP_DEPTH];

  typedef struct _NodePoint {
    bool wall, island;
    int room;
    bool seen;
  } NodePoint;
  NodePoint node[WIDTH_IN_NODES][DEPTH_IN_NODES];

  typedef struct _Room {
    int size;
    int x,y;
  } Room;
  int roomCounter, biggestRoom;
  Room room[ MAX_ROOM_COUNT ];


public:
	OutdoorGenerator( Scourge *scourge, int level, int depth, int maxDepth,
										bool stairsDown, bool stairsUp, 
										Mission *mission);
	virtual ~OutdoorGenerator();

protected:
	virtual void generate( Map *map, ShapePalette *shapePal );
	virtual bool drawNodes( Map *map, ShapePalette *shapePal );
	virtual MapRenderHelper* getMapRenderHelper();
	GLShape *getRandomTreeShape( ShapePalette *shapePal );
	void createGround();
	void randomize();
	void growCells();
	void setSeen( bool b );
	bool canReach( int sx, int sy, int ex, int ey );
	void findRooms();
	void connectPoints( int sx, int sy, int ex, int ey, bool isBiggestRoom );
	void connectRooms();
	void print();

};

#endif 
