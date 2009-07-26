/***************************************************************************
           renderedcreature.cpp  -  Map related creature functions
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
#include "../common/constants.h"
#include "renderedcreature.h"
#include "effect.h"
#include "map.h"
#include "mapsettings.h"
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
	this->offsX = this->offsY = this->offsZ = 0.0f;
	talkStartTime = 0;
	missionId = missionObjectiveIndex = 0;
}

RenderedCreature::~RenderedCreature() {
	if ( effect ) delete effect;
}

void RenderedCreature::startEffect( int effect_type, int duration, GLuint delay ) {
	// show an effect
	if ( isEffectOn() && effect_type == getEffectType() ) {
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
	if ( !effect ) {
		effect =
		  new Effect( levelMap, preferences, shapes, getShape() );
	}
	return effect;
}

bool RenderedCreature::addRecentDamage( int damage ) {
	if ( recentDamagesCount < MAX_RECENT_DAMAGE - 1 ) {
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
	for ( int t = i; t < recentDamagesCount - 1; t++ ) {
		recentDamages[ t ].damage = recentDamages[ t + 1 ].damage;
		recentDamages[ t ].pos = recentDamages[ t + 1 ].pos;
		recentDamages[ t ].lastTime = recentDamages[ t + 1 ].lastTime;
	}
	recentDamagesCount--;
}

bool RenderedCreature::findPlaceBoundedRadial( int startx, int starty, int radius ) {
	for( int r = 0; r < radius; r++ ) {
		//cerr << "+++ radius=" << r << " pos=" << (startx - r) << "," << (starty - r) << endl;
		if( findPlaceBounded( startx - r, starty - r, startx + r, starty + r ) ) {
			return true;
		}
	}
	return false;
}

// use this for small bounded areas
bool RenderedCreature::findPlaceBounded( int startx, int starty, int endx, int endy ) {
	//cerr << "--- findPlaceBounded: " << startx << "," << starty << " " << endx << "," << endy << endl;
	for( int x = startx; x < endx; x++ ) {
		for( int y = starty; y < endy; y++ ) {
			//cerr << "--- checking: " << x << "," << y << endl;
			if ( levelMap->canFit( x, y, getShape() ) ) {
				moveTo( x, y, 0 );
				if( levelMap->getSettings()->isPlayerEnabled() ) setSelXY( x, y );
				levelMap->setCreature( x, y, 0, this );
				return true;
			}
		}
	}
	return false;
}

// use this for large areas
bool RenderedCreature::findPlace( int startx, int starty, int *finalX, int *finalY ) {
	int fx = 0;
	int fy = 0;
	int sx = startx;
	int sy = starty;
	set<int> seen;
	if ( doFindStart( &sx, &sy ) ) {
		if ( !doFindPlace( sx, sy, &fx, &fy, &seen ) ) {
			cerr << "Warning: can't find place for creature." << endl;
		} else {
			moveTo( fx, fy, 0 );
			if( levelMap->getSettings()->isPlayerEnabled() ) setSelXY( fx, fy );
			levelMap->setCreature( fx, fy, 0, this );
			if ( finalX ) *finalX = fx;
			if ( finalY ) *finalY = fy;
			return true;
		}		
	} else {
		cerr << "Warning: can't find starting place." << endl;
	}
	return false;
}

// Find a free starting location (for the recursion) to the left and above the given start.
bool RenderedCreature::doFindStart( int *startx, int *starty ) {
	if ( getShape() == NULL ) {
		cerr << "Error: placing shapeless creature." << endl;
		return false;
	}
	int xx;
	int yy;
	for ( yy = *starty; yy >= 0; --yy ) {
		for ( xx = *startx; xx >= 0; ) {
			Location *pos = levelMap->getLocation( xx, yy, 0 );
			if ( !pos ) {
				*startx = xx;
				*starty = yy;
				return true;
			} else {
				xx = pos->x - 1;
			}
		}
		for ( xx = *startx; xx < MAP_WIDTH - 0; ) {
			Location *pos = levelMap->getLocation( xx, yy, 0 );
			if ( !pos ) {
				*startx = xx;
				*starty = yy;
				return true;
			} else {
				xx = pos->x + pos->shape->getWidth();
			}
		}
	}
	for ( yy = *starty; yy < MAP_DEPTH; ++yy ) {
		for ( xx = *startx; xx >= 0; ) {
			Location *pos = levelMap->getLocation( xx, yy, 0 );
			if ( !pos ) {
				*startx = xx;
				*starty = yy;
				return true;
			} else {
				xx = pos->x - 1;
			}
		}
		for ( xx = *startx; xx < MAP_WIDTH; ) {
			Location *pos = levelMap->getLocation( xx, yy, 0 );
			if ( !pos ) {
				*startx = xx;
				*starty = yy;
				return true;
			} else {
				xx = pos->x + pos->shape->getWidth();
			}
		}
	}
	return false;
}


bool RenderedCreature::isEmptyUnSeenPlace( int x, int y, set<int> *seen ) {
	return levelMap->isEmpty( x, y ) && seen->find( x + y * MAP_WIDTH ) == seen->end();
}

bool RenderedCreature::doFindPlace( int startx, int starty, int *finalX, int *finalY, set<int> *seen ) {
	if ( *finalX ) return true;

	if ( levelMap->canFit( startx, starty, getShape() ) ) {
		*finalX = startx;
		*finalY = starty;
		return true;
	}

	seen->insert( startx + starty * MAP_WIDTH );

	if ( isEmptyUnSeenPlace( startx + 1, starty, seen ) ) {
		if ( doFindPlace( startx + 1, starty, finalX, finalY, seen ) ) return true;
	}
	if ( isEmptyUnSeenPlace( startx, starty + 1, seen ) ) {
		if ( doFindPlace( startx, starty + 1, finalX, finalY, seen ) ) return true;
	}
	if ( isEmptyUnSeenPlace( startx - 1, starty, seen ) ) {
		if ( doFindPlace( startx - 1, starty, finalX, finalY, seen ) ) return true;
	}
	if ( isEmptyUnSeenPlace( startx, starty - 1, seen ) ) {
		if ( doFindPlace( startx, starty - 1, finalX, finalY, seen ) ) return true;
	}

	return false;
}



/**
 *  This method makes sure that players are not placed outside of walls
 * it ensures that their proposed location is reachable from the original
 * location w/o going thru walls.
 */
