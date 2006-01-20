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
  "CAVE_INDEX_N",
  "CAVE_INDEX_E",
  "CAVE_INDEX_S",
  "CAVE_INDEX_W",
  "CAVE_INDEX_NE",
  "CAVE_INDEX_SE",
  "CAVE_INDEX_SW",
  "CAVE_INDEX_NW",
  "CAVE_INDEX_INV_NE",
  "CAVE_INDEX_INV_SE",
  "CAVE_INDEX_INV_SW",
  "CAVE_INDEX_INV_NW",
  "CAVE_INDEX_CROSS_NW",
  "CAVE_INDEX_CROSS_NE",
  "CAVE_INDEX_BLOCK",
  "CAVE_INDEX_FLOOR"
};

float shade[] = {
  0.5f,
  0.5f,
  0.75f,
  0.75f,
  0.4f,
  0.65f,
  1.0f,
  0.65f,
  1,
  1
};

/*
 const float Map::shadowTransformMatrix[16] = { 
	1, 0, 0, 0,
	0, 1, 0, 0,
	0.75f, -0.25f, 0, 0,
	0, 0, 0, 1 };
*/

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

  bool textureWasEnabled = glIsEnabled( GL_TEXTURE_2D );
  if( !useShadow ) {
    glEnable( GL_TEXTURE_2D );
  }

  switch( mode ) {
  case MODE_FLAT: drawFlat( w, h, d ); break;
  case MODE_CORNER: drawCorner( w, h, d ); break;
  case MODE_BLOCK: drawBlock( w, h, d ); break;
  case MODE_FLOOR: drawFloor( w, h, d ); break;
  case MODE_INV: drawInverse( w, h, d ); break;
  default: cerr << "Unknown cave_shape mode: " << mode << endl;
  }


  if( !textureWasEnabled ) glDisable( GL_TEXTURE_2D );
  //useShadow = false;

}

