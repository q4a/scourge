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
#include "rpg/spell.h"
#include <vector>

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
  int currentCharges;
  float weight;
  Spell *spell;
  char itemName[255];
  MagicAttrib *magic;

public:
  Item(RpgItem *rpgItem);
  ~Item();

  static map<int, vector<string> *> soundMap;
  
  inline Color *getColor() { return color; }
  inline void setColor(Color *c) { color = c; }
  inline void setShape(GLShape *s) { shape = s; }
  inline GLShape *getShape() { return shape; }
  inline RpgItem *getRpgItem() { return rpgItem; }
  inline bool isBlocking() { return blocking; }
  inline void setBlocking(bool b) { blocking = b; }
  inline int getCurrentCharges() { return currentCharges; }
  inline void setCurrentCharges(int n) { if(n < 0)n=0; if(n>rpgItem->getMaxCharges())n=rpgItem->getMaxCharges(); currentCharges = n; } 
  inline float getWeight() { return weight; }
  inline void setWeight(float f) { if(f < 0.0f)f=0.1f; weight=f; }
  inline void setSpell(Spell *spell) { this->spell = spell; sprintf(this->itemName, "Scroll of %s", spell->getName()); }
  inline Spell *getSpell() { return spell; }


  void getDetailedDescription(char *s, bool precise=true);
  inline char *getItemName() { return itemName; }

  inline int getContainedItemCount() { return containedItemCount; }
  // return true if successful
  bool addContainedItem(Item *item);
  // return removed item, or NULL
  Item *removeContainedItem(int index);
  Item *getContainedItem(int index);
  bool isContainedItem(Item *item);

  // return true if the item is used up
  // this method also adjusts weight
  bool decrementCharges();

  const char *getRandomSound();
  
  static void initItems(ShapePalette *shapePal);

  void enchant(int level);
};

#endif