bool RenderedCreature::canReach( int startx, int starty, int firstx, int firsty, int xx, int yy, map<int, bool> *seen ) {
	// make sure we're not recursing too much
	if ( ( ( startx - xx ) * ( startx - xx ) ) + ( ( starty - yy ) * ( starty - yy ) ) > ( 50 * 50 ) ) {
		cerr << "warning: RenderedCreature::canReach recursion limit." << endl;
		return true;
	}

	if ( startx == xx && starty == yy ) return true;
	if ( startx < 0 || startx >= MAP_WIDTH || starty < 0 || starty >= MAP_DEPTH ) return false;

	int pos = startx + starty * MAP_WIDTH;
	if ( seen->find( pos ) != seen->end() ) return false;
	( *seen )[ pos ] = true;

	Location *location = levelMap->getLocation( startx, starty, 0 );
	if ( location &&
	        !( location->item || location->creature ) &&
	        !( location->x == firstx && location->y == firsty ) &&
	        !( location->x == xx && location->y == yy ) ) {
		return false;
	}

	if ( canReach( startx - 1, starty, firstx, firsty, xx, yy, seen ) ) return true;
	if ( canReach( startx + 1, starty, firstx, firsty, xx, yy, seen ) ) return true;
	if ( canReach( startx, starty - 1, firstx, firsty, xx, yy, seen ) ) return true;
	if ( canReach( startx, starty + 1, firstx, firsty, xx, yy, seen ) ) return true;
	return false;
}

void RenderedCreature::say( char const* text ) {
	talkStartTime = SDL_GetTicks();
	strncpy( speech, text, 2000 );

	clearSpeech();

	char tmp[3000];
	Util::addLineBreaks( speech, tmp, 45 );
	Util::getLines( tmp, &speechWrapped );
}

void RenderedCreature::clearSpeech() {
	speechWrapped.clear();
}

#define TALK_DURATION 5000

bool RenderedCreature::isTalking() {
	//if( SDL_GetTicks() > ( talkStartTime + TALK_DURATION ) ) { return false; } else { return true; }
	return speechWrapped.size() > 0 ? true : false;
}

char *RenderedCreature::getSpeech() {
	if ( isTalking() ) {
		return speech;
	} else {
		return NULL;
	}
}

std::vector<std::string> *RenderedCreature::getSpeechWrapped() {
	if ( isTalking() ) {
		return &speechWrapped;
	} else {
		return NULL;
	}
}
