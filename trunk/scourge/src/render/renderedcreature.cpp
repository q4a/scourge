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
#include <map>

using namespace std;

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

  talkStartTime = 0;
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
	int fx = 0;
	int fy = 0;
	int sx = startx;
	int sy = starty;
	set<int> seen;
	if( doFindStart( &sx, &sy ) ) {
		if( !doFindPlace( sx, sy, &fx, &fy, &seen ) ) {
			cerr << "Warning: can't find place for creature." << endl;
		} else {
			moveTo( fx, fy, 0 );
			setSelXY( fx, fy );
			levelMap->setCreature( fx, fy, 0, this );
			if( finalX ) *finalX = fx;
			if( finalY ) *finalY = fy;
		}
	} else {
		cerr << "Warning: can't find starting place." << endl;
	}
}

// Find a free starting location (for the recursion) to the left and above the given start.
bool RenderedCreature::doFindStart( int *startx, int *starty ) {
  int xx;
  int yy = *starty;
  while( yy >= MAP_OFFSET ) {
	xx = *startx;
	while( xx >= MAP_OFFSET ) {
	  Location *pos = levelMap->getLocation( xx, yy, 0 );
	  if( !pos ) {
		*startx = xx;
		*starty = yy;
		return true;
	  } else {
		xx = pos->x - 1;
	  }
	}
	xx = *startx;
	while( xx < MAP_WIDTH - MAP_OFFSET ) {
	  Location *pos = levelMap->getLocation( xx, yy, 0 );
	  if( !pos ) {
		*startx = xx;
		*starty = yy;
		return true;
	  } else {
		xx = pos->x + pos->shape->getWidth();
	  }
	}
	yy--;
  }
  yy = *starty;
  while( yy < MAP_DEPTH - MAP_OFFSET ) {
	xx = *startx;
	while( xx >= MAP_OFFSET ) {
	  Location *pos = levelMap->getLocation( xx, yy, 0 );
	  if( !pos ) {
		*startx = xx;
		*starty = yy;
		return true;
	  } else {
		xx = pos->x - 1;
	  }
	}
	xx = *startx;
	while( xx < MAP_WIDTH - MAP_OFFSET ) {
	  Location *pos = levelMap->getLocation( xx, yy, 0 );
	  if( !pos ) {
		*startx = xx;
		*starty = yy;
		return true;
	  } else {
		xx = pos->x + pos->shape->getWidth();
	  }
	}
	yy++;
  }
  return false;
}

#define EMPTY_POS(x,y) !(*finalX) && levelMap->isEmpty( (x), (y) ) && seen->find( (x) + (y) * MAP_WIDTH ) == seen->end()

bool RenderedCreature::doFindPlace( int startx, int starty, int *finalX, int *finalY, set<int> *seen ) {
	if( *finalX ) return true;

	if( levelMap->canFit( startx, starty, getShape() ) ) {
		*finalX = startx;
		*finalY = starty;
		return true;
	}

	seen->insert( startx + starty * MAP_WIDTH );

	if( EMPTY_POS( startx + 1, starty ) ) {
		if( doFindPlace( startx + 1, starty, finalX, finalY, seen ) ) return true;
	}
	if( EMPTY_POS( startx, starty + 1 ) ) {
		if( doFindPlace( startx, starty + 1, finalX, finalY, seen ) ) return true;
	}
	if( EMPTY_POS( startx - 1, starty ) ) {
		if( doFindPlace( startx - 1, starty, finalX, finalY, seen ) ) return true;
	}
	if( EMPTY_POS( startx, starty - 1 ) ) {
		if( doFindPlace( startx, starty - 1, finalX, finalY, seen ) ) return true;
	}
	
	return false;
}



/**
 * This can be a pretty slow method...
 */
#define FIND_PLACE_LIMIT 100

void RenderedCreature::findPlace_old( int startx, int starty, int *finalX, int *finalY ) {
  int dir = Constants::MOVE_UP;
  int ox = startx;
  int oy = starty;
  int xx = ox;
  int yy = oy;
  int r = 6;
  
  if( finalX ) *finalX = -1;
  if( finalY ) *finalY = -1;  
  
  // it assumes there is free space "somewhere" on this map...
  map<int,bool> seen;
  for( int count = 0; count < FIND_PLACE_LIMIT; count++ ) {
    seen.clear();

    // can player fit here?
    if( levelMap->canFit( xx, yy, getShape() ) && 
        canReach( startx, starty, startx, starty, xx, yy, &seen ) ) {
      //cerr << "Placed party member: " << t << " at: " << xx << "," << yy << endl;
      moveTo( xx, yy, 0 );
      setSelXY( xx, yy );
      levelMap->setCreature( xx, yy, 0, this );

      if( finalX ) *finalX = xx;
      if( finalY ) *finalY = yy;

      return;
    }
    //if( seen.size() ) cerr << "seen size=" << seen.size() << " pos=" << xx << "," << yy << " r=" << r << endl;

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

/**
 *  This method makes sure that players are not placed outside of walls
 * it ensures that their proposed location is reachable from the original
 * location w/o going thru walls.
 */
bool RenderedCreature::canReach( int startx, int starty, int firstx, int firsty, int xx, int yy, map<int,bool> *seen ) {
	// make sure we're not recursing too much
	if( ( ( startx - xx ) * ( startx - xx ) ) + ( ( starty - yy ) * ( starty - yy ) ) > ( 50 * 50 ) ) {
		cerr << "warning: RenderedCreature::canReach recursion limit." << endl;
		return true;
	}

  if( startx == xx && starty == yy ) return true;
  if( startx < 0 || startx >= MAP_WIDTH || starty < 0 || starty >= MAP_DEPTH ) return false;

  int pos = startx + starty * MAP_WIDTH;
  if( seen->find( pos ) != seen->end() ) return false;
  (*seen)[ pos ] = true;

  Location *location = levelMap->getLocation( startx, starty, 0 );
  if( location && 
      !( location->item || location->creature ) &&
      !( location->x == firstx && location->y == firsty ) &&
      !( location->x == xx && location->y == yy ) ) {
    return false;
  }

  if( canReach( startx - 1, starty, firstx, firsty, xx, yy, seen ) ) return true;
  if( canReach( startx + 1, starty, firstx, firsty, xx, yy, seen ) ) return true;
  if( canReach( startx, starty - 1, firstx, firsty, xx, yy, seen ) ) return true;
  if( canReach( startx, starty + 1, firstx, firsty, xx, yy, seen ) ) return true;
  return false;
}

void RenderedCreature::say( char const* text ) {
  talkStartTime = SDL_GetTicks();
  strncpy( speech, text, 2000 );

  speechWrapped.clear();

  char tmp[3000];
  Util::addLineBreaks( speech, tmp, 45 );
  Util::getLines( tmp, &speechWrapped );
}

#define TALK_DURATION 5000

bool RenderedCreature::isTalking() {
  if( SDL_GetTicks() > ( talkStartTime + TALK_DURATION ) ) { return false; } else { return true; }
}

char *RenderedCreature::getSpeech() {
  if( isTalking() ) { return speech; } else { return NULL; }
}

std::vector<std::string> *RenderedCreature::getSpeechWrapped() {
  if( isTalking() ) { return &speechWrapped; } else { return NULL; }
}
