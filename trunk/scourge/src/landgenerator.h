/***************************************************************************
                landgenerator.h  -  Generates outdoor maps
                             -------------------
    begin                : Sat March 28 2009
    copyright            : (C) 2009 by Gabor Torok
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
#ifndef LAND_GENERATOR_H
#define LAND_GENERATOR_H
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

#define WIDTH_IN_NODES 75
#define DEPTH_IN_NODES 75

//some other (hopefully) explanatory constants
#define MAP_STEP_WIDTH (MAP_WIDTH / OUTDOORS_STEP)
#define MAP_STEP_DEPTH (MAP_DEPTH / OUTDOORS_STEP)
#define MAP_STEP_SIZE (MAP_STEP_WIDTH * MAP_STEP_DEPTH)

/// Outdoor specific terrain generator.
class LandGenerator : public TerrainGenerator {
private:
	float ground[MAP_WIDTH][MAP_DEPTH];
	CellularAutomaton *cellular[2][2];
	int regionX, regionY;
	int bitmapIndex;
	SDL_Surface *bitmapSurface;

public:
	LandGenerator( Scourge *scourge, int level, int depth, int maxDepth,
	               bool stairsDown, bool stairsUp,
	               Mission *mission );
	virtual ~LandGenerator();

	void printMaze();
	inline void getName( char *s ) {
		strcpy( s, "outdoor" );
	}
	
	inline void setRegion( int x, int y ) { regionX = x; regionY = y; }
	inline int getRegionX() { return regionX; }
	inline int getRegionY() { return regionY; }
	
protected:
	virtual inline const char *getGateDownShapeName() {
		return "GATE_DOWN_OUTDOORS";
	}
	virtual void generate( Map *map, ShapePalette *shapePal );
	virtual bool drawNodes( Map *map, ShapePalette *shapePal );
	virtual MapRenderHelper* getMapRenderHelper();
	void createGround();

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
};

#endif
