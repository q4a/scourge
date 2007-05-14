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

class OutdoorGenerator : public TerrainGenerator {
private:
	float ground[MAP_WIDTH][MAP_DEPTH];

public:
	OutdoorGenerator( Scourge *scourge, int level, int depth, int maxDepth,
										bool stairsDown, bool stairsUp, 
										Mission *mission);
	virtual ~OutdoorGenerator();

protected:
	virtual void generate( Map *map, ShapePalette *shapePal );
	virtual bool drawNodes( Map *map, ShapePalette *shapePal );
	virtual MapRenderHelper* getMapRenderHelper();
};

#endif 
