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
  "CAVE_INDEX_FLOOR",
  "LAVA_SIDE_W",
  "LAVA_SIDE_E",
  "LAVA_SIDE_N",
  "LAVA_SIDE_S",
  "LAVA_SIDES_EW",
  "LAVA_SIDES_NS",
  "LAVA_INSIDE_TURN_NW",
  "LAVA_INSIDE_TURN_NE",
  "LAVA_INSIDE_TURN_SE",
  "LAVA_INSIDE_TURN_SW",
  "LAVA_OUTSIDE_TURN_NW",
  "LAVA_OUTSIDE_TURN_NE",
  "LAVA_OUTSIDE_TURN_SE",
  "LAVA_OUTSIDE_TURN_SW",
  "LAVA_TURNS_NW",
  "LAVA_TURNS_NE",
  "LAVA_TURNS_SE",
  "LAVA_TURNS_SW",
  "LAVA_U_N",
  "LAVA_U_E",
  "LAVA_U_S",
  "LAVA_U_W",
  "LAVA_ALL",
  "LAVA_NONE",

};

GLuint GLCaveShape::overlay_tex;
unsigned char GLCaveShape::overlay_data[OVERLAY_SIZE * OVERLAY_SIZE * 3];

vector<CVector3*> GLCaveShape::points;
vector<vector<CaveFace*>*> GLCaveShape::polys;

//#define DEBUG 1

#define LIGHT_ANGLE_HORIZ 125.0f
#define LIGHT_ANGLE_VERT 45.0f

GLuint GLCaveShape::lavaTex[ CAVE_INDEX_COUNT - LAVA_SIDE_W ];
unsigned char *GLCaveShape::lavaData[ CAVE_INDEX_COUNT - LAVA_SIDE_W ];

GLCaveShape::GLCaveShape( Shapes *shapes, GLuint texture[],
                          int width, int depth, int height, 
                          char *name, int index, 
                          int mode, int dir, int caveIndex, int lavaIndex ) :
GLShape( texture, width, depth, height, name, 0, color, index ) {
  this->shapes = shapes;
  this->mode = mode;
  this->dir = dir;
  this->caveIndex = caveIndex;
  this->lavaIndex = lavaIndex;
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

  bool textureWasEnabled = glIsEnabled( GL_TEXTURE_2D );
  if( !useShadow ) {
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
  }

  switch( mode ) {
  case MODE_FLAT: drawFaces(); break;
  case MODE_CORNER: drawFaces(); break;
  case MODE_BLOCK: drawBlock( w, h, d ); break;
  case MODE_FLOOR: drawFloor( w, h, d ); break;
  case MODE_INV: drawFaces(); break;
  case MODE_LAVA: drawLava( w, h, d ); break;
  default: cerr << "Unknown cave_shape mode: " << mode << endl;
  }


  if( !textureWasEnabled ) glDisable( GL_TEXTURE_2D );
  //useShadow = false;

  glDisable( GL_CULL_FACE );

}

void GLCaveShape::drawFaces() {
  vector<CaveFace*>* face = polys[ caveIndex ];
  if( !face ) cerr << "Can't find face for shape: " << 
    getName() << " caveIndex=" << caveIndex << endl;
#ifdef DEBUG
  for( int t = 0; t < 2; t++ ) {
    if( useShadow ) return;
    if( t == 1 ) {
      glDisable( GL_TEXTURE_2D );
      glDisable( GL_DEPTH_TEST );
    }
#endif
  for( int i = 0; i < (int)face->size(); i++ ) {
    CaveFace *p = (*face)[i];
    if( !useShadow ) {
      glColor3f( p->shade, p->shade, p->shade );
    }
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
#ifdef DEBUG
    glBegin( t == 0 ? GL_TRIANGLES : GL_LINE_LOOP );
#else
    glBegin( GL_TRIANGLES );
#endif    
    if( p->tex[0][0] > -1 ) glTexCoord2f( p->tex[0][0], p->tex[0][1] );
    CVector3 *xyz = points[ p->p1 ];
    glVertex3f( xyz->x, xyz->y, xyz->z );

    
    if( p->tex[1][0] > -1 ) glTexCoord2f( p->tex[1][0], p->tex[1][1] );
    xyz = points[ p->p2 ];
    glVertex3f( xyz->x, xyz->y, xyz->z );

    
    if( p->tex[2][0] > -1 ) glTexCoord2f( p->tex[2][0], p->tex[2][1] );
    xyz = points[ p->p3 ];
    glVertex3f( xyz->x, xyz->y, xyz->z );
    glEnd();
  }
#ifdef DEBUG
  }
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_DEPTH_TEST );
#endif
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

