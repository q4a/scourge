/***************************************************************************
                          character.h  -  description
                             -------------------
    begin                : Mon Jul 7 2003
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

#ifndef CHARACTER_H
#define CHARACTER_H

#include "../constants.h"

/**
  *@author Gabor Torok
  */
  
class Character  {
private:
  char *name;
  int startingHp;
  Uint8 shapeIndex;

public:
  // inventory locations
  static const int INVENTORY_HEAD = 1;
  static const int INVENTORY_NECK = 2;
  static const int INVENTORY_BACK = 4;
  static const int INVENTORY_CHEST = 8;
  static const int INVENTORY_LEFT_HAND = 16;
  static const int INVENTORY_RIGHT_HAND = 32;
  static const int INVENTORY_BELT = 64;
  static const int INVENTORY_LEGS = 128;
  static const int INVENTORY_FEET = 256;
  static const int INVENTORY_RING1 = 512;
  static const int INVENTORY_RING2 = 1024;
  static const int INVENTORY_RING3 = 2048;
  static const int INVENTORY_RING4 = 4096;
  static const int INVENTORY_WEAPON_RANGED = 8192;
  static const int INVENTORY_COUNT = 14;
  static char inventory_location[][80];

  Character(char *name, int startingHp, Uint8 shapeIndex);
  ~Character();

  inline char *getName() { return name; };
  inline int getStartingHp() { return startingHp; }  
  inline Uint8 getShapeIndex() { return shapeIndex; }

  
public:  

  enum {
	ranger = 0,
	knight,
	tinkerer,
	assassin,
	arcanist,
	loremaster,
	constants,
	summoner,
	naturalist,
	monk,
	
	// should be last one
	character_count
  };

  static Character *character_class[character_count];

};

#endif
