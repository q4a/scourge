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

  Character* Character::ranger = new Character("Ranger", 10);
  Character* Character::knight = new Character("Knight", 12);

  // rogues
  Character* Character::tinkerer = new Character("Tinkerer", 7);
  Character* Character::assassin = new Character("Assassin", 6);

  // rogue/fighter/healer
  Character* Character::arcanist = new Character("Arcanist", 6);
  Character* Character::loremaster = new Character("Loremaster", 6);

  // wizards
  Character* Character::conjurer = new Character("Conjurer", 4);
  Character* Character::summoner = new Character("Summoner", 4);

  // healers/religion
  Character* Character::naturalist = new Character("Naturalist", 5);
  Character* Character::monk = new Character("Monk", 6);


Character::Character(char *name, int startingHp) {  
  this->name = name;
  this->startingHp = startingHp;
}
Character::~Character(){
  
}

