/***************************************************************************
                          monster.h  -  description
                             -------------------
    begin                : Mon Jul 7 2003
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

#ifndef MONSTER_H
#define MONSTER_H

#include "../constants.h"
#include "rpgitem.h"

/**
  *@author Gabor Torok
  */

#define MAX_MONSTER_LEVEL 10
#define MAX_MONSTER_COUNT 50
  
class Monster  {
 public:
  static const int ITEM_COUNT = 50;

private:
  char *type;
  int hp;
  int level;
  Uint8 shapeIndex;
  char description[300];
  RpgItem *item[ITEM_COUNT]; // starting equipment
  int money;
  int speed;
	int baseArmor;

  static Monster *monsters[MAX_MONSTER_LEVEL][MAX_MONSTER_COUNT];
  static int monsterCount[MAX_MONSTER_LEVEL];

public:
  Monster(char *type, int level, int hp, Uint8 shapeIndex=Constants::BUGGERLING_INDEX, int baseArmor=0);
  ~Monster();

	inline int getBaseArmor() { return baseArmor; }
  inline char *getType() { return type; };
  inline int getHp() { return hp; }  
  inline int getLevel() { return level; }  
  inline Uint8 getShapeIndex() { return shapeIndex; }
  inline char *getDescription() { return description; }
  inline int getSpeed() { return speed; }
  inline RpgItem *getStartingItem(int index) { return item[index]; }

  static void initMonsters();
  static Monster *getRandomMonster(int level);

 protected:
  inline void setSpeed(int speed) { this->speed = speed; }
};

#endif
