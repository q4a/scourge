/***************************************************************************
                          debug.h  -  description
                             -------------------
    begin                : Sat Oct 8 2005
    copyright            : (C) 2005 by Gabor Torok
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

#ifndef DEBUG_VALUES_H
#define DEBUG_VALUES_H

/**
 * This is where debug flags go.
 * 
 * Include this file as needed but do not add to makefile.
 * Do not include this file in a .h file.
 * This way it will only recompile the affected .cpp files.
 */
 
// battle.cpp
#define DEBUG_BATTLE false

// calendar.cpp
#define CALENDAR_DEBUG 0

// creature.cpp             
#define GOD_MODE 0
#define MONSTER_IMORTALITY 0
                       
// partyeditor.cpp (if non-1, defaults are added)
#define STARTING_PARTY_LEVEL 1

// scourge.cpp                     
//#define CAVE_TEST 1
#define CAVE_TEST_LEVEL 4
#define BATTLES_ENABLED 1
//#define DEBUG_KEYS 1
//#define BASE_DEBUG 1

// sqbinding.cpp                        
#define DEBUG_SQUIRREL 0

// show secret doors
//#define DEBUG_SECRET_DOORS 1

// show every PC's path
#define PATH_DEBUG 0

#endif

