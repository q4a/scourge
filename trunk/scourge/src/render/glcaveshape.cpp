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

vector<CVector3> GLCaveShape::points;
vector<vector<CaveFace*>*> GLCaveShape::polys;

GLCaveShape::GLCaveShape( Shapes *shapes, GLuint texture[],
                          int width, int depth, int height, 
                          char *name, int index, 
                          int mode, int dir, int caveIndex ) :
GLShape( texture, width, depth, height, name, 0, color, index ) {
  this->shapes = shapes;
  this->mode = mode;
  this->dir = dir;
  this->caveIndex = caveIndex;
}

GLCaveShape::~GLCaveShape() {
}

void GLCaveShape::initialize() {
  assert( shapes->getCurrentTheme() &&
          shapes->getCurrentTheme()->isCave() );
  string ref = WallTheme::themeRefName[ WallTheme::THEME_REF_WALL ];
  wallTextureGroup = shapes->getCurrentTheme()->getTextureGroup( ref );
  ref = WallTheme::themeRefName[ WallTheme::THEME_REF_CORNER ];
  topTextureGroup = shapes->getCurrentTheme()->getTextureGroup( ref );
  ref = WallTheme::themeRefName[ WallTheme::THEME_REF_PASSAGE_FLOOR ];
  floorTextureGroup = shapes->getCurrentTheme()->getTextureGroup( ref );
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
  case MODE_FLAT: drawFaces(); break;
  case MODE_CORNER: drawFaces(); break;
  case MODE_BLOCK: drawBlock( w, h, d ); break;
  case MODE_FLOOR: drawFloor( w, h, d ); break;
  case MODE_INV: drawInverse( w, h, d ); break;
  default: cerr << "Unknown cave_shape mode: " << mode << endl;
  }


  if( !textureWasEnabled ) glDisable( GL_TEXTURE_2D );
  //useShadow = false;

}

void GLCaveShape::drawFaces() {
  vector<CaveFace*>* face = polys[ caveIndex ];
  if( !face ) cerr << "Can't find face for shape: " << 
    getName() << " caveIndex=" << caveIndex << endl;
  for( int i = 0; i < (int)face->size(); i++ ) {
    CaveFace *p = (*face)[i];
    if( !useShadow ) glColor3f( shade[ dir ], shade[ dir ], shade[ dir ] );
    switch( p->textureType ) {
    case CaveFace::WALL:
      glBindTexture( GL_TEXTURE_2D, wallTextureGroup[ GLShape::FRONT_SIDE ] );
    break;
    case CaveFace::TOP:
      glBindTexture( GL_TEXTURE_2D, wallTextureGroup[ GLShape::TOP_SIDE ] );
    break;
    case CaveFace::FLOOR:
      glBindTexture( GL_TEXTURE_2D, floorTextureGroup[ GLShape::TOP_SIDE ] );
    break;
    }
    glBegin( GL_TRIANGLES );
    
    CVector3 xyz = points[ p->p1 ];
    if( p->tex[0][0] > -1 ) glTexCoord2f( p->tex[0][0], p->tex[0][1] );
    glVertex3f( xyz.x, xyz.y, xyz.z );

    xyz = points[ p->p2 ];
    if( p->tex[1][0] > -1 ) glTexCoord2f( p->tex[1][0], p->tex[1][1] );
    glVertex3f( xyz.x, xyz.y, xyz.z );

    xyz = points[ p->p3 ];
    if( p->tex[2][0] > -1 ) glTexCoord2f( p->tex[2][0], p->tex[2][1] );
    glVertex3f( xyz.x, xyz.y, xyz.z );
    glEnd();
  }
}

void GLCaveShape::drawFlat( float w, float h, float d ) {
}

void GLCaveShape::drawCorner( float w, float h, float d ) {  
}