void GLCaveShape::drawLava( float w, float h, float d ) {
  if( useShadow ) return;
  glEnable( GL_TEXTURE_2D );
  if( lavaIndex + LAVA_SIDE_W == LAVA_NONE ) {
    glBindTexture( GL_TEXTURE_2D, floorTextureGroup[ GLShape::FRONT_SIDE ] );
  } else {
    //glEnable( GL_BLEND );
    //glBlendFunc(GL_DST_COLOR, GL_ZERO);
    //glBlendFunc(GL_SRC_COLOR, GL_ONE);
    //glBlendFunc(GL_DST_ALPHA, GL_ONE);

    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0x00 );

    glBindTexture( GL_TEXTURE_2D, lavaTex[ lavaIndex ] );
  }
  GLfloat n = 0.05 / DIV;
  //glColor4f( 1, 1, 1, 1 );
  glBegin( GL_QUADS );
  glNormal3f( 0, 0, 1 );
  glTexCoord2f( 0, 0 );
  glVertex3f( 0, 0, n );
  glTexCoord2f( 0, 1 );
  glVertex3f( 0, d, n );
  glTexCoord2f( 1, 1 );
  glVertex3f( w, d, n );
  glTexCoord2f( 1, 0 );
  glVertex3f( w, 0, n );
  glEnd();
  //glDisable( GL_BLEND );
  glDisable( GL_ALPHA_TEST );
}

void GLCaveShape::calculateNormals() {
  for( int i = 0; i < (int)polys.size(); i++ ) {
    vector<CaveFace*> *face = polys[i];
    for( int t = 0; t < (int)face->size(); t++ ) {
      CaveFace *cf = (*face)[t];
      findNormal( points[cf->p1], 
                  points[cf->p2], 
                  points[cf->p3], 
                  &(cf->normal) );  
    }
  }
}

void GLCaveShape::calculateLight() {
  for( int i = 0; i < (int)polys.size(); i++ ) {
    vector<CaveFace*> *face = polys[i];
    for( int t = 0; t < (int)face->size(); t++ ) {
      CaveFace *cf = (*face)[t];

      if( points[cf->p1]->z == points[cf->p2]->z &&
          points[cf->p2]->z == points[cf->p3]->z ) continue;


      // Simple light rendering:
      // need the normal as mapped on the xy plane
      // it's degree is the intensity of light it gets
      float lightAngle;
      cf->shade = 0;
      for( int a = 0; a < 2; a++ ) {
        float x = (cf->normal.x == 0 ? 0.01f : cf->normal.x);
        float y;
        if( a == 0 ) {
          y = cf->normal.y;          
          lightAngle = LIGHT_ANGLE_HORIZ;
        } else {
          y = cf->normal.z;          
          lightAngle = LIGHT_ANGLE_VERT;
        }
        float rad = atan(y / x);
        float angle = (180.0f * rad) / 3.14159;
        
        // read about the arctan problem: 
        // http://hyperphysics.phy-astr.gsu.edu/hbase/ttrig.html#c3
        int q = 1;
        if (x < 0) {     // Quadrant 2 & 3
          q = ( y >= 0 ? 2 : 3);
          angle += 180;
        } else if (y < 0) { // Quadrant 4
          q = 4;
          angle += 360;
        }
        
        // calculate the angle distance from the light
        float delta = 0;
        if (angle > lightAngle && angle < lightAngle + 180.0f) {
          delta = angle - lightAngle;
        } else {
          if (angle < lightAngle) angle += 360.0f;
          delta = (360 + lightAngle) - angle;
        }
        
        // reverse and convert to value between 0.2 and 1
        delta = 1.0f - ( 0.8f * (delta / 180.0f) );
        
        // store the value
        if( a == 0 ) {
          cf->shade = delta;
        } else {
          cf->shade += delta / 10.0f;
        }
      }
    }
  }
}

