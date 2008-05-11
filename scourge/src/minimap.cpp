/***************************************************************************
                          map.h  -  description
                             -------------------
    begin                : Thu Jan 29 2004
    copyright            : (C) 2004 by Daroth-U
    email                : daroth-u@ifrance.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "minimap.h"
#include "render/renderlib.h"
#include "sdlhandler.h"
#include "dungeongenerator.h"
#include "scourge.h"
#include "math.h"
#include "gui/canvas.h"
#include "item.h"
#include "creature.h"
#include "shapepalette.h"
#include "rpg/rpglib.h"

using namespace std;

#define DEBUG_MINIMAP 0
#define MINI_MAP_OFFSET_X 30
#define MINI_MAP_OFFSET_Y 50
#define MINI_MAP_SIZE 60
#define MINI_MAP_BLOCK 4

MiniMap :: MiniMap( Scourge *scourge, bool directMode ){
    this->scourge = scourge;
    this->directMode = directMode;
    showMiniMap = true;                 
    textureSizeH = textureSizeW = 512;
    textureInMemory = NULL;
    mustBuildTexture = true;
}

MiniMap :: ~MiniMap(){
  reset();
}

void MiniMap::reset() {
  if(textureInMemory != NULL){
    free(textureInMemory);
    textureInMemory = NULL;
    // delete the overlay texture
    glDeleteTextures(1, texture);
  }          
  //showMiniMap = true;            
  textureInMemory = NULL;
  mustBuildTexture = true;
}

void MiniMap::prepare() {

  // Create texture and copy minimap date from backbuffer on it    
  textureInMemory = (unsigned char *) malloc( textureSizeW * textureSizeH * 4);    

  glGenTextures(1, texture);    
  glBindTexture(GL_TEXTURE_2D, texture[0]); 
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);        
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);                                          // filtre appliqu� a la texture
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);  
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP ); 
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, textureSizeW, textureSizeH, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, textureInMemory );                       

  // draw the entire map
  glDisable( GL_CULL_FACE );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_TEXTURE_2D );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
 
  glPushMatrix();  
  glLoadIdentity();
  for( int x = 0; x < textureSizeW; x++ ) {
    for( int y = 0; y < textureSizeH; y++ ) {
      glColor3f( 0, 0, 0 );
      glBegin( GL_QUADS );
      glVertex2f( x, y );
      glVertex2f( x, y + 1 );
      glVertex2f( x + 1, y + 1 );
      glVertex2f( x + 1, y );
      glEnd();
    }
  }
  for( int x = 0; x < textureSizeW; x++ ) {
    for( int y = 0; y < textureSizeH; y++ ) {
      Location *pos = scourge->getSession()->getMap()->getLocation( x + MAP_OFFSET, y + MAP_OFFSET, 0 );
      if( pos && pos->shape && !pos->shape->isInteractive() && !( pos->item ) && !( pos->creature ) ) {
        glColor3f( 1, 1, 1 );
        glBegin( GL_QUADS );
        glVertex2f( x, textureSizeH - y );
        glVertex2f( x, textureSizeH - (y + 1) );
        glVertex2f( x + 1, textureSizeH - (y + 1) );
        glVertex2f( x + 1, textureSizeH - y );
        glEnd();
      }
    }
  }

  glPopMatrix();

  // Copy to a texture
  glLoadIdentity();
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  glCopyTexSubImage2D(
                     GL_TEXTURE_2D,
                     0,      // MIPMAP level
                     0,      // x texture offset
                     0,      // y texture offset
                     0,              // x window coordinates
                     scourge->getScreenHeight() - textureSizeH,   // y window coordinates
                     textureSizeW,    // width
                     textureSizeH     // height
                     ); 
  if(DEBUG_MINIMAP){  
    fprintf(stderr, "OpenGl result for minimap texture building : %s\n", Util::getOpenGLError());          
  }
  glPopMatrix();
  //  glPopAttrib();

  glDisable( GL_BLEND );
  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );
}

void MiniMap::drawMap() {
  if( !directMode ) {
    if( mustBuildTexture ) {
      mustBuildTexture = false;
      prepare();
    }
  }
  if( !showMiniMap ) return;

  bool useStencil = 
    ( scourge->getPreferences()->getStencilbuf() && 
      scourge->getPreferences()->getStencilBufInitialized() );

  int sx = scourge->getSession()->getMap()->getX() + 75 - ( MINI_MAP_SIZE / 2 );
  if( sx < MAP_OFFSET ) sx = MAP_OFFSET;
  int sy = scourge->getSession()->getMap()->getY() + 75 - ( MINI_MAP_SIZE / 2 );
  if( sy < MAP_OFFSET ) sy = MAP_OFFSET;
  int ex = sx + MINI_MAP_SIZE;
  if( ex > textureSizeW ) ex = textureSizeW;
  int ey = sy + MINI_MAP_SIZE;
  if( ey > textureSizeH ) ey = textureSizeW;

  glDisable( GL_CULL_FACE );
  glDisable( GL_DEPTH_TEST );
 
  glPushMatrix();  
  glLoadIdentity();
  glTranslatef( MINI_MAP_OFFSET_X, MINI_MAP_OFFSET_Y, 0 );
  glTranslatef( MINI_MAP_SIZE * MINI_MAP_BLOCK / 2, MINI_MAP_SIZE * MINI_MAP_BLOCK / 2, 0 );
  glRotatef( scourge->getSession()->getMap()->getZRot(), 0, 0, 1 );
  glTranslatef( -MINI_MAP_SIZE * MINI_MAP_BLOCK / 2, -MINI_MAP_SIZE * MINI_MAP_BLOCK / 2, 0 );

  if( useStencil ) {
    glClear( GL_STENCIL_BUFFER_BIT );
    glColorMask( 0, 0, 0, 0 );
    glEnable( GL_STENCIL_TEST );
    glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
    glStencilFunc( GL_ALWAYS, 1, 0xffffffff );

    glPushMatrix();
    glDisable(GL_BLEND);
    glEnable( GL_ALPHA_TEST );
    //glAlphaFunc( GL_EQUAL, 0xff );
		glAlphaFunc( GL_NOTEQUAL, 0 );
    glEnable(GL_TEXTURE_2D);
    glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->getMinimapMaskTexture() );
    glColor4f( 1, 1, 1, 1 );
    glBegin( GL_QUADS );   
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, MINI_MAP_SIZE * MINI_MAP_BLOCK ); 
    glTexCoord2f( 1, 1 );
    glVertex2d( MINI_MAP_SIZE * MINI_MAP_BLOCK, MINI_MAP_SIZE * MINI_MAP_BLOCK ); 
    glTexCoord2f( 1, 0 );
    glVertex2d( MINI_MAP_SIZE * MINI_MAP_BLOCK, 0 ); 
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );     
    glEnd();       
    glDisable(GL_TEXTURE_2D); 
    glDisable( GL_ALPHA_TEST );
    glEnable(GL_BLEND);
    glPopMatrix();

    glColorMask( 1, 1, 1, 1 );
    glStencilFunc( GL_EQUAL, 1, 0xffffffff );
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP ); 
    glDepthMask( GL_FALSE );
  }

  if( useStencil && directMode ) {
    glPushMatrix();
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable(GL_TEXTURE_2D);
    glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->getMinimapMaskTexture() );
    glColor4f( 0, 0, 0, 0.5f );
    glBegin( GL_QUADS );   
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, MINI_MAP_SIZE * MINI_MAP_BLOCK ); 
    glTexCoord2f( 1, 1 );
    glVertex2d( MINI_MAP_SIZE * MINI_MAP_BLOCK, MINI_MAP_SIZE * MINI_MAP_BLOCK ); 
    glTexCoord2f( 1, 0 );
    glVertex2d( MINI_MAP_SIZE * MINI_MAP_BLOCK, 0 ); 
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );     
    glEnd();       
    glDisable(GL_TEXTURE_2D); 
    glPopMatrix();
  }

  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  if( !directMode ) {
    // static background: draw as a texture
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    
    float w = MINI_MAP_SIZE * MINI_MAP_BLOCK;
    float h = MINI_MAP_SIZE * MINI_MAP_BLOCK;
    
    // map to percentage of the total size
    float div = textureSizeW;
    // if texture size < map_width we need this for y b/c it's upside down. 
    // x is fine; it starts at 0.
    float yoffs = MAP_OFFSET + textureSizeH - div;   
    float tsx = ( sx - MAP_OFFSET ) / div;
    float tsy = ( sy - yoffs ) / div;
    float tex = ( sx - MAP_OFFSET + MINI_MAP_SIZE ) / div;
    float tey = ( sy - yoffs + MINI_MAP_SIZE ) / div;
    
    glBegin(GL_QUADS); 
    glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
    glTexCoord2f( tsx, tey );
    glVertex2d( 0, h ); 
    glTexCoord2f( tex, tey );
    glVertex2d( w, h ); 
    glTexCoord2f( tex, tsy );
    glVertex2d( w, 0 ); 
    glTexCoord2f( tsx, tsy );
    glVertex2d( 0, 0 );     
    glEnd();       
    glDisable(GL_TEXTURE_2D); 
    glPopMatrix();
  }
    
  // North marker
  if( !useStencil ) {
    glColor4f( 0.5f, 0.5f, 0.5f, 0.5f );
    glBegin( GL_TRIANGLES );
    glVertex2f( 0, 0 );
    glVertex2f( 30, 0 );
    glVertex2f( 0, 30 );
    glEnd();
    glPushMatrix();
    glRotatef( -45, 0, 0, 1 );
    glTranslatef( -7, 20, 0 );
    glScalef( 1.5f, 1.5f, 1.5f );
    glColor4f( 1, 1, 1, 0.5f );
    scourge->getSDLHandler()->texPrint( 0, 0, "N" );
    glScalef( 1, 1, 1 );
    glPopMatrix();
    
    // outline  
    glColor4f( 0.5f, 0.5f, 0.5f, 0.5f );
    glBegin( GL_LINE_LOOP );
    glVertex2f( 0, 0 );
    glVertex2f( 0, MINI_MAP_SIZE * MINI_MAP_BLOCK );
    glVertex2f( MINI_MAP_SIZE * MINI_MAP_BLOCK, MINI_MAP_SIZE * MINI_MAP_BLOCK );
    glVertex2f( MINI_MAP_SIZE * MINI_MAP_BLOCK, 0 );
    glEnd();
  }

  // naive method: draw each block
  for( int x = sx; x < ex; x++ ) {
    if( x < 0 || x >= MAP_WIDTH ) continue;
    for( int y = sy; y < ey; y++ ) {
      if( y < 0 || y >= MAP_DEPTH ) continue;
      Location *pos = scourge->getSession()->getMap()->getLocation( x, y, 0 );
      if( pos && ( pos->creature || pos->item || 
                   ( pos->shape && 
                     ( directMode || pos->shape->isInteractive() ) ) ) ) {
        if( pos->creature ) {
          float mapSize = MINI_MAP_SIZE * MINI_MAP_BLOCK;
          float width = pos->creature->getShape()->getWidth() / 2.0f * MINI_MAP_BLOCK;
          float cx =  ( pos->creature->getX() - sx ) * MINI_MAP_BLOCK + width;
          float cy = ( pos->creature->getY() - sy ) * MINI_MAP_BLOCK - width;
          // clip to minimap
          if( cx - width > 0 && cx + width < mapSize &&
              cy - width > 0 && cy + width < mapSize ) {
            if( pos->creature->isMonster() ) {
              if( ((Creature*)pos->creature)->getMonster()->isNpc() ) {
                glColor4f( 1, 1, 0, 0.5f );
              } else {
                glColor4f( 1, 0, 0, 0.5f );
              }
            } else {
              if( pos->creature == scourge->getSession()->getParty()->getPlayer() ) {
                glColor4f( 1, 0, 1, 0.5f );
              } else {
                glColor4f( 0, 1, 0, 0.5f );
              }
            }
            glPushMatrix();          
            glTranslatef( cx, cy, 0 );
            glRotatef( ((AnimatedShape*)pos->creature->getShape())->getAngle(), 0, 0, 1 );
            glBegin( GL_TRIANGLES );
            glVertex2f( width, width );
            glVertex2f( -width, width );
            glVertex2f( 0, -width );
            glEnd();
            glPopMatrix();
          }
        } else {
          if( pos->item ) {
            glColor4f( 0, 0, 1, 0.5f );
          } else {
            glColor4f( 1, 0.7f, 0, 0.5f );
          }

          float xp = ( x - sx ) * MINI_MAP_BLOCK;
          float yp = ( y - sy ) * MINI_MAP_BLOCK;

          glBegin( GL_QUADS );
          glVertex2f( xp, yp );
          glVertex2f( xp, yp + MINI_MAP_BLOCK );
          glVertex2f( xp + MINI_MAP_BLOCK, yp + MINI_MAP_BLOCK );
          glVertex2f( xp + MINI_MAP_BLOCK, yp );
          glEnd();
        }
      }
    }
  }

  if( useStencil ) {
    glDepthMask( GL_TRUE );
    glDisable(GL_STENCIL_TEST);

    // the cutout  
    glPushMatrix();
    glDisable(GL_BLEND);
    glEnable( GL_ALPHA_TEST );
    //glAlphaFunc( GL_EQUAL, 0xff );
		glAlphaFunc( GL_NOTEQUAL, 0 );
    glEnable(GL_TEXTURE_2D);
    glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->getMinimapTexture() );
    glColor4f( 1, 1, 1, 1 );
    glBegin( GL_QUADS );   
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, MINI_MAP_SIZE * MINI_MAP_BLOCK ); 
    glTexCoord2f( 1, 1 );
    glVertex2d( MINI_MAP_SIZE * MINI_MAP_BLOCK, MINI_MAP_SIZE * MINI_MAP_BLOCK ); 
    glTexCoord2f( 1, 0 );
    glVertex2d( MINI_MAP_SIZE * MINI_MAP_BLOCK, 0 ); 
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );     
    glEnd();       
    glDisable(GL_TEXTURE_2D); 
    glDisable( GL_ALPHA_TEST );
    glEnable(GL_BLEND);
    glPopMatrix();
    
    // draw pointers for gates and teleporters
    if( scourge->getParty() && scourge->getParty()->getPartySize() ) {
    	drawPointers( scourge->getSession()->getMap()->getGates(), Color( 1, 0, 0, 1 ) );
    	drawPointers( scourge->getSession()->getMap()->getTeleporters(), Color( 0, 0, 1, 1 ) );
    }
  }

  glPopMatrix();
  glDisable( GL_BLEND );
  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );
}

void MiniMap::drawPointers( std::set<Location*> *p, Color color ) {
  // player's pos
  float px = scourge->getParty()->getPlayer()->getX();
  float py = scourge->getParty()->getPlayer()->getY();
  
  // center coord. of minimap
  float r = MINI_MAP_SIZE * MINI_MAP_BLOCK / 2;
  
  for( set<Location*>::iterator e = p->begin(); e != p->end(); ++e ) {
  	Location *pos = *e;
  	float angle = Util::getAngle( px, py, 0, 0, (float)pos->x, (float)pos->y, 0, 0 );
  	float nx = r + ( r - 10 ) * Constants::cosFromAngle( angle ) - 5;
  	float ny = r + ( r - 10 ) * Constants::sinFromAngle( angle );
  	glColor4f( color.r, color.g, color.b, color.a );
    glBegin( GL_QUADS );   
    glVertex2d( nx, ny + 4 ); 
    glVertex2d( nx + 4, ny + 4 ); 
    glVertex2d( nx + 4, ny ); 
    glVertex2d( nx, ny );     
    glEnd();
    glColor4f( 0, 0, 0, 1 );
    glBegin( GL_LINE_LOOP );   
    glVertex2d( nx - 1, ny + 6 ); 
    glVertex2d( nx + 6, ny + 6 ); 
    glVertex2d( nx + 6, ny - 1 ); 
    glVertex2d( nx - 1, ny - 1 );     
    glEnd();    
  }
  glColor4f( 1, 1, 1, 1 );
}
