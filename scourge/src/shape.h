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
#include "constants.h"

class Shape {
  
private:
  Uint8 index;
  char *name;
  int descriptionGroup;  
  bool stencil;

protected:
  int width, height, depth;
  
public: 
	Shape(int width, int depth, int height, char *name, int descriptionGroup);
	Shape(Shape *shape);
	virtual ~Shape();

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


};

#endif