void GLCaveShape::updatePointIndexes( int oldIndex, int newIndex ) {
  for( int i = 0; i < (int)polys.size(); i++ ) {
    vector<CaveFace*> *face = polys[i];
    for( int t = 0; t < (int)face->size(); t++ ) {
      CaveFace *cf = (*face)[t];
      if( cf->p1 == oldIndex ) cf->p1 = newIndex;
      if( cf->p2 == oldIndex ) cf->p2 = newIndex;
      if( cf->p3 == oldIndex ) cf->p3 = newIndex;
    }
  }
}

void GLCaveShape::removeDupPoints() {
  vector<CVector3*> newPoints;
  for( int i = 0; i < (int)points.size(); i++ ) {
    CVector3 *v = points[i];
    
    bool copyPoint = true;
    for( int t = 0; t < (int)newPoints.size(); t++ ) {
      CVector3 *v2 = newPoints[t];
      if( v->x == v2->x && 
          v->y == v2->y && 
          v->z == v2->z ) {
        copyPoint = false;
        updatePointIndexes( i, t );
      }
    }

    if( copyPoint ) {
      CVector3 *nv = new CVector3();
      nv->x = v->x;
      nv->y = v->y;
      nv->z = v->z;
      newPoints.push_back( nv );
      updatePointIndexes( i, newPoints.size() - 1 );
    }
  }
  for( int i = 0; i < (int)points.size(); i++ ) {
    delete points[i];
  }
  points.clear();
  for( int i = 0; i < (int)newPoints.size(); i++ ) {
    points.push_back( newPoints[ i ] );
  }
}

CVector3 *GLCaveShape::divideSegment( CVector3 *v1, CVector3 *v2 ) {
  CVector3 *v = new CVector3();
  v->x = ( v1->x + v2->x ) / 2.0f;
  v->y = ( v1->y + v2->y ) / 2.0f;
  v->z = ( v1->z + v2->z ) / 2.0f;
  return v;
}

void GLCaveShape::bulgePoints( CVector3 *n1, CVector3 *n2, CVector3 *n3 ) {

  // don't warp the top
  if( n1->z == n2->z && n2->z == n3->z ) return;

  // find the base points where z is the same
  CVector3 *b1, *b2, *a;
  if( n1->z == n2->z ) {
    b1 = n1;
    b2 = n2;
    a = n3;
  } else if( n1->z == n3->z ) {
    b1 = n1;
    b2 = n3;
    a = n2;
  } else {
    b1 = n2;
    b2 = n3;
    a = n1;
  }


  // find the points' normal
  CVector3 normal;
  findNormal( n1, n2, n3, &normal );  
  
  float f = 2.0f / DIV;
  // move base points along normal
  if( a->z == 0 ) {
    // move the anchor out if on bottom
    a->x += f * normal.x;
    a->y += f * normal.y;
  }

  // for flat shapes only, pull out the middle point
  if( toint( normal.x ) == 0 || toint( normal.y ) == 0 ) {
    if( b1->x == a->x || b1->y == a->y ) {
      b1->x += f * normal.x;
      b1->y += f * normal.y;
    } else {
      b2->x += f * normal.x;
      b2->y += f * normal.y;
    }
  }
}

