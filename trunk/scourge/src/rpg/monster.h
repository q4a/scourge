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

#include <vector>
#include <map>
#include "../constants.h"
#include "rpgitem.h"

/**
  *@author Gabor Torok
  */

using namespace std;
  
class Monster  {

 private:
  char *type;
  int hp;
  int level;
  char *model_name;
  char *skin_name;
  char description[300];
  int money;
  int speed;
  int baseArmor;
  vector<RpgItem*> items;

  static map<int, vector<Monster*>* > monsters;
  static map<string, Monster*> monstersByName;

public:
  Monster(char *type, int level, int hp, char *model, char *skin, int baseArmor=0);
  ~Monster();

  inline int getBaseArmor() { return baseArmor; }
  inline char *getType() { return type; };
  inline int getHp() { return hp; }  
  inline int getLevel() { return level; }  
  inline char *getModelName() { return model_name; }
  inline char *getSkinName() { return skin_name; }
  inline char *getDescription() { return description; }
  inline int getSpeed() { return speed; }
  inline int getStartingItemCount() { return items.size(); }
  inline RpgItem *getStartingItem(int index) { return items[index]; }
  inline void addItem(RpgItem *item) { items.push_back(item); }

  static void initMonsters();
  static Monster *getRandomMonster(int level);
  static Monster *getMonsterByName(char *name);

 protected:
  inline void setSpeed(int speed) { this->speed = speed; }
};

#endif
