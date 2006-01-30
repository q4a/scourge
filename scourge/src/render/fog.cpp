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
#include "map.h"
#include "location.h"
#include "mapadapter.h"

// FIXME: remove this later
#include "../scourge.h"

using namespace std;

//#define DEBUG_FOG 1

#define FOG_CHUNK_SIZE 4

#define FOG_WIDTH ( MAP_WIDTH / FOG_CHUNK_SIZE )
#define FOG_DEPTH ( MAP_DEPTH / FOG_CHUNK_SIZE )


Fog::Fog( Map *map, GLuint texture ) {
  this->map = map;
  this->texture = texture;
  this->quadric = gluNewQuadric();
  reset();
}

Fog::~Fog() {
  gluDeleteQuadric( quadric );
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
      double d = (double)( ( fx - x ) * ( fx - x) ) + 
        (double)( ( fy - y ) * ( fy - y ) );
      if( d <= 25.0f ) {
        fog[x][y] = FOG_CLEAR;
      } else if( fog[x][y] == FOG_CLEAR ) {
        fog[x][y] = FOG_VISITED;
      }
    }
  }
}

void Fog::draw( int sx, int sy, int w, int h, CFrustum *frustum ) {  
  int fx = sx / FOG_CHUNK_SIZE;
  int fy = sy / FOG_CHUNK_SIZE;
  int fw = w / FOG_CHUNK_SIZE;
  int fh = h / FOG_CHUNK_SIZE;
  float nn = FOG_CHUNK_SIZE / DIV;
  int ox = sx % FOG_CHUNK_SIZE;
  int oy = sy % FOG_CHUNK_SIZE;

  bool e[1000];
  int f[1000];
  GLfloat p[1000][4];
  int pCount = 0;
  for( int x = 0; x < fw; x ++ ) {
    for( int y = 0; y < fh; y ++ ) {
      int v = fog[ fx + x ][ fy + y ];
      switch( v ) {
      case FOG_CLEAR: continue;
      case FOG_UNVISITED: glColor4f( 1, 1, 1, 0.05f); break;
      case FOG_VISITED:  glColor4f( 1, 1, 1, 0.5f); break;
      }
      
      float xp = (float)( x * FOG_CHUNK_SIZE - ox ) / DIV;
      float yp = (float)( y * FOG_CHUNK_SIZE - oy ) / DIV;
      int z = getHighestZ( ( fx + x ) * FOG_CHUNK_SIZE, 
                           ( fy + y ) * FOG_CHUNK_SIZE, 
                           FOG_CHUNK_SIZE,
                           FOG_CHUNK_SIZE );
      float zp = (float)( z ) / DIV;

      if( !frustum->CubeInFrustum( xp, yp, 0.0f, nn ) ) continue;

      // get all screen points of the bounding box; draw bounding rectangle on screen
      float obj[20][3] = {
        { xp, yp, zp },
        { xp, yp+nn, zp },
        { xp+nn, yp+nn, zp },
        { xp+nn, yp, zp },

        { xp, yp, zp },
        { xp, yp, 0 },
        { xp+nn, yp, 0 },
        { xp+nn, yp, zp },

        { xp, yp+nn, zp },
        { xp, yp+nn, 0 },
        { xp+nn, yp+nn, 0 },
        { xp+nn, yp+nn, zp },

        { xp, yp, zp },
        { xp, yp, 0 },
        { xp, yp+nn, 0 },
        { xp, yp+nn, zp },

        { xp+nn, yp, zp },
        { xp+nn, yp, 0 },
        { xp+nn, yp+nn, 0 },
        { xp+nn, yp+nn, zp }
      };
      GLfloat maxScX, maxScY;
      GLfloat minScX, minScY;

      maxScX = maxScY = 0;
      minScX = minScY = 2000;
      for( int i = 0; i < 20; i++ ) {
        GLdouble scx, scy;
        getScreenXY( (GLdouble)obj[i][0], (GLdouble)obj[i][1], (GLdouble)obj[i][2], 
                     &scx, &scy );
        if( scx < minScX ) minScX = (GLfloat)scx;
        if( scx > maxScX ) maxScX = (GLfloat)scx;
        if( scy < minScY ) minScY = (GLfloat)scy;
        if( scy > maxScY ) maxScY = (GLfloat)scy;
      }

      f[pCount] = v;
      p[pCount][0] = minScX;
      p[pCount][1] = minScY;
      p[pCount][2] = maxScX - minScX;
      p[pCount][3] = maxScY - minScY;

      e[pCount] = false;
      if( //v != fog[ fx + x - 1 ][ fy + y - 1 ] ||
          v != fog[ fx + x ][ fy + y - 1 ] ||
          //v != fog[ fx + x + 1 ][ fy + y - 1 ] ||
          v != fog[ fx + x - 1 ][ fy + y ] ||
          v != fog[ fx + x + 1 ][ fy + y ] ||
          //v != fog[ fx + x - 1 ][ fy + y + 1 ] ||
          v != fog[ fx + x ][ fy + y + 1 ] //||
          //v != fog[ fx + x + 1 ][ fy + y + 1 ] 
          ) {
        e[pCount] = true;
      }

      pCount++;
      if( pCount >= 1000 ) break;
    }
  }




  glDisable( GL_TEXTURE_2D );
  glDisable( GL_CULL_FACE );    
  glDisable(GL_DEPTH_TEST);
  
  //glBindTexture( GL_TEXTURE_2D, texture );
  
  // stencil buffer
  glClear( GL_STENCIL_BUFFER_BIT );
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_EQUAL, 0, 0xffffffff);
  glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

  glEnable( GL_BLEND );
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  glColor4f( 0.65f, 0.45f, 0.60f, 0.5f);


  glPushMatrix();
  glLoadIdentity();
  for( int t = 0; t < 2; t++ ) {
    if( t == 1 ) {
      glDisable( GL_STENCIL_TEST );
      glDisable( GL_BLEND );
      glColor4f( 0.08f, 0.03f, 0.07f, 0.5f);
    }
    for( int i = 0; i < pCount; i++ ) {
  
      if( t == 1 && f[i] != FOG_UNVISITED ) continue;
      else if( t == 0 && f[i] != FOG_VISITED ) continue;
      
      GLfloat x = p[i][0];
      GLfloat y = p[i][1];
      GLfloat w = p[i][2];
      GLfloat h = p[i][3];

      if( e[i] ) {
        // 1 big circle
        glTranslatef( x + w/2, y + h/2, 0 );
        gluDisk( quadric, 0, w, 8, 1);
        glTranslatef( -(x + w/2), -(y + h/2), 0 );
      } else {
        glBegin( GL_QUADS );
        glVertex2f( x, y );
        glVertex2f( x, y + h );
        glVertex2f( x + w, y + h );
        glVertex2f( x + w, y );
        glEnd();
      }

    }
  }
  glPopMatrix();