void GLCaveShape::dividePolys() {
  for( int i = 0; i < (int)polys.size(); i++ ) {
    vector<CaveFace*> *v = polys[i];
    int originalSize = (int)v->size();
    for( int t = 0; t < originalSize; t++ ) {
      CaveFace *face = (*v)[t];
      if( points[face->p1]->z == points[face->p2]->z &&
          points[face->p2]->z == points[face->p3]->z ) continue;
      
      // create new points
      int index = points.size();
      CVector3 *n1 = divideSegment( points[face->p1], points[face->p2] );
      CVector3 *n2 = divideSegment( points[face->p2], points[face->p3] );
      CVector3 *n3 = divideSegment( points[face->p3], points[face->p1] );

      bulgePoints( n1, n2, n3 );

      points.push_back( n1 );
      points.push_back( n2 );
      points.push_back( n3 );

      // 3 new triangles
      v->push_back( new CaveFace( face->p1, index, index + 2, 
                                  face->tex[0][0], face->tex[0][1],
                                  ( face->tex[0][0] + face->tex[1][0] ) / 2.0f, ( face->tex[0][1] + face->tex[1][1] ) / 2.0f, 
                                  ( face->tex[0][0] + face->tex[2][0] ) / 2.0f, ( face->tex[0][1] + face->tex[2][1] ) / 2.0f, 
                                  face->textureType ) );
      v->push_back( new CaveFace( index, face->p2, index + 1, 
                                  ( face->tex[0][0] + face->tex[1][0] ) / 2.0f, ( face->tex[0][1] + face->tex[1][1] ) / 2.0f, 
                                  face->tex[1][0], face->tex[1][1],
                                  ( face->tex[1][0] + face->tex[2][0] ) / 2.0f, ( face->tex[1][1] + face->tex[2][1] ) / 2.0f, 
                                  face->textureType ) );
      v->push_back( new CaveFace( index + 2, index + 1, face->p3,
                                  ( face->tex[0][0] + face->tex[2][0] ) / 2.0f, ( face->tex[0][1] + face->tex[2][1] ) / 2.0f, 
                                  ( face->tex[1][0] + face->tex[2][0] ) / 2.0f, ( face->tex[1][1] + face->tex[2][1] ) / 2.0f, 
                                  face->tex[2][0], face->tex[2][1],
                                  face->textureType ) );
      // the middle one replaces the current face
      face->p1 = index;
      face->p2 = index + 1;
      face->p3 = index + 2;
      face->tex[0][0] = ( face->tex[0][0] + face->tex[1][0] ) / 2.0f;
      face->tex[0][1] = ( face->tex[0][1] + face->tex[1][1] ) / 2.0f;
      face->tex[1][0] = ( face->tex[1][0] + face->tex[2][0] ) / 2.0f;
      face->tex[1][1] = ( face->tex[1][1] + face->tex[2][1] ) / 2.0f;
      face->tex[2][0] = ( face->tex[0][0] + face->tex[2][0] ) / 2.0f;
      face->tex[2][1] = ( face->tex[0][1] + face->tex[2][1] ) / 2.0f;
    }
  }
}

GLCaveShape *GLCaveShape::shapeList[ CAVE_INDEX_COUNT ];

#define _point(_x,_y,_z) ( {\
  CVector3 *v = new CVector3();\
  v->x=_x; v->y=_y; v->z=_z;\
  points.push_back( v );\
} )
#define _poly(_index,_p1,_p2,_p3,_u1,_v1,_u2,_v2,_u3,_v3,_tt) ( {\
  polys[_index]->push_back(new CaveFace(_p1,_p2,_p3,_u1,_v1,_u2,_v2,_u3,_v3,_tt));\
} )

