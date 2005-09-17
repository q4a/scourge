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
#include <string>
#include "constants.h"
#include "gui/guitheme.h"

using namespace std;

class Map;
class Location;
class Shape;

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

  inline bool operator<(const CPathNode &b) {
	return this->f < b.f;
  }

  inline bool operator>(const CPathNode &b) {
	return this->f > b.f;
  }
  
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

  // some math functions
  static float dot_product(float v1[3], float v2[3]);
  static void normalize(float v[3]);
  static void cross_product(const float *v1, const float *v2, float *out);
  static void multiply_vector_by_matrix(const float m[9], float v[3]);
  static void multiply_vector_by_matrix2(const float m[16], float v[4]);
  
  // Return a string containing the last OpenGL error.
  // Useful to debug strange OpenGL behaviors
  static char * getOpenGLError();
  	
  // Returns next word from the given position  					
  static string getNextWord(const string theInput, int fromPos, int &endWord);

  // get the angle between two shapes (x,y,width,depth)
  static float getAngle(float sx, float sy, float sw, float sd,
                        float tx, float ty, float tw, float td);

  // get the difference between a and b ( returns a - b )
  static float diffAngle(float a, float b);

  // draw a percentage bar
  enum {
    HORIZONTAL_LAYOUT=0,
    VERTICAL_LAYOUT
  };

  static void drawBar( int x, int y, float barLength, float value, float maxValue,
                       float red=-1, float green=-1, float blue=-1, float gradient=true, 
                       GuiTheme *theme=NULL, int layout=HORIZONTAL_LAYOUT );
  
  inline static float distance2( float x, float y, float z ) { return ( x * x ) + ( y * y ) + ( z * z ); }

  static float getRandomSum( float base, int count, float div=3.0f );

  static char *toLowerCase( char *s );

};

#endif
