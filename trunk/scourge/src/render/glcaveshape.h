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

class Shapes;

class CaveFace {
public:
  int p1, p2, p3; // point indexes
  CVector3 normal;
  GLfloat tex[3][2]; // texture coordinates per point
  enum {
    WALL=0,
    TOP,
    FLOOR
  };
  int textureType;
  GLfloat shade;

  CaveFace( int p1, int p2, int p3, 
            GLfloat u1, GLfloat v1, GLfloat u2, GLfloat v2, GLfloat u3, GLfloat v3,
            int textureType ) {
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
    tex[0][0] = u1;
    tex[0][1] = v1;
    tex[1][0] = u2;
    tex[1][1] = v2;
    tex[2][0] = u3;
    tex[2][1] = v3;
    this->textureType = textureType;
    this->normal.x = this->normal.y = this->normal.z = 0;
    this->shade = 1;
  }

  ~CaveFace() {
  }
};

class GLCaveShape : public GLShape {
private:
  int mode;
  int dir;
  Shapes *shapes;
  GLuint *wallTextureGroup;
  GLuint *topTextureGroup;
  GLuint *floorTextureGroup;
  int caveIndex;

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

  static GLCaveShape *GLCaveShape::shapeList[];

  static std::vector<CVector3*> points;
  static std::vector<std::vector<CaveFace*>*> polys;

public:

  GLCaveShape( Shapes *shapes, GLuint texture[],
               int width, int depth, int height, 
               char *name, int index, 
               int mode, int dir, int caveIndex );
  virtual ~GLCaveShape();

  virtual void initialize();

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
  static void createShapes( GLuint texture[], int shapeCount, Shapes *shapes );
  static void initializeShapes();
  static inline GLCaveShape *getShape( int index ) { return shapeList[ index ]; }

protected:
  void drawFaces();
  void drawBlock( float w, float h, float d );
  void drawFloor( float w, float h, float d );

private:
  static void removeDupPoints();
  static void updatePointIndexes( int oldIndex, int newIndex );
  static void dividePolys();
  static CVector3 *divideSegment( CVector3 *v1, CVector3 *v2 );
  static void bulgePoints( CVector3 *n1, CVector3 *n2, CVector3 *n3 );
  static void calculateNormals();
  static void calculateLight();
};

#endif