void GLCaveShape::createShapes( GLuint texture[], int shapeCount, Shapes *shapes ) {
  for( int i = 0; i < CAVE_INDEX_COUNT - LAVA_SIDE_W; i++ ) {
    lavaTex[i] = 0;
    lavaData[i] = (GLubyte*)malloc( 4 * 256 * 256 );
  }

  createOverlayTexture();

  float w = (float)CAVE_CHUNK_SIZE / DIV;
  float d = (float)CAVE_CHUNK_SIZE / DIV;
  float h = (float)MAP_WALL_HEIGHT / DIV; // fixme: for floor it should be 0
  if (h == 0) h = 0.25 / DIV;

  for( int i = 0; i < CAVE_INDEX_COUNT; i++ ) {
    polys.push_back( new vector<CaveFace*>() );
  }

  // store the points: note, that the order matters! (when calc. normals)
  // DIR_S flat
  _point( w, 0, h );
  _point( w, d, 0 );
  _point( 0, d, 0 );
  _point( 0, 0, h );  
  _poly( CAVE_INDEX_S, 0, 2, 1, 0, 1, 1, 0, 1, 1, CaveFace::WALL );  
  _poly( CAVE_INDEX_S, 0, 3, 2, 0, 1, 0, 0, 1, 0, CaveFace::WALL );

  // DIR_N flat
  _point( w, d, h );
  _point( w, 0, 0 );
  _point( 0, 0, 0 );
  _point( 0, d, h );
  _poly( CAVE_INDEX_N, 4, 5, 6, 0, 1, 1, 1, 1, 0, CaveFace::WALL );
  _poly( CAVE_INDEX_N, 4, 6, 7, 0, 1, 1, 0, 0, 0, CaveFace::WALL );

  // DIR_E flat
  _point( 0, d, h );
  _point( w, d, 0 );
  _point( w, 0, 0 );
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
  _point( 0, d, h );
  _point( w, d, 0 );
  _point( 0, 0, 0 );
  _poly( CAVE_INDEX_NE, 16, 17, 18, 1, 1, 0, 1, 0.5f, 0, CaveFace::WALL );

  // DIR_SE corner
  _point( 0, 0, h );
  _point( 0, d, 0 );
  _point( w, 0, 0 );
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

  // DIR_NE inverse top
  _point( 0, 0, h );
  _point( w, d, h );
  _point( 0, d, h );
  _poly( CAVE_INDEX_INV_NE, 30, 29, 28, 0, 1, 1, 1, 0, 0, CaveFace::TOP );

  // DIR_SE inverse top
  _point( w, 0, h );
  _point( 0, d, h );
  _point( 0, 0, h );
  _poly( CAVE_INDEX_INV_SE, 33, 32, 31, 0, 0, 0, 1, 1, 0, CaveFace::TOP );
    
  // DIR_SW  inverse top
  _point( 0, 0, h );
  _point( w, d, h );
  _point( w, 0, h );
  _poly( CAVE_INDEX_INV_SW, 34, 35, 36, 0, 0, 1, 1, 1, 0, CaveFace::TOP );
    
  // DIR_NW  inverse top
  _point( w, 0, h );
  _point( 0, d, h );
  _point( w, d, h );
  _poly( CAVE_INDEX_INV_NW, 37, 38, 39, 1, 0, 0, 1, 1, 1, CaveFace::TOP );

  // DIR_NE inverse side
  _point( 0, 0, h );
  _point( w, d, h );
  _point( w, 0, 0 );
  _poly( CAVE_INDEX_INV_NE, 40, 41, 42, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );
  
  // DIR_SE inverse side
  _point( w, 0, h );
  _point( 0, d, h );
  _point( w, d, 0 );  
  _poly( CAVE_INDEX_INV_SE, 43, 44, 45, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );
  
  // DIR_SW inverse side
  _point( 0, d, 0 );
  _point( w, d, h );
  _point( 0, 0, h );
  _poly( CAVE_INDEX_INV_SW, 46, 47, 48, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );
  
  // DIR_NW inverse side
  _point( 0, 0, 0 );
  _point( 0, d, h );
  _point( w, 0, h );
  _poly( CAVE_INDEX_INV_NW, 49, 50, 51, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );
  
  // DIR_CROSS_NW inverse side
  _point( w, 0, h );
  _point( 0, d, h );
  _point( 0, 0, 0 );
  _poly( CAVE_INDEX_CROSS_NW, 52, 53, 54, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );

  _point( w, 0, h );
  _point( 0, d, h );
  _point( w, d, 0 );
  _poly( CAVE_INDEX_CROSS_NW, 55, 56, 57, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );
  
  // DIR_CROSS_NE inverse side
  _point( 0, 0, h );
  _point( w, d, h );
  _point( w, 0, 0 );
  _poly( CAVE_INDEX_CROSS_NE, 58, 59, 60, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );

  _point( 0, 0, h );
  _point( w, d, h );
  _point( 0, d, 0 );
  _poly( CAVE_INDEX_CROSS_NE, 61, 62, 63, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );

  // Remove dup. points: this creates a triangle mesh
  removeDupPoints();

  dividePolys();

  removeDupPoints();

  calculateNormals();

  calculateLight();


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

  for( int i = LAVA_SIDE_W; i < CAVE_INDEX_COUNT; i++ ) {
    shapeList[i] = 
      new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                       strdup( names[ i ] ), shapeCount++,
                       MODE_LAVA, 0, i, i - LAVA_SIDE_W );
  }
    
  for( int i = 0; i < CAVE_INDEX_COUNT; i++ ) {
    shapeList[i]->setSkipSide(false);
    if( i < CAVE_INDEX_BLOCK || i > CAVE_INDEX_FLOOR ) {
      shapeList[i]->setStencil( false );
      shapeList[i]->setLightBlocking( true );
    } else {
      shapeList[i]->setStencil( false );
      shapeList[i]->setLightBlocking( false );
    }
  }
}

