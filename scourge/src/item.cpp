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

Item::Item(RpgItem *rpgItem) {
  this->rpgItem = rpgItem;
}

Item::~Item(){
}

Item *Item::items[RpgItem::ITEM_COUNT];

// create one item for each rpgitem
void Item::initItems() {
  for(int i = 0; i < RpgItem::ITEM_COUNT; i++) {
	Item::items[i] = new Item(RpgItem::items[i]);
  }
}

Item *Item::getRandomItem(int level) {
  int n = (int) (3.0 * rand()/RAND_MAX);
  switch(n) {
  case 0: return items[RpgItem::SHORT_SWORD];
  case 1: return items[RpgItem::DAGGER];
  case 2: return items[RpgItem::BASTARD_SWORD];
  }
}

Item *Item::getRandomContainer() {
  int n = (int)(10.0 * rand()/RAND_MAX);
  switch(n) {
  case 0: return items[RpgItem::BOOKSHELF];
  case 1: return items[RpgItem::CHEST];
  default: return NULL;
  }
}

Item *Item::getRandomContainerNS() {
  int n = (int)(10.0 * rand()/RAND_MAX);
  switch(n) {
  case 0: return items[RpgItem::BOOKSHELF2];
  case 1: return items[RpgItem::CHEST2];
  default: return NULL;
  }
}
