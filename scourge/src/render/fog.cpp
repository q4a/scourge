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
#include "renderedcreature.h"

// FIXME: remove this later
#include "../scourge.h"

using namespace std;

//#define DEBUG_FOG 1

// how many map spaces in a fog chunk
#define FOG_CHUNK_SIZE 4

#define FOG_WIDTH ( MAP_WIDTH / FOG_CHUNK_SIZE )
#define FOG_DEPTH ( MAP_DEPTH / FOG_CHUNK_SIZE )

// color of transparent fog
//#define ER 118
//#define EG 110
//#define EB 130

#define ER 85
#define EG 80
#define EB 150

//#define DARK_R 0.05f
//#define DARK_G 0.04f
//#define DARK_B 0.055f

#define DARK_R 0.0f
#define DARK_G 0.0f
#define DARK_B 0.0f

// inverse color of dark shade
#define SR static_cast<int>( 255.0f * DARK_R )
#define SG static_cast<int>( 255.0f * DARK_G )
#define SB static_cast<int>( 255.0f * DARK_B )

//#define LAMP_RADIUS_SQUARED 36.0f
//#define LAMP_RADIUS_SQUARED 64.0f

Fog::Fog( Map *map, float lampRadiusSquared ) {
	this->lampRadiusSquared = lampRadiusSquared;
  players = new std::set<RenderedCreature *> [MAP_WIDTH * MAP_DEPTH];
  this->map = map;
  createOverlayTexture();
  createShadeTexture();
  reset();
}

Fog::~Fog() {
  glDeleteTextures(1, (GLuint*)&overlay_tex);
  glDeleteTextures(1, (GLuint*)&shade_tex);
  delete [] players;
}

void Fog::reset() {
  for( int x = 0; x < FOG_WIDTH; x++ ) {
    for( int y = 0; y < FOG_DEPTH; y++ ) {
      fog[x][y] = FOG_UNVISITED;
      players[x + (y * MAP_WIDTH)].clear();
    }
  }
}

int Fog::getValue( int mapx, int mapy ) { 
  return fog[mapx / FOG_CHUNK_SIZE][mapy / FOG_CHUNK_SIZE]; 
}

void Fog::visit( RenderedCreature *player ) {
  for( int x = 0; x < FOG_WIDTH; x++ ) {
    for( int y = 0; y < FOG_DEPTH; y++ ) {

      int fx = toint( player->getX() / FOG_CHUNK_SIZE );
      int fy = toint( player->getY() / FOG_CHUNK_SIZE );

      double d = static_cast<double>( ( fx - x ) * ( fx - x) ) + static_cast<double>( ( fy - y ) * ( fy - y ) );
      if( d <= lampRadiusSquared ) {
        fog[x][y] = FOG_CLEAR;
        players[x + (y * MAP_WIDTH)].insert( player );
      } else if( fog[x][y] == FOG_CLEAR ) {
        players[x + (y * MAP_WIDTH)].erase( player );
        if( players[x + (y * MAP_WIDTH)].empty() ) {
          fog[x][y] = FOG_VISITED;
        }
      }
    }
  }
}

