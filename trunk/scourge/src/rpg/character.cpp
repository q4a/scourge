/***************************************************************************
                          character.cpp  -  description
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

#include "character.h"

Character *Character::character_class[] = {
  new Character( "Ranger", 10 ),
  new Character( "Knight", 12 ),
  new Character( "Tinkerer", 7 ),
  new Character( "Assassin", 6 ),
  new Character( "Arcanist", 6 ),
  new Character( "Loremaster", 6 ),
  new Character( "Conjurer", 4 ),
  new Character( "Summoner", 4 ),
  new Character( "Naturalist", 5 ),
  new Character( "Monk", 6 )
};

Character::Character(char *name, int startingHp) {  
  this->name = name;
  this->startingHp = startingHp;
}
Character::~Character(){  
}