void GLCaveShape::drawFlat( float w, float h, float d ) {
  // FIXME: implement real normal-based shading (like in C3DSShape)    
  if( !useShadow ) glColor3f( shade[ dir ], shade[ dir ], shade[ dir ] );
  glBindTexture( GL_TEXTURE_2D, tex[ GLShape::FRONT_SIDE ] );
  glBegin( GL_QUADS );
  switch( dir ) {
  case DIR_S:
  glNormal3f( 0, -1, 0 );
  glTexCoord2f( 0, 1 );
  glVertex3f( 0, d, 0 );
  glTexCoord2f( 1, 1 );
  glVertex3f( w, d, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( w, 0, h );
  glTexCoord2f( 0, 0 );
  glVertex3f( 0, 0, h );
  break;
  case DIR_N:
  glNormal3f( 0, 1, 0 );
  glTexCoord2f( 0, 1 );
  glVertex3f( 0, 0, 0 );
  glTexCoord2f( 1, 1 );
  glVertex3f( w, 0, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( w, d, h );
  glTexCoord2f( 0, 0 );
  glVertex3f( 0, d, h );
  break;
  case DIR_E:
  glNormal3f( 1, 0, 0 );
  glTexCoord2f( 0, 1 );
  glVertex3f( w, 0, 0 );
  glTexCoord2f( 1, 1 );
  glVertex3f( w, d, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( 0, d, h );
  glTexCoord2f( 0, 0 );
  glVertex3f( 0, 0, h );
  break;
  case DIR_W:
  glNormal3f( -1, 0, 0 );
  glTexCoord2f( 0, 1 );
  glVertex3f( 0, 0, 0 );
  glTexCoord2f( 1, 1 );
  glVertex3f( 0, d, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( w, d, h );
  glTexCoord2f( 0, 0 );
  glVertex3f( w, 0, h );
  break;
  default:
  cerr << "bad dir for flat shape: " << dir << endl;
  }
  glEnd();
}

void GLCaveShape::drawCorner( float w, float h, float d ) {  
  // FIXME: implement real normal-based shading (like in C3DSShape)    
  if( !useShadow ) glColor3f( shade[ dir ], shade[ dir ], shade[ dir ] );
  glBindTexture( GL_TEXTURE_2D, tex[ GLShape::FRONT_SIDE ] );
  glBegin( GL_TRIANGLES );
  switch( dir ) {
  case DIR_NE:
  glNormal3f( 1, -1, 0 );
  glTexCoord2f( 1, 1 );
  glVertex3f( 0, 0, 0 );
  glTexCoord2f( 0, 1 );  
  glVertex3f( w, d, 0 );
  glTexCoord2f( 0.5f, 0 );
  glVertex3f( 0, d, h );
  break;
  case DIR_SE:
  glNormal3f( 1, 1, 0 );
  glTexCoord2f( 0, 1 );
  glVertex3f( w, 0, 0 );
  glTexCoord2f( 1, 1 );
  glVertex3f( 0, d, 0 );
  glTexCoord2f( 0.5f, 0 );
  glVertex3f( 0, 0, h );
  break;
  case DIR_SW:
  glNormal3f( -1, 1, 0 );
  glTexCoord2f( 0, 1 );  
  glVertex3f( 0, 0, 0 );
  glTexCoord2f( 1, 1 );
  glVertex3f( w, d, 0 );
  glTexCoord2f( 0.5f, 0 );
  glVertex3f( w, 0, h );
  break;
  case DIR_NW:
  glNormal3f( -1, -1, 0 );
  glTexCoord2f( 0, 1 );
  glVertex3f( w, 0, 0 );
  glTexCoord2f( 1, 1 );
  glVertex3f( 0, d, 0 );  
  glTexCoord2f( 0.5f, 0 );
  glVertex3f( w, d, h );
  break;
  default:
  cerr << "bad dir for corner shape: " << dir << endl;
  }
  glEnd();
}

void GLCaveShape::drawInverse( float w, float h, float d ) {  

    if( !( useShadow || dir == DIR_CROSS_NE || dir == DIR_CROSS_NW ) ) {
    // roof
    glBindTexture( GL_TEXTURE_2D, tex[ GLShape::TOP_SIDE ] );
    glBegin( GL_TRIANGLES );
    switch( dir ) {
    case DIR_NE:
    glNormal3f( 0, 0, 1 );
    glTexCoord2f( 0, 0 );
    glVertex3f( 0, 0, h );
    glTexCoord2f( 1, 1 );
    glVertex3f( w, d, h );
    glTexCoord2f( 0, 1 );
    glVertex3f( 0, d, h );
    break;
    case DIR_SE:
    glNormal3f( 0, 0, 1 );
    glTexCoord2f( 1, 0 );
    glVertex3f( w, 0, h );
    glTexCoord2f( 0, 1 );
    glVertex3f( 0, d, h );
    glTexCoord2f( 0, 0 );
    glVertex3f( 0, 0, h );
    break;
    case DIR_SW:
    glNormal3f( 0, 0, 1 );
    glTexCoord2f( 0, 0 );
    glVertex3f( 0, 0, h );
    glTexCoord2f( 1, 1 );
    glVertex3f( w, d, h );
    glTexCoord2f( 1, 0 );
    glVertex3f( w, 0, h );
    break;
    case DIR_NW:
    glNormal3f( 0, 0, 1 );
    glTexCoord2f( 1, 0 );
    glVertex3f( w, 0, h );
    glTexCoord2f( 0, 1 );
    glVertex3f( 0, d, h );
    glTexCoord2f( 1, 1 );
    glVertex3f( w, d, h );
    break;
    default:
    cerr << "bad dir for inverse shape: " << dir << endl;
    }
    glEnd();
  }

  // FIXME: implement real normal-based shading (like in C3DSShape)    
  if( !useShadow ) glColor3f( shade[ dir ], shade[ dir ], shade[ dir ] );
  glBindTexture( GL_TEXTURE_2D, tex[ GLShape::FRONT_SIDE ] );
  glBegin( GL_TRIANGLES );
  switch( dir ) {
  case DIR_NE:
  glNormal3f( 1, -1, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( 0, 0, h );
  glTexCoord2f( 0, 0 );
  glVertex3f( w, d, h );
  glTexCoord2f( 0.5f, 1 );
  glVertex3f( w, 0, 0 );
  break;
  case DIR_SE:
  glNormal3f( 1, 1, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( w, 0, h );
  glTexCoord2f( 0, 0 );
  glVertex3f( 0, d, h );
  glTexCoord2f( 0.5f, 1 );
  glVertex3f( w, d, 0 );
  break;
  case DIR_SW:
  glNormal3f( -1, 1, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( 0, 0, h );
  glTexCoord2f( 0, 0 );
  glVertex3f( w, d, h );
  glTexCoord2f( 0.5f, 1 );
  glVertex3f( 0, d, 0 );
  break;
  case DIR_NW:
  glNormal3f( -1, -1, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( w, 0, h );
  glTexCoord2f( 0, 0 );  
  glVertex3f( 0, d, h );
  glTexCoord2f( 0.5f, 1 );
  glVertex3f( 0, 0, 0 );
  break;
  case DIR_CROSS_NW:
  if( !useShadow ) glColor3f( shade[ DIR_NW ], shade[ DIR_NW ], shade[ DIR_NW ] );
  glNormal3f( -1, -1, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( w, 0, h );
  glTexCoord2f( 0, 0 );  
  glVertex3f( 0, d, h );
  glTexCoord2f( 0.5f, 1 );
  glVertex3f( 0, 0, 0 );
  
  if( !useShadow ) glColor3f( shade[ DIR_SE ], shade[ DIR_SE ], shade[ DIR_SE ] );
  glNormal3f( 1, 1, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( w, 0, h );
  glTexCoord2f( 0, 0 );
  glVertex3f( 0, d, h );
  glTexCoord2f( 0.5f, 1 );
  glVertex3f( w, d, 0 );;
  break;
  case DIR_CROSS_NE:
  if( !useShadow ) glColor3f( shade[ DIR_NE ], shade[ DIR_NE ], shade[ DIR_NE ] );
  glNormal3f( 1, -1, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( 0, 0, h );
  glTexCoord2f( 0, 0 );
  glVertex3f( w, d, h );
  glTexCoord2f( 0.5f, 1 );
  glVertex3f( w, 0, 0 );

  if( !useShadow ) glColor3f( shade[ DIR_SW ], shade[ DIR_SW ], shade[ DIR_SW ] );
  glNormal3f( -1, 1, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( 0, 0, h );
  glTexCoord2f( 0, 0 );
  glVertex3f( w, d, h );
  glTexCoord2f( 0.5f, 1 );
  glVertex3f( 0, d, 0 );
  break;
  default:
  cerr << "bad dir for inverse shape: " << dir << endl;
  }
  glEnd();  
}

void GLCaveShape::drawBlock( float w, float h, float d ) {
  if( useShadow ) return;

  glBindTexture( GL_TEXTURE_2D, tex[ GLShape::TOP_SIDE ] );
  glBegin( GL_QUADS );
  glNormal3f( 0, 0, 1 );
  glTexCoord2f( 0, 0 );
  glVertex3f( 0, 0, h );
  glTexCoord2f( 0, 1 );
  glVertex3f( 0, d, h );
  glTexCoord2f( 1, 1 );
  glVertex3f( w, d, h );
  glTexCoord2f( 1, 0 );
  glVertex3f( w, 0, h );
  glEnd();
}

void GLCaveShape::drawFloor( float w, float h, float d ) {
  if( useShadow ) return;

  // FIXME: use separate floor texture
  glBindTexture( GL_TEXTURE_2D, tex[ GLShape::TOP_SIDE ] );
  if( !useShadow ) glColor3f( 1, 0.7f, 0.7f );
  glBegin( GL_QUADS );
  glNormal3f( 0, 0, 1 );
  glTexCoord2f( 0, 0 );
  glVertex3f( 0, 0, h );
  glTexCoord2f( 0, 1 );
  glVertex3f( 0, d, h );
  glTexCoord2f( 1, 1 );
  glVertex3f( w, d, h );
  glTexCoord2f( 1, 0 );
  glVertex3f( w, 0, h );
  glEnd();
}


GLCaveShape *GLCaveShape::shapes[ CAVE_INDEX_COUNT ];

void GLCaveShape::initShapes( GLuint texture[], int shapeCount ) {  

  cerr << "GLCaveShape::initShapes, tex=" << texture[0] << "," << 
    texture[1] << "," << texture[2] << endl;

  shapes[CAVE_INDEX_N] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_N ] ), shapeCount++,
                     MODE_FLAT, DIR_N );
  shapes[CAVE_INDEX_E] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_E ] ), shapeCount++,
                     MODE_FLAT, DIR_E );
  shapes[CAVE_INDEX_S] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_S ] ), shapeCount++,
                     MODE_FLAT, DIR_S );
  shapes[CAVE_INDEX_W] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_W ] ), shapeCount++,
                     MODE_FLAT, DIR_W );
  shapes[CAVE_INDEX_NE] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_NE ] ), shapeCount++,
                     MODE_CORNER, DIR_NE );
  shapes[CAVE_INDEX_SE] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_SE ] ), shapeCount++,
                     MODE_CORNER, DIR_SE );
  shapes[CAVE_INDEX_SW] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_SW ] ), shapeCount++,
                     MODE_CORNER, DIR_SW );
  shapes[CAVE_INDEX_NW] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_NW ] ), shapeCount++,
                     MODE_CORNER, DIR_NW );
  shapes[CAVE_INDEX_INV_NE] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_INV_NE ] ), shapeCount++,
                     MODE_INV, DIR_NE );
  shapes[CAVE_INDEX_INV_SE] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_INV_SE ] ), shapeCount++,
                     MODE_INV, DIR_SE );
  shapes[CAVE_INDEX_INV_SW] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_INV_SW ] ), shapeCount++,
                     MODE_INV, DIR_SW );
  shapes[CAVE_INDEX_INV_NW] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_INV_NW ] ), shapeCount++,
                     MODE_INV, DIR_NW );
  shapes[CAVE_INDEX_CROSS_NE] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ DIR_CROSS_NE ] ), shapeCount++,
                     MODE_INV, DIR_CROSS_NE );
  shapes[CAVE_INDEX_CROSS_NW] = 
    new GLCaveShape( texture, 
                     CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ DIR_CROSS_NW ] ), shapeCount++,
                     MODE_INV, DIR_CROSS_NW );
  shapes[CAVE_INDEX_BLOCK] = 
    new GLCaveShape( texture,
                     CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_BLOCK ] ), shapeCount++,
                     MODE_BLOCK, 0 );
  shapes[CAVE_INDEX_FLOOR] = 
    new GLCaveShape( texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, 0,
                     strdup( names[ CAVE_INDEX_FLOOR ] ), shapeCount++,
                     MODE_FLOOR, 0 );

  for( int i = 0; i < CAVE_INDEX_COUNT; i++ ) {
    shapes[i]->setSkipSide(false);
    if( i < CAVE_INDEX_BLOCK ) {
      shapes[i]->setStencil( true );
      shapes[i]->setLightBlocking( true );
    } else {
      shapes[i]->setStencil( false );
      shapes[i]->setLightBlocking( false );
    }
    //if( !headless ) shapes[i]->initialize();
  }
}