void Fog::hideDeadParty() {
  for( int i = 0; i < map->getAdapter()->getPartySize(); i++ ) {
    if( map->getAdapter()->getParty(i)->getStateMod( StateMod::dead ) ) {
      for( int x = 0; x < FOG_WIDTH; x++ ) {
        for( int y = 0; y < FOG_DEPTH; y++ ) {
          if( fog[x][y] == FOG_CLEAR ) {
            players[x + (y * MAP_WIDTH)].erase( map->getAdapter()->getParty(i) );
            if( players[x + (y * MAP_WIDTH)].empty() ) {
              fog[x][y] = FOG_VISITED;
            }
          }
        }
      }
    }
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

void Fog::draw( int sx, int sy, int w, int h, CFrustum *frustum ) {  
  int fx = sx / FOG_CHUNK_SIZE;
  int fy = sy / FOG_CHUNK_SIZE;
  int fw = w / FOG_CHUNK_SIZE;
  int fh = h / FOG_CHUNK_SIZE;
  float nn = FOG_CHUNK_SIZE / DIV;
  int ox = sx % FOG_CHUNK_SIZE;
  int oy = sy % FOG_CHUNK_SIZE;

  GLfloat minLightX, minLightY, maxLightX, maxLightY;
  minLightX = minLightY = 2000;
  maxLightX = maxLightY = 0;
  bool e[1000];
  int f[1000];
  GLfloat p[1000][4];
  int pCount = 0;
  for( int x = 0; x < fw; x ++ ) {
    for( int y = 0; y < fh; y ++ ) {
      int v = fog[ fx + x ][ fy + y ];
      if( v == FOG_VISITED ) continue;

      float xp = static_cast<float>( x * FOG_CHUNK_SIZE - ox ) / DIV;
      float yp = static_cast<float>( y * FOG_CHUNK_SIZE - oy ) / DIV;
      int z = getHighestZ( ( fx + x ) * FOG_CHUNK_SIZE, ( fy + y ) * FOG_CHUNK_SIZE, FOG_CHUNK_SIZE, FOG_CHUNK_SIZE );
      float zp = static_cast<float>( z ) / DIV;

      // FIXME: we should check 2d inclusion in screen rect instead
      if( !frustum->CubeInFrustum( xp, yp, 0.0f, nn ) )
        continue;

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
        getScreenXY( (GLdouble)obj[i][0], (GLdouble)obj[i][1], (GLdouble)obj[i][2], &scx, &scy );

        // only show light area for current player
        if( v == FOG_CLEAR &&
            players[x + fx + ((y + fy) * MAP_WIDTH)].find( map->getAdapter()->getPlayer() ) != players[x + fx + ((y + fy) * MAP_WIDTH)].end() ) {
          if( scx < minLightX ) minLightX = (GLfloat)scx;
          if( scx > maxLightX ) maxLightX = (GLfloat)scx;
          if( scy < minLightY ) minLightY = (GLfloat)scy;
          if( scy > maxLightY ) maxLightY = (GLfloat)scy;
        } else {
          if( scx < minScX ) minScX = (GLfloat)scx;
          if( scx > maxScX ) maxScX = (GLfloat)scx;
          if( scy < minScY ) minScY = (GLfloat)scy;
          if( scy > maxScY ) maxScY = (GLfloat)scy;
        }
      }

      if( v == FOG_CLEAR )
        continue;

      f[pCount] = v;
      p[pCount][0] = minScX;
      p[pCount][1] = minScY;
      p[pCount][2] = maxScX - minScX;
      p[pCount][3] = maxScY - minScY;

      e[pCount] = false;
      if( v != fog[ fx + x ][ fy + y - 1 ] || v != fog[ fx + x - 1 ][ fy + y ] ||
          v != fog[ fx + x + 1 ][ fy + y ] || v != fog[ fx + x ][ fy + y + 1 ] ) {
        e[pCount] = true;
      }

      pCount++;
      if( pCount >= 1000 ) break;
    }
  }



  // ***************************
  // DRAW IT!

  glDisable( GL_TEXTURE_2D );
  glDisable( GL_CULL_FACE );    
  glDisable(GL_DEPTH_TEST);
  
  //glBindTexture( GL_TEXTURE_2D, texture );
  
  glEnable( GL_BLEND );
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
//  glColor4f( 0.65f, 0.45f, 0.60f, 0.5f);
  glColor4f( ER / 255.0f, EG / 255.0f, EB / 255.0f, 0.5f);

  glPushMatrix();
  glLoadIdentity();

  // draw a gray rect.
  if( map->getAdapter()->
      intersects( 0, 0, 
                  map->getAdapter()->getScreenWidth(), 
                  map->getAdapter()->getScreenHeight(), 
                  static_cast<int>(minLightX), static_cast<int>(minLightY),
                  static_cast<int>( maxLightX - minLightX ), 
                  static_cast<int>( maxLightY - minLightY ) ) ) {
    
    
    glBegin( GL_QUADS );
    
    glVertex2f( 0, 0 );
    glVertex2f( 0, minLightY );
    glVertex2f( map->getAdapter()->getScreenWidth(), minLightY );
    glVertex2f( map->getAdapter()->getScreenWidth(), 0 );

    glVertex2f( 0, maxLightY );
    glVertex2f( 0, map->getAdapter()->getScreenHeight() );
    glVertex2f( map->getAdapter()->getScreenWidth(), map->getAdapter()->getScreenHeight() );
    glVertex2f( map->getAdapter()->getScreenWidth(), maxLightY );

    glVertex2f( 0, minLightY );
    glVertex2f( 0, maxLightY );
    glVertex2f( minLightX, maxLightY );
    glVertex2f( minLightX, minLightY );

    glVertex2f( maxLightX, minLightY );
    glVertex2f( maxLightX, maxLightY );
    glVertex2f( map->getAdapter()->getScreenWidth(), maxLightY );
    glVertex2f( map->getAdapter()->getScreenWidth(), minLightY );

    glEnd();
  
    
    // draw the light circle
    glEnable( GL_TEXTURE_2D );
    glLoadIdentity();  
    glColor4f( 1, 1, 1, 0.5f);
    
    glBindTexture( GL_TEXTURE_2D, overlay_tex );
    glBegin( GL_QUADS );
    //  glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f( 0.0f, 0.0f );
    glVertex2f( minLightX, minLightY );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2f( minLightX, maxLightY );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex2f( maxLightX, maxLightY );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex2f( maxLightX, minLightY );
    glEnd();
    glDisable( GL_TEXTURE_2D );
  } else {
    glBegin( GL_QUADS );
    glVertex2f( 0, 0 );
    glVertex2f( 0, map->getAdapter()->getScreenHeight() );
    glVertex2f( map->getAdapter()->getScreenWidth(), 
                map->getAdapter()->getScreenHeight() );
    glVertex2f( map->getAdapter()->getScreenWidth(), 0 );
    glEnd();
  }

  // draw the dark (unvisited) fog
  glLoadIdentity();
  glDisable( GL_BLEND );
  //glColor4f( 0.08f, 0.03f, 0.07f, 0.5f);
  glColor3f( DARK_R, DARK_G, DARK_B );
  for( int i = 0; i < pCount; i++ ) {
    GLfloat x = p[i][0];
    GLfloat y = p[i][1];
    GLfloat w = p[i][2];
    GLfloat h = p[i][3];

    if( e[i] ) {

      glEnable( GL_TEXTURE_2D );
      glEnable( GL_BLEND );
      glBlendFunc(GL_DST_COLOR, GL_ZERO);
      glColor4f( 1, 1, 1, 0.5f );
      glBindTexture( GL_TEXTURE_2D, shade_tex );
      glBegin( GL_QUADS );
      glTexCoord2f( 0, 0 );
      glVertex2f( x - ( w / 2), y - ( h / 2 ) );
      //glVertex2f( x, y );
      glTexCoord2f( 0, 1 );
      glVertex2f( x - ( w / 2), y + h + ( h / 2 ) );
      //glVertex2f( x, y + h );
      glTexCoord2f( 1, 1 );
      glVertex2f( x + w + ( w / 2), y + h + ( h / 2 ) );
      //glVertex2f( x + w, y + h );
      glTexCoord2f( 1, 0 );
      glVertex2f( x + w + ( w / 2), y - ( h / 2 ) );
      //glVertex2f( x + w, y );
      glEnd();
      glDisable( GL_BLEND );
      glDisable( GL_TEXTURE_2D );
      glColor3f( DARK_R, DARK_G, DARK_B );
    } else {
      glBegin( GL_QUADS );
      glVertex2f( x, y );
      glVertex2f( x, y + h );
      glVertex2f( x + w, y + h );
      glVertex2f( x + w, y );
      glEnd();
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

void Fog::createOverlayTexture() {

  float half = (static_cast<float>(OVERLAY_SIZE) - 0.5f) / 2.0f;
  int maxP = 95;
  int minP = 90;

  // create the dark texture
  glGenTextures(1, (GLuint*)&overlay_tex);
  for( unsigned int i = 0; i < OVERLAY_SIZE; i++) {
    for( unsigned int j = 0; j < OVERLAY_SIZE; j++) {
      
      float id = static_cast<float>(i) - half;
      float jd = static_cast<float>(j) - half;

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
          static_cast<int>( static_cast<float>( percent - minP ) * 
                 ( static_cast<float>( 0xff - ER ) / static_cast<float>( maxP - minP ) ) );
        if( r < ER ) r = ER;
        g = 0xff - 
          static_cast<int>( static_cast<float>( percent - minP ) * 
                 ( static_cast<float>( 0xff - EG ) / static_cast<float>( maxP - minP ) ) );
        if( g < EG ) g = EG;
        b = 0xff - 
          static_cast<int>( static_cast<float>( percent - minP ) * 
                 ( static_cast<float>( 0xff - EB ) / static_cast<float>( maxP - minP ) ) );
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

void Fog::createShadeTexture() {

  float half = (static_cast<float>(OVERLAY_SIZE) - 0.5f) / 2.0f;
  int maxP = 90;
  int minP = 70;

  // create the dark texture
  glGenTextures(1, (GLuint*)&shade_tex);
  for( unsigned int i = 0; i < OVERLAY_SIZE; i++) {
    for( unsigned int j = 0; j < OVERLAY_SIZE; j++) {

			float id = static_cast<float>(i) - half;
			float jd = static_cast<float>(j) - half;

			// the distance
			float dist = sqrt( id * id + jd * jd );

			// the distance as a percent of the max distance
			float percent = dist / ( sqrt( half * half ) / 100.0f );

			int r, g, b;
			if( percent < minP ) {
				r = SR;
				g = SG;
				b = SB;
			} else if( percent >= maxP ) {
				r = 0xff;
				g = 0xff;
				b = 0xff;
			} else {
				r = SR + static_cast<int>( static_cast<float>( percent - minP ) * ( static_cast<float>( 0xff - SR ) / static_cast<float>( maxP - minP ) ) );
				if( r > 0xff )
					r = 0xff;
				g = SG + static_cast<int>( static_cast<float>( percent - minP ) * ( static_cast<float>( 0xff - SG ) / static_cast<float>( maxP - minP ) ) );
				if( g > 0xff )
					g = 0xff;
				b = SB + static_cast<int>( static_cast<float>( percent - minP ) * ( static_cast<float>( 0xff - SB ) / static_cast<float>( maxP - minP ) ) );
				if( b > 0xff )
					b = 0xff;
			}
			shade_data[i * OVERLAY_SIZE * 3 + j * 3 + 0] = (unsigned char)r;
			shade_data[i * OVERLAY_SIZE * 3 + j * 3 + 1] = (unsigned char)g;
			shade_data[i * OVERLAY_SIZE * 3 + j * 3 + 2] = (unsigned char)b;
		}
  }
  glBindTexture(GL_TEXTURE_2D, shade_tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, OVERLAY_SIZE, OVERLAY_SIZE, 0, 
			   GL_RGB, GL_UNSIGNED_BYTE, shade_data);
}

void Fog::load( FogInfo *fogInfo ) {
	for( int x = 0; x < MAP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_DEPTH; y++ ) {
			fog[x][y] = fogInfo->fog[x][y];
		}
	}
}

void Fog::save( FogInfo *fogInfo ) {
	for( int x = 0; x < MAP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_DEPTH; y++ ) {
			fogInfo->fog[x][y] = fog[x][y];
		}
	}
}