void GLCaveShape::drawInverse( float w, float h, float d ) {  
  if( !( useShadow || dir == DIR_CROSS_NE || dir == DIR_CROSS_NW ) ) {
    // roof
    glBindTexture( GL_TEXTURE_2D, wallTextureGroup[ GLShape::TOP_SIDE ] );
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
  glBindTexture( GL_TEXTURE_2D, wallTextureGroup[ GLShape::FRONT_SIDE ] );
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

  glBindTexture( GL_TEXTURE_2D, wallTextureGroup[ GLShape::TOP_SIDE ] );
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
  glBindTexture( GL_TEXTURE_2D, floorTextureGroup[ GLShape::TOP_SIDE ] );
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

void GLCaveShape::removeDupPoints() {
  vector<CVector3> newPoints;
  for( int i = 0; i < (int)points.size(); i++ ) {
    CVector3 v = points[i];
    
    bool copyPoint = true;
    for( int t = 0; t < (int)newPoints.size(); t++ ) {
      CVector3 v2 = newPoints[t];
      if( v.x == v2.x && v.y == v2.y && v.z == v2.z ) {
        copyPoint = false;
        for( int r = 0; r < (int)polys.size(); r++ ) {
          vector<CaveFace*> *face = polys[r];
          for( int g = 0; g < (int)face->size(); g++ ) {
            CaveFace *cf = (*face)[g];
            if( cf->p1 == i ) cf->p1 = t;
            if( cf->p2 == i ) cf->p2 = t;
            if( cf->p3 == i ) cf->p3 = t;
          }
        }
      }
    }

    if( copyPoint ) newPoints.push_back( v );
  }
  points.clear();
  for( int i = 0; i < (int)newPoints.size(); i++ ) {
    points.push_back( newPoints[ i ] );
  }
}

GLCaveShape *GLCaveShape::shapeList[ CAVE_INDEX_COUNT ];

#define _point(_x,_y,_z) ( {\
  CVector3 v;\
  v.x=_x; v.y=_y; v.z=_z;\
  points.push_back( v );\
} )
#define _poly(_index,_p1,_p2,_p3,_u1,_v1,_u2,_v2,_u3,_v3,_tt) ( {\
  polys[_index]->push_back(new CaveFace(_p1,_p2,_p3,_u1,_v1,_u2,_v2,_u3,_v3,_tt));\
} )

