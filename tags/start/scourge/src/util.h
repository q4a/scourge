/***************************************************************************
                          util.h  -  description
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

#ifndef UTIL_H
#define UTIL_H

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <vector>    // STL for Vector
#include <algorithm> // STL for Heap
#include "constants.h"
#include "map.h"

using namespace std;

class Map;

/**
  *@author Gabor Torok
  */

/**
  A path class used by A* algorithm
*/
class CPathNode{
public:
  int f, gone, heuristic; // f = gone + heuristic
  int x, y;               // Location of node
  int px, py;             // Locaton of parent node
};

class Util {
public:
  // todo: make #ifdefs for this
  const static char PATH_SEPARATOR = '/';
  
private:
  const static float PI = 3.14159;
public: 
	Util();
	~Util();

  inline static float degreesToRadians(float angle) { return PI / (180.0 / angle); }

  /**
    * Rotate the 2D point(x,y) by angle(in degrees). Return the result in px,py.
    */
  static void rotate(Sint16 x, Sint16 y, Sint16 *px, Sint16 *py, float angle);

  static void findPath(Sint16 sx, Sint16 sy, Sint16 sz,
                Sint16 dx, Sint16 dy, Sint16 dz,
                vector<Location> *pVector, Map *map, Shape *shape);
  
};

#endif
