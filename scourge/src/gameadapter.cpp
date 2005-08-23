/***************************************************************************
                          gameadapter.cpp  -  description
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

#include "gameadapter.h"
#include "session.h"
#include "preferences.h"
#include "item.h"
#include "creature.h"
#include "sound.h"
#include "render/renderlib.h"

GameAdapter::GameAdapter( Preferences *config ) {
  this->preferences = config;
}

GameAdapter::~GameAdapter() {
  delete preferences;
}

RenderedItem *GameAdapter::load( ItemInfo *info ) {
  return Item::load( session, info );
}

RenderedCreature *GameAdapter::load( CreatureInfo *info ) {
  return Creature::load( session, info );
}

RenderedCreature *GameAdapter::getPlayer() {
  return (RenderedCreature*)(getSession()->getParty()->getPlayer());
}

RenderedCreature *GameAdapter::getParty( int index ) {
  return (RenderedCreature*)(getSession()->getParty()->getParty( index ));
}

ServerAdapter::ServerAdapter( Preferences *config ) : GameAdapter( config ) {
}

ServerAdapter::~ServerAdapter() {
}

void ServerAdapter::start() {
#ifdef HAVE_SDL_NET
  session->runServer( preferences->getPort() );
#endif
}



ClientAdapter::ClientAdapter( Preferences *config ) : GameAdapter( config ) {
}

ClientAdapter::~ClientAdapter() {
}

void ClientAdapter::start() {
#ifdef HAVE_SDL_NET
  session->runClient( preferences->getHost(), 
                      preferences->getPort(), 
                      preferences->getUserName() );
#endif
}                               




SDLOpenGLAdapter::SDLOpenGLAdapter( Preferences *config ) : GameAdapter( config ) {
  sdlHandler = NULL;
  //lastMapX = lastMapY = lastMapZ = lastX = lastY = -1;
}

SDLOpenGLAdapter::~SDLOpenGLAdapter() {
  if( sdlHandler ) delete sdlHandler;
}

void SDLOpenGLAdapter::initVideo() {
  // Initialize the video mode
  sdlHandler = new SDLHandler( this ); 
  sdlHandler->setVideoMode( preferences ); 
}

void SDLOpenGLAdapter::getMapXYAtScreenXY( Uint16 *mapx, Uint16 *mapy ) {
  getMapXYAtScreenXY( getSDLHandler()->mouseX, 
                      getSDLHandler()->mouseY, 
                      mapx, 
                      mapy );
}

void SDLOpenGLAdapter::getMapXYAtScreenXY(Uint16 x, Uint16 y,
                                          Uint16 *mapx, Uint16 *mapy) {
  glPushMatrix();
  
  // Initialize the scene w/o y rotation.
  getSession()->getMap()->initMapView(true);
  
  double obj_x, obj_y, obj_z;
  double win_x = (double)x;
  double win_y = (double)sdlHandler->getScreen()->h - y - 1;
  double win_z = 0.5f;
  
  double projection[16];
  double modelview[16];
  GLint viewport[4];
  
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetIntegerv(GL_VIEWPORT, viewport);
  
  int res = gluUnProject(win_x, win_y, win_z,
                         modelview,
                         projection,
                         viewport,
                         &obj_x, &obj_y, &obj_z);
  
  glDisable( GL_SCISSOR_TEST );
  
  if(res) {
    //*mapx = levelMap->getX() + (Uint16)(((obj_x) * GLShape::DIV)) - 1;
    //*mapy = levelMap->getY() + (Uint16)(((obj_y) * GLShape::DIV)) + 2;

    *mapx = getSession()->getMap()->getX() + (Uint16)(((obj_x) * GLShape::DIV));
    *mapy = getSession()->getMap()->getY() + (Uint16)(((obj_y) * GLShape::DIV));

    //*mapz = (Uint16)0;
    //*mapz = (Uint16)(obj_z * GLShape::DIV);
    getSession()->getMap()->debugX = *mapx;
    getSession()->getMap()->debugY = *mapy;
    getSession()->getMap()->debugZ = 0;
  } else {
    //*mapx = *mapy = *mapz = MAP_WIDTH + 1;
    *mapx = *mapy = MAP_WIDTH + 1;
  }
  glPopMatrix();
}   

void SDLOpenGLAdapter::getMapXYZAtScreenXY(Uint16 *mapx, Uint16 *mapy, Uint16 *mapz) {
  // only do this if the mouse has moved some (optimization)
//  if(abs(lastX - x) < POSITION_SAMPLE_DELTA && abs(lastY - y) < POSITION_SAMPLE_DELTA) {
//    *mapx = lastMapX;
//    *mapy = lastMapY;
//    *mapz = lastMapZ;
//    return;
//  }

  GLuint buffer[512];
  GLint  hits, viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);
  glSelectBuffer(512, buffer);
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(0);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(getSDLHandler()->mouseX, 
                viewport[3] - getSDLHandler()->mouseY, 
                1, 1, viewport);
  sdlHandler->setOrthoView();

  glMatrixMode(GL_MODELVIEW);
  //levelMap->selectMode = true;
  getSession()->getMap()->draw();
  //levelMap->selectMode = false;

  glFlush();    
  hits = glRenderMode(GL_RENDER);
  //cerr << "hits=" << hits << endl;
  if(hits > 0) {           // If There Were More Than 0 Hits
    int choose = buffer[4];         // Make Our Selection The First Object
    int depth = buffer[1];          // Store How Far Away It Is

    for(int loop = 0; loop < hits; loop++) {   // Loop Through All The Detected Hits

      //            fprintf(stderr, "\tloop=%d 0=%u 1=%u 2=%u 3=%u 4=%u \n", loop, 
      //                    buffer[loop*5+0], buffer[loop*5+1], buffer[loop*5+2], 
      //                    buffer[loop*5+3],  buffer[loop*5+4]);
      if(buffer[loop*5+4] > 0) {
        decodeName(buffer[loop*5+4], mapx, mapy, mapz);
      }

      // If This Object Is Closer To Us Than The One We Have Selected
      if(buffer[loop*5+1] < GLuint(depth)) {
        choose = buffer[loop*5+4];        // Select The Closer Object
        depth = buffer[loop*5+1];     // Store How Far Away It Is
      }
    }

    //cerr << "choose=" << choose << endl;
    decodeName(choose, mapx, mapy, mapz);
  } else {
    *mapx = *mapy = MAP_WIDTH + 1;
  }

  // Restore the projection matrix
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Go back to modelview for normal rendering
  glMatrixMode(GL_MODELVIEW);

  getSession()->getMap()->debugX = *mapx;
  getSession()->getMap()->debugY = *mapy;
  getSession()->getMap()->debugZ = *mapz;
  /*
  lastMapX = *mapx;
  lastMapY = *mapy;
  lastMapZ = *mapz;
  lastX = getSDLHandler()->mouseX;
  lastY = getSDLHandler()->mouseY;
  */
}

