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
  */

class Item {
private:
  RpgItem *rpgItem;
  Sint16 x, y, z;
  static Item *items[];
  
public:
  Item(RpgItem *rpgItem);
  ~Item();
  
  static void initItems();
  static Item *getRandomItem(int level);
  static Item *getRandomContainer();
  static Item *getRandomContainerNS();
  inline static Item *getItem(int index) { return items[index]; }
  inline GLShape *getShape() { return ShapePalette::getInstance()->getItemShape(this->rpgItem->getShapeIndex()); }
  inline void moveTo(Sint16 x, Sint16 y, Sint16 z) { this->x = x; this->y = y; this->z = z; }
  inline Sint16 getX() { return x; }
  inline Sint16 getY() { return y; }
  inline Sint16 getZ() { return z; }
  inline char *getShortDescription() { return rpgItem->getShortDesc(); }
  inline RpgItem *getRpgItem() { return rpgItem; }
};

#endif
