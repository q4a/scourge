/***************************************************************************
                          item.cpp  -  description
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

#include "item.h"

Item *Item::items[] =  {
  new Item("Short sword", 2, 150, 100, 4,
           "A dwarven shortsword of average workmanship",
           "A stubby short sword",
           ShapePalette::SWORD_INDEX),
  new Item("Dagger", 1, 80, 100, 3,
           "There's nothing special about this dagger",
           "A small dagger",
           ShapePalette::SWORD_INDEX),
  new Item("Bastard sword", 4, 200, 100, 8,
           "A bastard sword can be wielded either by one or both hands... (If you still have both hands)",
           "A rusty bastard sword",
           ShapePalette::SWORD_INDEX)
};
int Item::itemCount = 3;

Item::Item(char *name, int weight, int price, int quality, int action, char *desc, char *shortDesc, int shape_index) {
  this->name = name;
  this->weight = weight;
  this->price = price;
  this->quality = quality;
  this->action = action;
  this->desc = desc;
  this->shortDesc = shortDesc;
  this->shape_index = shape_index;
}

Item::~Item(){
}

Item *Item::getRandomItem(int level) {
  int n = (int) ((float)itemCount * rand()/RAND_MAX);
  return items[n];
}
