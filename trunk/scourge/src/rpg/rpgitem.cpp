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

RpgItem *RpgItem::items[] =  {
  new RpgItem("Short sword", 2, 150, 100, 4,
           "A dwarven shortsword of average workmanship",
           "A stubby short sword",
           Constants::SWORD_INDEX),
  new RpgItem("Dagger", 1, 80, 100, 3,
           "There's nothing special about this dagger",
           "A small dagger",
           Constants::SWORD_INDEX),
  new RpgItem("Bastard sword", 4, 200, 100, 8,
           "A bastard sword can be wielded either by one or both hands... (If you still have both hands)",
           "A rusty bastard sword",
           Constants::SWORD_INDEX),
  new RpgItem("Chest", 100, 0, 100, 0,
           "A wooden chest with metal re-inforced edges",
           "An ancient chest",
           Constants::CHEST_INDEX),
  new RpgItem("Bastard sword", 200, 0, 100, 0,
           "A bookshelf containing tomes of old",
           "A large bookself",
           Constants::BOOKSHELF_INDEX),
  new RpgItem("Chest", 100, 0, 100, 0,
           "A wooden chest with metal re-inforced edges",
           "An ancient chest",
           Constants::CHEST2_INDEX),
  new RpgItem("Bastard sword", 200, 0, 100, 0,
           "A bookshelf containing tomes of old",
           "A large bookself",
           Constants::BOOKSHELF2_INDEX)
};

RpgItem::RpgItem(char *name, int weight, int price, int quality, 
				 int action, char *desc, char *shortDesc, int shape_index) {
  this->name = name;
  this->weight = weight;
  this->price = price;
  this->quality = quality;
  this->action = action;
  this->desc = desc;
  this->shortDesc = shortDesc;
  this->shape_index = shape_index;

}

RpgItem::~RpgItem() {
}
