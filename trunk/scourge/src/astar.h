/***************************************************************************
                          astar.h  -  description
                             -------------------
    begin                : Sun Jun 22 2003
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

#ifndef A_STAR_H
#define A_STAR_H

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <vector>    // STL for Vector
#include <algorithm> // STL for Heap
#include <string>
#include "common/constants.h"
#include "gui/guitheme.h"

class Map;
class Location;
class Shape;
class Creature;

/**
  *@author Gabor Torok
  */

/**
  A path class used by A* algorithm
*/
class CPathNode{
public:
  double f, gone, heuristic; // f = gone + heuristic
  int x, y;               // Location of node
  int px, py;             // Locaton of parent node

  inline bool operator<(const CPathNode &b) {
		return this->f > b.f;
  }

  inline bool operator>(const CPathNode &b) {
		return this->f < b.f;
  }
  
};

class AStar {
  
public: 
	AStar();
	~AStar();

  static void findPath( Sint16 sx, Sint16 sy, Sint16 sz,
												Sint16 dx, Sint16 dy, Sint16 dz,
												std::vector<Location> *pVector,
												Map *map,
												Creature *creature,
												Creature *player,
												int maxNodes,
												bool ignoreParty,
												bool ignoreEndShape );

	static bool isOutOfTheWay( Creature *a, std::vector<Location> *aPath, int aStart,
														 Creature *b, std::vector<Location> *bPath, int bStart );

protected:
  static bool isBlocked( Sint16 x, Sint16 y, Sint16 shapeX, Sint16 shapeY, Sint16 dx, Sint16 dy,
												 Creature *creature, Creature *player, Map *map, 
												 bool ignoreCreatures=false, bool ignoreEndShape=false );

};

#endif
