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
#include "cellular.h"

class Map;
class ShapePalette;
class Scourge;
class Mission;
class GLShape;
class CellularAutomaton;
class Texture;

/// Outdoor specific terrain generator.
class LandGenerator : public TerrainGenerator {
private:
	float ground[QUARTER_WIDTH_IN_NODES][QUARTER_DEPTH_IN_NODES];
	CellularAutomaton *cellular;
	int regionX, regionY; // which map region (of the large world map) are we in?
	int mapPosX, mapPosY; // where to render the map in OUTDOOR_STEP units
	bool willAddParty;
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

	inline int getVegetation( int x, int y ) { return cellular->getNode( x, y )->vegetation; }
	inline int getClimate( int x, int y ) { return cellular->getNode( x, y )->climate; }

	inline void setRegion( int x, int y ) { regionX = x; regionY = y; }
	inline int getRegionX() { return regionX; }
	inline int getRegionY() { return regionY; }
	
	inline void setMapPosition( int x, int y ) { mapPosX = x; mapPosY = y; }
	inline int getMapPositionX() { return mapPosX; }
	inline int getMapPositionY() { return mapPosY; }
	
	inline void setWillAddParty( bool b ) { willAddParty = b; }
	inline bool getWillAddParty() { return willAddParty; }
	
	void initOutdoorsGroundTexture( Map *map );
	
private:	
	void addHighVariation( Map *map, std::string ref, int z );
	bool isRockTexture( Map *map, int x, int y );
	bool isLakebedTexture( Map *map, int x, int y );
	bool isAllHigh( Map *map, int x, int y, int w, int h );
	
	
protected:
	void loadMapGridBitmap();
	void loadMapGridBitmapRegion();
	void packMapData( std::vector<GLubyte> &image );
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
	
	virtual bool addTeleporters( Map *map, ShapePalette *shapePal ) {
		return true; // all is well...
	}
	
	virtual void addContainers( Map *map, ShapePalette *shapePal ) {
	}
	
	virtual bool addStairs( Map *map, ShapePalette *shapePal ){
		return true;
	}

	virtual void addItems( Map *map, ShapePalette *shapePal ){
	}

	virtual void addMissionObjectives( Map *map, ShapePalette *shapePal ){
	}

	virtual void addMonsters( Map *map, ShapePalette *shapePal ){
	}

	virtual void addHarmlessCreatures( Map *map, ShapePalette *shapePal ){
	}

	virtual void addFurniture( Map *map, ShapePalette *shapePal ){
	}

	virtual bool addParty( Map *map, ShapePalette *shapePal, bool goingUp, bool goingDown );
	
	virtual void getPartyStartingLocation( int *xx, int *yy ) {
		*xx = 100;
		*yy = 56;
	}
	
	virtual void addRugs( Map *map, ShapePalette *shapePal ){
	}

	virtual void addTraps( Map *map, ShapePalette *shapePal ){
	}

};

#endif
