/***************************************************************************
                  mapmemory.h  -  Manages and renders the level map
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
#ifndef MAPMEMORY_H_
#define MAPMEMORY_H_
#pragma once

#include <vector>

class Map;
class Location;
class Preferences;
class Shapes;
class EffectLocation;

/// Utilities for cleaning up on level maps, and managing memory use.

class MapMemoryManager {
private:
	std::vector<Location*> unused;
	std::vector<EffectLocation*> unusedEffect;
	int maxSize;
	int accessCount;
	int usedCount, usedEffectCount;
public:
	/**
	    Maxsize of 0 means unlimited size cache.
	*/
	MapMemoryManager( int maxSize = 0 );
	~MapMemoryManager();
	Location *newLocation();
	void deleteLocation( Location *pos );
	EffectLocation *newEffectLocation( Map *map, Preferences *preferences, Shapes *shapes, int width, int height );
	void deleteEffectLocation( EffectLocation *pos );
private:
	void printStatus();
};


#endif /*MAPMEMORY_H_*/