void GLCaveShape::initializeShapes( Shapes *shapes ) {
  for( int i = 0; i < CAVE_INDEX_COUNT; i++ ) {
    //if( !headless ) 
      shapeList[i]->initialize();
  }
  
  // create lava textures
  createLavaTexture( shapes, LAVA_SIDE_W, shapes->getStencilImage( Shapes::STENCIL_SIDE ), 0 );
  createLavaTexture( shapes, LAVA_SIDE_E, shapes->getStencilImage( Shapes::STENCIL_SIDE ), 180 );
  createLavaTexture( shapes, LAVA_SIDE_N, shapes->getStencilImage( Shapes::STENCIL_SIDE ), 90 );
  createLavaTexture( shapes, LAVA_SIDE_S, shapes->getStencilImage( Shapes::STENCIL_SIDE ), 270 );

  createLavaTexture( shapes, LAVA_OUTSIDE_TURN_NW, shapes->getStencilImage( Shapes::STENCIL_OUTSIDE_TURN ), 0 );
  createLavaTexture( shapes, LAVA_OUTSIDE_TURN_NE, shapes->getStencilImage( Shapes::STENCIL_OUTSIDE_TURN ), 90 );
  createLavaTexture( shapes, LAVA_OUTSIDE_TURN_SE, shapes->getStencilImage( Shapes::STENCIL_OUTSIDE_TURN ), 180 );
  createLavaTexture( shapes, LAVA_OUTSIDE_TURN_SW, shapes->getStencilImage( Shapes::STENCIL_OUTSIDE_TURN ), 270 );

  createLavaTexture( shapes, LAVA_U_N, shapes->getStencilImage( Shapes::STENCIL_U ), 0 );
  createLavaTexture( shapes, LAVA_U_E, shapes->getStencilImage( Shapes::STENCIL_U ), 90 );
  createLavaTexture( shapes, LAVA_U_S, shapes->getStencilImage( Shapes::STENCIL_U ), 180 );
  createLavaTexture( shapes, LAVA_U_W, shapes->getStencilImage( Shapes::STENCIL_U ), 270 );

  createLavaTexture( shapes, LAVA_SIDES_EW, shapes->getStencilImage( Shapes::STENCIL_SIDES ), 0 );
  createLavaTexture( shapes, LAVA_SIDES_NS, shapes->getStencilImage( Shapes::STENCIL_SIDES ), 90 );

  createLavaTexture( shapes, LAVA_ALL, shapes->getStencilImage( Shapes::STENCIL_ALL ), 0 );
}