void GLCaveShape::createShapes( GLuint texture[], int shapeCount, Shapes *shapes ) {

  float w = (float)CAVE_CHUNK_SIZE / DIV;
  float d = (float)CAVE_CHUNK_SIZE / DIV;
  float h = (float)MAP_WALL_HEIGHT / DIV; // fixme: for floor it should be 0
  if (h == 0) h = 0.25 / DIV;

  for( int i = 0; i < CAVE_INDEX_COUNT; i++ ) {
    polys.push_back( new vector<CaveFace*>() );
  }

  // store the points
  // DIR_S flat
  _point( 0, d, 0 );
  _point( w, d, 0 );
  _point( w, 0, h );
  _point( 0, 0, h );  
  _poly( CAVE_INDEX_S, 0, 1, 2, 0, 1, 1, 1, 1, 0, CaveFace::WALL );  
  _poly( CAVE_INDEX_S, 0, 2, 3, 0, 1, 1, 0, 0, 0, CaveFace::WALL );

  // DIR_N flat
  _point( 0, 0, 0 );
  _point( w, 0, 0 );
  _point( w, d, h );
  _point( 0, d, h );
  _poly( CAVE_INDEX_N, 4, 5, 6, 0, 1, 1, 1, 1, 0, CaveFace::WALL );
  _poly( CAVE_INDEX_N, 4, 6, 7, 0, 1, 1, 0, 0, 0, CaveFace::WALL );

  // DIR_E flat
  _point( w, 0, 0 );
  _point( w, d, 0 );
  _point( 0, d, h );
  _point( 0, 0, h );
  _poly( CAVE_INDEX_E, 8, 9, 10, 0, 1, 1, 1, 1, 0, CaveFace::WALL );
  _poly( CAVE_INDEX_E, 8, 10, 11, 0, 1, 1, 0, 0, 0, CaveFace::WALL );

  // DIR_W flat  
  _point( 0, 0, 0 );
  _point( 0, d, 0 );
  _point( w, d, h );
  _point( w, 0, h );
  _poly( CAVE_INDEX_W, 12, 13, 14, 0, 1, 1, 1, 1, 0, CaveFace::WALL );
  _poly( CAVE_INDEX_W, 12, 14, 15, 0, 1, 1, 0, 0, 0, CaveFace::WALL );

  // DIR_NE corner
  _point( 0, 0, 0 );
  _point( w, d, 0 );
  _point( 0, d, h );
  _poly( CAVE_INDEX_NE, 16, 17, 18, 1, 1, 0, 1, 0.5f, 0, CaveFace::WALL );

  // DIR_SE corner
  _point( w, 0, 0 );
  _point( 0, d, 0 );
  _point( 0, 0, h );
  _poly( CAVE_INDEX_SE, 19, 20, 21, 0, 1, 1, 1, 0.5f, 0, CaveFace::WALL );
  
  // DIR_SW corner
  _point( 0, 0, 0 );
  _point( w, d, 0 );
  _point( w, 0, h );
  _poly( CAVE_INDEX_SW, 22, 23, 24, 0, 1, 1, 1, 0.5f, 0, CaveFace::WALL );
  
  // DIR_NW corner
  _point( w, 0, 0 );
  _point( 0, d, 0 );
  _point( w, d, h );
  _poly( CAVE_INDEX_NW, 25, 26, 27, 0, 1, 1, 1, 0.5f, 0, CaveFace::WALL );

  // Remove dup. points: this creates a triangle mesh
  removeDupPoints();


  shapeList[CAVE_INDEX_N] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_N ] ), shapeCount++,
                     MODE_FLAT, DIR_N, CAVE_INDEX_N );
  shapeList[CAVE_INDEX_E] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_E ] ), shapeCount++,
                     MODE_FLAT, DIR_E, CAVE_INDEX_E );
  shapeList[CAVE_INDEX_S] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_S ] ), shapeCount++,
                     MODE_FLAT, DIR_S, CAVE_INDEX_S );
  shapeList[CAVE_INDEX_W] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_W ] ), shapeCount++,
                     MODE_FLAT, DIR_W, CAVE_INDEX_W );
  shapeList[CAVE_INDEX_NE] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_NE ] ), shapeCount++,
                     MODE_CORNER, DIR_NE, CAVE_INDEX_NE );
  shapeList[CAVE_INDEX_SE] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_SE ] ), shapeCount++,
                     MODE_CORNER, DIR_SE, CAVE_INDEX_SE );
  shapeList[CAVE_INDEX_SW] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_SW ] ), shapeCount++,
                     MODE_CORNER, DIR_SW, CAVE_INDEX_SW );
  shapeList[CAVE_INDEX_NW] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_NW ] ), shapeCount++,
                     MODE_CORNER, DIR_NW, CAVE_INDEX_NW );
  shapeList[CAVE_INDEX_INV_NE] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_INV_NE ] ), shapeCount++,
                     MODE_INV, DIR_NE, CAVE_INDEX_INV_NE );
  shapeList[CAVE_INDEX_INV_SE] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_INV_SE ] ), shapeCount++,
                     MODE_INV, DIR_SE, CAVE_INDEX_INV_SE );
  shapeList[CAVE_INDEX_INV_SW] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_INV_SW ] ), shapeCount++,
                     MODE_INV, DIR_SW, CAVE_INDEX_INV_SW );
  shapeList[CAVE_INDEX_INV_NW] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_INV_NW ] ), shapeCount++,
                     MODE_INV, DIR_NW, CAVE_INDEX_INV_NW );
  shapeList[CAVE_INDEX_CROSS_NE] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ DIR_CROSS_NE ] ), shapeCount++,
                     MODE_INV, DIR_CROSS_NE, CAVE_INDEX_CROSS_NE );
  shapeList[CAVE_INDEX_CROSS_NW] = 
    new GLCaveShape( shapes, texture, 
                     CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ DIR_CROSS_NW ] ), shapeCount++,
                     MODE_INV, DIR_CROSS_NW, CAVE_INDEX_CROSS_NW );
  shapeList[CAVE_INDEX_BLOCK] = 
    new GLCaveShape( shapes, texture,
                     CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                     strdup( names[ CAVE_INDEX_BLOCK ] ), shapeCount++,
                     MODE_BLOCK, 0, CAVE_INDEX_BLOCK );
  shapeList[CAVE_INDEX_FLOOR] = 
    new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, 0,
                     strdup( names[ CAVE_INDEX_FLOOR ] ), shapeCount++,
                     MODE_FLOOR, 0, CAVE_INDEX_FLOOR );

  for( int i = 0; i < CAVE_INDEX_COUNT; i++ ) {
    shapeList[i]->setSkipSide(false);
    if( i < CAVE_INDEX_BLOCK ) {
      shapeList[i]->setStencil( false );
      shapeList[i]->setLightBlocking( true );
    } else {
      shapeList[i]->setStencil( false );
      shapeList[i]->setLightBlocking( false );
    }
  }
}

void GLCaveShape::initializeShapes() {
  for( int i = 0; i < CAVE_INDEX_COUNT; i++ ) {
    //if( !headless ) 
      shapeList[i]->initialize();
  }
}
