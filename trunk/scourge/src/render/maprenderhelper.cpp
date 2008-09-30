/***************************************************************************
      maprenderhelper.cpp  -  Helper functions for specific level types
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

#include "maprenderhelper.h"
#include "fog.h"
#include "shape.h"
#include "map.h"
#include "shapes.h"
#include "mapadapter.h"

using namespace std;

MapRenderHelper *MapRenderHelper::helpers[] = {
  new CaveRenderHelper(),
  new RoomRenderHelper(),
	new OutdoorRenderHelper(),
	new DebugOutdoorRenderHelper()
};

MapRenderHelper::MapRenderHelper() {
}

MapRenderHelper::~MapRenderHelper() {
}

CaveRenderHelper::CaveRenderHelper() {
  fog = NULL;
}

CaveRenderHelper::~CaveRenderHelper() {
  delete fog;
}

void CaveRenderHelper::setMap( Map *map ) {
  MapRenderHelper::setMap( map );
  delete fog;
  fog = new Fog( getMap(), getLampRadiusSquared() );
}

void CaveRenderHelper::reset() {
  fog->reset();
}

void CaveRenderHelper::draw( int x, int y, int w, int h ) {
	if( !getMap()->getSettings()->isGridShowing() ) {
		fog->draw( x, y, w, h, getMap()->getFrustum() );
	}
}

bool CaveRenderHelper::isVisible( int x, int y, Shape *shape ) {
	if( !getMap()->getSettings()->isGridShowing() ) {
		return( fog->getVisibility( x, y, shape ) == Fog::FOG_CLEAR );
	} else {
		return true;
	}
}

void CaveRenderHelper::visit( RenderedCreature *creature ) {
  fog->visit( creature );
}

void CaveRenderHelper::hideDeadParty() {
  fog->hideDeadParty();
}
  
void CaveRenderHelper::loadHelper( FogInfo *fogInfo ) {
	if( !getMap()->getSettings()->isGridShowing() ) {
		fog->load( fogInfo );
	}
}

void CaveRenderHelper::saveHelper( FogInfo *fogInfo ) {
	if( !getMap()->getSettings()->isGridShowing() ) {
		fog->save( fogInfo );
	}
}


OutdoorRenderHelper::OutdoorRenderHelper() : CaveRenderHelper() {
}

OutdoorRenderHelper::~OutdoorRenderHelper() {
}

DebugOutdoorRenderHelper::DebugOutdoorRenderHelper() : MapRenderHelper() {
}

DebugOutdoorRenderHelper::~DebugOutdoorRenderHelper() {
}


RoomRenderHelper::RoomRenderHelper() {
  overlay_tex = 0;
}

RoomRenderHelper::~RoomRenderHelper() {
  // delete the overlay texture
  glDeleteTextures(1, (GLuint*)&overlay_tex);
}

void RoomRenderHelper::setMap( Map *map ) {
  MapRenderHelper::setMap( map );
  if( overlay_tex ) glDeleteTextures(1, (GLuint*)&overlay_tex);
  createOverlayTexture();
}

// vary this number from 0.001 - 3.0 to get tighter shading
#define SHADE_LEVEL 1.0f

void RoomRenderHelper::createOverlayTexture() {
  // create the dark texture
  unsigned int i, j;
  glGenTextures(1, (GLuint*)&overlay_tex);
//  float tmp = 0.7f;
  for(i = 0; i < OVERLAY_SIZE; i++) {
    for(j = 0; j < OVERLAY_SIZE; j++) {
      float half = (static_cast<float>(OVERLAY_SIZE) - 0.5f) / 2.0f;
      float id = static_cast<float>(i) - half;
      float jd = static_cast<float>(j) - half;
      //float dd = 255.0f - ((255.0f / (half * half / 1.2f)) * (id * id + jd * jd));

      float dd = 255.0f - ((255.0f / (half * half / SHADE_LEVEL)) * (id * id + jd * jd));
      if(dd < 0.0f) dd = 0.0f;
      if(dd > 255.0f) dd = 255.0f;
      unsigned char d = (unsigned char)dd;
      overlay_data[i * OVERLAY_SIZE * 3 + j * 3 + 0] = d;
      overlay_data[i * OVERLAY_SIZE * 3 + j * 3 + 1] = d;
      overlay_data[i * OVERLAY_SIZE * 3 + j * 3 + 2] = d;
    }
  }
  glBindTexture(GL_TEXTURE_2D, overlay_tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5, OVERLAY_SIZE, OVERLAY_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, overlay_data);
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB5, OVERLAY_SIZE, OVERLAY_SIZE, GL_RGB, GL_UNSIGNED_BYTE, overlay_data);
}

void RoomRenderHelper::draw( int x, int y, int w, int h ) {
  if( getMap()->getAdapter()->isLevelShaded() && 
      !getMap()->getSettings()->isGridShowing() ) {
    glPushMatrix();
    glLoadIdentity();
    
    //glTranslatef(viewX, viewY, 0);
    
    //  glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    //glDisable( GL_TEXTURE_2D );
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    //scourge->setBlendFunc();
    
    glColor4f( 1, 1, 1, 0.5f);
    
    glBindTexture( GL_TEXTURE_2D, overlay_tex );
    glBegin( GL_TRIANGLE_STRIP );
//      glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f( 0.0f, 0.0f );
    glVertex3f(0, 0, 0);
    glTexCoord2f( 1.0f, 0.0f );
    glVertex3f(getMap()->getViewWidth(), 0, 0);
    glTexCoord2f( 0.0f, 1.0f );
    glVertex3f(0, getMap()->getViewHeight(), 0);
    glTexCoord2f( 1.0f, 1.0f );
    glVertex3f(getMap()->getViewWidth(), getMap()->getViewHeight(), 0);
    glEnd();
    
    //glEnable( GL_TEXTURE_2D );
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
  } 
}
