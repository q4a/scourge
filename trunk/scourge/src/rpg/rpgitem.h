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
#ifndef RPG_ITEM_H
#define RPG_ITEM_H

#include "../constants.h"
#include "character.h"

class RpgItem {
 private:
  int index;
  char *name, *desc, *shortDesc;
  int level;
  int type;
  int weight, price, quality;
  int action; // damage, defence, potion str.
  int speed; // 0-100, 100-slowest, 0-fastest
  int shape_index;
  int twohanded;
  int distance; // how far can it reach?
  int equip; // where can it be worn?
  int skill; // which skill to check when using the item

 public:
  enum itemNames {
    SHORT_SWORD=0,
    DAGGER,
    BASTARD_SWORD,
	LONG_SWORD,
	GREAT_SWORD,
	
	BATTLE_AXE,
	THROWING_AXE,

	HORNED_HELMET,

	CHEST,
	BOOKSHELF,
	CHEST2,
	BOOKSHELF2,

	CORPSE,
	
	// must be the last ones
	ITEM_COUNT
  };

  enum itemTypes {
	SWORD=0,
	AXE,
	BOW,
	CONTAINER,
	ARMOR,
	
	// must be last
	ITEM_TYPE_COUNT
  };

  enum twoHandedType {
	NOT_TWO_HANDED=0,
	ONLY_TWO_HANDED,
	OPTIONAL_TWO_HANDED
  };

  static RpgItem *items[];
  
  RpgItem(int index, char *name, int level, int type, int weight, int price, int quality, 
		  int action, int speed, char *desc, char *shortDesc, int equip, int shape_index, 
		  int twohanded=NOT_TWO_HANDED, int distance=1, int skill=-1);
  ~RpgItem();

  inline int getIndex() { return index; }
  inline char *getName() { return name; }
  inline int getAction() { return action; }
  inline int getSpeed() { return speed; }
  inline int getQuality() { return quality; }
  inline int getWeight() { return weight; }
  inline int getShapeIndex() { return shape_index; }
  inline char *getShortDesc() { return shortDesc; }  
  inline int getEquip() { return equip; }
  inline int getDistance() { return distance; }
  inline int getSkill() { return skill; } 
  inline int getType() { return type; }

  static RpgItem *getRandomItem(int level);
  static RpgItem *getRandomContainer();
  static RpgItem *getRandomContainerNS();
  inline static RpgItem *getItem(int index) { return items[index]; }
};

#endif
