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
private:
  char *type;
  int hp;
  int level;
  Uint8 shapeIndex;
  char description[300];
  RpgItem *weapon[10], *armor[10], *item[10]; // starting equipment
  int money;
  int speed;

  static Monster *monsters[MAX_MONSTER_LEVEL][MAX_MONSTER_COUNT];
  static int monsterCount[MAX_MONSTER_LEVEL];

public:
  Monster(char *type, int level, int hp, Uint8 shapeIndex=Constants::BUGGERLING_INDEX);
  ~Monster();

  inline char *getType() { return type; };
  inline int getHp() { return hp; }  
  inline int getLevel() { return level; }  
  inline Uint8 getShapeIndex() { return shapeIndex; }
  inline char *getDescription() { return description; }
  inline int getSpeed() { return speed; }

  static void initMonsters();
  static Monster *getRandomMonster(int level);

 protected:
  inline int setSpeed(int speed) { this->speed = speed; }
};

#endif
