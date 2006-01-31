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

using namespace std;

GameAdapter::GameAdapter( Preferences *config ) {
  this->preferences = config;
}

GameAdapter::~GameAdapter() {
  delete preferences;
}

bool GameAdapter::isMissionCreature( RenderedCreature *creature ) {
  if( !getSession()->getCurrentMission() ) return false;
  return( getSession()->getCurrentMission()->isMissionCreature( creature ) );
}

bool GameAdapter::hasParty() {
  return( getSession()->getParty() != NULL );
}

int GameAdapter::getPartySize() {
  return( getSession()->getParty() == NULL ? 
          0 : 
          getSession()->getParty()->getPartySize() );
}

void GameAdapter::loadMapData( const char *name ) {
  Mission::loadMapData( this, name );
}

void GameAdapter::saveMapData( const char *name ) {
  Mission::saveMapData( this, name );
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

void SDLOpenGLAdapter::playSound(const char *sound) { 
  getSDLHandler()->getSound()->playSound(sound); 
}

bool SDLOpenGLAdapter::intersects( int x, int y, int w, int h,
                                   int x2, int y2, int w2, int h2 ) {
  return getSDLHandler()->intersects( x, y, w, h,
                                      x2, y2, w2, h2 );
}

