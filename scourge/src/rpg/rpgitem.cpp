/***************************************************************************
                          rpgitem.cpp  -  description
                             -------------------
    begin                : Sun Sep 28 2003
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
#include "rpgitem.h"

/**
   These basic objects are enhanced by adding magical capabilities.
 */
RpgItem *RpgItem::items[] =  {

  // SWORDS:
  new RpgItem(SHORT_SWORD, "Short sword", 1, SWORD, 2, 150, 100, 4, 8, 
           "A dwarven shortsword of average workmanship",
           "A stubby short sword",
           Constants::SWORD_INDEX),
  new RpgItem(DAGGER, "Dagger", 1, SWORD, 1, 80, 100, 3, 5, 
           "There's nothing special about this dagger",
           "A small dagger",
           Constants::SWORD_INDEX),
  new RpgItem(BASTARD_SWORD, "Bastard sword", 1, SWORD, 4, 200, 100, 8, 11,
           "A bastard sword can be wielded either by one or both hands... (If you still have both hands)",
           "A rusty bastard sword",
           Constants::SWORD_INDEX, OPTIONAL_TWO_HANDED),
  new RpgItem(LONG_SWORD, "Long sword", 2, SWORD, 6, 250, 100, 10, 12,
           "The longsword is a knight's standard weapon",
           "A shiny, sharp longsword",
           Constants::SWORD_INDEX),
  new RpgItem(GREAT_SWORD, "Great sword", 2, SWORD, 10, 350, 100, 16, 14,
           "The two handed great sword can deliver a lot of damage",
           "A two-handed greatsword",
           Constants::SWORD_INDEX, ONLY_TWO_HANDED),


  // AXES
  new RpgItem(BATTLE_AXE, "Battleaxe", 1, AXE, 4, 150, 100, 6, 10,
           "A battle axe of average workmanship",
           "A well made battle axe",
           Constants::SWORD_INDEX),
  new RpgItem(THROWING_AXE, "Throwing axe", 1, AXE, 2, 100, 100, 4, 6,
           "A dull-looking axe for chucking",
           "A quick throwing axe",
           Constants::SWORD_INDEX),


  // CONTAINERS:
  new RpgItem(CHEST, "Chest", 1, CONTAINER, 100, 0, 100, 0, 0,
           "A wooden chest with metal re-inforced edges",
           "An ancient chest",
           Constants::CHEST_INDEX),
  new RpgItem(BOOKSHELF, "Bookshelf", 1, CONTAINER, 200, 0, 100, 0, 0,
           "A bookshelf containing tomes of old",
           "A large bookself",
           Constants::BOOKSHELF_INDEX),
  new RpgItem(CHEST2, "Chest", 1, CONTAINER, 100, 0, 100, 0, 0,
           "A wooden chest with metal re-inforced edges",
           "An ancient chest",
           Constants::CHEST2_INDEX),
  new RpgItem(BOOKSHELF2, "Bookshelf", 1, CONTAINER, 200, 0, 100, 0, 0,
           "A bookshelf containing tomes of old",
           "A large bookself",
           Constants::BOOKSHELF2_INDEX)
};

RpgItem::RpgItem(int index, char *name, int level, int type, int weight, int price, int quality, 
				 int action, int speed, char *desc, char *shortDesc, int shape_index,
				 int twohanded) {
  this->index = index;
  this->name = name;
  this->level = level;
  this->type = type;
  this->weight = weight;
  this->price = price;
  this->quality = quality;
  this->action = action;
  this->speed = speed;
  this->desc = desc;
  this->shortDesc = shortDesc;
  this->shape_index = shape_index;
  this->twohanded = twohanded;
}

RpgItem::~RpgItem() {
}