void SDLOpenGLAdapter::playSound(const char *sound) { 
  getSDLHandler()->getSound()->playSound(sound); 
}

void SDLOpenGLAdapter::decodeName(int name, Uint16* mapx, Uint16* mapy, Uint16* mapz) {
    char *s;
    if(name > 0) {
        // decode the encoded map coordinates
        *mapz = name / (MAP_WIDTH * MAP_DEPTH);
        if(*mapz > 0)
            name %= (MAP_WIDTH * MAP_DEPTH);
        *mapx = name % MAP_WIDTH;
        *mapy = name / MAP_WIDTH;
        Location *pos = getSession()->getMap()->getPosition(*mapx, *mapy, 0);
        if(pos) {
            if(pos->shape) s = pos->shape->getName();
            else if(pos->item && pos->item->getShape()) {
                s = pos->item->getShape()->getName();
            }
        } else s = NULL;
		//        fprintf(stderr, "\tmap coordinates: pos null=%s shape null=%s item null=%s %u,%u,%u name=%s\n", 
		//                (pos ? "no" : "yes"), (pos && pos->shape ? "no" : "yes"), (pos && pos->item ? "no" : "yes"), *mapx, *mapy, *mapz, (s ? s : "NULL"));
    } else {
        *mapx = MAP_WIDTH + 1;
        *mapy = 0;
        *mapz = 0;
		//        fprintf(stderr, "\t---\n");
    }
}