#ifdef DEBUG_FOG 
  glLoadIdentity();               
  glDisable( GL_BLEND );
  glColor3f( 1, 1, 1 );
  for( int i = 0; i < pCount; i++ ) {
    glBegin( GL_LINE_LOOP );
    glVertex2f( p[i][0], p[i][1] );
    glVertex2f( p[i][0], p[i][1] + p[i][3] );
    glVertex2f( p[i][0] + p[i][2], p[i][1] + p[i][3] );
    glVertex2f( p[i][0] + p[i][2], p[i][1] );
    glEnd();
  }
  glEnable( GL_BLEND );
#endif              

  glEnable( GL_BLEND );
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
}

// FIXME: highest static point can be stored in the fog[][] struct
int Fog::getHighestZ( int sx, int sy, int w, int h ) {
  int z = 0;
  for( int x = sx; x < sx + w; x++ ) {
    for( int y = sy; y < sy + h; y++ ) {
      Location *pos = map->getLocation( x, y, 0 );
      if( pos && pos->shape ) {
        int zz = pos->z + pos->shape->getHeight();
        if( zz > z ) z = zz;
      }
    }
  }
  return z;
}

void Fog::getScreenXY( GLdouble mapx, GLdouble mapy, GLdouble mapz,
                       GLdouble *screenx, GLdouble *screeny ) {
  GLdouble screenz;
  
  double projection[16];
  double modelview[16];
  GLint viewport[4];
  
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetIntegerv(GL_VIEWPORT, viewport);
  
  int res = gluProject(mapx, mapy, mapz,
                       modelview,
                       projection,
                       viewport,
                       screenx, screeny, &screenz);
  *screeny = ( map->getAdapter()->getScreenHeight() - *screeny );
  if(!res) {
    *screenx = *screeny = 2000;
  }
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

