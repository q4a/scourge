/***************************************************************************
                outdoorgenerator.h  -  Generates outdoor maps
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
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <set>
#include "terraingenerator.h"

class Map;
class ShapePalette;
class Scourge;
class Mission;
class GLShape;
class CellularAutomaton;

#define OUTDOOR_WIDTH_IN_NODES 50
#define OUTDOOR_DEPTH_IN_NODES 50

//some other (hopefully) explanatory constants
#define OUTDOOR_MAP_STEP_WIDTH (MAP_WIDTH / OUTDOORS_STEP)
#define OUTDOOR_MAP_STEP_DEPTH (MAP_DEPTH / OUTDOORS_STEP)
#define OUTDOOR_MAP_STEP_SIZE (OUTDOOR_MAP_STEP_WIDTH*OUTDOOR_MAP_STEP_DEPTH)

/// Outdoor specific terrain generator.
class OutdoorGenerator : public TerrainGenerator {
private:
	float ground[MAP_WIDTH][MAP_DEPTH];
	CellularAutomaton *cellular[2][2];
	std::map<int, GLShape*> keepFloor;
	int roadX, roadY;

public:
	OutdoorGenerator( Scourge *scourge, int level, int depth, int maxDepth,
	                  bool stairsDown, bool stairsUp,
	                  Mission *mission );
	virtual ~OutdoorGenerator();

	void printMaze();
	inline void getName( char *s ) {
		strcpy( s, "outdoor" );
	}

	void getPartyStartingLocation( int *xx, int *yy );
	void addVillage( Map *map, ShapePalette *shapePal, int *x, int *y );
	void flattenChunkWithLimits( Map *map, Sint16 mapX, Sint16 mapY, Sint16 mapEndX, Sint16 mapEndY, float minLimit, float maxLimit );
	void addFloor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool doFlattenChunk, GLShape *shape );

	// -=K=-: just a wrapper class of static-sized-bool-array;
	// far quicker than large dynamic sets with their inner trees and stuff
	class AroundMapLooker {
	public:
		AroundMapLooker() {
			clear();
		}
		//compiler-genned destructor and operator = are OK
		void clear() {
			for ( int x = 0; x < OUTDOOR_MAP_STEP_WIDTH; ++x ) {
				for ( int y = 0; y < OUTDOOR_MAP_STEP_DEPTH; ++y ) {
					seen[x][y] = false;
				}
			}
		}
		bool& at( int x, int y ) {
			assert( 0 <= x && x < OUTDOOR_MAP_STEP_WIDTH &&  0 <= y && y < OUTDOOR_MAP_STEP_DEPTH );
			return seen[x][y];
		}
	private:
		bool seen[OUTDOOR_MAP_STEP_WIDTH][OUTDOOR_MAP_STEP_DEPTH];
	};
protected:
	virtual inline const char *getGateDownShapeName() {
		return "GATE_DOWN_OUTDOORS";
	}
	void removeLakes( Map *map, int x, int y );
	void createRoads( Map *map, ShapePalette *shapePal, int x, int y );
	void addOutdoorTexture( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, int ref, float angle = 0.0f, bool horiz = false, bool vert = false );
	void createHouses( Map *map, ShapePalette *shapePal, int x, int y );
	virtual void generate( Map *map, ShapePalette *shapePal );
	int getMountainSize( int x, int y, Map *map, AroundMapLooker& lake );
	virtual bool drawNodes( Map *map, ShapePalette *shapePal );
	virtual MapRenderHelper* getMapRenderHelper();
	void createGround();
	virtual void addFurniture( Map *map, ShapePalette *shapePal );
	void addContainers( Map *map, ShapePalette *shapePal );
	virtual void lockDoors( Map *map, ShapePalette *shapePal );
	virtual void addNpcs( Map *map, ShapePalette *shapePal, int villageX, int villageY, int villageWidth, int villageHeight );
	virtual void createNpc( Map *map, ShapePalette *shapePal, int x, int y );
	virtual bool isShapeOnFloor( Shape *shape, int x, int y, Map *map );

	/**
	 * Outdoors have low level monsters only.
	 * FIXME: this should really be "outdoor type monsters".
	 * Bears, lions, etc.
	 */
	virtual inline int getBaseMonsterLevel() {
		return 1;
	}

	virtual bool getUseBadassMonsters() {
		return false;
	}

	virtual void addMonsters( Map *map, ShapePalette *shapePal );

	//virtual void addRugs( Map *map, ShapePalette *shapePal );
	virtual void addTraps( Map *map, ShapePalette *shapePal );
	virtual void deleteFreeSpaceMap( Map *map, ShapePalette *shapePal );
};

#endif
