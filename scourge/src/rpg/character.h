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

public:
  Character(char *name, int startingHp);
  ~Character();

  inline char *getName() { return name; };
  inline int getStartingHp() { return startingHp; }  
  
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
