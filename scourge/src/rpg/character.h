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

class Character;  
  
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
  // fighters
  static Character *ranger;
  static Character *knight;

  // rogues
  static Character *tinkerer;
  static Character *assassin;

  // rogue/fighter/healer
  static Character *arcanist;
  static Character *loremaster;

  // wizards
  static Character *conjurer;
  static Character *summoner;

  // healers/religion
  static Character *naturalist;
  static Character *monk;
};

#endif
