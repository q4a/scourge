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
  this->shapeIndex = this->rpgItem->getShapeIndex();
  this->color = NULL;
  this->shape = ShapePalette::getInstance()->getShape(shapeIndex);
  // for now objects larger than 1 height will block (we can change this later)
  // this is so the player is not blocked by swords and axes on the ground
  this->blocking = (shape->getHeight() > 1);
  this->containedItemCount = 0;
}

Item::~Item(){
}

bool Item::addContainedItem(Item *item) { 
  if(containedItemCount < MAX_CONTAINED_ITEMS) {
	containedItems[containedItemCount++] = item; 
	return true;
  } else return false;
}
 
Item *Item::removeContainedItem(int index) {
  Item *item = NULL;
  if(index >= 0 && index < containedItemCount) {
	item = containedItems[index];
	containedItemCount--;
	for(int i = index; i < containedItemCount; i++) {
	  containedItems[i] = containedItems[i + 1];
	}
  }
  return item;
}

Item *Item::getContainedItem(int index) {
  return ((index >= 0 && index < containedItemCount) ? containedItems[index] : NULL);
}

bool Item::isContainedItem(Item *item) {
	for(int i = 0; i < containedItemCount; i++) {
		if(containedItems[i] == item || 
			 (containedItems[i]->getRpgItem()->getType() == RpgItem::CONTAINER &&
				containedItems[i]->isContainedItem(item))) return true;
	}
	return false;
}

void Item::getDetailedDescription(char *s, bool precise){
    int type;
    RpgItem * rpgItem;
    
    rpgItem  = getRpgItem();
    type = rpgItem->getType();
    if(type == RpgItem::DRINK || type == RpgItem::POTION || type == RpgItem::FOOD){
        sprintf(s, "(Q:%d,W:%2.2f, N:%d/%d) %s", 
            rpgItem->getQuality(), 
			rpgItem->getWeight(),
			rpgItem->getCurrentCharges(),
			rpgItem->getMaxCharges(),
			(precise ? rpgItem->getName() : rpgItem->getShortDesc()));
    }
    else{   
	   sprintf(s, "(A:%d,S:%d,Q:%d,W:%2.2f) %s", 
			rpgItem->getAction(), 
			rpgItem->getSpeed(), 
			rpgItem->getQuality(), 
			rpgItem->getWeight(),
			(precise ? rpgItem->getName() : rpgItem->getShortDesc()));
    }
}

