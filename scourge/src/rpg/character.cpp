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
  new Character( "Ranger", 10, "ROGUE", "models/m1.bmp" ),
  new Character( "Knight", 12, "FIGHTER", "models/m2.bmp" ),
  new Character( "Tinkerer", 7, "ROGUE", "models/m1.bmp" ),
  new Character( "Assassin", 6, "ROGUE", "models/m1.bmp" ),
  new Character( "Arcanist", 6, "ROGUE", "models/m1.bmp" ),
  new Character( "Loremaster", 6, "CLERIC", "models/m3.bmp" ),
  new Character( "Conjurer", 4, "WIZARD", "models/m4.bmp" ),
  new Character( "Summoner", 4, "WIZARD", "models/m4.bmp" ),
  new Character( "Naturalist", 5, "CLERIC", "models/m3.bmp" ),
  new Character( "Monk", 6, "CLERIC", "models/m3.bmp" )
};

Character::Character(char *name, int startingHp, char *model, char *skin) {  
  this->name = name;
  this->startingHp = startingHp;
  this->model_name = model;
  this->skin_name = skin;
  sprintf(description, "FIXME: need a description");
}
Character::~Character(){  
}