void GLCaveShape::createLavaTexture( Shapes *shapes, int index, GLubyte *stencil, int rot ) {

  GLubyte *lavaThemeData = shapes->getCurrentTheme()->getLavaData();

  if( lavaTex[ index - LAVA_SIDE_W ] ) {
    //free( lavaData[ index ] );
    glDeleteTextures( 1, &( lavaTex[ index - LAVA_SIDE_W ] ) );
  }
  glGenTextures(1, (GLuint*)&( lavaTex[ index - LAVA_SIDE_W ] ));
  for( int x = 0; x < 256; x++ ) {
    for( int y = 0; y < 256; y++ ) {
      GLubyte b;
      switch( rot ) {
      case 270: b = ( stencil[ ( x * 256 ) + ( 255 - y ) ] ); break;
      case 180: b = ( stencil[ ( 255 - x ) + ( ( 255 - y ) * 256 ) ] ); break;
      case 90: b = ( stencil[ ( ( 255 - x ) * 256 ) + y ] ); break;
      default: b = ( stencil[ x + y * 256 ] ); break;
      }

      switch( b ) {
      case 2:
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 0 ] = 
        ( lavaThemeData ? lavaThemeData[ ( ( 3 * x ) + ( y * 256 * 3 ) ) + 0 ] : 
          (GLubyte)0xff );
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 1 ] = 
        ( lavaThemeData ? lavaThemeData[ ( ( 3 * x ) + ( y * 256 * 3 ) ) + 1 ] : 
          (GLubyte)0x00 );
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 2 ] = 
        ( lavaThemeData ? lavaThemeData[ ( ( 3 * x ) + ( y * 256 * 3 ) ) + 2 ] : 
          (GLubyte)0x00 );
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 3 ] = 
        (GLubyte)0xff;
      break;
      case 1:
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 0 ] = 
        (GLubyte)0x00;
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 1 ] = 
        (GLubyte)0x00;
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 2 ] = 
        (GLubyte)0x00;
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 3 ] = 
        (GLubyte)0xff;
      break;
      default:
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 0 ] = 
        (GLubyte)0x00;
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 1 ] = 
        (GLubyte)0x00;
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 2 ] = 
        (GLubyte)0x00;
      lavaData[ index - LAVA_SIDE_W ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 3 ] = 
        (GLubyte)0x00;
      }
    }
  }

  glBindTexture( GL_TEXTURE_2D, lavaTex[ index - LAVA_SIDE_W ] );
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  
  //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, 
               GL_RGBA, GL_UNSIGNED_BYTE, lavaData[ index - LAVA_SIDE_W ] );

}

#define ER 0
#define EG 0
#define EB 0

void GLCaveShape::createOverlayTexture() {

  float half = ((float)OVERLAY_SIZE - 0.5f) / 2.0f;
  int maxP = 75;
  int minP = 50;

  // create the dark texture
  glGenTextures(1, (GLuint*)&overlay_tex);
  for( unsigned int i = 0; i < OVERLAY_SIZE; i++) {
    for( unsigned int j = 0; j < OVERLAY_SIZE; j++) {
      
      float id = (float)i - half;
      float jd = (float)j - half;

      // the distance
      float dist = sqrt( id * id + jd * jd );

      // the distance as a percent of the max distance
      float percent = dist / ( sqrt( half * half ) / 100.0f );

      int r, g, b;
      if( percent < minP ) {
        r = 0xff;
        g = 0xff;
        b = 0xff;
      } else if( percent >= maxP ) {
        r = ER;
        g = EG;
        b = EB;
      } else {        
        r = 0xff - 
          (int)( (float)( percent - minP ) * 
                 ( (float)( 0xff - ER ) / (float)( maxP - minP ) ) );
        if( r < ER ) r = ER;
        g = 0xff - 
          (int)( (float)( percent - minP ) * 
                 ( (float)( 0xff - EG ) / (float)( maxP - minP ) ) );
        if( g < EG ) g = EG;
        b = 0xff - 
          (int)( (float)( percent - minP ) * 
                 ( (float)( 0xff - EB ) / (float)( maxP - minP ) ) );
        if( b < EB ) b = EB;
      }
      overlay_data[i * OVERLAY_SIZE * 3 + j * 3 + 0] = (unsigned char)r;
      overlay_data[i * OVERLAY_SIZE * 3 + j * 3 + 1] = (unsigned char)g;
      overlay_data[i * OVERLAY_SIZE * 3 + j * 3 + 2] = (unsigned char)b;
    }
  }
  glBindTexture(GL_TEXTURE_2D, overlay_tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, OVERLAY_SIZE, OVERLAY_SIZE, 0, 
			   GL_RGB, GL_UNSIGNED_BYTE, overlay_data);
}

