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
#include "common/constants.h"
#include "gui/guitheme.h"

class Map;
class Location;
class Shape;

/**
  *@author Gabor Torok
  */

#define PI 3.14159

class Util {
public:
  // todo: make #ifdefs for this
  const static char PATH_SEPARATOR = '/';
  
public: 
	Util();
	~Util();

  inline static float degreesToRadians(float angle) { return PI / (180.0 / angle); }

  /**
    * Rotate the 2D point(x,y) by angle(in degrees). Return the result in px,py.
    */
  static void rotate(Sint16 x, Sint16 y, Sint16 *px, Sint16 *py, float angle);

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
  static std::string getNextWord(const std::string theInput, int fromPos, int &endWord);

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

	/**
	 * Is px,py in the field of vision defined by x,y,angle?
	 */
	static bool isInFOV( float x, float y, float angle, float px, float py );

	static char *addLineBreaks( const char *in, char *out, int lineLength=40 );

	static void getLines( const char *in, std::vector<std::string> *out );

	static float getLight( float *normal, float angle=135.0f );

};

#endif
