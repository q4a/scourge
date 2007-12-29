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

namespace Util {

	inline float degreesToRadians(float angle) { return PI / (180.0 / angle); }

	/**
	* Rotate the 2D point(x,y) by angle(in degrees). Return the result in px,py.
	*/
	void rotate(Sint16 x, Sint16 y, Sint16 *px, Sint16 *py, float angle);

	// some math functions
	float dot_product(float v1[3], float v2[3]);
	void normalize(float v[3]);
	void cross_product(const float *v1, const float *v2, float *out);
	void multiply_vector_by_matrix(const float m[9], float v[3]);
	void multiply_vector_by_matrix2(const float m[16], float v[4]);

	/**
	* Return a string containing the last OpenGL error
	* Useful to debug strange OpenGL behaviors
	*/
	char* getOpenGLError();

	/**
	* Returns next word from the given position
	*/
	std::string getNextWord(const std::string& theInput, int fromPos, int &endWord);

	/**
	* get the angle between two shapes (x,y,width,depth)
	*/
	float getAngle(float sx, float sy, float sw, float sd, float tx, float ty, float tw, float td);

	/**
	* get the difference between a and b ( returns a - b )
	*/
	float diffAngle(float a, float b);

	// draw a percentage bar
	enum {
		HORIZONTAL_LAYOUT=0,
		VERTICAL_LAYOUT
	};

	void drawBar( int x, int y, float barLength, float value, float maxValue,
								float red=-1, float green=-1, float blue=-1, float gradient=true, 
								GuiTheme *theme=NULL, int layout=HORIZONTAL_LAYOUT );

	inline float distance2( float x, float y, float z ) { return ( x * x ) + ( y * y ) + ( z * z ); }

	float getRandomSum( float base, int count, float div=3.0f );

	char *toLowerCase( char *s );

	/**
	* Is px,py in the field of vision defined by x,y,angle?
	*/
	bool isInFOV( float x, float y, float angle, float px, float py );

	char *addLineBreaks( const char *in, char *out, int lineLength=40 );

	void getLines( const char *in, std::vector<std::string> *out );

	float getLight( float *normal, float angle=135.0f );

	float getLightComp( float x, float y, float lightAngle );

	template<class T> T Tokenize( const std::string &s, const std::string &strDelim ) {
		T tokens;
		std::string::size_type nFirstPos, nLastPos;
	
		/**
		* Skip delimiters at beginning
		*/
		nLastPos = s.find_first_not_of( strDelim, 0 );
	
		/**
		* Find first non delimiter
		*/
		nFirstPos = s.find_first_of( strDelim, nLastPos );
	
		while ( std::string::npos != nFirstPos || std::string::npos != nLastPos ) {
			/**
			* Found token!
			*/
			tokens.push_back( s.substr( nLastPos, nFirstPos-nLastPos ) );
			nLastPos = s.find_first_not_of( strDelim, nFirstPos );
			nFirstPos = s.find_first_of( strDelim, nLastPos );
		}
		return tokens;
	}

	template <class T> struct equal_ignore_case : public std::binary_function<T, T, bool> {
		bool operator()(const T& first_argument_type, const T& second_argument_type) const
		{
			return std::toupper(first_argument_type) == std::toupper(second_argument_type);
		}
	};

	bool StringCaseCompare(const std::string sStr1, const std::string sStr2);
}

#endif
