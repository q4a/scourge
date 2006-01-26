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

void Fog::draw( int sx, int sy, int w, int h ) {
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_CULL_FACE );    
  //glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );
  glDisable(GL_DEPTH_TEST);
  //glDisable( GL_TEXTURE_2D );
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  for( int x = sx; x < sx + w; x += FOG_CHUNK_SIZE ) {
    for( int y = sy; y < sy + h; y += FOG_CHUNK_SIZE ) {
      int v = getValue( x, y );
      if( v == FOG_CLEAR ) {
        continue;
      } else if( v == FOG_UNVISITED ) {
        glColor4f( 0, 0, 0, 0 );
      } else if( v == FOG_VISITED ) {
        glColor4f( 0.5f, 0.5f, 0.5f, 0.5f );
      }
      glPushMatrix();    
      float xpos2 = (float)(x - sx) / DIV;
      float ypos2 = (float)(y - sy) / DIV;
      //float zpos2 = 16.0f / DIV;
      float zpos2 = 0;
      glTranslatef( xpos2, ypos2, zpos2 );
      glBegin( GL_QUADS );
      glVertex2f( 0, 0 );
      glVertex2f( 0, FOG_CHUNK_SIZE / DIV );
      glVertex2f( FOG_CHUNK_SIZE / DIV, FOG_CHUNK_SIZE / DIV );
      glVertex2f( FOG_CHUNK_SIZE / DIV, 0 );
      glEnd();
      glPopMatrix();
    }
  }
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
}

