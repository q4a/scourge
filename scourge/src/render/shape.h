/***************************************************************************
                          shape.h  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#ifndef SHAPE_H
#define SHAPE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"

class Shape {
  
private:
  Uint8 index;
  char *name;
  int descriptionGroup;  
  bool stencil;
  Color *outlineColor;
  bool interactive;
	int groundSX, groundEX, groundSY, groundEY;
	float outdoorWeight;
	bool outdoorShadow;
	bool wind;

protected:
  int width, height, depth;
  
public: 
	Shape(int width, int depth, int height, char *name, int descriptionGroup);
	Shape(Shape *shape);
	virtual ~Shape();

	inline void setDebugGroundPos( int sx, int sy, int ex, int ey ) { 
		this->groundSX = sx;
		this->groundSY = sy;
		this->groundEX = ex;
		this->groundEY = ey;
	}

	inline int getGroundSX() { return groundSX; }
	inline int getGroundSY() { return groundSY; }
	inline int getGroundEX() { return groundEX; }
	inline int getGroundEY() { return groundEY; }

  /**
    Call this once before the shape is to be displayed.
  */
  virtual inline void intialize() { }

  /**
    The widht (x) of the shape-block's base
  */
  inline int getWidth() { return width; }
  /**
    The depth (y) of the shape-block's base
  */  
  inline int getDepth() { return depth; }
  /**
    The height (z) of the shape-block
  */    
  inline int getHeight() { return height; }

  inline char *getName() { return name; }
  inline int getDescriptionGroup() { return descriptionGroup; }

  virtual void draw() = 0;
	virtual inline void drawHeightMap( float ground[][MAP_DEPTH], int groundX, int groundY ) { draw(); }
  virtual void outline( const Color *color ) { outline( color->r, color->g, color->b ); };
  virtual void outline( float r, float g, float b ) {};
  virtual void setupToDraw() = 0;
  
  inline void setIndex(Uint8 index) { this->index = index; }
  
  inline Uint8 getIndex() { return index; }

  /**
    Draw after all the others?
    */
  virtual bool drawFirst() = 0;
  // if true, the next two functions are called
  virtual bool drawLater() = 0;
  virtual void setupBlending() = 0;
  virtual void endBlending() = 0;


  inline void setStencil(bool b) { stencil = b; }
  inline bool isStencil() { return stencil; }

  inline void setOutlineColor( Color *color ) { this->outlineColor = color; }
  inline Color *getOutlineColor() { return this->outlineColor; }

  virtual inline void setInteractive( bool b ) { interactive = b; }
  virtual inline bool isInteractive() { return interactive; }

	virtual inline void setOutdoorWeight( float f ) { outdoorWeight = f; }
  virtual inline float getOutdoorWeight() { return outdoorWeight; }

	virtual inline void setOutdoorShadow( bool b ) { outdoorShadow = b; }
	virtual inline bool isOutdoorShadow() { return outdoorShadow; }

	virtual inline void setWind( bool b ) { wind = b; }
	virtual inline bool isWind() { return wind; }

  virtual inline bool isFlatCaveshape() { return false; }
};

#endif
