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
class CellularAutomaton;

#define WIDTH_IN_NODES 50
#define DEPTH_IN_NODES 50

class OutdoorGenerator : public TerrainGenerator {
private:
	float ground[MAP_WIDTH][MAP_DEPTH];
  CellularAutomaton *cellular[2][2];

public:
	OutdoorGenerator( Scourge *scourge, int level, int depth, int maxDepth,
										bool stairsDown, bool stairsUp, 
										Mission *mission);
	virtual ~OutdoorGenerator();

protected:
	virtual void generate( Map *map, ShapePalette *shapePal );
	int getMountainSize( int x, int y, Map *map, std::vector<int> *lake );
	virtual bool drawNodes( Map *map, ShapePalette *shapePal );
	virtual MapRenderHelper* getMapRenderHelper();
	GLShape *getRandomTreeShape( ShapePalette *shapePal );
	void createGround();
	virtual void addFurniture( Map *map, ShapePalette *shapePal );
	virtual void lockDoors( Map *map, ShapePalette *shapePal );

	/**
	 * Outdoors have low level monsters only.	
	 * FIXME: this should really be "outdoor type monsters".
	 * Bears, lions, etc.
	 */
	virtual inline int getBaseMonsterLevel() { return 1; }

	virtual bool getUseBadassMonsters() { return false; }

	virtual void addMonsters(Map *map, ShapePalette *shapePal);

};

#endif 
