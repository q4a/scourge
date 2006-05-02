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
#include "shapepalette.h"
#include "rpg/rpglib.h"

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

RenderedItem *GameAdapter::createItem( char *item_name, int level, int depth ) {
  RpgItem *rpgItem = RpgItem::getItemByName( item_name );
	if( !rpgItem ) {
		cerr << "*** Error: can't find rpg item: " << item_name << endl;
		return NULL;
	}
	if( strcmp( item_name, "SCROLL" ) ) {
		// FIXME: special case... do we need it? Test it.
	}
	Item *item = session->newItem( rpgItem, level );

	if( rpgItem->isContainer() ) {
		fillContainer( item, level, depth );
	}

	return item;
}

void GameAdapter::fillContainer( Item *container, int level, int depth ) {
	// some items
	int n = (int)(3.0f * rand() / RAND_MAX);
	for(int i = 0; i < n; i++) {
		Item *containedItem = createRandomItem( level, depth );
		if( containedItem ) {
			container->addContainedItem(containedItem, true);
		}
	}
	// some spells
	if(!((int)(25.0f * rand() / RAND_MAX))) {
		int n = (int)(2.0f * rand() / RAND_MAX) + 1;
		for(int i = 0; i < n; i++) {
			Spell *spell = MagicSchool::getRandomSpell(level);
			if(spell) {
				Item *scroll = session->newItem(RpgItem::getItemByName("Scroll"), level, spell);
				container->addContainedItem(scroll, true);
			}
		}
	}
}

Item *GameAdapter::createRandomItem( int level, int depth ) {
  // special items
  for( int i = 0; i < RpgItem::getSpecialCount(); i++ ) {
    RpgItem *rpgItem = RpgItem::getSpecial( i );
    if( rpgItem->getMinLevel() <= level &&
        rpgItem->getMinDepth() <= depth &&
        !( session->getSpecialItem( rpgItem ) ) &&
        0 == (int)( (float)rpgItem->getRareness() * rand() / RAND_MAX ) ) {
      // set loading to true so the level is exact and the item is not enchanted
      Item *item = session->newItem( rpgItem, level, NULL );
      return item;
    }
  }

  // general items
  RpgItem *rpgItem = RpgItem::getRandomItem( depth );
  if(!rpgItem) {
    cerr << "Warning: no items found." << endl;
    return NULL;
  }
  // Make a random level object
  return session->newItem(rpgItem, level);
}

RenderedCreature *GameAdapter::createMonster( char *monster_name ) {
	Creature *creature = NULL;
  Monster *monster = Monster::getMonsterByName( monster_name );
  if( !monster ) {
    cerr << "*** Error: can't find monster: " << monster_name << endl;
		return NULL;
	}
	GLShape *shape = session->getShapePalette()->
		getCreatureShape( monster->getModelName(), 
											monster->getSkinName(), 
											monster->getScale(),
											monster );
	creature = session->newCreature( monster, shape, true );

  //creature->calculateExpOfNextLevel();

  //creature->evalSpecialSkills();

  return creature;
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

