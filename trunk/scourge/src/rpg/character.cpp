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

char Character::inventory_location[][80] = {
  "head",
  "neck",
  "back",
  "chest",
  "left hand",
  "right hand",
  "belt",
  "legs",
  "feet",
  "ring1",
  "ring2",
  "ring3",
  "ring4",
  "ranged weapon"
};

Character *Character::character_class[] = {
  new Character( "Ranger", 10, Constants::ROGUE_INDEX ),
  new Character( "Knight", 12, Constants::FIGHTER_INDEX ),
  new Character( "Tinkerer", 7, Constants::ROGUE_INDEX ),
  new Character( "Assassin", 6, Constants::ROGUE_INDEX ),
  new Character( "Arcanist", 6, Constants::ROGUE_INDEX ),
  new Character( "Loremaster", 6, Constants::CLERIC_INDEX ),
  new Character( "Conjurer", 4, Constants::WIZARD_INDEX ),
  new Character( "Summoner", 4, Constants::WIZARD_INDEX ),
  new Character( "Naturalist", 5, Constants::CLERIC_INDEX ),
  new Character( "Monk", 6, Constants::CLERIC_INDEX )
};

Character::Character(char *name, int startingHp, Uint8 shapeIndex) {  
  this->name = name;
  this->startingHp = startingHp;
  this->shapeIndex = shapeIndex;
}
Character::~Character(){  
}
