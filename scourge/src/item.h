/***************************************************************************
                          item.h  -  description
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

#ifndef ITEM_H
#define ITEM_H

#include "constants.h"
#include "glshape.h"
#include "shapepalette.h"
#include "rpg/rpgitem.h"

class Scourge;

/**
  *@author Gabor Torok

  This class is both the UI representation (shape) of the rpgItem and it's state (for example, wear).
  All instances of the RpgItem point to the same RpgItem, but a new Item is created for each.

  */

class Item {
 public:
  static const int MAX_CONTAINED_ITEMS = 100;

 private:
  RpgItem *rpgItem;
  int shapeIndex;
  Color *color;
  GLShape *shape;
  bool blocking;
  Item *containedItems[MAX_CONTAINED_ITEMS];
  int containedItemCount;
  
public:
  Item(RpgItem *rpgItem);
  ~Item();
  
  inline Color *getColor() { return color; }
  inline void setColor(Color *c) { color = c; }
  inline void setShape(GLShape *s) { shape = s; }
  inline GLShape *getShape() { return shape; }
  inline RpgItem *getRpgItem() { return rpgItem; }
  inline bool isBlocking() { return blocking; }

  inline void getDetailedDescription(char *s, bool precise=true) {
	sprintf(s, "(A:%d,S:%d,Q:%d,W:%d) %s", 
			getRpgItem()->getAction(), 
			getRpgItem()->getSpeed(), 
			getRpgItem()->getQuality(), 
			getRpgItem()->getWeight(),
			(precise ? getRpgItem()->getName() : getRpgItem()->getShortDesc()));
  }

  inline int getContainedItemCount() { return containedItemCount; }
  // return true if successful
  bool addContainedItem(Item *item);
  // return removed item, or NULL
  Item *removeContainedItem(int index);
  Item *getContainedItem(int index);
};

#endif
