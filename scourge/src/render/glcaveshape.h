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
    DIR_NW,
    DIR_CROSS_NW,
    DIR_CROSS_NE
  };

  static char *names[];

  static GLCaveShape *GLCaveShape::shapes[];

public:

  GLCaveShape( GLuint texture[],
               int width, int depth, int height, 
               char *name, int index, 
               int mode, int dir );
  virtual ~GLCaveShape();

  void draw();

  enum {
    CAVE_INDEX_N=0,
    CAVE_INDEX_E,
    CAVE_INDEX_S,
    CAVE_INDEX_W,
    CAVE_INDEX_NE,
    CAVE_INDEX_SE,
    CAVE_INDEX_SW,
    CAVE_INDEX_NW,
    CAVE_INDEX_INV_NE,
    CAVE_INDEX_INV_SE,
    CAVE_INDEX_INV_SW,
    CAVE_INDEX_INV_NW,
    CAVE_INDEX_CROSS_NW,
    CAVE_INDEX_CROSS_NE,
    CAVE_INDEX_BLOCK,
    CAVE_INDEX_FLOOR,

    CAVE_INDEX_COUNT
  };
  static void initShapes( GLuint texture[], int shapeCount );
  static inline GLCaveShape *getShape( int index ) { return shapes[ index ]; }

protected:
  void drawFlat( float w, float h, float d );
  void drawCorner( float w, float h, float d );
  void drawInverse( float w, float h, float d );
  void drawBlock( float w, float h, float d );
  void drawFloor( float w, float h, float d );
};

#endif

