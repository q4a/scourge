/***************************************************************************
                          fog.cpp  -  description
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

#include "fog.h"
#include "glshape.h"
#include "frustum.h"

using namespace std;

#define FOG_CHUNK_SIZE 4

#define FOG_WIDTH ( MAP_WIDTH / FOG_CHUNK_SIZE )
#define FOG_DEPTH ( MAP_DEPTH / FOG_CHUNK_SIZE )


Fog::Fog() {
  reset();
}

Fog::~Fog() {
}

void Fog::reset() {
  for( int x = 0; x < FOG_WIDTH; x++ ) {
    for( int y = 0; y < FOG_DEPTH; y++ ) {
      fog[x][y] = FOG_UNVISITED;
    }
  }
}

int Fog::getValue( int mapx, int mapy ) { 
  return fog[mapx / FOG_CHUNK_SIZE][mapy / FOG_CHUNK_SIZE]; 
}

void Fog::visit( int mapx, int mapy ) {
  int fx = mapx / FOG_CHUNK_SIZE;
  int fy = mapy / FOG_CHUNK_SIZE;
  for( int x = 0; x < FOG_WIDTH; x++ ) {
    for( int y = 0; y < FOG_DEPTH; y++ ) {
      if( abs( x - fx ) <= 2 && abs( y - fy ) <= 2 ) {
        fog[x][y] = FOG_CLEAR;
      } else if( fog[x][y] == FOG_CLEAR ) {
        fog[x][y] = FOG_VISITED;
      }
    }
  }
}

void Fog::draw( int sx, int sy, int w, int h, CFrustum *frustum ) {
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_CULL_FACE );    
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  
  int fx = sx / FOG_CHUNK_SIZE;
  int fy = sy / FOG_CHUNK_SIZE;
  int fw = w / FOG_CHUNK_SIZE;
  int fh = h / FOG_CHUNK_SIZE;
  float nn = FOG_CHUNK_SIZE / DIV;
  int ox = sx % FOG_CHUNK_SIZE;
  int oy = sy % FOG_CHUNK_SIZE;

  for( int x = 0; x < fw; x ++ ) {
    for( int y = 0; y < fh; y ++ ) {
      int v = fog[ fx + x ][ fy + y ];
      switch( v ) {
      case FOG_CLEAR: continue;
      case FOG_UNVISITED: glColor4f( 0, 0, 0, 0 ); break;
      case FOG_VISITED:glColor4f( 0.5f, 0.5f, 0.5f, 0.5f ); break;
      }
      
      float xp = (float)( x * FOG_CHUNK_SIZE - ox ) / DIV;
      float yp = (float)( y * FOG_CHUNK_SIZE - oy ) / DIV;

      if( !frustum->CubeInFrustum( xp, yp, 0.0f, nn ) ) continue;

      glBegin( GL_QUADS );
      glVertex2f( xp,      yp );
      glVertex2f( xp,      yp + nn );
      glVertex2f( xp + nn, yp + nn );
      glVertex2f( xp + nn, yp );
      glEnd();
    }
  }

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
}

int Fog::getVisibility( int xp, int yp, Shape *shape ) {
  int v = FOG_UNVISITED;
  for( int x = 0; x < shape->getWidth(); x++ ) {
    for( int y = 0; y < shape->getDepth(); y++ ) {
      int vv = getValue( xp + x, yp - y );
      if( vv == FOG_CLEAR ) return FOG_CLEAR;
      else if( vv == FOG_VISITED ) v = vv;
    }
  }
  return v;
}

