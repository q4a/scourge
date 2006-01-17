/***************************************************************************
                          glshape.h  -  description
                             -------------------
    begin                : Thu Jul 10 2003
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

#ifndef GLCAVE_SHAPE_H
#define GLCAVE_SHAPE_H

#include "render.h"
#include "glshape.h"
#include <vector>

class GLCaveShape : public GLShape {
private:
  int mode;
  int dir;

public:

  enum {
    MODE_FLAT=0,
    MODE_CORNER,
    MODE_BLOCK,
    MODE_FLOOR,
    MODE_INV
  };

  enum {
    DIR_N=0,
    DIR_E,
    DIR_S,
    DIR_W,
    DIR_NE,
    DIR_SE,
    DIR_SW,
    DIR_NW
  };

  static char *names[];
  static char *inverseNames[];

  GLCaveShape( GLuint texture[],
               int width, int depth, int height, 
               char *name, int index, 
               int mode, int dir );
  virtual ~GLCaveShape();

  void draw();

protected:
  void drawFlat( float w, float h, float d );
  void drawCorner( float w, float h, float d );
  void drawInverse( float w, float h, float d );
  void drawBlock( float w, float h, float d );
  void drawFloor( float w, float h, float d );
};

#endif

