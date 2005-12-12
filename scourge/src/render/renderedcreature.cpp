/***************************************************************************
                          renderedcreature.h  -  description
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
#include "renderedcreature.h"
#include "effect.h"
#include "map.h"
#include "glshape.h"

RenderedCreature::RenderedCreature( Preferences *preferences, 
                                    Shapes *shapes, 
                                    Map *levelMap ) {
  this->preferences = preferences;
  this->shapes = shapes;
  this->levelMap = levelMap;
  x = y = z = 0; 
  damageEffectCounter = 0;
  effectDuration = Constants::DAMAGE_DURATION;
  effect = NULL;
  effectType = Constants::EFFECT_FLAMES;
  recentDamagesCount = 0;
}

RenderedCreature::~RenderedCreature() {
  if( effect ) delete effect;
}

void RenderedCreature::startEffect( int effect_type, int duration, GLuint delay ) {
  // show an effect
  if( isEffectOn() && effect_type == getEffectType() ) {
    return;
  }  
  getEffect()->deleteParticles();
  resetDamageEffect();
  setEffectType( effect_type );
  effectDuration = duration;

  // need to do this to make sure effect shows up
  levelMap->refresh();
}

Effect *RenderedCreature::getEffect() {
  if( !effect ) {
    effect = 
      new Effect( levelMap, preferences, shapes, getShape() );
  }
  return effect;
}

bool RenderedCreature::addRecentDamage( int damage ) { 
if( recentDamagesCount < MAX_RECENT_DAMAGE - 1 ) {
  recentDamages[ recentDamagesCount ].damage = damage;
    recentDamages[ recentDamagesCount ].pos = 0.0f;
    recentDamages[ recentDamagesCount ].lastTime = SDL_GetTicks();
    recentDamagesCount++;
    return true;
  } else {
    return false;
  }
}

void RenderedCreature::removeRecentDamage( int i ) {
  for( int t = i; t < recentDamagesCount - 1; t++ ) {
    recentDamages[ t ].damage = recentDamages[ t + 1 ].damage;
    recentDamages[ t ].pos = recentDamages[ t + 1 ].pos;
    recentDamages[ t ].lastTime = recentDamages[ t + 1 ].lastTime;
  }
  recentDamagesCount--;
}

void RenderedCreature::findPlace( int startx, int starty, int *finalX, int *finalY ) {
  int dir = Constants::MOVE_UP;
  int ox = startx;
  int oy = starty;
  int xx = ox;
  int yy = oy;
  int r = 6;
  // potential inf. loop?
  // it assumes there is free space "somewhere" on this map...
  while( true ) {
    // can player fit here?
    if( !levelMap->isBlocked( xx, yy, 0, 0, 0, 0, getShape(), NULL ) ) {
      //cerr << "Placed party member: " << t << " at: " << xx << "," << yy << endl;
      moveTo( xx, yy, 0 );
      setSelXY( xx, yy );
      levelMap->setCreature( xx, yy, 0, this );

      if( finalX ) *finalX = xx;
      if( finalY ) *finalY = yy;

      return;
    }

    // try radially around the player
    switch( dir ) {
    case Constants::MOVE_UP:
      yy--; 
    if( yy <= MAP_OFFSET || abs( oy - yy ) > r ) dir = Constants::MOVE_RIGHT;
    break;
    case Constants::MOVE_RIGHT:
      xx++; 
    if( xx >= MAP_WIDTH - MAP_OFFSET || abs( ox - xx ) > r ) dir = Constants::MOVE_DOWN;
    break;
    case Constants::MOVE_DOWN:
      yy++; 
    if( yy >= MAP_DEPTH - MAP_OFFSET || abs( oy - yy ) > r ) dir = Constants::MOVE_LEFT;
    break;
    case Constants::MOVE_LEFT:
      xx--; 
    if( xx <= MAP_OFFSET || abs( ox - xx ) > r ) {
      dir = Constants::MOVE_UP;
      r += getShape()->getWidth();
    }
    break;
    }
  }
}

