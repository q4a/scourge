/***************************************************************************
                          glcaveshape.cpp  -  description
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
#include "glcaveshape.h"
#include "shapes.h"

using namespace std;

char *GLCaveShape::names[] = {
  "CAVE_FLAT_NORTH",
  "CAVE_FLAT_EAST",
  "CAVE_FLAT_SOUTH",
  "CAVE_FLAT_WEST",
  "CAVE_CORNER_NE",
  "CAVE_CORNER_SE",
  "CAVE_CORNER_SW",
  "CAVE_CORNER_NW",
  "CAVE_BLOCK",
  "CAVE_FLOOR"
};

char *GLCaveShape::inverseNames[] = {
  "CAVE_INV_NORTH",
  "CAVE_INV_EAST",
  "CAVE_INV_SOUTH",
  "CAVE_INV_WEST",
  "CAVE_INV_NE",
  "CAVE_INV_SE",
  "CAVE_INV_SW",
  "CAVE_INV_NW"
};

float shade[] = {
  0.875f,
  0.875f,
  0.625f,
  0.625f,
  1,
  0.75f,
  0.5f,
  0.75f

//  0.25f, 0.5f, 0.75f, 1,
//  0.125f, 0.375f, 0.625f, 0.875f
};

GLCaveShape::GLCaveShape( GLuint texture[],
                          int width, int depth, int height, 
                          char *name, int index, 
                          int mode, int dir ) :
GLShape( texture, width, depth, height, name, 0, color, index ) {
  this->mode = mode;
  this->dir = dir;
}

GLCaveShape::~GLCaveShape() {
}

void GLCaveShape::draw() {

  float w = (float)width / DIV;
  float d = (float)depth / DIV;
  float h = (float)height / DIV;
  if (h == 0) h = 0.25 / DIV;

  glDisable( GL_CULL_FACE );

  switch( mode ) {
  case MODE_FLAT: drawFlat( w, h, d ); break;
  case MODE_CORNER: drawCorner( w, h, d ); break;
  case MODE_BLOCK: drawBlock( w, h, d ); break;
  case MODE_FLOOR: drawFloor( w, h, d ); break;
  case MODE_INV: drawInverse( w, h, d ); break;
  default: cerr << "Unknown cave_shape mode: " << mode << endl;
  }

}

void GLCaveShape::drawFlat( float w, float h, float d ) {
  glColor4f( shade[dir], shade[dir], shade[dir], 1 );
  glBegin( GL_QUADS );
  switch( dir ) {
  case DIR_S:
  glVertex3f( 0, d, 0 );
  glVertex3f( w, d, 0 );
  glVertex3f( w, 0, h );
  glVertex3f( 0, 0, h );
  break;
  case DIR_N:
  glVertex3f( 0, 0, 0 );
  glVertex3f( w, 0, 0 );
  glVertex3f( w, d, h );
  glVertex3f( 0, d, h );
  break;
  case DIR_E:
  glVertex3f( w, 0, 0 );
  glVertex3f( w, d, 0 );
  glVertex3f( 0, d, h );
  glVertex3f( 0, 0, h );
  break;
  case DIR_W:
  glVertex3f( 0, 0, 0 );
  glVertex3f( 0, d, 0 );
  glVertex3f( w, d, h );
  glVertex3f( w, 0, h );
  break;
  default:
  cerr << "bad dir for flat shape: " << dir << endl;
  }
  glEnd();
}

void GLCaveShape::drawCorner( float w, float h, float d ) {  
  drawFloor( w, h, d );

  glColor4f( shade[dir], shade[dir], shade[dir], 1 );
  glBegin( GL_TRIANGLES );
  switch( dir ) {
  case DIR_NE:
  glVertex3f( 0, 0, 0 );
  glVertex3f( w, d, 0 );
  glVertex3f( 0, d, h );
  break;
  case DIR_SE:
  glVertex3f( w, 0, 0 );
  glVertex3f( 0, d, 0 );
  glVertex3f( 0, 0, h );
  break;
  case DIR_SW:
  glVertex3f( 0, 0, 0 );
  glVertex3f( w, d, 0 );
  glVertex3f( w, 0, h );
  break;
  case DIR_NW:
  glVertex3f( w, 0, 0 );
  glVertex3f( 0, d, 0 );
  glVertex3f( w, d, h );
  break;
  default:
  cerr << "bad dir for corner shape: " << dir << endl;
  }
  glEnd();
}

void GLCaveShape::drawInverse( float w, float h, float d ) {  
  glColor4f( shade[dir], shade[dir], shade[dir], 1 );
  glBegin( GL_TRIANGLES );
  switch( dir ) {
  case DIR_NE:
  glVertex3f( 0, 0, h );
  glVertex3f( w, d, h );
  glVertex3f( w, 0, 0 );
  break;
  case DIR_SE:
  glVertex3f( w, 0, h );
  glVertex3f( 0, d, h );
  glVertex3f( w, d, 0 );
  break;
  case DIR_SW:
  glVertex3f( 0, 0, h );
  glVertex3f( w, d, h );
  glVertex3f( 0, d, 0 );
  break;
  case DIR_NW:
  glVertex3f( w, 0, h );
  glVertex3f( 0, d, h );
  glVertex3f( 0, 0, 0 );
  break;
  default:
  cerr << "bad dir for corner shape: " << dir << endl;
  }
  glEnd();

  // roof
  glColor4f( 0.5f, 0.35f, 0.15f, 1 );
  glBegin( GL_TRIANGLES );
  switch( dir ) {
  case DIR_NE:
  glVertex3f( 0, 0, h );
  glVertex3f( w, d, h );
  glVertex3f( 0, d, h );
  break;
  case DIR_SE:
  glVertex3f( w, 0, h );
  glVertex3f( 0, d, h );
  glVertex3f( 0, 0, h );
  break;
  case DIR_SW:
  glVertex3f( 0, 0, h );
  glVertex3f( w, d, h );
  glVertex3f( w, 0, h );
  break;
  case DIR_NW:
  glVertex3f( w, 0, h );
  glVertex3f( 0, d, h );
  glVertex3f( w, d, h );
  break;
  default:
  cerr << "bad dir for corner shape: " << dir << endl;
  }
  glEnd();
}

void GLCaveShape::drawBlock( float w, float h, float d ) {
  glBegin( GL_QUADS );
  glColor4f( 0.5f, 0.35f, 0.15f, 1 );
  glVertex3f( 0, 0, h );
  glVertex3f( 0, d, h );
  glVertex3f( w, d, h );
  glVertex3f( w, 0, h );
  glEnd();
}

void GLCaveShape::drawFloor( float w, float h, float d ) {
  glBegin( GL_QUADS );
  glColor4f( 0.25f, 0.15f, 0.05f, 1 );
  glVertex3f( 0, 0, 0 );
  glVertex3f( 0, d, 0 );
  glVertex3f( w, d, 0 );
  glVertex3f( w, 0, 0 );
  glEnd();
}

