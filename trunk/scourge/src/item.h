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

#include "glshape.h"
#include "shapepalette.h"

class Scourge;

/**
  *@author Gabor Torok
  */

class Item {
private:
  char *name, *desc, *shortDesc;
  int weight, price, quality;
  int action; // damage, defence, potion str.
  int shape_index;
  Sint16 x, y, z;
  
public:
  enum itemNames {
    SHORT_SWORD=0,
    DAGGER,
    BASTARD_SWORD,
	CHEST,
	BOOKSHELF,
	CHEST2,
	BOOKSHELF2,

	// must be the last ones
	ITEM_COUNT
  };

  static int random_item[];
  static const int RANDOM_ITEM_COUNT=3;

  static Item *items[];
  static int itemCount;

	Item(char *name, int weight, int price, int quality,
       int action, char *desc, char *shortDesc, int shape_index);
	~Item();

  static Item *getRandomItem(int level);
  static Item *getRandomContainer();
  static Item *getRandomContainerNS();
  inline static Item *getItem(int index) { return items[index]; }
  inline GLShape *getShape() { return ShapePalette::getInstance()->getItemShape(this->shape_index); }
  inline void moveTo(Sint16 x, Sint16 y, Sint16 z) { this->x = x; this->y = y; this->z = z; }
  inline Sint16 getX() { return x; }
  inline Sint16 getY() { return y; }
  inline Sint16 getZ() { return z; }
  inline char *getShortDescription() { return shortDesc; }
};

#endif
