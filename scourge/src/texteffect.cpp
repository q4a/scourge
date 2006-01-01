/***************************************************************************
                          texteffect.cpp  -  description
                             -------------------
    begin                : Sept 13, 2005
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
#include "texteffect.h"
#include "scourge.h"
#include "sdlhandler.h"
 
#define MENU_ITEM_WIDTH 256
#define MENU_ITEM_HEIGHT 32
#define MENU_ITEM_ZOOM 1.5f
#define MENU_ITEM_PARTICLE_ZOOM 1.1f
#define MAX_PARTICLE_LIFE 50

TextEffect::TextEffect( Scourge *scourge, int x, int y, char *text ) {
  this->scourge = scourge;
  strncpy( this->text, text, 254 );
  this->text[ 254 ] = '\0';
  this->x = x;
  this->y = y;

  lastTickMenu = 0;
  for( int t = 0; t < 20; t++ ) particle[t].life = 0;
  textureInMemory = NULL;
}

TextEffect::~TextEffect() {
  if( textureInMemory ) {
    glDeleteTextures( 1, texture );
    free( textureInMemory );
  }
}

void TextEffect::draw() {

  //glDepthMask( GL_FALSE );
  glDisable(GL_DEPTH_TEST);

  glDisable( GL_CULL_FACE );
  if( !textureInMemory ) {
    buildTextures();
  }

  float zoom = MENU_ITEM_ZOOM;
  glEnable( GL_TEXTURE_2D );
  zoom = ( active ? MENU_ITEM_ZOOM * 1.5f : MENU_ITEM_ZOOM );
  glPushMatrix();
  glLoadIdentity();
  glTranslatef( x + 40, y + 20, 0 );
  glBindTexture( GL_TEXTURE_2D, texture[0] );
  if( active ) {
    //glColor4f( 1, 0.6f, 0.5f, 1 );
    glColor4f( 0.9f, 0.7f, 0.15f, 1 );
  } else {
    glColor4f( 1, 1, 1, 1 );
  }

  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE );
  //scourge->setBlendFunc();

  glBegin( GL_QUADS );
  glNormal3f( 0, 0, 1 );
  glTexCoord2f( 0, 1 );
  glVertex2f( 0, 0 );
  glTexCoord2f( 1, 1 );
  glVertex2f( MENU_ITEM_WIDTH * zoom, 0 );
  glTexCoord2f( 1, 0 );
  glVertex2f( MENU_ITEM_WIDTH * zoom, MENU_ITEM_HEIGHT * zoom );
  glTexCoord2f( 0, 0 );
  glVertex2f( 0, MENU_ITEM_HEIGHT * zoom );
  glEnd();
  glPopMatrix();

  glEnable( GL_BLEND );
  //glDepthMask( GL_FALSE );

  //glEnable( GL_ALPHA_TEST );
  //glAlphaFunc( GL_NOTEQUAL, 0x00 );

  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  //scourge->setBlendFunc();
  //drawEffect( 6.0f, 5 );

  //glDisable( GL_ALPHA_TEST );

  glBlendFunc( GL_DST_COLOR, GL_ONE );
  //glBlendFunc( GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA );
  //scourge->setBlendFunc();
  drawEffect( 4.0f, 20 );

  glDisable( GL_BLEND );  
  //glDepthMask( GL_TRUE );
  glDisable( GL_TEXTURE_2D );
  
  // move menu
  Uint32 tt = SDL_GetTicks();
  if( tt - lastTickMenu > 40 ) {
    lastTickMenu = tt;
    for( int i = 0; i < 20; i++ ) {
      particle[i].x += cos( Constants::toRadians( particle[i].dir ) ) * particle[i].step;
      particle[i].y += sin( Constants::toRadians( particle[i].dir ) ) * particle[i].step;
      particle[i].life++;
      if( particle[i].life >= MAX_PARTICLE_LIFE ) {
        particle[i].life = 0;
      }
    }
  }

  //glDepthMask( GL_TRUE );
  glEnable(GL_DEPTH_TEST);
}

void TextEffect::drawEffect( float divisor, int count ) {
  if( active ) {
    for( int i = 0; i < count; i++ ) {
      if( !( particle[i].life ) ) {
        particle[i].life = (int)( (float)MAX_PARTICLE_LIFE * rand() / RAND_MAX );
        particle[i].x = particle[i].y = 0;
        particle[i].r = 200 + (int)( 40.0f * rand() / RAND_MAX );
        particle[i].g = 170 + (int)( 40.0f * rand() / RAND_MAX );
        particle[i].b = 80 + (int)( 40.0f * rand() / RAND_MAX );
        particle[i].dir = 10.0f * rand() / RAND_MAX;
        particle[i].zoom = 2.0f + ( 2.0f * rand() / RAND_MAX );
        switch( (int)( 4.0f * rand() / RAND_MAX ) ) {
        case 0: particle[i].dir = 360.0f - particle[i].dir; break;
        case 1: particle[i].dir = 180.0f - particle[i].dir; break;
        case 2: particle[i].dir = 180.0f + particle[i].dir; break;
        //default: // do nothing
        }
        particle[i].step = 4.0f * rand() / RAND_MAX;
      }
      
      glPushMatrix();
      glLoadIdentity();
      glTranslatef( x + particle[i].x, 
                    y + particle[i].y, 0 );
      glBindTexture( GL_TEXTURE_2D, texture[0] );
      
      float a = (float)( MAX_PARTICLE_LIFE - particle[i].life ) / (float)( MAX_PARTICLE_LIFE );
      //if( i == 0 ) cerr << "life=" << particle[i].life << " a=" << a << endl;
      glColor4f( (float)( particle[i].r ) / ( 256.0f * divisor ), 
                 (float)( particle[i].g ) / ( 256.0f * divisor ), 
                 (float)( particle[i].b ) / ( 256.0f * divisor ), 
                 a / divisor );
      
      glBegin( GL_QUADS );
      glNormal3f( 0, 0, 1 );
      glTexCoord2f( 0, 1 );
      glVertex2f( 0, 0 );
      glTexCoord2f( 1, 1 );
      glVertex2f( MENU_ITEM_WIDTH * particle[i].zoom, 0 );
      glTexCoord2f( 1, 0 );
      glVertex2f( MENU_ITEM_WIDTH * particle[i].zoom, MENU_ITEM_HEIGHT * particle[i].zoom );
      glTexCoord2f( 0, 0 );
      glVertex2f( 0, MENU_ITEM_HEIGHT * particle[i].zoom );
      glEnd();
      glPopMatrix();  
    }
  }
}

void TextEffect::buildTextures() {

  int width = MENU_ITEM_WIDTH;
  int height = MENU_ITEM_HEIGHT;


  glPushMatrix();
  glLoadIdentity();
  glDisable( GL_TEXTURE_2D );
  glColor4f( 0, 0, 0, 0 );
  glBegin( GL_QUADS );
  glVertex2f( x, y - 20 );
  glVertex2f( x + width, y - 20 );
  glVertex2f( x + width, y - 20 + height );
  glVertex2f( x, y - 20 + height );
  glEnd();
  glEnable( GL_TEXTURE_2D );


  scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );

  //int width = scourge->getSDLHandler()->textWidth( text );

  // Create texture and copy minimap date from backbuffer on it    
  textureInMemory = (unsigned char *)malloc( width * height * 4 );
    
  glGenTextures(1, texture);    
  glBindTexture(GL_TEXTURE_2D, texture[0]); 
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);        
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);                                          // filtre appliquÿ a la texture
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);  
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP ); 
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, textureInMemory );
  
  // Draw image
  //x = x;
  //y = y;
  glColor4f( 1, 1, 1, 1 );
  scourge->getSDLHandler()->texPrint( x, y, text );
  //y += height;
  
  // Copy to a texture
  glLoadIdentity();
  glEnable( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, texture[0] );
  glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 
                    x, scourge->getSDLHandler()->getScreen()->h - ( y - 20 + height ), 
                    width, height, 0 );
  scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
  glPopMatrix();
}

