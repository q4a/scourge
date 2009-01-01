/***************************************************************************
                map.cpp  -  Manages and renders the level map
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

#include "mapmemory.h"
#include "map.h"
#include "mapsettings.h"
#include "effect.h"
#include "location.h"
#include "shapes.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

/**
  Maxsize of 0 means unlimited size cache.
*/
MapMemoryManager::MapMemoryManager( int maxSize ) {
	this->maxSize = maxSize;
	this->accessCount = 0;
	this->usedCount = 0;
	// make room for at least this many pointers
	unused.reserve( 200000 );

	this->usedEffectCount = 0;
	unusedEffect.reserve( 50000 );
}

MapMemoryManager::~MapMemoryManager() {
	for ( int i = 0; i < static_cast<int>( unused.size() ); i++ ) {
		delete unused[i];
	}
	unused.clear();
	for ( int i = 0; i < static_cast<int>( unusedEffect.size() ); i++ ) {
		delete unusedEffect[i];
	}
	unusedEffect.clear();
}

/// Creates a new, empty map location. Reuses an unused location if possible.

Location *MapMemoryManager::newLocation() {
	Location *pos;
	if ( !unused.empty() ) {
		pos = unused[ unused.size() - 1 ];
		unused.pop_back();
	} else {
		pos = new Location();
	}
	usedCount++;

	printStatus();

	// reset it
	pos->x = pos->y = pos->z = 0;
	pos->heightPos = 0;
	pos->shape = NULL;
	pos->item = NULL;
	pos->creature = NULL;
	pos->outlineColor = NULL;
	pos->texIndex = NULL;

	return pos;
}

/// Creates a new, empty effect location. Reuses an unused location if possible.

EffectLocation *MapMemoryManager::newEffectLocation( Map *theMap, Preferences *preferences, Shapes *shapes, int width, int height ) {
	EffectLocation *pos;
	if ( !unusedEffect.empty() ) {
		pos = unusedEffect[ unusedEffect.size() - 1 ];
		unusedEffect.pop_back();
		pos->effect->reset();
	} else {
		pos = new EffectLocation;
		//pos->effect = new Effect( theMap, preferences, shapes, 4, 4 );
		pos->effect = new Effect( theMap, preferences, shapes, width, height );
		pos->effect->deleteParticles();
	}
	usedEffectCount++;

	printStatus();

	// reset it
	pos->x = pos->y = pos->z = 0;
	pos->effectDuration = 0;
	pos->damageEffectCounter = 0;
	pos->effectType = 0;
	pos->effectDelay = 0;
	pos->heightPos = 0;

	return pos;
}

/// Deletes a map location and adds it to the "unused" array.

void MapMemoryManager::deleteLocation( Location *pos ) {
	if ( !maxSize || static_cast<int>( unused.size() ) < maxSize ) {
		unused.push_back( pos );
	} else {
		delete pos;
	}
	usedCount--;
	printStatus();
}

/// Deletes the special effect at a location and adds it to the "unused" array.

void MapMemoryManager::deleteEffectLocation( EffectLocation *pos ) {
	if ( !maxSize || static_cast<int>( unusedEffect.size() ) < maxSize ) {
		unusedEffect.push_back( pos );
	} else {
		delete pos;
	}
	usedEffectCount--;
	printStatus();
}

/// Outputs memory status to stderr.

void MapMemoryManager::printStatus() {
	if ( ++accessCount > 5000 ) {
		cerr << "Map size: " << usedCount << " Kb:" << ( static_cast<float>( sizeof( Location )*usedCount ) / 1024.0f ) <<
		" Cache: " << unused.size() << " Kb:" << ( static_cast<float>( sizeof( Location )*unused.size() ) / 1024.0f ) << endl;
		cerr << "Effect size: " << usedEffectCount << " Kb:" << ( static_cast<float>( sizeof( EffectLocation )*usedEffectCount ) / 1024.0f ) <<
		" Cache: " << unusedEffect.size() << " Kb:" << ( static_cast<float>( sizeof( EffectLocation )*unusedEffect.size() ) / 1024.0f ) << endl;
		accessCount = 0;
	}
}
