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
class CellularAutomaton;

class CaveMaker : public TerrainGenerator {
private:
  int w, h;
  CellularAutomaton *cellular;

public:

  CaveMaker( Scourge *scourge, int level, int depth, int maxDepth, 
             bool stairsDown, bool stairsUp, 
             Mission *mission);
  virtual ~CaveMaker();

	void printMaze();
	inline void getName(char *s) { strcpy( s, "cave" ); }

protected:
  virtual bool drawNodes( Map *map, ShapePalette *shapePal );
  virtual void generate( Map *map, ShapePalette *shapePal );
  virtual void addFurniture(Map *map, ShapePalette *shapePal);
  virtual MapRenderHelper *getMapRenderHelper();

	// Make cave rooms easier.
	virtual inline float getMonsterLevelMod() { return 0.5f; }
	virtual bool getUseBadassMonsters() { return false; }

};

#endif

